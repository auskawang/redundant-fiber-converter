/**
 * @file bxp_spi.c
 * @brief STM32N6 SPI bus driver implementation for switch control.
 */

#include "bxp_spi.h"
#include "board_pins.h"
#include "lan96455_regs.h"
#include "stm32n6xx_hal.h"
#include <string.h>

/* SPI Handle defined in CubeMX / HAL initialization */
//extern SPI_HandleTypeDef hspi1;

void bxp_spi_init(void)
{
    /* SPI peripheral setup stub (e.g. SPI1 Master, CPOL 0, CPHA 0, 8-bit frames) */
}

int bxp_spi_read_reg(uint32_t reg_addr, uint32_t *value)
{
    if (value == NULL)
    {
        return -1;
    }

    uint8_t tx_buf[4];
    uint8_t rx_buf[4] = {0};
    uint8_t dummy_tx = 0xFF;
    uint8_t dummy_rx = 0;

    /* Frame 32-bit address with Read command (Big-Endian / MSB-first on wire) */
    tx_buf[0] = (uint8_t)((reg_addr >> 24) & 0xFF);
    tx_buf[1] = (uint8_t)((reg_addr >> 16) & 0xFF);
    tx_buf[2] = (uint8_t)((reg_addr >> 8) & 0xFF);
    tx_buf[3] = (uint8_t)(reg_addr & 0xFF);

    HAL_GPIO_WritePin(PIN_SWITCH_CS_N_PORT, PIN_SWITCH_CS_N_PIN, GPIO_PIN_RESET);

    /* 1. Transmit address */
    /* HAL_SPI_Transmit(&hspi1, tx_buf, 4, HAL_MAX_DELAY); */

    /* 2. Clock out dummy padding bytes using operational constant */
    for (uint8_t i = 0; i < LAN9645X_SPI_OPERATIONAL_PADDING; i++)
    {
        /* HAL_SPI_TransmitReceive(&hspi1, &dummy_tx, &dummy_rx, 1, HAL_MAX_DELAY); */
    }

    /* 3. Receive 32-bit data (Big-Endian from switch default) */
    /* HAL_SPI_Receive(&hspi1, rx_buf, 4, HAL_MAX_DELAY); */

    HAL_GPIO_WritePin(PIN_SWITCH_CS_N_PORT, PIN_SWITCH_CS_N_PIN, GPIO_PIN_SET);

    /* Assemble received word: switch default transmits MSB first */
    *value = ((uint32_t)rx_buf[0] << 24) |
             ((uint32_t)rx_buf[1] << 16) |
             ((uint32_t)rx_buf[2] << 8)  |
             ((uint32_t)rx_buf[3]);

    return 0;
}

int bxp_spi_read_reg_default_padding(uint32_t reg_addr, uint32_t *value)
{
    if (value == NULL)
    {
        return -1;
    }

    uint8_t tx_buf[4];
    uint8_t rx_buf[4] = {0};
    uint8_t dummy_tx = 0xFF;
    uint8_t dummy_rx = 0;

    /* Frame 32-bit address with Read command (Big-Endian / MSB-first on wire) */
    tx_buf[0] = (uint8_t)((reg_addr >> 24) & 0xFF);
    tx_buf[1] = (uint8_t)((reg_addr >> 16) & 0xFF);
    tx_buf[2] = (uint8_t)((reg_addr >> 8) & 0xFF);
    tx_buf[3] = (uint8_t)(reg_addr & 0xFF);

    HAL_GPIO_WritePin(PIN_SWITCH_CS_N_PORT, PIN_SWITCH_CS_N_PIN, GPIO_PIN_RESET);

    /* 1. Transmit address */
    /* HAL_SPI_Transmit(&hspi1, tx_buf, 4, HAL_MAX_DELAY); */

    /* 2. Clock out dummy padding bytes using default constant */
    for (uint8_t i = 0; i < LAN9645X_SPI_DEFAULT_PADDING; i++)
    {
        /* HAL_SPI_TransmitReceive(&hspi1, &dummy_tx, &dummy_rx, 1, HAL_MAX_DELAY); */
    }

    /* 3. Receive 32-bit data (Big-Endian from switch default) */
    /* HAL_SPI_Receive(&hspi1, rx_buf, 4, HAL_MAX_DELAY); */

    HAL_GPIO_WritePin(PIN_SWITCH_CS_N_PORT, PIN_SWITCH_CS_N_PIN, GPIO_PIN_SET);

    /* Assemble received word: switch default transmits MSB first */
    *value = ((uint32_t)rx_buf[0] << 24) |
             ((uint32_t)rx_buf[1] << 16) |
             ((uint32_t)rx_buf[2] << 8)  |
             ((uint32_t)rx_buf[3]);

    return 0;
}

int bxp_spi_write_reg(uint32_t reg_addr, uint32_t value)
{
    uint8_t tx_buf[8];

    /* Address bytes (Big-Endian / MSB-first on wire) */
    tx_buf[0] = (uint8_t)((reg_addr >> 24) & 0xFF);
    tx_buf[1] = (uint8_t)((reg_addr >> 16) & 0xFF);
    tx_buf[2] = (uint8_t)((reg_addr >> 8) & 0xFF);
    tx_buf[3] = (uint8_t)(reg_addr & 0xFF);

    /* Data payload (Big-Endian / MSB-first on wire matching switch default) */
    tx_buf[4] = (uint8_t)((value >> 24) & 0xFF);
    tx_buf[5] = (uint8_t)((value >> 16) & 0xFF);
    tx_buf[6] = (uint8_t)((value >> 8) & 0xFF);
    tx_buf[7] = (uint8_t)(value & 0xFF);

    HAL_GPIO_WritePin(PIN_SWITCH_CS_N_PORT, PIN_SWITCH_CS_N_PIN, GPIO_PIN_RESET);

    /* Transmit address + data */
    /* HAL_SPI_Transmit(&hspi1, tx_buf, 8, HAL_MAX_DELAY); */

    HAL_GPIO_WritePin(PIN_SWITCH_CS_N_PORT, PIN_SWITCH_CS_N_PIN, GPIO_PIN_SET);
    return 0;
}
