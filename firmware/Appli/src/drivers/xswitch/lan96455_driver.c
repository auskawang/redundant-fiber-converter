/* USER CODE BEGIN */
/**
 * @file lan96455_driver.c
 * @brief LAN96455 Ethernet switch driver implementation.
 */

#include "lan96455_driver.h"
#include "lan96455_regs.h"
#include "bxp_spi.h"
#include "diagnostics.h"
#include "stm32n6xx_hal.h"

#define LAN96455_TIMEOUT_MS             100U

/* =========================================================================
 * Private Function Prototypes
 * ========================================================================= */
static inline int lan_wr(uint32_t reg, uint32_t val);
static inline uint32_t lan_rd(uint32_t reg);

static int lan96455_core_reset(void);
static int lan96455_mac_table_init(void);
static int lan96455_vlan_baseline_init(void);
static int lan96455_pgid_init(void);
static int lan96455_setup_port_queues(void);
static int lan96455_general_init(void);
static int lan96455_init_unicast_flood_groups(void);
static int lan96455_init_physical_ports(void);

int lan96455_read_reg(uint32_t reg_addr, uint32_t *value)
{
    if (value == NULL)
    {
        return -1;
    }
    return bxp_spi_read_reg(reg_addr, value);
}

int lan96455_write_reg(uint32_t reg_addr, uint32_t value)
{
    return bxp_spi_write_reg(reg_addr, value);
}

static inline int lan_wr(uint32_t reg, uint32_t val)
{
    return lan96455_write_reg(reg, val);
}

static inline uint32_t lan_rd(uint32_t reg)
{
    uint32_t val = 0;
    lan96455_read_reg(reg, &val);
    return val;
}

int lan96455_verify_spi_comm(void)
{
    uint32_t val = 0;

    if (lan_wr(DEVCPU_GCB_IF_CFGSTAT, DEVCPU_GCB_IF_CFGSTAT_DUMMY_BYTES) != 0)
    {
        return -1;
    }

    HAL_Delay(5);
    if (lan96455_read_reg(DEVCPU_GCB_IF_CFGSTAT, &val) != 0)
    {
        return -1;
    }

    if ((val & 0xFU) != (DEVCPU_GCB_IF_CFGSTAT_DUMMY_BYTES & 0xFU))
    {
        return -2;
    }

    return 0;
}

/* =========================================================================
 * Block 1: Switch Core Reset
 * ========================================================================= */

static int lan96455_core_reset(void)
{
    uint32_t val = 0;
    uint32_t start_tick = 0;

    /* 1. Assert Core Reset */
    if (lan_wr(SYS_RESET_CFG, SYS_RESET_CFG_CORE_DIS) != 0)
    {
        return -1;
    }

    /* 2. Start hardware BIST RAM initialization */
    if (lan_wr(SYS_RAM_INIT, SYS_RAM_INIT_START) != 0)
    {
        return -1;
    }

    /* 3. Poll until RAM initialization completes (RAM_INIT bit clears to 0) */
    start_tick = HAL_GetTick();
    do {
        val = lan_rd(SYS_RAM_INIT);
        if ((HAL_GetTick() - start_tick) >= LAN96455_TIMEOUT_MS)
        {
            return -1;
        }
    } while ((val & SYS_RAM_INIT_BUSY) != 0);

    /* 4. Release Core Reset */
    if (lan_wr(SYS_RESET_CFG, SYS_RESET_CFG_CORE_ENA) != 0)
    {
        return -1;
    }

    return 0;
}

/* =========================================================================
 * Block 2: MAC Address Table Initialization
 * ========================================================================= */

static int lan96455_mac_table_init(void)
{
    uint32_t val = 0;
    uint32_t start_tick = 0;

    /* Issue CMD_INIT (5) to clear the entire 8192-entry MAC table */
    if (lan_wr(ANA_MACACCESS, ANA_MACACCESS_CMD_INIT) != 0)
    {
        return -1;
    }

    /* Poll until the command completes and returns to CMD_IDLE (0) */
    start_tick = HAL_GetTick();
    do {
        val = lan_rd(ANA_MACACCESS);
        if ((HAL_GetTick() - start_tick) >= LAN96455_TIMEOUT_MS)
        {
            return -1;
        }
    } while ((val & ANA_MACACCESS_CMD_MASK) != ANA_MACACCESS_CMD_IDLE);

    return 0;
}

