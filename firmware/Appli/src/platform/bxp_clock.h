/**
 * @file bxp_clock.h
 * @brief STM32N6 clock tree configuration and power scaling.
 */

#ifndef BXP_CLOCK_H
#define BXP_CLOCK_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Configures system clock tree, PLLs, and peripheral bus dividers.
 */
void bxp_clock_init(void);

#ifdef __cplusplus
}
#endif

#endif /* BXP_CLOCK_H */
