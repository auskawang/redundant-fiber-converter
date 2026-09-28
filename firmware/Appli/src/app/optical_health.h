/**
 * @file optical_health.h
 * @brief Optical telemetry (DDM) monitoring: RSSI, temperature, voltage, and bias current.
 */

#ifndef OPTICAL_HEALTH_H
#define OPTICAL_HEALTH_H

#include <stdint.h>
#include <stdbool.h>
#include "sfp_driver.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    OPTICAL_STATUS_OK = 0,
    OPTICAL_STATUS_WARN_LOW_POWER,
    OPTICAL_STATUS_WARN_HIGH_TEMP,
    OPTICAL_STATUS_CRITICAL_LOSS,
    OPTICAL_STATUS_DISCONNECTED,
    OPTICAL_STATUS_UNAVAILABLE
} optical_status_t;

/**
 * @brief Initializes optical monitoring subsystem.
 */
void optical_health_init(void);

/**
 * @brief Periodically polls DDM from SFP transceiver and evaluates health.
 */
void optical_health_process(void);

/**
 * @brief Returns the latest optical telemetry reading.
 * @param[out] ddm Pointer to output ddm data struct.
 */
void optical_health_get_latest(sfp_ddm_data_t *ddm);

/**
 * @brief Returns current overall optical health status.
 */
optical_status_t optical_health_get_status(void);

#ifdef __cplusplus
}
#endif

#endif /* OPTICAL_HEALTH_H */
