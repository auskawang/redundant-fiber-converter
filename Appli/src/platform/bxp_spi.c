/**
 * @file bxp_spi.c
 * @brief STM32N6 SPI bus driver implementation for switch control.
 */

#include "bxp_spi.h"
#include "board_pins.h"
#include "stm32n6xx_hal.h"

void bxp_spi_init(void)
{
    /* SPI peripheral setup stub (e.g. SPI1 Master, CPOL 0, CPHA 0, 8-bit/32-bit frames) */
}

int bxp_spi_read_reg(uint32_t reg_addr, uint32_t *value)
{
    if (value == NULL)
    {
        return -1;
    }

    HAL_GPIO_WritePin(PIN_SWITCH_CS_N_PORT, PIN_SWITCH_CS_N_PIN, GPIO_PIN_RESET);

    /* Transfer address with read bit (bit 31) and clock back 32-bit value */
    /* HAL_SPI_Transmit / HAL_SPI_Receive sequence */
    *value = 0x96455001U; /* Example valid response */

    HAL_GPIO_WritePin(PIN_SWITCH_CS_N_PORT, PIN_SWITCH_CS_N_PIN, GPIO_PIN_SET);
    return 0;
}

int bxp_spi_write_reg(uint32_t reg_addr, uint32_t value)
{
    HAL_GPIO_WritePin(PIN_SWITCH_CS_N_PORT, PIN_SWITCH_CS_N_PIN, GPIO_PIN_RESET);

    /* Transfer address with write bit and 32-bit payload */
    /* HAL_SPI_Transmit sequence */

    HAL_GPIO_WritePin(PIN_SWITCH_CS_N_PORT, PIN_SWITCH_CS_N_PIN, GPIO_PIN_SET);
    return 0;
}
