/* USER CODE BEGIN */
/**
 * @file optical_health.c
 * @brief Optical DDM evaluation and health monitoring implementation.
 */

#include "optical_health.h"
#include "app_config.h"
#include "path_manager.h"
#include "diagnostics.h"
#include "stm32n6xx_hal.h"

static sfp_status_t latest_status;
static optical_status_t current_status = OPTICAL_STATUS_OK;
static uint32_t last_poll_tick = 0;

void optical_health_init(void)
{
    latest_status = (sfp_status_t){0};
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

    if (!sfp_is_port_present(SFP_PORT_PRIMARY))
    {
        latest_status = (sfp_status_t){0};
        if (current_status != OPTICAL_STATUS_DISCONNECTED)
        {
            current_status = OPTICAL_STATUS_DISCONNECTED;
            diagnostics_log_event(DIAG_SEV_ERROR, "OpticalHealth: SFP module removed");
            path_manager_trigger_event(PATH_EVENT_LINK_DOWN);
        }
        return;
    }

    if (sfp_driver_get_diagnostics(SFP_PORT_PRIMARY, &latest_status) == SFP_OK &&
        latest_status.valid)
    {
        latest_status.module_present = true;
        /* Evaluate RSSI */
        if (latest_status.rx_power_dbm < (float)SFP_RSSI_MIN_DBM_MV / 10.0f)
        {
            if (current_status != OPTICAL_STATUS_WARN_LOW_POWER)
            {
                current_status = OPTICAL_STATUS_WARN_LOW_POWER;
                diagnostics_log_event(DIAG_SEV_WARNING, "OpticalHealth: Low optical RX power warning");
                path_manager_trigger_event(PATH_EVENT_OPTICAL_DEGRADED);
            }
        }
        /* Evaluate Temperature */
        else if (latest_status.temperature_c > (float)SFP_TEMP_MAX_MILLICELSIUS / 1000.0f)
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

void optical_health_get_latest(sfp_status_t *status)
{
    if (status != NULL)
    {
        *status = latest_status;
    }
}

optical_status_t optical_health_get_status(void)
{
    return current_status;
}
/* USER CODE END */
