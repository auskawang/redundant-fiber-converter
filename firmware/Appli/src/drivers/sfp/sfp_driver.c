#include "sfp_driver.h"
#include "main.h"
#include "bxp_i2c.h"
#include <math.h>
#include <string.h>

static uint16_t be16(const uint8_t *p)
{
    return (uint16_t)(((uint16_t)p[0] << 8) | p[1]);
}

static float power_dbm(uint16_t raw)
{
    /* SFF-8472: one count = 0.1 uW = 0.0001 mW. */
    return raw == 0 ? -INFINITY : 10.0f * log10f((float)raw / 10000.0f);
}

static void clear_telemetry(sfp_status_t *status)
{
    status->ddm_valid = false;
    status->rx_power_dbm = NAN;
    status->tx_power_dbm = NAN;
    status->tx_bias_current_ma = NAN;
    status->temperature_c = NAN;
    status->supply_voltage_v = NAN;
}

/* Fixed CubeMX mappings; CubeMX initializes GPIOs and peripherals. */
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

int sfp_driver_set_mod_led(sfp_port_t port, bool on)
{
    if ((unsigned)port >= SFP_PORT_COUNT) return SFP_ERR_ARGUMENT;
    HAL_GPIO_WritePin(pins[port].mod_led_port, pins[port].mod_led_pin,
                      on ? GPIO_PIN_SET : GPIO_PIN_RESET);
    return SFP_OK;
}

int sfp_driver_set_fault_led(sfp_port_t port, bool on)
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

int sfp_driver_read_gpio_status(sfp_port_t port, bool *present, bool *los, bool *tx_fault)
{
    if (present == NULL || los == NULL || tx_fault == NULL)
        return SFP_ERR_ARGUMENT;
    *present = *los = *tx_fault = false;
    if ((unsigned)port >= SFP_PORT_COUNT) return SFP_ERR_ARGUMENT;
    const sfp_pins_t *p = &pins[port];
    *present = HAL_GPIO_ReadPin(p->present_port, p->present_pin) == GPIO_PIN_RESET;
    *los = HAL_GPIO_ReadPin(p->los_port, p->los_pin) == GPIO_PIN_SET;
    *tx_fault = HAL_GPIO_ReadPin(p->fault_port, p->fault_pin) == GPIO_PIN_SET;
    sfp_driver_set_mod_led(port, *present);
    sfp_driver_set_fault_led(port, *present && *tx_fault);
    return SFP_OK;
}

void sfp_driver_init(void)
{
    for (unsigned p = 0; p < SFP_PORT_COUNT; ++p) {
        sfp_driver_set_mod_led((sfp_port_t)p, false);
        sfp_driver_set_fault_led((sfp_port_t)p, false);
    }
}

void sfp_driver_process(void)
{
    for (unsigned p = 0; p < SFP_PORT_COUNT; ++p) {
        bool present, los, fault;
        sfp_driver_read_gpio_status((sfp_port_t)p, &present, &los, &fault);
    }
}

int sfp_driver_get_status(sfp_port_t port, sfp_status_t *status)
{
    if (status == NULL)
        return SFP_ERR_ARGUMENT;
    memset(status, 0, sizeof(*status));
    clear_telemetry(status);
    if ((unsigned)port >= SFP_PORT_COUNT)
        return SFP_ERR_ARGUMENT;

    int rc = sfp_driver_read_gpio_status(port, &status->module_present,
                                 &status->los, &status->tx_fault);
    if (rc != SFP_OK)
        return rc;
    status->gpio_valid = true;
    if (!status->module_present)
        return SFP_ERR_ABSENT;

    uint8_t type = 0;
    if (bxp_i2c_read_port((uint8_t)port, SFP_I2C_ADDR_A0, 92, &type, 1) != 0)
        return SFP_ERR_IO;
    /* Require DDM and internal calibration. External calibration and legacy
     * address-change modules need additional support; never misinterpret them. */
    if ((type & 0x60U) != 0x60U || (type & 0x14U) != 0)
        return SFP_ERR_UNSUPPORTED;

    uint8_t ready = 1;
    if (bxp_i2c_read_port((uint8_t)port, SFP_I2C_ADDR_A2, 110, &ready, 1) != 0)
        return SFP_ERR_IO;
    if ((ready & 1U) != 0)
        return SFP_ERR_NOT_READY;

    /* Include status byte 110 to check Data_Ready_Bar again after sampling. */
    uint8_t raw[15] = {0};
    if (bxp_i2c_read_port((uint8_t)port, SFP_I2C_ADDR_A2, 96, raw, sizeof(raw)) != 0)
        return SFP_ERR_IO;
    rc = sfp_driver_read_gpio_status(port, &status->module_present,
                             &status->los, &status->tx_fault);
    status->gpio_valid = rc == SFP_OK;
    if (rc != SFP_OK)
        return rc;
    if (!status->module_present)
        return SFP_ERR_ABSENT;
    if ((raw[14] & 1U) != 0)
        return SFP_ERR_NOT_READY;

    int32_t temperature = be16(raw);
    if ((temperature & 0x8000) != 0)
        temperature -= 65536;
    status->temperature_c = (float)temperature / 256.0f;
    status->supply_voltage_v = (float)be16(raw + 2) / 10000.0f;
    status->tx_bias_current_ma = (float)be16(raw + 4) / 500.0f;
    status->tx_power_dbm = power_dbm(be16(raw + 6));
    status->rx_power_dbm = power_dbm(be16(raw + 8));
    status->ddm_valid = true;
    return SFP_OK;
}

bool sfp_driver_is_present(sfp_port_t port)
{
    bool present, los, fault;
    return sfp_driver_read_gpio_status(port, &present, &los, &fault) == SFP_OK
           && present;
}

int sfp_driver_read_ddm(sfp_port_t port, sfp_ddm_data_t *ddm)
{
    if (ddm == NULL)
        return SFP_ERR_ARGUMENT;
    memset(ddm, 0, sizeof(*ddm));
    sfp_status_t status;
    int rc = sfp_driver_get_status(port, &status);
    if (rc != SFP_OK)
        return rc;
    ddm->temperature_millicelsius = (int32_t)lroundf(status.temperature_c * 1000.0f);
    ddm->supply_voltage_mv = (uint32_t)lroundf(status.supply_voltage_v * 1000.0f);
    ddm->tx_bias_current_ua = (uint32_t)lroundf(status.tx_bias_current_ma * 1000.0f);
    ddm->tx_power_dbm_tenths = isinf(status.tx_power_dbm) ? SFP_POWER_DBM_TENTHS_ZERO
                             : (int32_t)lroundf(status.tx_power_dbm * 10.0f);
    ddm->rx_power_dbm_tenths = isinf(status.rx_power_dbm) ? SFP_POWER_DBM_TENTHS_ZERO
                             : (int32_t)lroundf(status.rx_power_dbm * 10.0f);
    return SFP_OK;
}

int sfp_driver_read_id(sfp_port_t port, uint8_t *buffer, uint16_t length)
{
    if ((unsigned)port >= SFP_PORT_COUNT || buffer == NULL || length == 0 || length > 256)
        return SFP_ERR_ARGUMENT;
    return bxp_i2c_read_port((uint8_t)port, SFP_I2C_ADDR_A0, 0, buffer, length) == 0
           ? SFP_OK : SFP_ERR_IO;
}
