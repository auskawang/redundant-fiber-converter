/* USER CODE BEGIN */
/**
 * @file lan96455_driver.h
 * @brief LAN96455F initialization, forwarding path selection, and SPI diagnostics.
 * @note These operations block. Call them from foreground or task context.
 * @ingroup switch_driver
 */

/**
 * @defgroup switch_driver ETH_SW
 * @brief LAN96455F Ethernet switch initialization, SPI register access,
 *        port forwarding configuration, and failover path control.
 * @{
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
 * @brief Initialize the switch and enable forwarding through the primary SFP.
 * @pre SPI4 and the STM32 Ethernet MAC address have been initialized.
 * @return 0 on success, negative error code (-1) on failure or timeout.
 */
int lan96455_init(void);

/**
 * @brief Route copper and STM32 traffic through the selected SFP.
 * @note A failed register write can leave the path partially updated.
 * @param mode Path selection (LAN96455_PATH_PRIMARY or LAN96455_PATH_FALLBACK).
 * @return 0 on success, negative error code (-1) on failure.
 */
int lan96455_set_port_connections(lan96455_path_mode_t mode);

/**
 * @brief Configure little-endian register data and verify the read-padding field.
 * @note Zero padding is expected, so an all-zero readback passes this check.
 * @return 0 on a match, -1 on transfer failure, or -2 on a padding mismatch.
 */
int lan96455_verify_spi_comm(void);

#ifdef __cplusplus
}
#endif

#endif /* LAN96455_DRIVER_H */
/** @} */
/* USER CODE END */
