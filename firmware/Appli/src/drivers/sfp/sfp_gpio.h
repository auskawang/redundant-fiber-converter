/**
 * @file sfp_gpio.h
 * @brief SFP control and status discrete pins (TX_DISABLE, LOS IRQs, MOD_ABS).
 */

#ifndef SFP_GPIO_H
#define SFP_GPIO_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Enables or disables the SFP optical transmitter.
 * @param disable True to disable TX laser, False to enable.
 */
void sfp_gpio_set_tx_disable(bool disable);

/**
 * @brief Checks if SFP Loss of Signal (LOS) is asserted.
 * @return True if optical signal is lost.
 */
bool sfp_gpio_is_los(void);

/**
 * @brief Checks if SFP module is absent.
 * @return True if module is physically present.
 */
bool sfp_gpio_is_module_present(void);

/**
 * @brief Checks if TX Fault condition is active.
 * @return True if transmitter fault detected.
 */
bool sfp_gpio_is_tx_fault(void);

/**
 * @brief Interrupt handler callback for SFP LOS state changes.
 */
void sfp_gpio_los_isr_callback(void);

#ifdef __cplusplus
}
#endif

#endif /* SFP_GPIO_H */
