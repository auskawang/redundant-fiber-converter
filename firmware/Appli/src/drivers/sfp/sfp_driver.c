/**
 * @file sfp_driver.c
 * @brief SFP transceiver driver — GPIO control and SFF-8472 DDM telemetry acquisition.
 *
 * Implements hardware abstraction for the two Coherent FTLF8519P3BNL SFP cages.
 * GPIO-based signals (MOD_ABS, LOS, TX_FAULT, TX_DISABLE) are accessed directly
 * via HAL.  Diagnostic telemetry is fetched over I2C from the SFF-8472 A2h memory
 * map and converted into engineering units.
 *
 * @note All public functions that perform I2C transactions block the calling context
 *       and must never be invoked from an interrupt service routine.
 */

#include "sfp_driver.h"
#include "main.h"
#include <math.h>
#include <string.h>

/** @brief Number of raw DDM diagnostic bytes read from SFP A2h, starting at offset 96. */
#define SFP_DDM_RAW_LEN 10
#define SFP_I2C_TIMEOUT_MS 100U

extern I2C_HandleTypeDef hi2c2;
extern I2C_HandleTypeDef hi2c4;

static I2C_HandleTypeDef *sfp_i2c_bus(sfp_port_t port)
{
    switch (port) {
    case SFP_PORT_PRIMARY: return &hi2c2;
    case SFP_PORT_BACKUP:  return &hi2c4;
    default:               return NULL;
    }
}

static bool sfp_i2c_transfer_valid(uint16_t address, uint8_t offset,
                                  const void *buffer, uint16_t length)
{
    return buffer != NULL && length != 0 &&
           (uint32_t)offset + length <= 256U &&
           (address == SFP_I2C_ADDR_A0 || address == SFP_I2C_ADDR_A2);
}

static int sfp_i2c_read(sfp_port_t port, uint16_t address, uint8_t offset,
                        uint8_t *buffer, uint16_t length)
{
    I2C_HandleTypeDef *bus = sfp_i2c_bus(port);
    if (bus == NULL || !sfp_i2c_transfer_valid(address, offset, buffer, length))
        return -1;

    return HAL_I2C_Mem_Read(bus, address, offset, I2C_MEMADD_SIZE_8BIT,
                            buffer, length, SFP_I2C_TIMEOUT_MS) == HAL_OK ? 0 : -2;
}

static int sfp_i2c_write(sfp_port_t port, uint16_t address, uint8_t offset,
                         const uint8_t *data, uint16_t length)
{
    I2C_HandleTypeDef *bus = sfp_i2c_bus(port);
    if (bus == NULL || !sfp_i2c_transfer_valid(address, offset, data, length))
        return -1;

    return HAL_I2C_Mem_Write(bus, address, offset, I2C_MEMADD_SIZE_8BIT,
                             (uint8_t *)data, length, SFP_I2C_TIMEOUT_MS) == HAL_OK ? 0 : -2;
}

/**
 * @brief GPIO pin mapping for one SFP cage.
 *
 * Groups every HAL GPIO port/pin pair used by a single cage.
 */
typedef struct {
    GPIO_TypeDef *present_port, *los_port, *fault_port, *disable_port;
    GPIO_TypeDef *mod_led_port, *fault_led_port;
    uint16_t present_pin, los_pin, fault_pin, disable_pin, mod_led_pin, fault_led_pin;
} sfp_pins_t;

static const sfp_pins_t pins[SFP_PORT_COUNT] = {
    {SFP0_MOD_DEF0_GPIO_Port, SFP0_LOS_GPIO_Port, SFP0_TX_FAULT_GPIO_Port,
     SFP0_TX_DISABLE_GPIO_Port, SFP0_MOD_DEF0_LED_GPIO_Port, SFP0_TX_FAULT_LED_GPIO_Port,
     SFP0_MOD_DEF0_Pin, SFP0_LOS_Pin, SFP0_TX_FAULT_Pin,
     SFP0_TX_DISABLE_Pin, SFP0_MOD_DEF0_LED_Pin, SFP0_TX_FAULT_LED_Pin},
    {SFP1_MOD_DEF0_GPIO_Port, SFP1_LOS_GPIO_Port, SFP1_TX_FAULT_GPIO_Port,
     SFP1_TX_DISABLE_GPIO_Port, SFP1_MOD_DEF0_LED_GPIO_Port, SFP1_TX_FAULT_LED_GPIO_Port,
     SFP1_MOD_DEF0_Pin, SFP1_LOS_Pin, SFP1_TX_FAULT_Pin,
     SFP1_TX_DISABLE_Pin, SFP1_MOD_DEF0_LED_Pin, SFP1_TX_FAULT_LED_Pin}
};

