/**
 * @file bxp_gpio.h
 * @brief STM32N6 GPIO clocking, pin configuration, and EXTI interrupt routing.
 */

#ifndef BXP_GPIO_H
#define BXP_GPIO_H
#include <stdbool.h>
void bxp_gpio_process(void);
bool bxp_gpio_is_ready(void);
void bxp_gpio_peer_toggle(void);
void bxp_gpio_power_sequence(void);
void bxp_gpio_reset_switch(void);
void bxp_gpio_reset_phys(void);

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Configures GPIO clocks, modes (input, output, alternate function), and EXTI lines.
 */
void bxp_gpio_init(void);

#ifdef __cplusplus
}
#endif

#endif /* BXP_GPIO_H */
