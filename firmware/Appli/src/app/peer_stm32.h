#ifndef PEER_STM32_H
#define PEER_STM32_H

#include "peer_comm.h"

bool peer_stm32_init(PeerComm *peer, FailoverState *failover,
                     const uint8_t local_mac[6], const uint8_t remote_mac[6],
                     uint32_t boot_id, uint32_t now_ms);

#endif
