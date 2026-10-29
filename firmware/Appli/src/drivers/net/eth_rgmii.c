/* USER CODE BEGIN */
/** @file eth_rgmii.c Ethernet management frame transport. */
#include "eth_rgmii.h"
#include "stm32n6xx_hal.h"
#include "mgmt_frame.h"
#include <string.h>

#define FRAME_POOL_COUNT 16U
#define FRAME_HEADER_SIZE 14U
extern ETH_HandleTypeDef heth1;

typedef struct frame_slot {
    uint8_t bytes[ETH_MAX_FRAME_SIZE];
    struct frame_slot *next;
    uint16_t length;
    bool allocated;
} frame_slot_t;

/* TODO: Establish DMA-accessible placement, per-buffer alignment and cache
 * maintenance before hardware use. Keep metadata off DMA buffer cache lines. */
static frame_slot_t pool[FRAME_POOL_COUNT];
static uint8_t free_indices[FRAME_POOL_COUNT];
static uint32_t free_count;
static volatile uint32_t rx_pending;
static bool initialized;
/* Foreground-only queue retains drained packets until callers consume them. */
static frame_slot_t *received[FRAME_POOL_COUNT];
static uint32_t received_head, received_count;
/* TODO: Fill in the peer board's destination MAC address. */
static const uint8_t peer_mac[6] = {0};

static uint32_t lock_interrupts(void)
{
    uint32_t previous = __get_PRIMASK();
    __disable_irq();
    return previous;
}

static frame_slot_t *request_buffer(void)
{
    uint32_t previous = lock_interrupts();
    frame_slot_t *slot = NULL;
    if (free_count != 0U) {
        slot = &pool[free_indices[--free_count]];
        slot->allocated = true;
        slot->next = NULL;
        slot->length = 0;
    }
    __set_PRIMASK(previous);
    return slot;
}

static void free_buffer(frame_slot_t *slot)
{
    /* Internal callers supply pool entries; suppress double returns. */
    uint32_t previous = lock_interrupts();
    if (slot->allocated) {
        slot->allocated = false;
        free_indices[free_count++] = (uint8_t)(slot - pool);
    }
    __set_PRIMASK(previous);
}

int eth_rgmii_init(void)
{
    ETH_MACMTLMappingTypeDef mapping;
    if (initialized) {
        return 0;
    }
    /* MX_ETH1_Init must initialize the handle and descriptor rings first. */
    if (heth1.gState != HAL_ETH_STATE_READY) {
        return -1;
    }
    memset(pool, 0, sizeof(pool));
    for (uint32_t i = 0; i < FRAME_POOL_COUNT; ++i) {
        free_indices[i] = (uint8_t)i;
    }
    free_count = FRAME_POOL_COUNT;
    received_head = received_count = 0;
    rx_pending = 0;
    if (HAL_ETHEx_GetMACMTLMappingConfig(&heth1, &mapping) != HAL_OK) {
        return -1;
    }
    mapping.UntaggedPacketQueue = 0U; /* TODO: Dig deeper into queue 0 -> DMA channel 0 management routing. */
    mapping.TypeFieldBasedRxQueuingEnable = DISABLE;
    mapping.MulticastBroadcastQueue = 0U;
    if (HAL_ETHEx_SetMACMTLMappingConfig(&heth1, &mapping) != HAL_OK) {
        return -1;
    }
    /* Retain HAL default queue-to-channel mapping and both descriptor rings.
     * TODO: Wire init and the eight-byte API into peer_comm/mgmt_frame; the
     * existing heartbeat serializer still supplies a larger complete frame. */
    if (HAL_ETH_Start_IT(&heth1) != HAL_OK) {
        return -1;
    }
    initialized = true;
    return 0;
}

