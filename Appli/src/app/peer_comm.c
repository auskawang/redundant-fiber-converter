/**
 * @file peer_comm.c
 * @brief Inter-STM32 heartbeat and sync implementation.
 */

#include "peer_comm.h"
#include "board_pins.h"
#include "app_config.h"
#include "stm32n6xx_hal.h"
#include "mgmt_frame.h"
#include "diagnostics.h"

static uint32_t last_tx_tick = 0;
static uint32_t last_rx_tick = 0;
static uint32_t tx_seq_num = 0;
static bool peer_alive = true;

void peer_comm_init(void)
{
    last_tx_tick = HAL_GetTick();
    last_rx_tick = HAL_GetTick();
    peer_alive = true;
    diagnostics_log_event(DIAG_SEV_INFO, "PeerComm: Channel initialized");
}

void peer_comm_process(void)
{
    uint32_t now = HAL_GetTick();

    /* Heartbeat Transmission */
    if ((now - last_tx_tick) >= HEARTBEAT_TX_INTERVAL_MS)
    {
        last_tx_tick = now;

        peer_msg_t msg = {
            .sequence_number = tx_seq_num++,
            .timestamp_ms = now,
            .active_path = 0,
            .link_health_flags = 0x01,
            .cutover_requested = false
        };

        mgmt_frame_send_peer_heartbeat(&msg);

        /* Toggle direct GPIO sync pulse */
        HAL_GPIO_TogglePin(PIN_PEER_SYNC_OUT_PORT, PIN_PEER_SYNC_OUT_PIN);
    }

    /* Heartbeat Timeout Check */
    if ((now - last_rx_tick) > HEARTBEAT_TIMEOUT_MS)
    {
        if (peer_alive)
        {
            peer_alive = false;
            diagnostics_log_event(DIAG_SEV_ERROR, "PeerComm: Peer heartbeat timeout!");
        }
    }
}

void peer_comm_notify_cutover(bool backup_active)
{
    peer_msg_t msg = {
        .sequence_number = tx_seq_num++,
        .timestamp_ms = HAL_GetTick(),
        .active_path = backup_active ? 1 : 0,
        .link_health_flags = 0x01,
        .cutover_requested = true
    };
    mgmt_frame_send_peer_heartbeat(&msg);
}

bool peer_comm_is_peer_alive(void)
{
    return peer_alive;
}

void peer_comm_rx_callback(const peer_msg_t *msg)
{
    if (msg != NULL)
    {
        last_rx_tick = HAL_GetTick();
        peer_alive = true;
    }
}
