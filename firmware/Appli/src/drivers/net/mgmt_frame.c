/* USER CODE BEGIN */
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

/* A heartbeat is two ordered eight-byte payloads. The sequence tag on
 * both halves prevents a lost half being joined to the next transmission. */
enum { HEARTBEAT_FIRST = 0xA0, HEARTBEAT_LAST = 0xA1 };
static uint8_t tx_sequence;

static void put_u32(uint8_t *out, uint32_t value)
{
    out[0] = (uint8_t)(value >> 24);
    out[1] = (uint8_t)(value >> 16);
    out[2] = (uint8_t)(value >> 8);
    out[3] = (uint8_t)value;
}

static uint32_t read_u32(const uint8_t *in)
{
    return ((uint32_t)in[0] << 24) | ((uint32_t)in[1] << 16) |
           ((uint32_t)in[2] << 8) | in[3];
}

int mgmt_frame_send_peer_heartbeat(const peer_msg_t *msg)
{
    if (!msg) {
        return -1;
    }
    uint8_t first[ETH_PAYLOAD_SIZE] = {HEARTBEAT_FIRST, ++tx_sequence};
    uint8_t last[ETH_PAYLOAD_SIZE] = {HEARTBEAT_LAST, tx_sequence};
    put_u32(first + 2, msg->sequence_number);
    first[6] = msg->active_path;
    first[7] = msg->link_health_flags;
    put_u32(last + 2, msg->timestamp_ms);
    last[6] = msg->cutover_requested;
    int status = eth_rgmii_send_frame(first, sizeof(first));
    return status == 0 ? eth_rgmii_send_frame(last, sizeof(last)) : status;
}

int mgmt_frame_parse(const uint8_t *frame, uint16_t length, peer_msg_t *msg)
{
    static uint8_t first[ETH_PAYLOAD_SIZE];
    static bool assembling;
    if (frame && msg && length == ETH_PAYLOAD_SIZE) {
        if (frame[0] == HEARTBEAT_FIRST) {
            memcpy(first, frame, sizeof(first));
            assembling = true;
            return 1;
        }
        if (!assembling || frame[0] != HEARTBEAT_LAST || first[1] != frame[1]) {
            return -2;
        }
        assembling = false;
        msg->sequence_number = read_u32(first + 2);
        msg->timestamp_ms = read_u32(frame + 2);
        msg->active_path = first[6];
        msg->link_health_flags = first[7];
        msg->cutover_requested = frame[6] != 0;
        return 0;
    }
    /* Preserve decoding of previously captured full heartbeat frames. */
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
/* USER CODE END */
