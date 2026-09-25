#include "bxp_i2c.h"
#include "bxp_board.h"
#include "bxp_status.h"
#ifdef HAL_I2C_MODULE_ENABLED
static I2C_HandleTypeDef *bus;
void bxp_i2c_bind(I2C_HandleTypeDef *h) { bus=h; }
void bxp_i2c_init(void) { /* Configuration belongs to bxp_board_init. */ }
bool bxp_i2c_is_ready(void) { return bus && bus->State != HAL_I2C_STATE_RESET; }
static int result(HAL_StatusTypeDef s) { return s==HAL_OK?BXP_OK:s==HAL_TIMEOUT?BXP_TIMEOUT:s==HAL_BUSY?BXP_BUSY:BXP_ERROR; }
int bxp_i2c_read(uint16_t addr,uint8_t reg,uint8_t *data,uint16_t len) {
 if (!data || !len || addr>0xfe || (addr&1)) return BXP_INVALID;
 if (!bxp_i2c_is_ready()) return BXP_UNAVAILABLE;
 return result(HAL_I2C_Mem_Read(bus,addr,reg,I2C_MEMADD_SIZE_8BIT,data,len,100));
}
int bxp_i2c_write(uint16_t addr,uint8_t reg,const uint8_t *data,uint16_t len) {
 if (!data || !len || addr>0xfe || (addr&1)) return BXP_INVALID;
 if (!bxp_i2c_is_ready()) return BXP_UNAVAILABLE;
 return result(HAL_I2C_Mem_Write(bus,addr,reg,I2C_MEMADD_SIZE_8BIT,(uint8_t *)data,len,100));
}
#else
/* This ZIP omits the optional HAL bus driver. */
void bxp_i2c_init(void) {}
bool bxp_i2c_is_ready(void) { return false; }
int bxp_i2c_read(uint16_t a,uint8_t m,uint8_t *b,uint16_t n) { (void)m; return !b||!n||a>0xfe||(a&1)?BXP_INVALID:BXP_UNAVAILABLE; }
int bxp_i2c_write(uint16_t a,uint8_t m,const uint8_t *b,uint16_t n) { (void)m; return !b||!n||a>0xfe||(a&1)?BXP_INVALID:BXP_UNAVAILABLE; }
#endif