/* =========================================================================
 * Block 3: Baseline VLAN (Transparent Mode)
 * ========================================================================= */

static int lan96455_vlan_baseline_init(void)
{
    uint32_t val = 0;
    uint32_t start_tick = 0;

    /* 1. Clear the hardware VLAN table */
    if (lan_wr(ANA_VLANACCESS, ANA_VLANACCESS_CMD_INIT) != 0)
    {
        return -1;
    }

    start_tick = HAL_GetTick();
    do {
        val = lan_rd(ANA_VLANACCESS);
        if ((HAL_GetTick() - start_tick) >= LAN96455_TIMEOUT_MS)
        {
            return -1;
        }
    } while ((val & ANA_VLANACCESS_CMD_MASK) != 0);

    /* 2. Set VLAN ingress filter mask to all ports to prevent ingress drops */
    if (lan_wr(ANA_VLANMASK, ALL_PORTS_MASK) != 0)
    {
        return -1;
    }

    /* 3. Disable 802.1Q tagging and rewriting on active egress ports */
    if (lan_wr(REW_PORT_VLAN_CFG_PORT0, 0) != 0) return -1;
    if (lan_wr(REW_TAG_CFG_PORT0, REW_TAG_CFG_DISABLED) != 0) return -1;

    if (lan_wr(REW_PORT_VLAN_CFG_PORT4, 0) != 0) return -1;
    if (lan_wr(REW_TAG_CFG_PORT4, REW_TAG_CFG_DISABLED) != 0) return -1;

    if (lan_wr(REW_PORT_VLAN_CFG_PORT5, 0) != 0) return -1;
    if (lan_wr(REW_TAG_CFG_PORT5, REW_TAG_CFG_DISABLED) != 0) return -1;

    if (lan_wr(REW_PORT_VLAN_CFG_PORT6, 0) != 0) return -1;
    if (lan_wr(REW_TAG_CFG_PORT6, REW_TAG_CFG_DISABLED) != 0) return -1;

    return 0;
}

/* =========================================================================
 * Block 4: Flushing Queues (Public Module)
 * ========================================================================= */

int lan96455_flush_queues(void)
{
    /* 1. Assert queue flush */
    if (lan_wr(QS_XTR_FLUSH, QS_XTR_FLUSH_ALL) != 0)
    {
        return -1;
    }

    /* 2. Allow in-flight packet buffers to drain (1 millisecond) */
    HAL_Delay(1);

    /* 3. Return queues to normal operation */
    if (lan_wr(QS_XTR_FLUSH, QS_XTR_FLUSH_NORMAL) != 0)
    {
        return -1;
    }

    return 0;
}

/* =========================================================================
 * Block 5: Setting the PGID (Table Initialization)
 * ========================================================================= */

static int lan96455_pgid_init(void)
{
    /* Configure all PGID entries to ignore VLAN boundaries and clear forwarding */
    for (int i = 0; i < PGID_ENTRIES_COUNT; i++)
    {
        if (lan_wr(ANA_PGID_CFG_ENTRY(i), ANA_PGID_CFG_OBEY_VLAN_DIS) != 0)
        {
            return -1;
        }
        if (lan_wr(ANA_PGID_ENTRY(i), 0) != 0)
        {
            return -1;
        }
    }

    return 0;
}

/* =========================================================================
 * Block 6: Setting Up Queues for Each Port (Buffer Watermarks)
 * ========================================================================= */