/**
 * @brief Convert a raw SFF-8472 optical power count to dBm.
 *
 * SFF-8472 specifies one LSB = 0.1 µW = 0.0001 mW.  The dBm value is
 * derived from the standard formula: dBm = 10 × log₁₀(mW).
 *
 * @param[in] raw 16-bit unsigned power count from the DDM register.
 * @return Optical power in dBm, or @c -INFINITY when @p raw is zero
 *         (no detectable optical signal).
 */
static float power_dbm(uint16_t raw)
{
    return raw == 0 ? -INFINITY : 10.0f * log10f((float)raw / 10000.0f);
}

static uint16_t be16(const uint8_t *p)
{
    return (uint16_t)(((uint16_t)p[0] << 8) | p[1]);
}

/**
 * @brief Set all floating-point telemetry fields in a status struct to NaN.
 * @param[out] status Struct whose telemetry fields are to be invalidated.
 */
static void clear_telemetry(sfp_status_t *status)
{
    status->rx_power_dbm = NAN;
    status->tx_power_dbm = NAN;
    status->tx_bias_current_ma = NAN;
    status->temperature_c = NAN;
    status->supply_voltage_v = NAN;
    status->valid = false;
}

/**
 * @brief Convert a raw SFF-8472 DDM byte buffer into engineering-unit fields.
 *
 * Parses the ::SFP_DDM_RAW_LEN-byte block read from A2h registers 96–105 and
 * writes each quantity into @p status using the SFF-8472 scaling factors:
 *
 * | Bytes | Quantity          | Encoding            | Scale            |
 * |-------|-------------------|---------------------|------------------|
 * | [0:1] | Temperature       | Signed Q8.8 (°C)    | ÷ 256            |
 * | [2:3] | Supply voltage    | Unsigned (100 µV)   | ÷ 10000 → V      |
 * | [4:5] | TX bias current   | Unsigned (2 µA)     | ÷ 500 → mA       |
 * | [6:7] | TX output power   | Unsigned (0.1 µW)   | power_dbm()      |
 * | [8:9] | RX input power    | Unsigned (0.1 µW)   | power_dbm()      |
 *
 * @param[in]  raw    Pointer to a ::SFP_DDM_RAW_LEN-byte I2C read buffer.
 * @param[out] status Destination struct; telemetry fields are overwritten.
 *
 * @note Assumes internally-calibrated SFP modules.
 */
static void convert_telemetry(const uint8_t *raw, sfp_status_t *status)
{
    status->temperature_c      = (float)(int16_t)be16(raw) / 256.0f;
    status->supply_voltage_v   = (float)be16(raw + 2) / 10000.0f;
    status->tx_bias_current_ma = (float)be16(raw + 4) / 500.0f;
    status->tx_power_dbm       = power_dbm(be16(raw + 6));
    status->rx_power_dbm       = power_dbm(be16(raw + 8));
    status->valid = true;
}

/**
 * @brief Drive the green module-present LED for one SFP cage.
 *
 * @param[in] port SFP cage index (::SFP_PORT_PRIMARY or ::SFP_PORT_BACKUP).
 * @param[in] on   @c true to turn, @c false to turn off.
 * @return ::SFP_OK, or ::SFP_ERR_ARGUMENT if @p port is out of range.
 */
static int sfp_driver_set_mod_led(sfp_port_t port, bool on)
{
    if ((unsigned)port >= SFP_PORT_COUNT) return SFP_ERR_ARGUMENT;
    HAL_GPIO_WritePin(pins[port].mod_led_port, pins[port].mod_led_pin,
                      on ? GPIO_PIN_SET : GPIO_PIN_RESET);
    return SFP_OK;
}

/**
 * @brief Drive the red TX-fault LED for one SFP cage.
 *
 * @param[in] port SFP cage index (::SFP_PORT_PRIMARY or ::SFP_PORT_BACKUP).
 * @param[in] on   @c true to turn, @c false to turn off.
 * @return ::SFP_OK, or ::SFP_ERR_ARGUMENT if @p port is out of range.
 */
static int sfp_driver_set_fault_led(sfp_port_t port, bool on)
{
    if ((unsigned)port >= SFP_PORT_COUNT) return SFP_ERR_ARGUMENT;
    HAL_GPIO_WritePin(pins[port].fault_led_port, pins[port].fault_led_pin,
                      on ? GPIO_PIN_SET : GPIO_PIN_RESET);
    return SFP_OK;
}

int sfp_driver_set_tx_disable(sfp_port_t port, bool disable)
{
    if ((unsigned)port >= SFP_PORT_COUNT) return SFP_ERR_ARGUMENT;
    HAL_GPIO_WritePin(pins[port].disable_port, pins[port].disable_pin,
                      disable ? GPIO_PIN_SET : GPIO_PIN_RESET);
    return SFP_OK;
}

