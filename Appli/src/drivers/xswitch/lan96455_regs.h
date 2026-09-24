/**
 * @file lan96455_regs.h
 * @brief LAN9646 / LAN96455 Ethernet switch register address definitions and bitmasks.
 */

#ifndef LAN96455_REGS_H
#define LAN96455_REGS_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Global Control Registers */
#define LAN96455_REG_CHIP_ID            0x0000U
#define LAN96455_REG_SW_RESET           0x0004U
#define LAN96455_REG_GLOBAL_CTRL        0x0008U
#define LAN96455_REG_INT_STATUS         0x0010U
#define LAN96455_REG_INT_MASK           0x0014U

/* Port Configuration Registers (Offset per port: 0x0100 * port_idx) */
#define LAN96455_PORT_BASE(p)           (0x1000U + ((uint32_t)(p) * 0x0100U))
#define LAN96455_REG_PORT_CTRL(p)       (LAN96455_PORT_BASE(p) + 0x00U)
#define LAN96455_REG_PORT_STATUS(p)     (LAN96455_PORT_BASE(p) + 0x04U)
#define LAN96455_REG_PORT_FWD_MASK(p)   (LAN96455_PORT_BASE(p) + 0x08U)
#define LAN96455_REG_PORT_VLAN(p)       (LAN96455_PORT_BASE(p) + 0x0CU)

/* MAC Address Table / Forwarding Control */
#define LAN96455_REG_MAC_TABLE_CTRL     0x0200U
#define LAN96455_REG_MAC_FLUSH_CTRL     0x0204U

/* Bitmask Definitions */
#define LAN96455_SW_RESET_CMD           (1U << 0)
#define LAN96455_PORT_ENABLE            (1U << 0)
#define LAN96455_PORT_LINK_UP           (1U << 1)
#define LAN96455_MAC_FLUSH_ALL          (1U << 0)
#define LAN96455_MAC_FLUSH_DYNAMIC      (1U << 1)

#ifdef __cplusplus
}
#endif

#endif /* LAN96455_REGS_H */
