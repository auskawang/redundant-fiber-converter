/**
 * @file sfp_driver.c
 * @brief SFP I2C driver implementation for DDM reading.
 */

#include "sfp_driver.h"
#include "sfp_gpio.h"
#include "bxp_i2c.h"
#include "stm32n6xx_hal.h"
#include <string.h>

void sfp_driver_init(void)
{
    /* I2C peripheral is initialized via bxp_i2c_init() */
}

bool sfp_driver_is_present(void)
{
    return sfp_gpio_is_module_present();
}

int sfp_driver_read_ddm(sfp_ddm_data_t *ddm)
{
    if (ddm == NULL)
    {
        return -1;
    }

    uint8_t raw[10];
    /* DDM A2h memory map: Temp (96-97), Vcc (98-99), TX Bias (100-101), TX Power (102-103), RX Power (104-105) */
    if (bxp_i2c_read(SFP_I2C_ADDR_A2, 96, raw, sizeof(raw)) != 0)
    {
        return -2;
    }

    /* Convert raw 16-bit big-endian readings */
    int16_t raw_temp = (int16_t)((raw[0] << 8) | raw[1]);
    uint16_t raw_vcc = (uint16_t)((raw[2] << 8) | raw[3]);
    uint16_t raw_bias = (uint16_t)((raw[4] << 8) | raw[5]);
    uint16_t raw_tx = (uint16_t)((raw[6] << 8) | raw[7]);
    uint16_t raw_rx = (uint16_t)((raw[8] << 8) | raw[9]);

    ddm->temperature_millicelsius = ((int32_t)raw_temp * 1000) / 256;
    ddm->supply_voltage_mv = raw_vcc / 10;
    ddm->tx_bias_current_ua = (uint32_t)raw_bias * 2;
    ddm->tx_power_dbm_tenths = (int32_t)raw_tx; /* Scaled per MSA standard */
    ddm->rx_power_dbm_tenths = (int32_t)raw_rx;

    return 0;
}

int sfp_driver_read_id(uint8_t *buffer, uint16_t length)
{
    if (buffer == NULL || length == 0)
    {
        return -1;
    }
    return bxp_i2c_read(SFP_I2C_ADDR_A0, 0, buffer, length);
}
