#include <CppUTest/TestHarness.h>
#include <CppUTest/CommandLineTestRunner.h>

#include <cmath>
#include <cstdint>
#include <cstring>
#include "stm32n6xx_hal.h"
#include "main.h"

extern "C" {
GPIO_TypeDef test_gpio_a;
GPIO_TypeDef test_gpio_g;
GPIO_TypeDef test_gpio_n;
GPIO_TypeDef test_gpio_p;
I2C_HandleTypeDef hi2c2;
I2C_HandleTypeDef hi2c4;
}

extern "C" {
#include "sfp_driver.h"
}

namespace {
uint8_t ddm_bytes[10];
int i2c_result;

void set_input(GPIO_TypeDef *port, uint16_t pin, GPIO_PinState state)
{
    if (state == GPIO_PIN_SET)
        port->levels |= pin;
    else
        port->levels &= static_cast<uint16_t>(~pin);
}

bool pin_is_set(GPIO_TypeDef *port, uint16_t pin)
{
    return (port->levels & pin) != 0;
}
}

extern "C" HAL_StatusTypeDef HAL_I2C_Mem_Read(I2C_HandleTypeDef *bus, uint16_t address,
                                              uint16_t mem_addr, uint16_t offset_size,
                                              uint8_t *buffer, uint16_t length, uint32_t)
{
    if (i2c_result != 0 || (bus != &hi2c2 && bus != &hi2c4) ||
        offset_size != I2C_MEMADD_SIZE_8BIT)
        return HAL_ERROR;

    if (address == SFP_I2C_ADDR_A0 && mem_addr == 92 && length == 1)
        buffer[0] = 0x60; // DDM supported and internally calibrated.
    else if (address == SFP_I2C_ADDR_A2 && mem_addr == 110 && length == 1)
        buffer[0] = 0; // Data ready.
    else if (address == SFP_I2C_ADDR_A2 && mem_addr == 96 && length == sizeof(ddm_bytes))
        std::memcpy(buffer, ddm_bytes, sizeof(ddm_bytes));
    else
        return HAL_ERROR;
    return HAL_OK;
}

extern "C" HAL_StatusTypeDef HAL_I2C_Mem_Write(I2C_HandleTypeDef *, uint16_t,
                                               uint16_t, uint16_t, uint8_t *,
                                               uint16_t, uint32_t)
{
    return HAL_OK;
}

extern "C" GPIO_PinState HAL_GPIO_ReadPin(GPIO_TypeDef *port, uint16_t pin)
{
    return pin_is_set(port, pin) ? GPIO_PIN_SET : GPIO_PIN_RESET;
}

extern "C" void HAL_GPIO_WritePin(GPIO_TypeDef *port, uint16_t pin, GPIO_PinState state)
{
    set_input(port, pin, state);
}

TEST_GROUP(SfpDriverDiagnostics)
{
    void setup()
    {
        // 1.5 C, 1 V, 2 mA, 0 dBm TX, and -10 dBm RX.
        const uint8_t sample[] = {
            0x01, 0x80, 0x27, 0x10, 0x03, 0xE8,
            0x27, 0x10, 0x03, 0xE8
        };
        std::memcpy(ddm_bytes, sample, sizeof(ddm_bytes));
        i2c_result = 0;
    }
};

TEST(SfpDriverDiagnostics, DecodesBigEndianTelemetryAndPowerThroughPublicApi)
{
    sfp_status_t status;

    CHECK_EQUAL(SFP_OK, sfp_driver_get_diagnostics(SFP_PORT_PRIMARY, &status));
    CHECK_EQUAL(true, status.valid);
    DOUBLES_EQUAL(1.5, status.temperature_c, 0.0001);
    DOUBLES_EQUAL(1.0, status.supply_voltage_v, 0.0001);
    DOUBLES_EQUAL(2.0, status.tx_bias_current_ma, 0.0001);
    DOUBLES_EQUAL(0.0, status.tx_power_dbm, 0.0001);
    DOUBLES_EQUAL(-10.0, status.rx_power_dbm, 0.0001);
}

TEST(SfpDriverDiagnostics, ZeroRawOpticalPowerBecomesNegativeInfinity)
{
    ddm_bytes[6] = ddm_bytes[7] = 0;
    ddm_bytes[8] = ddm_bytes[9] = 0;
    sfp_status_t status;

    CHECK_EQUAL(SFP_OK, sfp_driver_get_diagnostics(SFP_PORT_PRIMARY, &status));
    CHECK_EQUAL(true, std::isinf(status.tx_power_dbm));
    CHECK_EQUAL(true, status.tx_power_dbm < 0.0f);
    CHECK_EQUAL(true, std::isinf(status.rx_power_dbm));
    CHECK_EQUAL(true, status.rx_power_dbm < 0.0f);
}

