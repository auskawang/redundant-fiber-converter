#ifndef PEER_COMM_H
#define PEER_COMM_H

#include <stdbool.h>
#include <stdint.h>
#include "failover.h"

/* Required by the existing mgmt_frame.h and its legacy heartbeat codec. */
typedef struct {
    uint32_t sequence_number;
    uint32_t timestamp_ms;
    uint8_t active_path;
    uint8_t link_health_flags;
    bool cutover_requested;
} peer_msg_t;

typedef enum {
    PEER_IDLE, PEER_REQUESTING, PEER_PREPARED,
    PEER_COMMITTING, PEER_ARMED, PEER_VERIFYING, PEER_UNCERTAIN
} PeerPhase;

typedef int (*PeerTransmit)(void *context, const uint8_t *frame, uint16_t length);
typedef int (*PeerSetPath)(void *context, uint8_t path);

typedef struct {
    uint8_t local_mac[6];
    uint8_t remote_mac[6];
    uint32_t boot_id; /* Nonzero and unique across reboots of this board. */
    PeerTransmit transmit;
    PeerSetPath set_path;
    void *context;
} PeerConfig;

typedef struct {
    PeerConfig config;
    FailoverState *failover;
    PeerPhase phase;
    uint8_t target;
    bool confirmed;
    bool apply_failed;
    uint32_t counter;
    uint32_t origin;
    uint32_t request;
    uint32_t responder_boot;
    uint32_t token;
    uint32_t deadline;
    uint32_t next_tx;
    uint32_t last_tick;
} PeerComm;

/* Call after switch and failover initialization. Single main-loop owner. */
bool peer_comm_init(PeerComm *peer, const PeerConfig *config, FailoverState *failover,
                    uint32_t now_ms);
/* Call with fresh local samples. Returns false from receive for unrelated or invalid frames. */
void peer_comm_tick(PeerComm *peer, const FiberInput fibers[FIBER_COUNT], uint32_t now_ms);
bool peer_comm_receive(PeerComm *peer, const uint8_t *frame, uint16_t length, uint32_t now_ms);

#endif
