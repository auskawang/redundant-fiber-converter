/**
 * @file lan96455_driver.h
 * @brief LAN9646 / LAN96455 SPI communication and core register access.
 */

#ifndef LAN96455_DRIVER_H
#define LAN96455_DRIVER_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initializes the switch driver and verifies chip ID.
 * @return 0 on success, negative error code on failure.
 */
int lan96455_init(void);

/**
 * @brief Reads a 32-bit register from the switch via SPI.
 * @param reg_addr Register address.
 * @param[out] value Pointer to read output.
 * @return 0 on success, negative error code on failure.
 */
int lan96455_read_reg(uint32_t reg_addr, uint32_t *value);

/**
 * @brief Writes a 32-bit register to the switch via SPI.
 * @param reg_addr Register address.
 * @param value Value to write.
 * @return 0 on success, negative error code on failure.
 */
int lan96455_write_reg(uint32_t reg_addr, uint32_t value);

/**
 * @brief Flushes the dynamic MAC address lookup table.
 * @return 0 on success, negative error code on failure.
 */
int lan96455_flush_mac_table(void);

#ifdef __cplusplus
}
#endif

#endif /* LAN96455_DRIVER_H */