TEST(SfpDriverDiagnostics, ReturnsIoErrorWhenI2cReadFails)
{
    i2c_result = -1;
    sfp_status_t status;

    CHECK_EQUAL(SFP_ERR_IO, sfp_driver_get_diagnostics(SFP_PORT_PRIMARY, &status));
    CHECK_EQUAL(false, status.valid);
}

TEST_GROUP(SfpDriverInit)
{
    void setup()
    {
        test_gpio_a.levels = 0;
        test_gpio_g.levels = 0;
        test_gpio_n.levels = 0;
        test_gpio_p.levels = 0;
        i2c_result = 0;
    }
};

TEST(SfpDriverInit, ModLedTurnsOnWhenModPinActive)
{
    // MOD_DEF0 is active low; both modules are present.
    set_input(SFP0_MOD_DEF0_GPIO_Port, SFP0_MOD_DEF0_Pin, GPIO_PIN_RESET);
    set_input(SFP1_MOD_DEF0_GPIO_Port, SFP1_MOD_DEF0_Pin, GPIO_PIN_RESET);
    CHECK_EQUAL(SFP_OK, sfp_driver_init());
    CHECK_EQUAL(true, pin_is_set(SFP0_MOD_DEF0_LED_GPIO_Port, SFP0_MOD_DEF0_LED_Pin));
    CHECK_EQUAL(true, pin_is_set(SFP1_MOD_DEF0_LED_GPIO_Port, SFP1_MOD_DEF0_LED_Pin));
}

TEST(SfpDriverInit, FaultLedTurnsOnWhenModAndFaultPinsActive)
{
    // MOD_DEF0 is active low; TX_FAULT is active high.
    set_input(SFP0_MOD_DEF0_GPIO_Port, SFP0_MOD_DEF0_Pin, GPIO_PIN_RESET);
    set_input(SFP1_MOD_DEF0_GPIO_Port, SFP1_MOD_DEF0_Pin, GPIO_PIN_RESET);
    set_input(SFP0_TX_FAULT_GPIO_Port, SFP0_TX_FAULT_Pin, GPIO_PIN_SET);
    set_input(SFP1_TX_FAULT_GPIO_Port, SFP1_TX_FAULT_Pin, GPIO_PIN_SET);

    CHECK_EQUAL(SFP_OK, sfp_driver_init());
    CHECK_EQUAL(true, pin_is_set(SFP0_TX_FAULT_LED_GPIO_Port, SFP0_TX_FAULT_LED_Pin));
    CHECK_EQUAL(true, pin_is_set(SFP1_TX_FAULT_LED_GPIO_Port, SFP1_TX_FAULT_LED_Pin));
}

TEST(SfpDriverInit, AbsentModuleTurnsBothLedsOff)
{
    set_input(SFP0_TX_FAULT_GPIO_Port, SFP0_TX_FAULT_Pin, GPIO_PIN_SET);
    set_input(SFP1_TX_FAULT_GPIO_Port, SFP1_TX_FAULT_Pin, GPIO_PIN_SET);
    CHECK_EQUAL(SFP_OK, sfp_driver_init());
    CHECK_EQUAL(true, pin_is_set(SFP0_MOD_DEF0_LED_GPIO_Port, SFP0_MOD_DEF0_LED_Pin));
    CHECK_EQUAL(true, pin_is_set(SFP1_MOD_DEF0_LED_GPIO_Port, SFP1_MOD_DEF0_LED_Pin));
    CHECK_EQUAL(true, pin_is_set(SFP0_TX_FAULT_LED_GPIO_Port, SFP0_TX_FAULT_LED_Pin));
    CHECK_EQUAL(true, pin_is_set(SFP1_TX_FAULT_LED_GPIO_Port, SFP1_TX_FAULT_LED_Pin));

    // Both modules become absent while TX_FAULT remains asserted.
    set_input(SFP0_MOD_DEF0_GPIO_Port, SFP0_MOD_DEF0_Pin, GPIO_PIN_SET);
    set_input(SFP1_MOD_DEF0_GPIO_Port, SFP1_MOD_DEF0_Pin, GPIO_PIN_SET);
    CHECK_EQUAL(SFP_OK, sfp_driver_init());
    CHECK_EQUAL(false, pin_is_set(SFP0_MOD_DEF0_LED_GPIO_Port, SFP0_MOD_DEF0_LED_Pin));
    CHECK_EQUAL(false, pin_is_set(SFP1_MOD_DEF0_LED_GPIO_Port, SFP1_MOD_DEF0_LED_Pin));
    CHECK_EQUAL(false, pin_is_set(SFP0_TX_FAULT_LED_GPIO_Port, SFP0_TX_FAULT_LED_Pin));
    CHECK_EQUAL(false, pin_is_set(SFP1_TX_FAULT_LED_GPIO_Port, SFP1_TX_FAULT_LED_Pin));
}

int main(int argc, char **argv)
{
    return CommandLineTestRunner::RunAllTests(argc, argv);
}