int eth_rgmii_send_frame(const uint8_t *payload, uint16_t length)
{
    if (!initialized || payload == NULL || length != ETH_PAYLOAD_SIZE) {
        return -1;
    }
    frame_slot_t *slot = request_buffer();
    if (slot == NULL) {
        return -2;
    }
    memset(slot->bytes, 0, sizeof(slot->bytes));
    memcpy(slot->bytes, peer_mac, 6);
    /* Reuse the fixed source MAC already configured by MX_ETH1_Init. */
    memcpy(slot->bytes + 6, heth1.Init.MACAddr, 6);
    slot->bytes[12] = (uint8_t)(ETHERTYPE_MGMT_FRAME >> 8);
    slot->bytes[13] = (uint8_t)ETHERTYPE_MGMT_FRAME;
    memcpy(slot->bytes + FRAME_HEADER_SIZE, payload, ETH_PAYLOAD_SIZE);
    ETH_BufferTypeDef buffer = {
        .buffer = slot->bytes,
        .len = FRAME_HEADER_SIZE + ETH_PAYLOAD_SIZE,
        .next = NULL
    };
    ETH_TxPacketConfigTypeDef config = {0};
    config.TxDMACh = 0U; /* TODO: Revisit channel 0 selection and queue mapping. */
    config.Attributes = ETH_TX_PACKETS_FEATURES_CRCPAD;
    config.CRCPadCtrl = ETH_CRC_PAD_INSERT;
    config.Length = buffer.len;
    config.TxBuffer = &buffer;
    config.pData = slot;
    /* TODO: Compare TX configuration with the online example previously viewed.
     * HAL consumes local descriptions here; the frame bytes remain in the pool. */
    uint32_t previous = lock_interrupts();
    HAL_StatusTypeDef status = HAL_ETH_Transmit_IT(&heth1, &config);
    __set_PRIMASK(previous);
    if (status != HAL_OK) {
        free_buffer(slot);
        return -3;
    }
    return 0;
}

void HAL_ETH_TxCpltCallback(ETH_HandleTypeDef *heth)
{
    if (heth != &heth1) {
        return;
    }
    for (uint32_t ch = 0; ch < ETH_DMA_TX_CH_CNT; ++ch) {
        if ((heth->TxCH & (1UL << ch)) != 0U) {
            heth->TxOpCH = ch;
            (void)HAL_ETH_ReleaseTxPacket(heth);
        }
    }
}

void HAL_ETH_TxFreeCallback(uint32_t *packet)
{
    free_buffer((frame_slot_t *)packet);
}

void HAL_ETH_RxAllocateCallback(uint8_t **buffer)
{
    frame_slot_t *slot = request_buffer();
    *buffer = slot != NULL ? slot->bytes : NULL;
    /* TODO: Retry RX replenishment after exhaustion without relying on a new
     * receive interrupt. No retry mechanism is implemented yet. */
}

void HAL_ETH_RxLinkCallback(void **start, void **end, uint8_t *buffer,
                          uint16_t length)
{
    /* bytes is the first member, so its address identifies the pool entry. */
    frame_slot_t *slot = (frame_slot_t *)buffer;
    slot->length = length;
    slot->next = NULL;
    if (*start == NULL) {
        *start = slot;
    } else {
        ((frame_slot_t *)*end)->next = slot;
    }
    *end = slot;
}

void HAL_ETH_RxCpltCallback(ETH_HandleTypeDef *heth)
{
    if (heth == &heth1) {
        rx_pending |= heth->RxCH;
    }
}

int eth_rgmii_receive_frame(uint8_t *buffer, uint16_t *length)
{
    if (!initialized || buffer == NULL || length == NULL) {
        return -1;
    }
    *length = 0;
    uint32_t previous = lock_interrupts();
    uint32_t pending = rx_pending;
    rx_pending = 0; /* Clear BEFORE draining; preserve notifications during processing. */
    __set_PRIMASK(previous);
    for (uint32_t ch = 0; ch < ETH_DMA_RX_CH_CNT; ++ch) {
        if ((pending & (1UL << ch)) == 0U) {
            continue;
        }
        heth1.RxOpCH = ch;
        void *packet;
        while (HAL_ETH_ReadData(&heth1, &packet) == HAL_OK) {
            /* Queue capacity equals pool capacity; each packet owns at least
             * one slot, so there cannot be more queued packets than entries. */
            received[(received_head + received_count) % FRAME_POOL_COUNT] = packet;
            ++received_count;
        }
    }
    if (received_count == 0U) {
        return -2;
    }
    frame_slot_t *packet = received[received_head];
    received_head = (received_head + 1U) % FRAME_POOL_COUNT;
    --received_count;
    /* TODO: Validate length, addresses, EtherType, tags and errors; handle
     * unexpected/fragmented frames. For now assume the agreed untagged frame
     * with its eight-byte payload entirely in the first receive buffer. */
    memcpy(buffer, packet->bytes + FRAME_HEADER_SIZE, ETH_PAYLOAD_SIZE);
    *length = ETH_PAYLOAD_SIZE;
    while (packet != NULL) {
        frame_slot_t *next = packet->next;
        free_buffer(packet);
        packet = next;
    }
    return 0;
}
/* USER CODE END */
