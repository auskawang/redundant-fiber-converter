/**
 * @file app_init.h
 * @brief Board power sequencing and peripheral hardware RESET_N control.
 */

#ifndef APP_INIT_H
#define APP_INIT_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Runs board power sequencing and asserts/de-asserts RESET_N lines.
 */
void app_init_power_sequence(void);

/**
 * @brief Resets the LAN9646/LAN96455 switch via its dedicated RESET_N pin.
 */
void app_init_reset_switch(void);

/**
 * @brief Resets external Ethernet PHYs via their RESET_N pin.
 */
void app_init_reset_phys(void);

#ifdef __cplusplus
}
#endif

#endif /* APP_INIT_H */
