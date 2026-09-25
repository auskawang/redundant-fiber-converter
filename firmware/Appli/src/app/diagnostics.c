/**
 * @file diagnostics.c
 * @brief Diagnostic ring buffer and statistics tracking implementation.
 */

#include "diagnostics.h"
#include "bxp_time.h"
#include <string.h>

static diag_log_entry_t ring_buffer[DIAG_RING_BUFFER_SIZE];
static uint16_t ring_head = 0;
static uint16_t ring_count = 0;
static diag_stats_t system_stats;
static uint32_t last_stat_tick = 0;

void diagnostics_init(void)
{
    ring_head = 0;
    ring_count = 0;
    memset(&system_stats, 0, sizeof(system_stats));
    last_stat_tick = bxp_time_ms();
}

void diagnostics_process(void)
{
    uint32_t now = bxp_time_ms();
    if ((now - last_stat_tick) >= DIAG_STAT_INTERVAL_MS)
    {
        last_stat_tick = now;
        /* Periodic background statistics rollups / health checks */
    }
}

void diagnostics_log_event(diag_severity_t severity, const char *message)
{
    ring_buffer[ring_head].timestamp_ms = bxp_time_ms();
    ring_buffer[ring_head].severity = severity;
    ring_buffer[ring_head].message = message;

    ring_head = (ring_head + 1) % DIAG_RING_BUFFER_SIZE;
    if (ring_count < DIAG_RING_BUFFER_SIZE)
    {
        ring_count++;
    }
}

void diagnostics_get_stats(diag_stats_t *stats)
{
    if (stats != NULL)
    {
        *stats = system_stats;
    }
}

uint16_t diagnostics_log_count(void) { return ring_count; }
int diagnostics_get_log(uint16_t i, diag_log_entry_t *out) {
 if (!out || i>=ring_count) return -1;
 *out=ring_buffer[(ring_head+DIAG_RING_BUFFER_SIZE-ring_count+i)%DIAG_RING_BUFFER_SIZE]; return 0;
}
