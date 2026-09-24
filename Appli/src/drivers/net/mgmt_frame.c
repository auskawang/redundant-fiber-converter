/**
 * @file mgmt_frame.c
 * @brief Management frame serialization implementation.
 */

#include "mgmt_frame.h"
#include "eth_rgmii.h"
#include <string.h>

#pragma pack(push, 1)
typedef struct {
    uint8_t  dest_mac[6];
    uint8_t  src_mac[6];
    uint16_t ethertype;
    uint32_t seq;
    uint32_t timestamp;
    uint8_t  active_path;
    uint8_t  health;
    uint8_t  cutover;
    uint8_t  padding[42];
} l2_heartbeat_pkt_t;
#pragma pack(pop)

int mgmt_frame_send_peer_heartbeat(const peer_msg_t *msg)
{
    if (msg == NULL)
    {
        return -1;
    }

    l2_heartbeat_pkt_t pkt;
    memset(&pkt, 0, sizeof(pkt));

    /* Multicast / direct peer destination MAC: 01:80:C2:00:00:0E */
    pkt.dest_mac[0] = 0x01; pkt.dest_mac[1] = 0x80; pkt.dest_mac[2] = 0xC2;
    pkt.dest_mac[3] = 0x00; pkt.dest_mac[4] = 0x00; pkt.dest_mac[5] = 0x0E;

    pkt.ethertype = __builtin_bswap16(ETHERTYPE_MGMT_FRAME);
    pkt.seq = msg->sequence_number;
    pkt.timestamp = msg->timestamp_ms;
    pkt.active_path = msg->active_path;
    pkt.health = msg->link_health_flags;
    pkt.cutover = msg->cutover_requested ? 1 : 0;

    return eth_rgmii_send_frame((const uint8_t *)&pkt, sizeof(pkt));
}

int mgmt_frame_parse(const uint8_t *frame, uint16_t length, peer_msg_t *msg)
{
    if (frame == NULL || length < sizeof(l2_heartbeat_pkt_t) || msg == NULL)
    {
        return -1;
    }

    const l2_heartbeat_pkt_t *pkt = (const l2_heartbeat_pkt_t *)frame;
    if (__builtin_bswap16(pkt->ethertype) != ETHERTYPE_MGMT_FRAME)
    {
        return -2;
    }

    msg->sequence_number = pkt->seq;
    msg->timestamp_ms = pkt->timestamp;
    msg->active_path = pkt->active_path;
    msg->link_health_flags = pkt->health;
    msg->cutover_requested = (pkt->cutover != 0);

    return 0;
}
