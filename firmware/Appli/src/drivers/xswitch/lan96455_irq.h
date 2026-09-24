/**
 * @file lan96455_irq.h
 * @brief LAN9646 / LAN96455 interrupt service routines and port change notifications.
 */

#ifndef LAN96455_IRQ_H
#define LAN96455_IRQ_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Enables hardware interrupt lines on the switch.
 * @return 0 on success, negative error code on failure.
 */
int lan96455_irq_enable(void);

/**
 * @brief Top-half / ISR callback triggered when switch asserts its IRQ_N line.
 */
void lan96455_irq_handler_callback(void);

#ifdef __cplusplus
}
#endif

#endif /* LAN96455_IRQ_H */
