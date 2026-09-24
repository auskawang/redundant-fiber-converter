/**
 * @file eth_rgmii.h
 * @brief STM32N6 Ethernet MAC and RGMII interface frame transmission and reception.
 */

#ifndef ETH_RGMII_H
#define ETH_RGMII_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define ETH_MAX_FRAME_SIZE      1518U

/**
 * @brief Initializes the STM32N6 Ethernet MAC in RGMII mode.
 * @return 0 on success, negative error code on failure.
 */
int eth_rgmii_init(void);

/**
 * @brief Transmits a Layer-2 Ethernet frame.
 * @param frame Pointer to raw Ethernet frame buffer.
 * @param length Length of the frame in bytes.
 * @return 0 on success, negative error code on failure.
 */
int eth_rgmii_send_frame(const uint8_t *frame, uint16_t length);

/**
 * @brief Polls or retrieves a received Layer-2 frame.
 * @param[out] buffer Destination buffer.
 * @param[out] length Pointer to received byte count.
 * @return 0 if frame received, negative if buffer empty or error.
 */
int eth_rgmii_receive_frame(uint8_t *buffer, uint16_t *length);

#ifdef __cplusplus
}
#endif

#endif /* ETH_RGMII_H */