static int lan96455_setup_port_queues(void)
{
    /* 1. Set reserved buffer size to 1700 bytes (26 cells of 64B) per queue */
    for (int i = 0; i <= QSYS_Q_RSRV_COUNT; i++)
    {
        if (lan_wr(QSYS_RES_CFG_EGRESS(i), BUFFER_RESERVE_1700B) != 0)
        {
            return -1;
        }
        if (lan_wr(QSYS_RES_CFG_INGRESS(i), BUFFER_RESERVE_1700B) != 0)
        {
            return -1;
        }
    }

    /* 2. Configure tail-drop watermarks per active port */
    if (lan_wr(SYS_ATOP_PORT0, TAIL_DROP_PER_PORT) != 0) return -1;
    if (lan_wr(SYS_ATOP_PORT4, TAIL_DROP_PER_PORT) != 0) return -1;
    if (lan_wr(SYS_ATOP_PORT5, TAIL_DROP_PER_PORT) != 0) return -1;
    if (lan_wr(SYS_ATOP_PORT6, TAIL_DROP_PER_PORT) != 0) return -1;

    /* 3. Set total shared memory tail-drop threshold */
    if (lan_wr(SYS_ATOP_TOT_CFG, TAIL_DROP_TOTAL_BUF) != 0)
    {
        return -1;
    }

    return 0;
}

/* =========================================================================
 * Block 7: General Initialization (Interface & Aging Setup)
 * ========================================================================= */

static int lan96455_general_init(void)
{
    /* 1. Set Little-Endian mode for ARM Cortex-M55 */
    if (lan_wr(DEVCPU_GCB_IF_CTRL, DEVCPU_GCB_IF_CTRL_LITTLE_ENDIAN) != 0)
    {
        return -1;
    }

    /* 2. Set read dummy padding bytes for SPI clock frequency */
    if (lan_wr(DEVCPU_GCB_IF_CFGSTAT, DEVCPU_GCB_IF_CFGSTAT_DUMMY_BYTES) != 0)
    {
        return -1;
    }

    /* 3. Set frame aging to 2 seconds (prevents buffer lockup if a fiber is pulled) */
    if (lan_wr(SYS_FRM_AGING, SYS_FRM_AGING_2SEC) != 0)
    {
        return -1;
    }
    if (lan_wr(ANA_AUTOAGE, ANA_AUTOAGE_PERIOD_1SEC) != 0)
    {
        return -1;
    }

    /* 4. Disable dynamic learning for dropped frames */
    if (lan_wr(ANA_ADVLEARN, ANA_ADVLEARN_VLAN_CHK_DIS) != 0)
    {
        return -1;
    }

    return 0;
}

/* =========================================================================
 * Block 8: Initializing Unicast Global Forwarding Group
 * ========================================================================= */

static int lan96455_init_unicast_flood_groups(void)
{
    /* Multicast groups default to 0 (no flood) */
    if (lan_wr(ANA_PGID_MC, 0) != 0) return -1;
    if (lan_wr(ANA_PGID_MCIPV4, 0) != 0) return -1;
    if (lan_wr(ANA_PGID_MCIPV6, 0) != 0) return -1;

    /* Direct unknown unicast to front ports candidate pool */
    if (lan_wr(ANA_PGID_UC, ALL_PHYS_PORTS_MASK) != 0) return -1;

    /* Direct broadcast to front ports candidate pool */
    if (lan_wr(ANA_PGID_BC, ALL_PHYS_PORTS_MASK) != 0) return -1;

    return 0;
}

/* =========================================================================
 * Block 9: Physical Port Initialization
 * ========================================================================= */

