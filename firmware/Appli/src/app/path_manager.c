/**
 * @file path_manager.c
 * @brief Primary/backup failover state machine implementation.
 */

#include "path_manager.h"
#include "lan96455_driver.h"
#include "lan96455_vlan.h"
#include "peer_comm.h"
#include "diagnostics.h"

static path_state_t current_state = PATH_STATE_INIT;

void path_manager_init(void)
{
    current_state = PATH_STATE_INIT;
    diagnostics_log_event(DIAG_SEV_WARNING, "PathManager: UNAVAILABLE - switch protocol pending");
}

void path_manager_process(void)
{
    switch (current_state)
    {
        case PATH_STATE_PRIMARY_ACTIVE:
            /* Normal operation on primary link */
            break;

        case PATH_STATE_FAILOVER_IN_PROGRESS:
            /* Execute cutover: reprogram switch VLAN / forwarding mask */
            lan96455_vlan_apply_backup_path();
            lan96455_flush_mac_table();
            peer_comm_notify_cutover(true);
            current_state = PATH_STATE_SECONDARY_ACTIVE;
            diagnostics_log_event(DIAG_SEV_WARNING, "PathManager: Cutover complete -> SECONDARY_ACTIVE");
            break;

        case PATH_STATE_SECONDARY_ACTIVE:
            /* Running on backup link */
            break;

        case PATH_STATE_FAULT:
            /* Both links unavailable or hardware error */
            break;

        default:
            break;
    }
}

void path_manager_trigger_event(path_event_t event)
{
    switch (event)
    {
        case PATH_EVENT_LINK_DOWN:
        case PATH_EVENT_OPTICAL_DEGRADED:
        case PATH_EVENT_PEER_TIMEOUT:
            if (current_state == PATH_STATE_PRIMARY_ACTIVE)
            {
                current_state = PATH_STATE_FAILOVER_IN_PROGRESS;
                diagnostics_log_event(DIAG_SEV_WARNING, "PathManager: Triggering failover to backup path");
            }
            break;

        case PATH_EVENT_RESTORE:
            if (current_state == PATH_STATE_SECONDARY_ACTIVE)
            {
                lan96455_vlan_apply_primary_path();
                lan96455_flush_mac_table();
                peer_comm_notify_cutover(false);
                current_state = PATH_STATE_PRIMARY_ACTIVE;
                diagnostics_log_event(DIAG_SEV_INFO, "PathManager: Restored to PRIMARY_ACTIVE");
            }
            break;

        default:
            break;
    }
}

path_state_t path_manager_get_state(void)
{
    return current_state;
}
