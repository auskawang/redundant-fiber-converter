#include "app_init.h"
#include "bxp_gpio.h"
void app_init_power_sequence(void) { bxp_gpio_power_sequence(); }
void app_init_reset_switch(void) { bxp_gpio_reset_switch(); }
void app_init_reset_phys(void) { bxp_gpio_reset_phys(); }
