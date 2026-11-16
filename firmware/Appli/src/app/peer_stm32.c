#include "peer_stm32.h"
#include "eth_rgmii.h"
#include "lan96455_driver.h"

#include <string.h>

static int transmit(void *context, const uint8_t *frame, uint16_t length)
{
    (void)context;
    return eth_rgmii_send_frame(frame, length);
}

static int set_path(void *context, uint8_t path)
{
    (void)context;
    return lan96455_set_port_connections(path == 0 ? LAN96455_PATH_PRIMARY :
                                                     LAN96455_PATH_FALLBACK);
}

bool peer_stm32_init(PeerComm *peer, FailoverState *failover,
                     const uint8_t local_mac[6], const uint8_t remote_mac[6],
                     uint32_t boot_id, uint32_t now_ms)
{
    if (!local_mac || !remote_mac) {
        return false;
    }
    PeerConfig config = { .boot_id = boot_id, .transmit = transmit, .set_path = set_path };
    memcpy(config.local_mac, local_mac, sizeof(config.local_mac));
    memcpy(config.remote_mac, remote_mac, sizeof(config.remote_mac));
    return peer_comm_init(peer, &config, failover, now_ms);
}
