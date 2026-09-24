/**
 * @file mgmt_frame.h
 * @brief Proprietary Layer-2 management frame formatting and serialization for peer sync.
 */

#ifndef MGMT_FRAME_H
#define MGMT_FRAME_H

#include <stdint.h>
#include <stdbool.h>
#include "peer_comm.h"

#ifdef __cplusplus
extern "C" {
#endif

#define ETHERTYPE_MGMT_FRAME    0x88B5U /**< IEEE 802 Local Experimental Ethertype */

/**
 * @brief Serializes and transmits an inter-STM32 heartbeat frame over Ethernet.
 * @param msg Pointer to heartbeat data.
 * @return 0 on success, negative error code on failure.
 */
int mgmt_frame_send_peer_heartbeat(const peer_msg_t *msg);

/**
 * @brief Parses an incoming Layer-2 management frame.
 * @param frame Raw Ethernet frame.
 * @param length Frame length.
 * @param[out] msg Extracted peer message.
 * @return 0 on success, negative if frame is invalid or not a management frame.
 */
int mgmt_frame_parse(const uint8_t *frame, uint16_t length, peer_msg_t *msg);

#ifdef __cplusplus
}
#endif

#endif /* MGMT_FRAME_H */
