#include "bxp_spi.h"
#include "bxp_board.h"
#include "bxp_status.h"
#ifdef HAL_SPI_MODULE_ENABLED
static SPI_HandleTypeDef *bus;
static void (*chip_select)(int);
void bxp_spi_bind(SPI_HandleTypeDef *h,void (*select)(int)) { bus=h; chip_select=select; }
void bxp_spi_init(void) { }
/* Raw transport only. Caller supplies a confirmed wire protocol; CS spans frame. */
int bxp_spi_transfer(const uint8_t *tx,uint8_t *rx,uint16_t len) {
 if (!tx || !rx || !len) return BXP_INVALID;
 if (!bus || !chip_select || bus->State==HAL_SPI_STATE_RESET) return BXP_UNAVAILABLE;
 if (bus->Init.DataSize!=SPI_DATASIZE_8BIT) return BXP_INVALID;
 chip_select(1);
 HAL_StatusTypeDef rc=HAL_SPI_TransmitReceive(bus,(uint8_t *)tx,rx,len,100);
 chip_select(0);
 return rc==HAL_OK?BXP_OK:rc==HAL_TIMEOUT?BXP_TIMEOUT:rc==HAL_BUSY?BXP_BUSY:BXP_ERROR;
}
/* TODO: validate exact switch model, command framing, endian and register map.
 * Raw HAL transfer is implemented; register protocol is intentionally unavailable. */
int bxp_spi_read_reg(uint32_t addr,uint32_t *value) {
 (void)addr; return value?BXP_UNAVAILABLE:BXP_INVALID;
}
int bxp_spi_write_reg(uint32_t addr,uint32_t value) {
 (void)addr; (void)value; return BXP_UNAVAILABLE;
}
#else
/* This ZIP omits the optional HAL bus driver. */
void bxp_spi_init(void) {}
int bxp_spi_transfer(const uint8_t *t,uint8_t *r,uint16_t n) { return t&&r&&n?BXP_UNAVAILABLE:BXP_INVALID; }
int bxp_spi_read_reg(uint32_t a,uint32_t *v) { (void)a; return v?BXP_UNAVAILABLE:BXP_INVALID; }
int bxp_spi_write_reg(uint32_t a,uint32_t v) { (void)a; (void)v; return BXP_UNAVAILABLE; }
#endif
