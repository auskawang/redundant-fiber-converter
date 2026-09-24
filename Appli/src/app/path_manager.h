/**
 * @file path_manager.h
 * @brief Primary/backup failover state machine and path switching logic.
 */

#ifndef PATH_MANAGER_H
#define PATH_MANAGER_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    PATH_STATE_INIT = 0,
    PATH_STATE_PRIMARY_ACTIVE,
    PATH_STATE_BACKUP_STANDBY,
    PATH_STATE_FAILOVER_IN_PROGRESS,
    PATH_STATE_SECONDARY_ACTIVE,
    PATH_STATE_FAULT
} path_state_t;

typedef enum {
    PATH_EVENT_NONE = 0,
    PATH_EVENT_LINK_DOWN,
    PATH_EVENT_OPTICAL_DEGRADED,
    PATH_EVENT_PEER_TIMEOUT,
    PATH_EVENT_MANUAL_SWAP,
    PATH_EVENT_RESTORE
} path_event_t;

/**
 * @brief Initializes the path manager state machine.
 */
void path_manager_init(void);

/**
 * @brief Periodic processing of failover logic and state transitions.
 */
void path_manager_process(void);

/**
 * @brief Posts an event to the path manager state machine.
 * @param event The event to trigger.
 */
void path_manager_trigger_event(path_event_t event);

/**
 * @brief Gets the current path manager state.
 * @return Current path_state_t.
 */
path_state_t path_manager_get_state(void);

#ifdef __cplusplus
}
#endif

#endif /* PATH_MANAGER_H */
