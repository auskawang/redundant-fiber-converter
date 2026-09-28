#ifndef BXP_BOARD_H
#define BXP_BOARD_H
#include "stm32n6xx_hal.h"
/* Board-only integration: override init, configure clocks/AF/timing/baud and bind
 * initialized static HAL handles. No guessed peripheral or pin assignments. */
void bxp_board_init(void);
#ifdef HAL_I2C_MODULE_ENABLED
void bxp_i2c_bind(I2C_HandleTypeDef *handle);
#endif
#ifdef HAL_UART_MODULE_ENABLED
void bxp_uart_bind(UART_HandleTypeDef *handle);
#endif
#ifdef HAL_SPI_MODULE_ENABLED
void bxp_spi_bind(SPI_HandleTypeDef *handle, void (*select)(int active));
#endif
#endif
