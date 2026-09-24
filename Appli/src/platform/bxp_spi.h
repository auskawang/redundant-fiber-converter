/**
 * @file bxp_spi.h
 * @brief STM32N6 SPI peripheral initialization and low-level register transfer routines.
 */

#ifndef BXP_SPI_H
#define BXP_SPI_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initializes SPI peripheral for communication with the LAN96455 switch.
 */
void bxp_spi_init(void);

/**
 * @brief Performs a 32-bit register read over SPI.
 * @param reg_addr 32-bit address.
 * @param[out] value Pointer to store result.
 * @return 0 on success, negative on error.
 */
int bxp_spi_read_reg(uint32_t reg_addr, uint32_t *value);

/**
 * @brief Performs a 32-bit register write over SPI.
 * @param reg_addr 32-bit address.
 * @param value 32-bit value to write.
 * @return 0 on success, negative on error.
 */
int bxp_spi_write_reg(uint32_t reg_addr, uint32_t value);

#ifdef __cplusplus
}
#endif

#endif /* BXP_SPI_H */
