/**
 * @file bxp_i2c.c
 * @brief STM32N6 I2C master driver implementation for SFP telemetry.
 */

#include "bxp_i2c.h"
#include "board_pins.h"
#include "stm32n6xx_hal.h"

void bxp_i2c_init(void)
{
    /* I2C peripheral init stub (Standard mode 100kHz or Fast mode 400kHz) */
}

int bxp_i2c_read(uint16_t dev_addr, uint8_t mem_addr, uint8_t *buffer, uint16_t length)
{
    if (buffer == NULL || length == 0)
    {
        return -1;
    }

    /* Emulate reading or wrap HAL_I2C_Mem_Read(&hi2c1, dev_addr, mem_addr, I2C_MEMADD_SIZE_8BIT, buffer, length, 100) */
    return 0;
}

int bxp_i2c_write(uint16_t dev_addr, uint8_t mem_addr, const uint8_t *data, uint16_t length)
{
    if (data == NULL || length == 0)
    {
        return -1;
    }

    /* Wrap HAL_I2C_Mem_Write */
    return 0;
}
