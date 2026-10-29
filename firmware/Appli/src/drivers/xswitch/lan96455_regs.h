/* USER CODE BEGIN */
/**
 * @file lan96455_regs.h
 * @brief LAN96455 Ethernet switch register address definitions and bitmasks.
 *
 * Organized by hardware component / subsystem.
 */

#ifndef _LAN96455_REGISTER_H_
#define _LAN96455_REGISTER_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* =========================================================================
 * 1. Target Base Addresses
 * ========================================================================= */
#define TARGET_DEVCPU_GCB_BASE              0x000000U
#define TARGET_DEV_BASE                     0x010000U
#define TARGET_QS_BASE                      0x020000U
#define TARGET_HSIO_BASE                    0x030000U
#define TARGET_SYS_BASE                     0x040000U
#define TARGET_ANA_BASE                     0x050000U
#define TARGET_QSYS_BASE                    0x060000U
#define TARGET_REW_BASE                     0x070000U

/* =========================================================================
 * 2. Port Definitions & Bitmasks
 * ========================================================================= */
#define PORT_PC                             0
#define PORT_STM32_RGMII                    4
#define PORT_SFP_PRIMARY                    5
#define PORT_SFP_BACKUP                     6

#define PORT_MASK_PC                        (1U << PORT_PC)
#define PORT_MASK_STM32                     (1U << PORT_STM32_RGMII)
#define PORT_MASK_SFP_PRIMARY               (1U << PORT_SFP_PRIMARY)
#define PORT_MASK_SFP_BACKUP                (1U << PORT_SFP_BACKUP)

#define ALL_PORTS_MASK                      0x1FFU  /* Ports 0 through 8 */
#define ALL_PHYS_PORTS_MASK                 0x1FFU

/* =========================================================================
 * 3. DEVCPU_GCB Component (General Configuration Block / Interface Control)
 * ========================================================================= */
#define DEVCPU_GCB_IF_CTRL                  (TARGET_DEVCPU_GCB_BASE + 0x0000U)
#define DEVCPU_GCB_IF_CFGSTAT               (TARGET_DEVCPU_GCB_BASE + 0x0004U)

#define DEVCPU_GCB_IF_CTRL_LITTLE_ENDIAN    0x00000000U
#define DEVCPU_GCB_IF_CFGSTAT_DUMMY_BYTES   0x00000001U
#define LAN9645X_SPI_DEFAULT_PADDING        15U         /**< Power-up / post-reset default dummy bytes */
#define LAN9645X_SPI_OPERATIONAL_PADDING    2U          /**< Operational dummy bytes at target SPI clock */

/* =========================================================================
 * 4. SYS Component (System, Core Reset, Memory BIST, Buffer Watermarks, Aging)
 * ========================================================================= */
#define SYS_RESET_CFG                       (TARGET_SYS_BASE + 0x0114U)
#define SYS_RAM_INIT                        (TARGET_SYS_BASE + 0x0274U)
#define SYS_FRM_AGING                       (TARGET_SYS_BASE + 0x0174U)
#define SYS_ATOP_BASE                       (TARGET_SYS_BASE + 0x0180U)
#define SYS_ATOP_PORT(p)                    (SYS_ATOP_BASE + ((uint32_t)(p) << 2))

#define SYS_ATOP_PORT0                      SYS_ATOP_PORT(0)
#define SYS_ATOP_PORT4                      SYS_ATOP_PORT(4)
#define SYS_ATOP_PORT5                      SYS_ATOP_PORT(5)
#define SYS_ATOP_PORT6                      SYS_ATOP_PORT(6)

#define SYS_ATOP_TOT_CFG                    (TARGET_SYS_BASE + 0x01ACU)

#define SYS_RESET_CFG_CORE_DIS              0x00000000U
#define SYS_RESET_CFG_CORE_ENA              0x00000001U
#define SYS_RAM_INIT_START                  0x00000001U
#define SYS_RAM_INIT_BUSY                   0x00000001U
#define SYS_FRM_AGING_2SEC                  0x00000002U

