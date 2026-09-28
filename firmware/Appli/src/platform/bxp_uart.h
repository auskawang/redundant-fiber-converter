#ifndef BXP_UART_H
#define BXP_UART_H
#include <stdint.h>
#include <stdbool.h>
void bxp_uart_init(void);
bool bxp_uart_is_ready(void);
/* Nonblocking RX: OK means one byte; TIMEOUT means no byte available. */
int bxp_uart_read(uint8_t *byte);
/* Bounded blocking output, main-loop use only. Console must use 8-bit data. */
int bxp_uart_write(const uint8_t *data,uint16_t length);
#endif
