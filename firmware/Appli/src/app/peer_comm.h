/**
 * @file peer_comm.h
 * @brief Inter-STM32 heartbeat protocol and failover synchronization.
 */

#ifndef PEER_COMM_H
#define PEER_COMM_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    uint32_t sequence_number;
    uint32_t timestamp_ms;
    uint8_t  active_path;       /**< 0: Primary, 1: Backup */
    uint8_t  link_health_flags; /**< Bitfield of optical/link health */
    bool     cutover_requested;
} peer_msg_t;

/**
 * @brief Initializes peer communication channel and sync GPIOs.
 */
void peer_comm_init(void);

/**
 * @brief Periodic processing of heartbeat transmission and timeout detection.
 */
void peer_comm_process(void);

/**
 * @brief Sends a cutover notification to the peer STM32.
 * @param backup_active True if switching to backup path.
 */
void peer_comm_notify_cutover(bool backup_active);

/**
 * @brief Checks if peer STM32 is alive and communicating.
 * @return True if peer heartbeat is within timeout threshold.
 */
bool peer_comm_is_peer_alive(void);
void peer_comm_rx_callback(const peer_msg_t *msg);

#ifdef __cplusplus
}
#endif

#endif /* PEER_COMM_H */