static int lan96455_init_physical_ports(void)
{
    const int active_ports[] = {PORT_PC, PORT_STM32_RGMII, PORT_SFP_PRIMARY, PORT_SFP_BACKUP};

    for (int p = 0; p < 4; p++)
    {
        int port = active_ports[p];

        /* 1. Release PCS Tx and Rx resets */
        if (lan_wr(DEV_CLOCK_CFG_PORT(port), DEV_CLOCK_CFG_PCS_RUN) != 0)
        {
            return -1;
        }

        /* 2. Set MTU frame length to standard 1518 bytes */
        if (lan_wr(DEV_MAC_MAXLEN_CFG_PORT(port), MAC_MAXLEN_1518) != 0)
        {
            return -1;
        }

        /* 3. Drop frames with invalid multicast source MAC */
        if (lan_wr(ANA_DROP_CFG_PORT(port), ANA_DROP_CFG_DROP_MC_SMAC) != 0)
        {
            return -1;
        }

        /* 4. Enable frame reception, assign port ID, and DISABLE dynamic learning */
        if (lan_wr(ANA_PORT_CFG_PORT(port), ANA_PORT_CFG_VAL(port)) != 0)
        {
            return -1;
        }

        /* 5. Port 4 Specific: Enable RGMII interface connected to STM32 MAC */
        if (port == PORT_STM32_RGMII)
        {
            if (lan_wr(HSIO_HW_CFG, HSIO_HW_CFG_RGMII_PORT4_ENA) != 0)
            {
                return -1;
            }
            if (lan_wr(QSYS_MISC_CFG, QSYS_MISC_CFG_DEV4_RGMII_ENA) != 0)
            {
                return -1;
            }
        }
    }

    return 0;
}

/* =========================================================================
 * Block 10: Setting Port Connections (Public Module)
 * ========================================================================= */

int lan96455_set_port_connections(lan96455_path_mode_t mode)
{
    if (mode == LAN96455_PATH_PRIMARY)
    {
        /* 1. Initial State: Primary Optical Path (Port 5) Active */
        // PC Ingress -> Only allowed out Primary SFP (Port 5)
        if (lan_wr(ANA_PGID_SRC_PORT0, PORT_MASK_SFP_PRIMARY) != 0) return -1;

        // STM32 Ingress -> Only allowed out Primary SFP (Port 5)
        if (lan_wr(ANA_PGID_SRC_PORT4, PORT_MASK_SFP_PRIMARY) != 0) return -1;

        // Primary SFP Ingress -> Allowed to PC and STM32
        if (lan_wr(ANA_PGID_SRC_PORT5, PORT_MASK_PC | PORT_MASK_STM32) != 0) return -1;

        // Backup SFP Ingress -> Blocked
        if (lan_wr(ANA_PGID_SRC_PORT6, 0) != 0) return -1;
    }
    else if (mode == LAN96455_PATH_FALLBACK)
    {
        /* 2. Failover State: Instantaneous Switchover to Backup Optical Path (Port 6) */
        // Switch PC to Backup SFP (Port 6)
        if (lan_wr(ANA_PGID_SRC_PORT0, PORT_MASK_SFP_BACKUP) != 0) return -1;

        // Switch STM32 to Backup SFP (Port 6)
        if (lan_wr(ANA_PGID_SRC_PORT4, PORT_MASK_SFP_BACKUP) != 0) return -1;

        // Enable Backup SFP Ingress -> Allowed to PC and STM32
        if (lan_wr(ANA_PGID_SRC_PORT6, PORT_MASK_PC | PORT_MASK_STM32) != 0) return -1;

        // Isolate Primary SFP (Port 5)
        if (lan_wr(ANA_PGID_SRC_PORT5, 0) != 0) return -1;
    }
    else
    {
        return -1;
    }

    return 0;
}

/* =========================================================================
 * Complete Single Initialization Sequence
 * ========================================================================= */

int lan96455_init(void)
{
    if (lan96455_core_reset() != 0)
    {
        return -1;
    }

    if (lan96455_mac_table_init() != 0)
    {
        return -1;
    }

    if (lan96455_vlan_baseline_init() != 0)
    {
        return -1;
    }

    if (lan96455_flush_queues() != 0)
    {
        return -1;
    }

    if (lan96455_pgid_init() != 0)
    {
        return -1;
    }

    if (lan96455_setup_port_queues() != 0)
    {
        return -1;
    }

    if (lan96455_general_init() != 0)
    {
        return -1;
    }

    if (lan96455_init_unicast_flood_groups() != 0)
    {
        return -1;
    }

    if (lan96455_init_physical_ports() != 0)
    {
        return -1;
    }

    if (lan96455_set_port_connections(LAN96455_PATH_PRIMARY) != 0)
    {
        return -1;
    }

    return 0;
}
/* USER CODE END */