/**
 * @brief Sample GPIO-based SFP signals and update the cage LEDs.
 *
 * Reads the MOD_ABS, LOS, and TX_FAULT pins directly from the STM32 GPIO.
 * Also updates the module-present (green) and fault (red) LEDs to reflect
 * the current hardware state:
 * - Green LED ON  ↔ module physically inserted.
 * - Red LED ON    ↔ module present **and** TX_FAULT asserted.
 * - Both LEDs OFF ↔ module absent.
 *
 * @param[in]  port     SFP cage index (::SFP_PORT_PRIMARY or ::SFP_PORT_BACKUP).
 * @param[out] present  Set to @c true if MOD_ABS is asserted (active-low).
 * @param[out] los      Set to @c true if LOS is asserted (active-high).
 * @param[out] tx_fault Set to @c true if TX_FAULT is asserted (active-high).
 * @return ::SFP_OK on success, or ::SFP_ERR_ARGUMENT if any pointer is NULL
 *         or @p port is out of range.
 */
static int sfp_driver_read_gpio_status(sfp_port_t port, bool *present, bool *los, bool *tx_fault)
{
    if (present == NULL || los == NULL || tx_fault == NULL)
        return SFP_ERR_ARGUMENT;
    *present = *los = *tx_fault = false;
    if ((unsigned)port >= SFP_PORT_COUNT) return SFP_ERR_ARGUMENT;
    const sfp_pins_t *p = &pins[port];
    *present = HAL_GPIO_ReadPin(p->present_port, p->present_pin) == GPIO_PIN_RESET;
    *los = HAL_GPIO_ReadPin(p->los_port, p->los_pin) == GPIO_PIN_SET;
    *tx_fault = HAL_GPIO_ReadPin(p->fault_port, p->fault_pin) == GPIO_PIN_SET;
    
    //Turn on the green LED when module presence is confirmed. 
    //Turn on the red LED when the module is present and TX_FAULT is asserted. Keep both LEDs OFF when the module is absent.
    sfp_driver_set_mod_led(port, *present);
    sfp_driver_set_fault_led(port, *present && *tx_fault);
    return SFP_OK;
}

bool sfp_is_port_present(sfp_port_t port)
{
    if ((unsigned)port >= SFP_PORT_COUNT)
        return false;

    return HAL_GPIO_ReadPin(pins[port].present_port, pins[port].present_pin) == GPIO_PIN_RESET;
}

int sfp_driver_init(void)
{
    //enable both ports through LS
    HAL_GPIO_WritePin(STM_SFP0_LS_EN_GPIO_Port, STM_SFP0_LS_EN_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(STM_SFP1_LS_EN_GPIO_Port, STM_SFP1_LS_EN_Pin, GPIO_PIN_SET);

    for (unsigned p = 0; p < SFP_PORT_COUNT; ++p) {
        //check that both SFPs supports diagnostic reading and are internally calibrated
        uint8_t type = 0;
        if (sfp_i2c_read((sfp_port_t)p, SFP_I2C_ADDR_A0, 92, &type, 1) != 0)
            return SFP_ERR_IO;
        if (!(type & (3U << 5))) //bit 6 of byte 92 represents support
            return SFP_ERR_UNSUPPORTED;

        //poll till the sfp is ready for diags
        uint8_t ready = 1;
        while (ready) {
            if (sfp_i2c_read((sfp_port_t)p, SFP_I2C_ADDR_A2, 110, &ready, 1) != 0) {
                return SFP_ERR_IO;
            }
        }

        //Initialize LED outputs; read the present and tx_fault status for LEDs
        const sfp_pins_t *pp = &pins[p];
        bool present = HAL_GPIO_ReadPin(pp->present_port, pp->present_pin) == GPIO_PIN_RESET;
        bool tx_fault = HAL_GPIO_ReadPin(pp->fault_port, pp->fault_pin) == GPIO_PIN_SET;
        sfp_driver_set_mod_led((sfp_port_t)p, present);
        sfp_driver_set_fault_led((sfp_port_t)p, present & tx_fault);
    }

    return SFP_OK;
}

int sfp_driver_get_diagnostics(sfp_port_t port, sfp_status_t *status)
{
    if (status == NULL)
        return SFP_ERR_ARGUMENT;
    if ((unsigned)port >= SFP_PORT_COUNT)
        return SFP_ERR_ARGUMENT;

    //prepare status buffer
    memset(status, 0, sizeof(*status));
    clear_telemetry(status);
    
    //read internal diags
    uint8_t raw[SFP_DDM_RAW_LEN] = {0};
    if (sfp_i2c_read(port, SFP_I2C_ADDR_A2, 96, raw, SFP_DDM_RAW_LEN) != 0)
        return SFP_ERR_IO;

    convert_telemetry(raw, status);
    return SFP_OK;
}
