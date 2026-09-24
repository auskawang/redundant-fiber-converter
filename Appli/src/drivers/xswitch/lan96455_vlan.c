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
    /* Forward port 0 (Host) to port 1 (Primary SFP/PHY) */
    lan96455_vlan_set_port_mask(0, (1U << 1));
    lan96455_vlan_set_port_mask(1, (1U << 0));
    /* Disable forwarding to port 2 (Backup) */
    lan96455_vlan_set_port_mask(2, 0x00);

    diagnostics_log_event(DIAG_SEV_INFO, "Switch: Primary forwarding mask active");
    return 0;
}

int lan96455_vlan_apply_backup_path(void)
{
    /* Forward port 0 (Host) to port 2 (Backup SFP/PHY) */
    lan96455_vlan_set_port_mask(0, (1U << 2));
    lan96455_vlan_set_port_mask(2, (1U << 0));
    /* Disable forwarding to port 1 (Primary) */
    lan96455_vlan_set_port_mask(1, 0x00);

    diagnostics_log_event(DIAG_SEV_WARNING, "Switch: Backup forwarding mask active");
    return 0;
}

int lan96455_vlan_set_port_mask(uint8_t port, uint8_t mask)
{
    if (port >= 6)
    {
        return -1;
    }
    return lan96455_write_reg(LAN96455_REG_PORT_FWD_MASK(port), (uint32_t)mask);
}
