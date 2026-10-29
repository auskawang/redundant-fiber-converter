/* USER CODE BEGIN */
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

#define ETH_MAX_FRAME_SIZE      64U
#define ETH_PAYLOAD_SIZE        8U

/**
 * @brief Initializes the pool and starts interrupts after MX_ETH1_Init.
 * @return 0 on success, negative error code on failure.
 */
int eth_rgmii_init(void);

/**
 * @brief Copies an eight-byte payload into a fixed Ethernet frame and submits it.
 * @param payload Eight payload bytes; reusable after this call.
 * @param length Must equal ETH_PAYLOAD_SIZE.
 * @return 0 on success, negative error code on failure.
 */
int eth_rgmii_send_frame(const uint8_t *payload, uint16_t length);

/**
 * @brief Services RX notifications and copies one queued eight-byte payload.
 * Call repeatedly from foreground until -2 (empty), never from an ISR.
 * Init/send/receive require a single foreground caller (no task reentrancy).
 * @param[out] buffer Destination with space for at least ETH_PAYLOAD_SIZE bytes.
 * @param[out] length Receives eight on success, zero when empty.
 * @return 0 if frame received, negative if buffer empty or error.
 */
int eth_rgmii_receive_frame(uint8_t *buffer, uint16_t *length);

#ifdef __cplusplus
}
#endif

#endif /* ETH_RGMII_H */
/* USER CODE END */
