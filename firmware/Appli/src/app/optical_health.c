#include <stddef.h>
/**
 * @file optical_health.c
 * @brief Optical DDM evaluation and health monitoring implementation.
 */

#include "optical_health.h"
#include "app_config.h"
#include "path_manager.h"
#include "diagnostics.h"
#include "bxp_time.h"
#include "bxp_gpio.h"
#include "bxp_i2c.h"

static sfp_ddm_data_t latest_ddm;
static optical_status_t current_status = OPTICAL_STATUS_UNAVAILABLE;
static uint32_t last_poll_tick = 0;

void optical_health_init(void)
{
    last_poll_tick = bxp_time_ms();
    current_status = OPTICAL_STATUS_UNAVAILABLE;
    diagnostics_log_event(DIAG_SEV_INFO, "OpticalHealth: Initialized");
}

void optical_health_process(void)
{
    uint32_t now = bxp_time_ms();
    if ((now - last_poll_tick) < DDM_POLL_INTERVAL_MS)
    {
        return;
    }
    last_poll_tick = now;

    if (!bxp_gpio_is_ready() || !bxp_i2c_is_ready()) { current_status=OPTICAL_STATUS_UNAVAILABLE; return; }
    if (!sfp_driver_is_present())
    {
        if (current_status != OPTICAL_STATUS_DISCONNECTED)
        {
            current_status = OPTICAL_STATUS_DISCONNECTED;
            diagnostics_log_event(DIAG_SEV_ERROR, "OpticalHealth: SFP module removed");
            path_manager_trigger_event(PATH_EVENT_LINK_DOWN);
        }
        return;
    }

    if (sfp_driver_read_ddm(&latest_ddm) == 0)
    {
        /* TODO: raw optical power conversion/calibration is not validated.
         * Do not evaluate raw counts against dBm thresholds. */
        current_status = OPTICAL_STATUS_UNAVAILABLE;
        return;

    }
    else { current_status=OPTICAL_STATUS_UNAVAILABLE; }
}

void optical_health_get_latest(sfp_ddm_data_t *ddm)
{
    if (ddm != NULL)
    {
        *ddm = latest_ddm;
    }
}

optical_status_t optical_health_get_status(void)
{
    return current_status;
}
