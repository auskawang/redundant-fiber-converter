#include "bxp_uart.h"
#include "bxp_board.h"
#include "bxp_status.h"
#ifdef HAL_UART_MODULE_ENABLED
static UART_HandleTypeDef *uart;
void bxp_uart_bind(UART_HandleTypeDef *h) { uart=h; }
void bxp_uart_init(void) { }
bool bxp_uart_is_ready(void) { return uart && uart->gState != HAL_UART_STATE_RESET && uart->Init.WordLength==UART_WORDLENGTH_8B && uart->Init.Parity==UART_PARITY_NONE; }
static int result(HAL_StatusTypeDef s) { return s==HAL_OK?BXP_OK:s==HAL_TIMEOUT?BXP_TIMEOUT:s==HAL_BUSY?BXP_BUSY:BXP_ERROR; }
int bxp_uart_read(uint8_t *byte) {
 if (!byte) return BXP_INVALID;
 if (!bxp_uart_is_ready()) return BXP_UNAVAILABLE;
 return result(HAL_UART_Receive(uart,byte,1,0));
}
int bxp_uart_write(const uint8_t *data,uint16_t len) {
 if (!data || !len) return BXP_INVALID;
 if (!bxp_uart_is_ready()) return BXP_UNAVAILABLE;
 return result(HAL_UART_Transmit(uart,(uint8_t *)data,len,50));
}
#else
/* This ZIP omits the optional HAL bus driver. */
void bxp_uart_init(void) {}
bool bxp_uart_is_ready(void) { return false; }
int bxp_uart_read(uint8_t *b) { return b?BXP_UNAVAILABLE:BXP_INVALID; }
int bxp_uart_write(const uint8_t *b,uint16_t n) { return b&&n?BXP_UNAVAILABLE:BXP_INVALID; }
#endif
