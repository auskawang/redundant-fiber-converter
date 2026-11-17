#include "peer_app.h"
#include "peer_stm32.h"
#include "eth_rgmii.h"
#include "sfp_driver.h"
#include "stm32n6xx_hal.h"

#include <math.h>
#include <string.h>

enum { SAMPLE_INTERVAL_MS = 50, SAMPLE_MAX_AGE_MS = 250, RX_BUDGET = 8 };
_Static_assert(sizeof(((PeerApp *)0)->rx_frame) >= ETH_MAX_FRAME_SIZE, "RX buffer too small");
_Static_assert(SFP_PORT_PRIMARY == 0 && SFP_PORT_BACKUP == 1 && SFP_PORT_COUNT == FIBER_COUNT,
               "SFP port mapping must match failover paths");

bool peer_app_init(PeerApp *app, const uint8_t local_mac[6],
                   const uint8_t remote_mac[6], uint32_t boot_id,
                   bool calibrate_on_boot, uint32_t now)
{
    if (!app) {
        return false;
    }
    memset(app, 0, sizeof(*app));
    app->calibrate = calibrate_on_boot;
    app->next_sample = now;
    failover_init(&app->failover);
    return peer_stm32_init(&app->peer, &app->failover, local_mac, remote_mac, boot_id, now) &&
           eth_start() == 0;
}

void peer_app_process(PeerApp *app, const PeerOpticalPins pins[FIBER_COUNT])
{
    uint32_t now = HAL_GetTick();
    bool scheduled = app->peer.phase == PEER_COMMITTING || app->peer.phase == PEER_ARMED;
    for (unsigned i = 0; i < FIBER_COUNT; ++i) {
        FiberInput *fiber = &app->fibers[i];
        fiber->present = pins[i].present;
        fiber->los = pins[i].los;
        fiber->fault = pins[i].tx_fault;
        if (!fiber->present || fiber->los || fiber->fault ||
            now - app->sampled_at[i] >= SAMPLE_MAX_AGE_MS) {
            fiber->valid = 0;
        }
    }

    if (!scheduled && now - app->next_sample < UINT32_C(0x80000000)) {
        uint8_t i = app->next_port;
        FiberInput *fiber = &app->fibers[i];
        sfp_status_t status = {0};
        fiber->valid = fiber->present && !fiber->los && !fiber->fault &&
                       sfp_driver_get_diagnostics((sfp_port_t)i, &status) == SFP_OK &&
                       status.valid && isfinite(status.rx_power_dbm);
        now = HAL_GetTick();
        app->sampled_at[i] = now;
        if (fiber->valid) {
            fiber->rx_dbm = status.rx_power_dbm;
        }
        app->next_port = (uint8_t)(1u - i);
        app->next_sample = now + SAMPLE_INTERVAL_MS;
    }

    peer_comm_tick(&app->peer, app->fibers, now);
    if (app->calibrate && app->peer.phase == PEER_IDLE) {
        for (uint8_t i = 0; i < FIBER_COUNT; ++i) {
            if (!app->failover.baseline_valid[i] &&
                app->failover.good_ms[i] >= RECOVERY_CONFIRM_MS) {
                failover_capture_baseline(&app->failover, i, &app->fibers[i]);
            }
        }
    }

    for (unsigned n = 0; n < RX_BUDGET; ++n) {
        uint16_t length = 0;
        if (eth_rgmii_receive_frame(app->rx_frame, &length) != 0) {
            break;
        }
        if (length <= sizeof(app->rx_frame)) {
            peer_comm_receive(&app->peer, app->rx_frame, length, HAL_GetTick());
        }
    }
}
