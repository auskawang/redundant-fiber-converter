/* USER CODE BEGIN */
/** @file eth_rgmii.c Ethernet management frame transport. */
#include "eth_rgmii.h"
#include "stm32n6xx_hal.h"
#include "mgmt_frame.h"
#include <string.h>

#define RX_SLOT_COUNT 16U
#define FRAME_HEADER_SIZE 14U
extern ETH_HandleTypeDef heth1;

#define TX_SLOT_COUNT ETH_TX_DESC_CNT
#define TX_SLOT_SIZE __SCB_DCACHE_LINE_SIZE
__attribute__((section(".eth_dma_buffers"), aligned(__SCB_DCACHE_LINE_SIZE)))
static uint8_t rx_pool[RX_SLOT_COUNT * ETH_DMA_SLOT_SIZE];
__attribute__((section(".eth_dma_buffers"), aligned(__SCB_DCACHE_LINE_SIZE)))
static uint8_t tx_pool[TX_SLOT_COUNT * TX_SLOT_SIZE];
_Static_assert(ETH_DMA_SLOT_SIZE % __SCB_DCACHE_LINE_SIZE == 0, "RX slot alignment");
_Static_assert(TX_SLOT_SIZE >= FRAME_HEADER_SIZE + ETH_PAYLOAD_SIZE, "TX frame fits slot");
_Static_assert(RX_SLOT_COUNT < 32 && TX_SLOT_COUNT < 32, "Pool allocation bitmap width");
static uint32_t rx_free_slots;
static uint32_t tx_free_slots;
static volatile uint32_t rx_pending;
static volatile bool tx_pending;
static uint32_t rx_resume;
static bool initialized;
static uint8_t *received[RX_SLOT_COUNT];
static uint32_t received_head, received_count;
static uint8_t peer_mac[6];

static uint8_t *request_buffer(uint8_t *pool, unsigned slot_size, uint32_t *free_slots)
{
    //none of the bits in free_slots is set, meaning that none of the buffers are available
    if (*free_slots == 0) {
        return NULL;
    }

    //use the buffer associated with the rightmost set bit
    unsigned index = __builtin_ctz(*free_slots);
    *free_slots &= ~(1UL << index); //clear the bit

    //return the buffer associated with the index
    return &pool[index * slot_size];
}

static void free_rx_buffer(uint8_t *slot)
{
    unsigned index = (unsigned)(slot - rx_pool) / ETH_DMA_SLOT_SIZE;
    rx_free_slots |= 1UL << index;  //set bit to denote that its free for use
}

static void free_tx_buffer(uint8_t *slot)
{
    unsigned index = (unsigned)(slot - tx_pool) / TX_SLOT_SIZE;
    tx_free_slots |= 1UL << index; //set bit to denote that its free for use
}

int eth_start(const uint8_t remote_mac[6])
{
    if (!remote_mac) {
        return -1;
    }
    if (initialized) {
        return memcmp(remote_mac, peer_mac, sizeof(peer_mac)) == 0 ? 0 : -1;
    }
    if (heth1.gState != HAL_ETH_STATE_READY || heth1.Init.RxBuffLen != ETH_DMA_SLOT_SIZE) {
        return -1;
    }
    memcpy(peer_mac, remote_mac, sizeof(peer_mac));
    rx_free_slots = (1UL << RX_SLOT_COUNT) - 1U;
    tx_free_slots = (1UL << TX_SLOT_COUNT) - 1U;
    received_head = received_count = 0;
    rx_pending = rx_resume = 0;
    tx_pending = false;
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
    uint8_t *slot = request_buffer(tx_pool, TX_SLOT_SIZE, &tx_free_slots);
    if (slot == NULL) {
        return -2;
    }
    memcpy(slot, peer_mac, 6);
    memcpy(slot + 6, heth1.Init.MACAddr, 6);
    slot[12] = ETHERTYPE_MGMT_FRAME >> 8;
    slot[13] = (uint8_t)ETHERTYPE_MGMT_FRAME;
    memcpy(slot + FRAME_HEADER_SIZE, payload, ETH_PAYLOAD_SIZE);
    ETH_BufferTypeDef buffer = {
        .buffer = slot,
        .len = FRAME_HEADER_SIZE + ETH_PAYLOAD_SIZE,
        .next = NULL
    };
    ETH_TxPacketConfigTypeDef config = {0};
    config.TxDMACh = 0U;

    config.Attributes = ETH_TX_PACKETS_FEATURES_CRCPAD;
    config.CRCPadCtrl = ETH_CRC_PAD_INSERT;

    config.Length = buffer.len;
    config.TxBuffer = &buffer;
    config.pData = slot;

    HAL_StatusTypeDef status = HAL_ETH_Transmit_IT(&heth1, &config);
    if (status != HAL_OK) {
        free_tx_buffer(slot);
        return -3;
    }
    return 0;
}

