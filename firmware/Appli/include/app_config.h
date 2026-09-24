/**
 * @file app_config.h
 * @brief Global build flags, feature toggles, and system operational thresholds.
 */

#ifndef APP_CONFIG_H
#define APP_CONFIG_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* System Information */
#define APP_FW_VERSION_MAJOR        1
#define APP_FW_VERSION_MINOR        0
#define APP_FW_VERSION_PATCH        0

/* Heartbeat & Failover Timing (in milliseconds) */
#define HEARTBEAT_TX_INTERVAL_MS    10U     /**< Interval for sending peer heartbeats */
#define HEARTBEAT_TIMEOUT_MS        50U     /**< Timeout threshold to trigger failover */
#define PEER_COMM_MAX_RETRIES       3U      /**< Max lost packets before declaring peer dead */

/* Optical (SFP) Health Thresholds */
#define DDM_POLL_INTERVAL_MS        100U    /**< SFP DDM sampling interval */
#define SFP_RSSI_MIN_DBM_MV         (-200)  /**< Minimum optical RSSI threshold (0.1 dBm units) */
#define SFP_TEMP_MAX_MILLICELSIUS   85000   /**< Maximum SFP temperature (85 deg C) */
#define SFP_TX_BIAS_MAX_UA          80000   /**< Maximum TX bias current (uA) */

/* Diagnostics & Logging */
#define DIAG_RING_BUFFER_SIZE       128U    /**< Size of the event log ring-buffer */
#define DIAG_STAT_INTERVAL_MS       1000U   /**< Link stats aggregation interval */

/* Switch Configuration */
#define LAN96455_SPI_BAUDRATE_HZ    25000000U /**< SPI clock frequency for LAN9646/LAN96455 */
#define LAN96455_PORT_COUNT         6U        /**< Port count */

#ifdef __cplusplus
}
#endif

#endif /* APP_CONFIG_H */
