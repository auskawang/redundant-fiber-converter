/* USER CODE BEGIN */
/** @file eth_rgmii.c Ethernet management frame transport. */
#include "eth_rgmii.h"
#include "stm32n6xx_hal.h"
#include "mgmt_frame.h"
#include <string.h>

#define FRAME_POOL_COUNT 16U
#define FRAME_HEADER_SIZE 14U
extern ETH_HandleTypeDef heth1;

/* TODO: Establish DMA-accessible placement, per-buffer alignment and cache
 * maintenance before hardware use. Keep metadata off DMA buffer cache lines. */
static uint8_t pool[ETH_MAX_FRAME_SIZE * FRAME_POOL_COUNT]; 
static uint8_t free_indices[FRAME_POOL_COUNT];
static uint32_t free_count;
static volatile uint32_t rx_pending;
static bool initialized;
/* Foreground-only queue retains drained packets until callers consume them. */
static uint8_t *received[FRAME_POOL_COUNT];
static uint32_t received_head, received_count;
/* TODO: Fill in the peer board's destination MAC address. */
static const uint8_t peer_mac[6] = {0};

static uint32_t lock_interrupts(void)
{
    uint32_t previous = __get_PRIMASK();
    __disable_irq();
    return previous;
}

static uint8_t *request_buffer(void)
{
    uint32_t previous = lock_interrupts();
    uint8_t *slot = NULL;
    if (free_count != 0U) {
        slot = &pool[free_indices[--free_count] * ETH_MAX_FRAME_SIZE];
    }
    __set_PRIMASK(previous);
    return slot;
}

static void free_buffer(uint8_t *slot)
{
    /* Internal callers supply pool entries; suppress double returns. */
    uint32_t previous = lock_interrupts();
    free_indices[free_count++] = (uint8_t)((slot - pool) / ETH_MAX_FRAME_SIZE);
    __set_PRIMASK(previous);
}

int eth_start(void)
{
    if (initialized) {
        return 0;
    }
    //TODO: add MAC configuration
  /**
   * MACConf.DuplexMode = duplex;
      MACConf.Speed = speed;
      MACConf.DropTCPIPChecksumErrorPacket = DISABLE; //changed
      MACConf.ForwardRxErrorPacket = ENABLE;   //FEP Bit
      MACConf.ForwardRxUndersizedGoodPacket = ENABLE; //FUP Bit
      MACConf.LoopbackMode= DISABLE;
   */
    //HAL_ETH_Init sets gState to HAL_ETH_STATE_READY when done
    if (heth1.gState != HAL_ETH_STATE_READY) {
        return -1;
    }
    memset(pool, 0, sizeof(pool));
    for (uint8_t i = 0; i < FRAME_POOL_COUNT; i++) {
        free_indices[i] = i;
    }
    free_count = FRAME_POOL_COUNT;
    received_head = received_count = 0;
    rx_pending = 0;

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
    uint8_t *slot = request_buffer();
    if (slot == NULL) {
        return -2;
    }
    //building the ethernet frame
    memset(slot, 0, ETH_MAX_FRAME_SIZE);    //clear out frame
    memcpy(slot, peer_mac, 6);   //dest addr mac
    memcpy(slot + 6, heth1.Init.MACAddr, 6); //src addr mac
    uint16_t eth_type = ETHERTYPE_MGMT_FRAME;
    memcpy(slot + 12, &eth_type, 2);  //eth type
    memcpy(slot + FRAME_HEADER_SIZE, payload, ETH_PAYLOAD_SIZE); //payload (8 bytes)
    ETH_BufferTypeDef buffer = {
        .buffer = slot,
        .len = FRAME_HEADER_SIZE + ETH_PAYLOAD_SIZE,
        .next = NULL
    };
    ETH_TxPacketConfigTypeDef config = {0};
    //selecting channel
    config.TxDMACh = 0U;

    //crc and pad automatic insertion
    config.Attributes = ETH_TX_PACKETS_FEATURES_CRCPAD;
    config.CRCPadCtrl = ETH_CRC_PAD_INSERT;

    config.Length = buffer.len;
    config.TxBuffer = &buffer;
    config.pData = slot;    //when the transmit callback returns it will give us pData so that we can later free the slot back into the pool

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
    //signal non-interrupt task to call releaseTxPacket
}

void HAL_ETH_TxFreeCallback(uint32_t *packet)
{
    free_buffer((uint8_t *)packet);
}

void HAL_ETH_RxAllocateCallback(uint8_t **buffer)
{
    uint8_t *slot = request_buffer();
    *buffer = slot != NULL ? slot : NULL;
    /* TODO: Retry RX replenishment after exhaustion without relying on a new
     * receive interrupt. No retry mechanism is implemented yet. */
}

void HAL_ETH_RxLinkCallback(void **start, void **end, uint8_t *buffer,
                          uint16_t length)
{
    /* bytes is the first member, so its address identifies the pool entry. */
    uint8_t *slot = (uint8_t *)buffer;
    if (*start == NULL) {
        *start = slot;
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
    uint8_t *packet = received[received_head];
    received_head = (received_head + 1U) % FRAME_POOL_COUNT;
    --received_count;
    /* TODO: Validate length, addresses, EtherType, tags and errors; handle
     * unexpected/fragmented frames. For now assume the agreed untagged frame
     * with its eight-byte payload entirely in the first receive buffer. */
    memcpy(buffer, packet + FRAME_HEADER_SIZE, ETH_PAYLOAD_SIZE);
    *length = ETH_PAYLOAD_SIZE;
    free_buffer(packet);
    return 0;
}
/* USER CODE END */
