/**
 * @file optical_health.c
 * @brief Optical DDM evaluation and health monitoring implementation.
 */

#include "optical_health.h"
#include "app_config.h"
#include "path_manager.h"
#include "diagnostics.h"
#include "stm32n6xx_hal.h"

static sfp_ddm_data_t latest_ddm;
static optical_status_t current_status = OPTICAL_STATUS_OK;
static uint32_t last_poll_tick = 0;

void optical_health_init(void)
{
    last_poll_tick = HAL_GetTick();
    current_status = OPTICAL_STATUS_OK;
    diagnostics_log_event(DIAG_SEV_INFO, "OpticalHealth: Initialized");
}

void optical_health_process(void)
{
    uint32_t now = HAL_GetTick();
    if ((now - last_poll_tick) < DDM_POLL_INTERVAL_MS)
    {
        return;
    }
    last_poll_tick = now;

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
        /* Evaluate RSSI */
        if (latest_ddm.rx_power_dbm_tenths < SFP_RSSI_MIN_DBM_MV)
        {
            if (current_status != OPTICAL_STATUS_WARN_LOW_POWER)
            {
                current_status = OPTICAL_STATUS_WARN_LOW_POWER;
                diagnostics_log_event(DIAG_SEV_WARNING, "OpticalHealth: Low optical RX power warning");
                path_manager_trigger_event(PATH_EVENT_OPTICAL_DEGRADED);
            }
        }
        /* Evaluate Temperature */
        else if (latest_ddm.temperature_millicelsius > SFP_TEMP_MAX_MILLICELSIUS)
        {
            if (current_status != OPTICAL_STATUS_WARN_HIGH_TEMP)
            {
                current_status = OPTICAL_STATUS_WARN_HIGH_TEMP;
                diagnostics_log_event(DIAG_SEV_WARNING, "OpticalHealth: High SFP temperature warning");
            }
        }
        else
        {
            if (current_status != OPTICAL_STATUS_OK)
            {
                current_status = OPTICAL_STATUS_OK;
                diagnostics_log_event(DIAG_SEV_INFO, "OpticalHealth: Optical telemetry normalized");
            }
        }
    }
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