#define TAIL_DROP_PER_PORT                  (1700U / 64U)
#define TAIL_DROP_TOTAL_BUF                 (128U * 1024U / 64U)

/* =========================================================================
 * 5. ANA Component (Analyzer, MAC/VLAN Table, Port Drops, Aging, PGID Routing)
 * ========================================================================= */
#define ANA_MACACCESS                       (TARGET_ANA_BASE + 0x061CU)
#define ANA_MACACCESS_CMD_IDLE              0x00000000U
#define ANA_MACACCESS_CMD_INIT              0x00000005U
#define ANA_MACACCESS_CMD_MASK              0x0000000FU

#define ANA_VLANACCESS                      (TARGET_ANA_BASE + 0x0670U)
#define ANA_VLANMASK                        (TARGET_ANA_BASE + 0x02A8U)
#define ANA_VLANACCESS_CMD_INIT             0x00000000U
#define ANA_VLANACCESS_CMD_MASK             0x00000003U

#define ANA_AUTOAGE                         (TARGET_ANA_BASE + 0x0280U)
#define ANA_ADVLEARN                        (TARGET_ANA_BASE + 0x02A4U)
#define ANA_AUTOAGE_PERIOD_1SEC             0x00000001U
#define ANA_ADVLEARN_VLAN_CHK_DIS           0x00000001U

#define ANA_DROP_CFG_BASE                   (TARGET_ANA_BASE + 0x0260U)
#define ANA_DROP_CFG_PORT(p)                (ANA_DROP_CFG_BASE + ((uint32_t)(p) << 2))
#define ANA_DROP_CFG_DROP_MC_SMAC           0x00000001U

#define ANA_PORT_CFG_BASE                   (TARGET_ANA_BASE + 0x0700U)
#define ANA_PORT_CFG_PORT(p)                (ANA_PORT_CFG_BASE + ((uint32_t)(p) << 2))
#define ANA_PORT_CFG_VAL(port)              ((1U << 4) | (0U << 3) | (((uint32_t)(port)) & 0x7U))

#define ANA_PGID_BASE                       (TARGET_ANA_BASE + 0x6C00U)
#define PGID_ENTRIES_COUNT                  92U
#define ANA_PGID_ENTRY(i)                   (ANA_PGID_BASE + (((uint32_t)(i)) << 3))
#define ANA_PGID_CFG_ENTRY(i)               (ANA_PGID_BASE + (((uint32_t)(i)) << 3) + 4U)
#define ANA_PGID_CFG_OBEY_VLAN_DIS          0x00000000U

/* PGID Index Constants */
#define PGID_MC_INDEX                       64U
#define PGID_MCIPV4_INDEX                   65U
#define PGID_MCIPV6_INDEX                   66U
#define PGID_SRC_BASE_INDEX                 80U
#define PGID_UC_INDEX                       82U
#define PGID_BC_INDEX                       83U

/* Flooding PGID Registers */
#define ANA_PGID_MC                         (ANA_PGID_BASE + (PGID_MC_INDEX << 3))
#define ANA_PGID_MCIPV4                     (ANA_PGID_BASE + (PGID_MCIPV4_INDEX << 3))
#define ANA_PGID_MCIPV6                     (ANA_PGID_BASE + (PGID_MCIPV6_INDEX << 3))
#define ANA_PGID_UC                         (ANA_PGID_BASE + (PGID_UC_INDEX << 3))
#define ANA_PGID_BC                         (ANA_PGID_BASE + (PGID_BC_INDEX << 3))

/* Ingress Source Port Mask Registers (PGID_SRC: 80-88) */
#define ANA_PGID_SRC_PORT0                  (ANA_PGID_BASE + ((PGID_SRC_BASE_INDEX + 0U) << 3))
#define ANA_PGID_SRC_PORT4                  (ANA_PGID_BASE + ((PGID_SRC_BASE_INDEX + 4U) << 3))
#define ANA_PGID_SRC_PORT5                  (ANA_PGID_BASE + ((PGID_SRC_BASE_INDEX + 5U) << 3))
#define ANA_PGID_SRC_PORT6                  (ANA_PGID_BASE + ((PGID_SRC_BASE_INDEX + 6U) << 3))

