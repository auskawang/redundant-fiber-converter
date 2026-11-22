#ifndef PEER_APP_H
#define PEER_APP_H

#include "peer_comm.h"

typedef struct {
    bool present;
    bool los;
    bool tx_fault;
} PeerOpticalPins;

typedef struct {
    PeerComm peer;
    FailoverState failover;
    FiberInput fibers[FIBER_COUNT];
    uint32_t sampled_at[FIBER_COUNT];
    uint32_t next_sample;
    uint8_t next_port;
    bool calibrate;
} PeerApp;

bool peer_app_init(PeerApp *app, const uint8_t local_mac[6],
                   const uint8_t remote_mac[6], uint32_t boot_id,
                   bool calibrate_on_boot, uint32_t now_ms);
void peer_app_process(PeerApp *app, const PeerOpticalPins pins[FIBER_COUNT]);

#endif
