/**
 * @file lan96455_regs.h
 * @brief LAN9646 / LAN9645x Ethernet switch register address definitions and bitmasks.
 *
 * Mapped according to the Linux MFD and DSA driver architecture (lan9645x-spi / lan9645x_main).
 */

#ifndef LAN96455_REGS_H
#define LAN96455_REGS_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ========================================================================== */
/* DEVCPU_GCB (General Configuration Block) Register Map                       */
/* ========================================================================== */
#define DEVCPU_GCB_BASE                     0x00004000U

/* Chip Identification and Reset */
#define DEVCPU_GCB_CHIP_ID                  (DEVCPU_GCB_BASE + 0x0000U)
#define DEVCPU_GCB_SOFT_RST                 (DEVCPU_GCB_BASE + 0x0010U)

/* DEVCPU_GCB:CHIP_REGS:SOFT_RST bitmasks */
#define SOFT_CHIP_RST                       (1U << 0)   /**< Full chip reset */
#define SOFT_SWC_RST                        (1U << 1)   /**< Soft switch core reset */

/* Serial Interface (SI / SPI) Control and Configuration */
#define DEVCPU_GCB_IF_CTRL                  (DEVCPU_GCB_BASE + 0x001CU)
#define DEVCPU_GCB_IF_CFGSTAT               (DEVCPU_GCB_BASE + 0x0020U)

/* IF_CTRL bit definitions */
#define IF_CTRL_BIG_ENDIAN                  (0U)        /**< Hardware default: Big-Endian byte order */
#define IF_CTRL_LITTLE_ENDIAN               (1U << 0)
#define IF_CTRL_MSB_FIRST                   (0U)        /**< Hardware default: MSB-first bit order */

/* IF_CFGSTAT bitmasks & constants */
#define IF_CFGSTAT_PADDING_MASK             0x0000000FU
#define IF_CFGSTAT_PADDING_SHIFT            0U
#define LAN9645X_SPI_DEFAULT_PADDING        15U         /**< Power-up / post-reset default dummy bytes */
#define LAN9645X_SPI_OPERATIONAL_PADDING    2U          /**< Operational dummy bytes at target SPI clock */

/* ========================================================================== */
/* SYS (System & Switch Core) Register Map                                   */
/* ========================================================================== */
#define SYS_BASE                            0x00010000U

#define SYS_RESET_CFG                       (SYS_BASE + 0x0000U)
#define SYS_RAM_INIT                        (SYS_BASE + 0x0004U)

/* SYS:RESET_CFG bitmasks */
#define SYS_RESET_CFG_CORE_ENA              (1U << 0)   /**< Switch packet engine core enable */

/* SYS:RAM_INIT bitmasks */
#define SYS_RAM_INIT_RAM_INIT               (1U << 0)   /**< Start internal SRAM initialization / BIST */

/* ========================================================================== */
/* QS (Queue System) Register Map                                             */
/* ========================================================================== */
#define QS_BASE                             0x00020000U
#define QS_XTR_FLUSH                        (QS_BASE + 0x0004U)
#define QS_XTR_FLUSH_DRAIN                  (1U << 0)

/* ========================================================================== */
/* Port Configuration Registers                                               */
/* ========================================================================== */
#define LAN96455_PORT_BASE(p)               (0x00030000U + ((uint32_t)(p) * 0x0100U))
#define LAN96455_REG_PORT_CTRL(p)           (LAN96455_PORT_BASE(p) + 0x00U)
#define LAN96455_REG_PORT_STATUS(p)         (LAN96455_PORT_BASE(p) + 0x04U)
#define LAN96455_REG_PORT_FWD_MASK(p)       (LAN96455_PORT_BASE(p) + 0x08U)
#define LAN96455_REG_PORT_VLAN(p)           (LAN96455_PORT_BASE(p) + 0x0CU)

#define LAN96455_PORT_ENABLE                (1U << 0)
#define LAN96455_PORT_LINK_UP               (1U << 1)

/* ========================================================================== */
/* Timing and Timeout Thresholds                                              */
/* ========================================================================== */
#define LAN9645X_RESET_HOLD_MS              10U
#define LAN9645X_STABILIZATION_DELAY_MS     15U
#define LAN9645X_GCB_RST_TIMEOUT_MS         100U
#define LAN9645X_RAM_INIT_TIMEOUT_MS        100U
#define LAN9645X_POLL_INTERVAL_MS           1U

#ifdef __cplusplus
}
#endif

#endif /* LAN96455_REGS_H */