/* =========================================================================
 * 6. REW Component (Rewriter & Tagging Configuration)
 * ========================================================================= */
#define REW_PORT_VLAN_CFG_BASE              (TARGET_REW_BASE + 0x0000U)
#define REW_TAG_CFG_BASE                    (TARGET_REW_BASE + 0x0004U)

#define REW_PORT_VLAN_CFG(p)                (REW_PORT_VLAN_CFG_BASE + (((uint32_t)(p)) << 4))
#define REW_TAG_CFG(p)                      (REW_TAG_CFG_BASE + (((uint32_t)(p)) << 4))

#define REW_PORT_VLAN_CFG_PORT0             REW_PORT_VLAN_CFG(0)
#define REW_PORT_VLAN_CFG_PORT4             REW_PORT_VLAN_CFG(4)
#define REW_PORT_VLAN_CFG_PORT5             REW_PORT_VLAN_CFG(5)
#define REW_PORT_VLAN_CFG_PORT6             REW_PORT_VLAN_CFG(6)

#define REW_TAG_CFG_PORT0                   REW_TAG_CFG(0)
#define REW_TAG_CFG_PORT4                   REW_TAG_CFG(4)
#define REW_TAG_CFG_PORT5                   REW_TAG_CFG(5)
#define REW_TAG_CFG_PORT6                   REW_TAG_CFG(6)

#define REW_TAG_CFG_DISABLED                0x00000000U

/* =========================================================================
 * 7. QS Component (Queue System / Buffer Flushing)
 * ========================================================================= */
#define QS_XTR_FLUSH                        (TARGET_QS_BASE + 0x003CU)
#define QS_XTR_FLUSH_ALL                    0x00000003U
#define QS_XTR_FLUSH_NORMAL                 0x00000000U

/* =========================================================================
 * 8. QSYS Component (Queue System Resource Configuration & Misc Setup)
 * ========================================================================= */
#define QSYS_RES_CFG_BASE                   (TARGET_QSYS_BASE + 0x0000U)
#define QSYS_Q_RSRV_COUNT                   215U

#define QSYS_RES_CFG_EGRESS(i)              (QSYS_RES_CFG_BASE + (((uint32_t)(i)) << 2))
#define QSYS_RES_CFG_INGRESS(i)             (QSYS_RES_CFG_BASE + (((512U + (uint32_t)(i))) << 2))
#define BUFFER_RESERVE_1700B                (1700U / 64U)

#define QSYS_MISC_CFG                       (TARGET_QSYS_BASE + 0x0118U)
#define QSYS_MISC_CFG_DEV4_RGMII_ENA        0x00000001U

/* =========================================================================
 * 9. DEV Component (Device / Port Clocks & Frame Length)
 * ========================================================================= */
#define DEV_CLOCK_CFG_BASE                  (TARGET_DEV_BASE + 0x0000U)
#define DEV_MAC_MAXLEN_CFG_BASE             (TARGET_DEV_BASE + 0x0020U)

#define DEV_CLOCK_CFG_PORT(p)               (DEV_CLOCK_CFG_BASE + (((uint32_t)(p)) << 6))
#define DEV_MAC_MAXLEN_CFG_PORT(p)          (DEV_MAC_MAXLEN_CFG_BASE + (((uint32_t)(p)) << 6))

#define DEV_CLOCK_CFG_PCS_RUN               0x00000000U
#define MAC_MAXLEN_1518                     0x000005EEU

/* =========================================================================
 * 10. HSIO Component (High Speed I/O Configuration)
 * ========================================================================= */
#define HSIO_HW_CFG                         (TARGET_HSIO_BASE + 0x0000U)
#define HSIO_HW_CFG_RGMII_PORT4_ENA         ((1U << 4) | (1U << 0))

#ifdef __cplusplus
}
#endif

#endif /* _LAN96455_REGISTER_H_ */
/* USER CODE END */
