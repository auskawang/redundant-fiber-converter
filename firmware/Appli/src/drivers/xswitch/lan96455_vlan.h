/**
 * @file lan96455_vlan.h
 * @brief Port forwarding masks, VLAN isolation, and failover path redirection.
 */

#ifndef LAN96455_VLAN_H
#define LAN96455_VLAN_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Configures switch port forwarding for the primary network path.
 * @return 0 on success, negative error code on failure.
 */
int lan96455_vlan_apply_primary_path(void);

/**
 * @brief Reconfigures switch port forwarding masks to redirect traffic to the backup path.
 * @return 0 on success, negative error code on failure.
 */
int lan96455_vlan_apply_backup_path(void);

#ifdef __cplusplus
}
#endif

#endif /* LAN96455_VLAN_H */
