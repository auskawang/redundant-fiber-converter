/**
 * @file diagnostics.h
 * @brief Diagnostic event logging, link statistics, and error ring buffer.
 */

#ifndef DIAGNOSTICS_H
#define DIAGNOSTICS_H

#include <stdint.h>
#include <stdbool.h>
#include "app_config.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    DIAG_SEV_DEBUG = 0,
    DIAG_SEV_INFO,
    DIAG_SEV_WARNING,
    DIAG_SEV_ERROR
} diag_severity_t;

typedef struct {
    uint32_t timestamp_ms;
    diag_severity_t severity;
    const char *message;
} diag_log_entry_t;

typedef struct {
    uint32_t rx_packet_count;
    uint32_t tx_packet_count;
    uint32_t crc_error_count;
    uint32_t failover_count;
} diag_stats_t;

/**
 * @brief Initializes diagnostics ring buffer and counters.
 */
void diagnostics_init(void);

/**
 * @brief Periodic statistics calculation and health reporting.
 */
void diagnostics_process(void);

/**
 * @brief Logs an event into the diagnostic ring buffer.
 * @param severity Severity level.
 * @param message Description string.
 */
void diagnostics_log_event(diag_severity_t severity, const char *message);

/**
 * @brief Returns current aggregated statistics.
 */
void diagnostics_get_stats(diag_stats_t *stats);
uint16_t diagnostics_log_count(void);
int diagnostics_get_log(uint16_t index, diag_log_entry_t *entry);
/* Messages passed to log_event must have static lifetime; main-loop only. */

#ifdef __cplusplus
}
#endif

#endif /* DIAGNOSTICS_H */
