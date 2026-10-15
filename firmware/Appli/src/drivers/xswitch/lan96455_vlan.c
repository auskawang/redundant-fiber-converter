/* USER CODE BEGIN */
/**
 * @file lan96455_vlan.c
 * @brief Port forwarding masks and VLAN path steering implementation.
 */

#include "lan96455_vlan.h"
#include "lan96455_driver.h"
#include "lan96455_regs.h"
#include "diagnostics.h"

int lan96455_vlan_apply_primary_path(void)
{
    return lan96455_set_port_connections(LAN96455_PATH_PRIMARY);
}

int lan96455_vlan_apply_backup_path(void)
{
    return lan96455_set_port_connections(LAN96455_PATH_FALLBACK);
}
/* USER CODE END */
