/* USER CODE BEGIN */
/**
 * @file lan96455_driver.h
 * @brief LAN9646 / LAN9645x SPI communication and core register access.
 */

#ifndef LAN96455_DRIVER_H
#define LAN96455_DRIVER_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Forwarding path selection mode for port connections and failover.
 */
typedef enum {
    LAN96455_PATH_PRIMARY = 0,   /**< Route user & management traffic to Primary SFP (Port 5) */
    LAN96455_PATH_FALLBACK = 1   /**< Route user & management traffic to Backup SFP (Port 6) */
} lan96455_path_mode_t;

/**
 * @brief Complete single-function startup initialization for the Ethernet switch.
 * @return 0 on success, negative error code (-1) on failure or timeout.
 */
int lan96455_init(void);

/**
 * @brief Flushes internal packet queues and allows in-flight frames to drain.
 * @return 0 on success, negative error code (-1) on failure.
 */
int lan96455_flush_queues(void);

/**
 * @brief Configures port forwarding connections based on the selected path mode.
 * @param mode Path selection (LAN96455_PATH_PRIMARY or LAN96455_PATH_FALLBACK).
 * @return 0 on success, negative error code (-1) on failure.
 */
int lan96455_set_port_connections(lan96455_path_mode_t mode);

/**
 * @brief Reads a 32-bit register from the switch via SPI with operational padding bytes.
 * @param reg_addr 32-bit register address.
 * @param[out] value Pointer to read output.
 * @return 0 on success, negative error code on failure.
 */
int lan96455_read_reg(uint32_t reg_addr, uint32_t *value);

/**
 * @brief Writes a 32-bit register to the switch via SPI.
 * @param reg_addr 32-bit register address.
 * @param value Value to write.
 * @return 0 on success, negative error code on failure.
 */
int lan96455_write_reg(uint32_t reg_addr, uint32_t value);

/**
 * @brief Verifies bidirectional SPI communication with the switch via write-and-readback.
 * @return 0 on success, negative error code on failure.
 */
int lan96455_verify_spi_comm(void);

#ifdef __cplusplus
}
#endif

#endif /* LAN96455_DRIVER_H */
/* USER CODE END */
