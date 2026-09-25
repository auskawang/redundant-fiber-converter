#include "bxp_time.h"
#include "stm32n6xx_hal.h"
uint32_t bxp_time_ms(void) { return HAL_GetTick(); }
