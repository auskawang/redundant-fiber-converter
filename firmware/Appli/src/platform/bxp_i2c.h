/**
 * @file bxp_i2c.h
 * @brief STM32N6 I2C peripheral initialization and master transfer routines for SFP.
 */

#ifndef BXP_I2C_H
#define BXP_I2C_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initializes I2C peripheral for SFP DDM/EEPROM communication.
 */
void bxp_i2c_init(void);
bool bxp_i2c_is_ready(void);

/**
 * @brief Reads data from an I2C device with an 8-bit memory/register offset.
 * @param dev_addr 7-bit device address (shifted left by 1).
 * @param mem_addr 8-bit memory offset.
 * @param[out] buffer Output destination buffer.
 * @param length Bytes to read.
 * @return 0 on success, negative error code on failure.
 */
int bxp_i2c_read(uint16_t dev_addr, uint8_t mem_addr, uint8_t *buffer, uint16_t length);

/**
 * @brief Writes data to an I2C device with an 8-bit memory/register offset.
 * @param dev_addr 7-bit device address (shifted left by 1).
 * @param mem_addr 8-bit memory offset.
 * @param data Data buffer to write.
 * @param length Bytes to write.
 * @return 0 on success, negative error code on failure.
 */
int bxp_i2c_write(uint16_t dev_addr, uint8_t mem_addr, const uint8_t *data, uint16_t length);

#ifdef __cplusplus
}
#endif

#endif /* BXP_I2C_H */
