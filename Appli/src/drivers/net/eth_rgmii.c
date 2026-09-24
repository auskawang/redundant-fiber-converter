/**
 * @file eth_rgmii.c
 * @brief Ethernet RGMII driver implementation.
 */

#include "eth_rgmii.h"
#include "stm32n6xx_hal.h"
#include "diagnostics.h"

int eth_rgmii_init(void)
{
    diagnostics_log_event(DIAG_SEV_INFO, "ETH_RGMII: Initializing MAC interface");
    return 0;
}

int eth_rgmii_send_frame(const uint8_t *frame, uint16_t length)
{
    if (frame == NULL || length == 0 || length > ETH_MAX_FRAME_SIZE)
    {
        return -1;
    }
    /* HAL_ETH_Transmit stub */
    return 0;
}

int eth_rgmii_receive_frame(uint8_t *buffer, uint16_t *length)
{
    if (buffer == NULL || length == NULL)
    {
        return -1;
    }
    /* HAL_ETH_ReadData stub */
    *length = 0;
    return -2; /* No frame available */
}