void HAL_ETH_TxCpltCallback(ETH_HandleTypeDef *heth)
{
    if (heth == &heth1 && (heth->TxCH & ETH_DMA_CH0)) {
        tx_pending = true;
    }
}

void HAL_ETH_TxFreeCallback(uint32_t *packet)
{
    free_tx_buffer((uint8_t *)packet);
}

void HAL_ETH_RxAllocateCallback(uint8_t **buffer)
{
    *buffer = request_buffer(rx_pool, ETH_DMA_SLOT_SIZE, &rx_free_slots);
}

void HAL_ETH_RxLinkCallback(void **start, void **end, uint8_t *buffer,
                          uint16_t length)
{
    (void)length;
    if (*start == NULL) {
        *start = *end = buffer;
    } else {
        //getting to here means that multiple buffers are being used to hold data contents for a single eth frame, which shouldn't happen
        //since our data is expected to just be 8 bytes, and each data frame is 1536 bytes
        free_rx_buffer(buffer);
    }
}

void HAL_ETH_RxCpltCallback(ETH_HandleTypeDef *heth)
{
    if (heth == &heth1) {
        rx_pending |= heth->RxCH;
    }
}

static bool valid_packet(unsigned ch, uint8_t *packet)
{
    const ETH_RxDescListTypeDef *rx = &heth1.RxDescList[ch];
    /* Accept the header and payload plus optional Ethernet padding and FCS. */
    if (rx->RxDescCnt != 1 || rx->RxDataLength < FRAME_HEADER_SIZE + ETH_PAYLOAD_SIZE ||
        rx->RxDataLength > 64 || (rx->pRxLastRxDesc & ETH_DMARXNDESCWBF_ES)) {
        return false;
    }
    return memcmp(packet, heth1.Init.MACAddr, 6) == 0 &&
           memcmp(packet + 6, peer_mac, 6) == 0 &&
           packet[12] == (ETHERTYPE_MGMT_FRAME >> 8) && packet[13] == (uint8_t)ETHERTYPE_MGMT_FRAME;
}

void eth_rgmii_poll(void)
{
    if (!initialized) {
        return;
    }
    uint32_t previous = __get_PRIMASK();
    __disable_irq();
    uint32_t pending = rx_pending | rx_resume;
    bool release_tx = tx_pending;
    rx_pending = 0;
    tx_pending = false;
    __set_PRIMASK(previous);

    if (release_tx) {
        heth1.TxOpCH = 0;
        HAL_ETH_ReleaseTxPacket(&heth1);
    }
    rx_resume = 0;
    for (unsigned ch = 0; ch < ETH_DMA_RX_CH_CNT; ++ch) {
        uint32_t bit = 1UL << ch;
        /* ReadData also rebuilds descriptors after a previous pool shortage. */
        if (!(pending & bit) && !(rx_free_slots && heth1.RxDescList[ch].RxBuildDescCnt)) {
            continue;
        }
        heth1.RxOpCH = ch;
        for (unsigned n = 0; n < ETH_RX_DESC_CNT; ++n) {
            void *packet;
            if (HAL_ETH_ReadData(&heth1, &packet) != HAL_OK) {
                break;
            }
            if (valid_packet(ch, packet)) {
                received[(received_head + received_count) % RX_SLOT_COUNT] = packet;
                ++received_count;
            } else {
                free_rx_buffer(packet);
            }
            if (n + 1 == ETH_RX_DESC_CNT) {
                rx_resume |= bit;
            }
        }
    }
}

int eth_rgmii_receive_frame(uint8_t *buffer, uint16_t *length)
{
    if (!initialized || buffer == NULL || length == NULL) {
        return -1;
    }
    *length = 0;
    if (received_count == 0U) {
        return -2;
    }
    uint8_t *packet = received[received_head];
    received_head = (received_head + 1U) % RX_SLOT_COUNT;
    --received_count;
    memcpy(buffer, packet + FRAME_HEADER_SIZE, ETH_PAYLOAD_SIZE);
    *length = ETH_PAYLOAD_SIZE;
    free_rx_buffer(packet);
    return 0;
}
/* USER CODE END */
