/* USER CODE BEGIN */
/**
 * @file lan96455_driver.c
 * @brief LAN96455F switch bring-up and redundant fiber path selection.
 *
 * Programs the copper, SerDes, and RGMII interfaces over SPI4 and sets the
 * forwarding masks that connect the copper and management ports to one SFP.
 * @note SPI transactions, polling, and startup delays block the caller.
 */

#include "lan96455_driver.h"
#include "lan96455_regs.h"
#include "stm32n6xx_hal.h"

extern SPI_HandleTypeDef hspi4;
extern ETH_HandleTypeDef heth1;

static const uint8_t active_ports[] = {
    PORT_PC, PORT_PC_SECONDARY, PORT_SFP_PRIMARY, PORT_SFP_BACKUP, PORT_STM32_RGMII
};
#define ACTIVE_PORT_COUNT (sizeof(active_ports) / sizeof(active_ports[0]))

#define LAN96455_SPI_HEADER_BYTES       3U
#define LAN96455_SPI_WRITE_FLAG         0x80U
#define LAN96455_TIMEOUT_MS             100U

/* =========================================================================
 * Private Function Prototypes
 * ========================================================================= */
static int lan_rmw(uint32_t reg, uint32_t mask, uint32_t value);

static int lan96455_core_reset(void);
static int lan96455_mac_table_init(void);
static int lan96455_vlan_baseline_init(void);
static int lan96455_pgid_init(void);
static int lan96455_setup_port_queues(void);
static int lan96455_general_init(void);
static int lan96455_init_unicast_flood_groups(void);
static int lan96455_init_physical_ports(void);
static int lan96455_configure_port(unsigned port);

/** @brief Read one little-endian register using a byte address and zero padding. */
static int lan96455_read_reg(uint32_t reg_addr, uint32_t *value)
{
    if (value == NULL)
    {
        return -1;
    }

    const uint32_t word_addr = (reg_addr & 0x01FFFFFFU) >> 2;
    uint8_t tx[LAN96455_SPI_HEADER_BYTES + sizeof(*value)] = {
        (uint8_t)(word_addr >> 16), (uint8_t)(word_addr >> 8), (uint8_t)word_addr
    };
    uint8_t rx[sizeof(tx)];

    if (HAL_SPI_TransmitReceive(&hspi4, tx, rx, sizeof(tx), LAN96455_TIMEOUT_MS) != HAL_OK)
    {
        return -1;
    }


    *value = (uint32_t)rx[LAN96455_SPI_HEADER_BYTES] |
             ((uint32_t)rx[LAN96455_SPI_HEADER_BYTES + 1U] << 8) |
             ((uint32_t)rx[LAN96455_SPI_HEADER_BYTES + 2U] << 16) |
             ((uint32_t)rx[LAN96455_SPI_HEADER_BYTES + 3U] << 24);
    return 0;
}

/** @brief Send the word address and little-endian register value in one SPI transfer. */
static int lan96455_write_reg(uint32_t reg_addr, uint32_t value)
{
    const uint32_t word_addr = (reg_addr & 0x01FFFFFFU) >> 2;
    uint8_t tx[] = {
        (uint8_t)((word_addr >> 16) | LAN96455_SPI_WRITE_FLAG),
        (uint8_t)(word_addr >> 8),
        (uint8_t)word_addr,
        /* Data bytes are little-endian. */
        (uint8_t)value,
        (uint8_t)(value >> 8),
        (uint8_t)(value >> 16),
        (uint8_t)(value >> 24)
    };

    return HAL_SPI_Transmit(&hspi4, tx, sizeof(tx), LAN96455_TIMEOUT_MS) == HAL_OK ? 0 : -1;
}

static int lan_rmw(uint32_t reg, uint32_t mask, uint32_t value)
{
    uint32_t current;
    if (lan96455_read_reg(reg, &current) != 0)
    {
        return -1;
    }
    return lan96455_write_reg(reg, (current & ~mask) | (value & mask));
}

int lan96455_verify_spi_comm(void)
{
    uint32_t val = 0;

    if (lan96455_write_reg(DEVCPU_GCB_IF_CTRL, DEVCPU_GCB_IF_CTRL_LITTLE_ENDIAN) != 0)
    {
        return -1;
    }

    if (lan96455_write_reg(DEVCPU_GCB_IF_CFGSTAT, DEVCPU_GCB_IF_CFGSTAT_DUMMY_BYTES) != 0)
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


static int lan96455_core_reset(void)
{
    uint32_t val = 0;
    uint32_t start_tick = 0;

    if (lan96455_read_reg(SYS_RESET_CFG, &val) != 0) return -1;
    if (val & SYS_RESET_CFG_CORE_ENA) return 0;

    //disable switch core
    if (lan96455_write_reg(SYS_RESET_CFG, SYS_RESET_CFG_CORE_DIS) != 0)
    {
        return -1;
    }

    //start ram initialization
    if (lan96455_write_reg(SYS_RAM_INIT, SYS_RAM_INIT_START) != 0)
    {
        return -1;
    }

    //poll until ram is finished initializing
    start_tick = HAL_GetTick();
    do {
        if (lan96455_read_reg(SYS_RAM_INIT, &val) != 0)
        {
            return -1;
        }
        if ((HAL_GetTick() - start_tick) >= LAN96455_TIMEOUT_MS)
        {
            return -1;
        }
    } while ((val & SYS_RAM_INIT_BUSY) != 0);

    //enable switch core
    if (lan96455_write_reg(SYS_RESET_CFG, SYS_RESET_CFG_CORE_ENA) != 0)
    {
        return -1;
    }

    return 0;
}


static int lan96455_mac_table_init(void)
{
    //clear the mac table
    if (lan96455_write_reg(ANA_MACACCESS, ANA_MACACCESS_CMD_INIT) != 0) return -1;

    uint32_t val;
    const uint32_t start_tick = HAL_GetTick();
    do {
        if (lan96455_read_reg(ANA_MACACCESS, &val) != 0)
        {
            return -1;
        }
        if ((HAL_GetTick() - start_tick) >= LAN96455_TIMEOUT_MS)
        {
            return -1;
        }
    } while ((val & ANA_MACACCESS_CMD_MASK) != ANA_MACACCESS_CMD_IDLE);

    return 0;
}


/** @brief Direct frames for the local STM32 MAC to the RGMII port using a locked entry. */
static int lan96455_install_management_mac(const uint8_t *mac)
{
    //the stm32 lives on port 7, this function creates an entry in the MAC table that says
    //for incoming packets having the dest mac equal to the stm32, look up the this entry in another table (PGID table)
    //that entry will have a bitstring with bit 7 set, denoting port 7
    if (mac == NULL) return -1;
    uint32_t low = ((uint32_t)mac[2] << 24) | ((uint32_t)mac[3] << 16) |
                   ((uint32_t)mac[4] << 8) | mac[5];
    if (lan96455_write_reg(ANA_MACHDATA, ((uint32_t)mac[0] << 8) | mac[1]) != 0 ||
        lan96455_write_reg(ANA_MACLDATA, low) != 0 ||
        lan96455_write_reg(ANA_PGID_ENTRY(PORT_STM32_RGMII), PORT_MASK_STM32) != 0 ||
        lan96455_write_reg(ANA_MACACCESS, ANA_MAC_LEARN_LOCKED(PORT_STM32_RGMII)) != 0) return -1;

    uint32_t val;
    const uint32_t start_tick = HAL_GetTick();
    do {
        if (lan96455_read_reg(ANA_MACACCESS, &val) != 0)
        {
            return -1;
        }
        if ((HAL_GetTick() - start_tick) >= LAN96455_TIMEOUT_MS)
        {
            return -1;
        }
    } while ((val & ANA_MACACCESS_CMD_MASK) != ANA_MACACCESS_CMD_IDLE);

    return 0;
}


static int lan96455_vlan_baseline_init(void)
{
    uint32_t val = 0;
    uint32_t start_tick = 0;

    //clear the vlan table
    if (lan96455_write_reg(ANA_VLANACCESS, ANA_VLANACCESS_CMD_INIT) != 0)
    {
        return -1;
    }

    start_tick = HAL_GetTick();
    do {
        if (lan96455_read_reg(ANA_VLANACCESS, &val) != 0)
        {
            return -1;
        }
        if ((HAL_GetTick() - start_tick) >= LAN96455_TIMEOUT_MS)
        {
            return -1;
        }
    } while ((val & ANA_VLANACCESS_CMD_MASK) != 0);

    /* Disable port-based VLAN ingress membership checks on every port. */
    if (lan96455_write_reg(ANA_VLANMASK, ANA_VLANMASK_FILTER_NONE) != 0)
    {
        return -1;
    }

    /* 3. Disable 802.1Q tagging and rewriting on active egress ports */
    for (unsigned i = 0; i < ACTIVE_PORT_COUNT; ++i)
    {
        unsigned port = active_ports[i];
        if (lan96455_write_reg(ANA_VLAN_CFG_PORT(port), 0) != 0 ||
            lan96455_write_reg(REW_PORT_VLAN_CFG(port), 0) != 0 ||
            lan96455_write_reg(REW_TAG_CFG(port), REW_TAG_CFG_DISABLED) != 0) return -1;
    }
    return 0;
}


/** @brief Pulse the flush bits to clear the CPU extraction FIFOs. */
static int lan96455_flush_queues(void)
{
    /* 1. Assert queue flush */
    if (lan96455_write_reg(QS_XTR_FLUSH, QS_XTR_FLUSH_ALL) != 0)
    {
        return -1;
    }

    /* 2. Allow in-flight packet buffers to drain (1 millisecond) */
    HAL_Delay(1);

    /* 3. Return queues to normal operation */
    if (lan96455_write_reg(QS_XTR_FLUSH, QS_XTR_FLUSH_NORMAL) != 0)
    {
        return -1;
    }

    return 0;
}


static int lan96455_pgid_init(void)
{
    for (int i = 0; i < PGID_ENTRIES_COUNT; i++)
    {
        //disable VLAN for these ports
        if (lan96455_write_reg(ANA_PGID_CFG_ENTRY(i), ANA_PGID_CFG_OBEY_VLAN_DIS) != 0)
        {
            return -1;
        }
        //clear the source masks 
        if (i >= PGID_SRC_BASE_INDEX && lan96455_write_reg(ANA_PGID_ENTRY(i), 0) != 0)
        {
            return -1;
        }
    }

    return 0;
}


static int lan96455_setup_port_queues(void)
{
    //set reserved buffer size to 1700 bytes (26 cells of 64B) per queue 
    for (int i = 0; i <= QSYS_Q_RSRV_LAST; i++)
    {
        if (lan96455_write_reg(QSYS_RES_CFG_EGRESS(i), BUFFER_RESERVE_1700B) != 0)
        {
            return -1;
        }
        if (lan96455_write_reg(QSYS_RES_CFG_INGRESS(i), BUFFER_RESERVE_1700B) != 0)
        {
            return -1;
        }
    }

    //saving space for backed up frames in queues
    for (unsigned i = 0; i < ACTIVE_PORT_COUNT; ++i)
        if (lan96455_write_reg(SYS_ATOP_PORT(active_ports[i]), (1U << 8) | 16U) != 0) return -1;
    if (lan96455_write_reg(SYS_ATOP_TOT_CFG, (1U << 8) | 160U) != 0) return -1;
    return 0;
}


static int lan96455_general_init(void)
{
    //if a frame stays in queue for longer than 2 seconds than discard it
    if (lan96455_write_reg(SYS_FRM_AGING, SYS_FRM_AGING_2SEC) != 0)
    {
        return -1;
    }
    return 0;
}


/** @brief Assign flood groups and limit their destinations to copper and fiber ports. */
static int lan96455_init_unicast_flood_groups(void)
{
    const uint32_t users = PORT_MASK_COPPER | PORT_MASK_SFP_PRIMARY | PORT_MASK_SFP_BACKUP;
    if (lan96455_write_reg(ANA_FLOODING_IPMC, ANA_IPMC_GROUPS) != 0) return -1;
    for (unsigned priority = 0; priority < 8; ++priority)
        if (lan96455_write_reg(ANA_FLOODING(priority), ANA_FLOOD_GROUPS) != 0) return -1;
    if (lan96455_write_reg(ANA_PGID_MC, users) != 0 ||
        lan96455_write_reg(ANA_PGID_MCIPV4, users) != 0 ||
        lan96455_write_reg(ANA_PGID_MCIPV6, users) != 0 ||
        lan96455_write_reg(ANA_PGID_BC, users) != 0 ||
        lan96455_write_reg(ANA_PGID_UC, users) != 0) return -1;

    return 0;
}


/** @brief Enable ingress on each wired port with dynamic learning disabled. */
static int lan96455_init_physical_ports(void)
{
    for (unsigned p = 0; p < ACTIVE_PORT_COUNT; ++p)
    {
        unsigned port = active_ports[p];
        if (lan96455_configure_port(port) != 0) return -1;

        /* 3. Drop frames with invalid multicast source MAC */
        if (lan96455_write_reg(ANA_DROP_CFG_PORT(port), ANA_DROP_CFG_DROP_MC_SMAC) != 0)
        {
            return -1;
        }

        /* 4. Enable frame reception, assign port ID, and DISABLE dynamic learning */
        if (lan96455_write_reg(ANA_PORT_CFG_PORT(port), ANA_PORT_CFG_VAL(port)) != 0)
        {
            return -1;
        }

    }

    return 0;
}


int lan96455_set_port_connections(lan96455_path_mode_t mode)
{
    if (mode != LAN96455_PATH_PRIMARY && mode != LAN96455_PATH_FALLBACK) return -1;
    const unsigned active = mode == LAN96455_PATH_PRIMARY ? PORT_SFP_PRIMARY : PORT_SFP_BACKUP;
    const unsigned standby = mode == LAN96455_PATH_PRIMARY ? PORT_SFP_BACKUP : PORT_SFP_PRIMARY;
    if (lan96455_write_reg(ANA_PGID_SRC(standby), 0) != 0 ||
        lan96455_write_reg(ANA_PGID_SRC(PORT_PC), 1U << active) != 0 ||
        lan96455_write_reg(ANA_PGID_SRC(PORT_PC_SECONDARY), 1U << active) != 0 ||
        lan96455_write_reg(ANA_PGID_SRC(PORT_STM32_RGMII), 1U << active) != 0 ||
        lan96455_write_reg(ANA_PGID_SRC(active), PORT_MASK_COPPER | PORT_MASK_STM32) != 0) return -1;
    return 0;
}

/* =========================================================================
 * Complete Single Initialization Sequence
 * ========================================================================= */

int lan96455_init(void)
{
    if (lan96455_verify_spi_comm() != 0)
    {
        return -1;
    }

    if (lan96455_core_reset() != 0)
    {
        return -1;
    }

    /* Also isolate traffic during a foreground reinitialization. */
    for (unsigned i = 0; i < ACTIVE_PORT_COUNT; ++i)
        if (lan_rmw(QSYS_SW_PORT_MODE_PORT(active_ports[i]), QSYS_SW_PORT_MODE_PORT_ENA, 0) != 0)
            return -1;

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

    if (lan96455_install_management_mac(heth1.Init.MACAddr) != 0) return -1;

    if (lan96455_set_port_connections(LAN96455_PATH_PRIMARY) != 0)
    {
        return -1;
    }

    for (unsigned i = 0; i < ACTIVE_PORT_COUNT; ++i)
    {
        if (lan_rmw(QSYS_SW_PORT_MODE_PORT(active_ports[i]),
                    QSYS_SW_PORT_MODE_PORT_ENA | QSYS_SW_PORT_MODE_SCH_NEXT_MASK |
                    QSYS_SW_PORT_MODE_INGRESS_DROP | QSYS_SW_PORT_MODE_TX_PFC_MASK,
                    QSYS_SW_PORT_MODE_PORT_ENA | QSYS_SW_PORT_MODE_SCH_NEXT_1 |
                    QSYS_SW_PORT_MODE_INGRESS_DROP) != 0) return -1;
    }
    return 0;
}

/** @brief Start one SerDes lane and check its PLL and transmitter status. */
static int lan96455_start_serdes(unsigned lane)
{
    const uint32_t sd = HSIO_SD_CFG(lane);
    const uint32_t pll = HSIO_MPLL_CFG(lane);
    const uint32_t resets = HSIO_SD_CFG_RX_RESET | HSIO_SD_CFG_TX_RESET;
    uint32_t strap, status;
    if (lan96455_read_reg(CHIP_TOP_STRAPPING, &strap) != 0) return -1;
    const unsigned reference = strap & 3U;
    const uint32_t pll_config = (reference == 1U || reference == 2U)
        ? HSIO_MPLL_CFG_REF_CLKDIV2 | 40U : 100U;
    if (lan_rmw(sd, resets, resets) != 0) return -1;
    HAL_Delay(1);
    if (lan_rmw(sd, resets | HSIO_SD_CFG_LANE_10BIT_SEL | HSIO_SD_CFG_RX_RATE |
                HSIO_SD_CFG_TX_RATE | HSIO_SD_CFG_RX_INVERT | HSIO_SD_CFG_TX_INVERT |
                HSIO_SD_CFG_LANE_LOOPBK_EN, HSIO_SD_CFG_LANE_10BIT_SEL |
                (2U << HSIO_SD_CFG_RX_RATE_POS) | (2U << HSIO_SD_CFG_TX_RATE_POS)) != 0 ||
        lan_rmw(pll, HSIO_MPLL_CFG_MPLL_MULTIPLIER | HSIO_MPLL_CFG_REF_CLKDIV2, pll_config) != 0 ||
        lan_rmw(sd, HSIO_SD_CFG_RX_TERM_EN, 0) != 0 ||
        lan_rmw(pll, HSIO_MPLL_CFG_REF_SSP_EN, HSIO_MPLL_CFG_REF_SSP_EN) != 0) return -1;
    HAL_Delay(1);
    if (lan_rmw(sd, HSIO_SD_CFG_PHY_RESET, 0) != 0) return -1;
    HAL_Delay(1);
    if (lan_rmw(pll, HSIO_MPLL_CFG_MPLL_EN, HSIO_MPLL_CFG_MPLL_EN) != 0) return -1;
    HAL_Delay(7);
    if (lan96455_read_reg(HSIO_SD_STAT(lane), &status) != 0 ||
        !(status & HSIO_SD_STAT_MPLL_STATE)) return -1;
    if (lan_rmw(sd, HSIO_SD_CFG_TX_CM_EN, HSIO_SD_CFG_TX_CM_EN) != 0) return -1;
    HAL_Delay(1);
    if (lan96455_read_reg(HSIO_SD_STAT(lane), &status) != 0 ||
        !(status & HSIO_SD_STAT_TX_CM_STATE)) return -1;
    const uint32_t enable = HSIO_SD_CFG_RX_PLL_EN | HSIO_SD_CFG_TX_EN;
    if (lan_rmw(sd, enable, enable) != 0) return -1;
    HAL_Delay(1);
    const uint32_t ready = HSIO_SD_STAT_RX_PLL_STATE | HSIO_SD_STAT_TX_STATE;
    if (lan96455_read_reg(HSIO_SD_STAT(lane), &status) != 0 ||
        (status & ready) != ready) return -1;
    const uint32_t data_enable = HSIO_SD_CFG_TX_DATA_EN | HSIO_SD_CFG_RX_DATA_EN;
    return lan_rmw(sd, data_enable, data_enable);
}

/** @brief Configure the physical interface and gigabit MAC for one wired port. */
static int lan96455_configure_port(unsigned port)
{

    uint32_t clock_base;


    if (lan96455_read_reg(DEV_CLOCK_CFG_PORT(port), &clock_base) != 0)
        return -1;
    clock_base = (clock_base & ~(DEV_CLOCK_CFG_RESET_MASK |
                                 DEV_CLOCK_CFG_LINK_SPEED_MASK)) |
                 DEV_CLOCK_CFG_LINK_SPEED_1000;

    /* Both CLOCK_CFG writes retain exactly the same non-reset fields. */
    if (lan96455_write_reg(DEV_CLOCK_CFG_PORT(port), clock_base | DEV_CLOCK_CFG_PORT_MAC_RESET) != 0)
        return -1;

    if (port == PORT_STM32_RGMII)
    {
        if (lan_rmw(HSIO_HW_CFG, HSIO_HW_CFG_RGMII_0_CFG | HSIO_HW_CFG_GMII_ENA_PORT(port),
                    HSIO_HW_CFG_GMII_ENA_PORT(port)) != 0 ||
            lan_rmw(QSYS_MISC_CFG, QSYS_MISC_CFG_DEV4_RGMII_ENA, 0) != 0) return -1;
        /* Linux's no-MAC-delay DLL settings: select clock, release DLL reset,
         * disable tuning/delayed output. RGMII 0 uses DLL 0 (RX) and DLL 1 (TX). */
        for (uint32_t clock = 0; clock < 2U; clock++)
        {
            if (lan_rmw(HSIO_DLL_CFG_CLOCK(clock),
                        HSIO_DLL_CFG_DLL_CLK_ENA | HSIO_DLL_CFG_DLL_RST |
                        HSIO_DLL_CFG_DLL_ENA | HSIO_DLL_CFG_DELAY_ENA,
                        HSIO_DLL_CFG_DLL_CLK_ENA) != 0)
                return -1;
        }

        if (lan_rmw(HSIO_RGMII_CFG_INTERFACE(0),
                    HSIO_RGMII_CFG_TX_CLK_MASK | HSIO_RGMII_CFG_RX_RST |
                    HSIO_RGMII_CFG_TX_RST, HSIO_RGMII_CFG_TX_CLK_1000) != 0)
            return -1;

    }
    else if (port == PORT_PC || port == PORT_PC_SECONDARY)
    {
        if (lan_rmw(HSIO_HW_CFG, HSIO_HW_CFG_GMII_ENA_PORT(port), HSIO_HW_CFG_GMII_ENA_PORT(port)) != 0 ||
            lan_rmw(CHIP_TOP_CUPHY_PORT_CFG(port), 1U, 1U) != 0) return -1;
    }
    else
    {
        /* Matching fixed 1000BASE-X endpoints, Clause 37 AN disabled. */
        if (lan_rmw(HSIO_HW_CFG, HSIO_HW_CFG_QSGMII_ENA, 0) != 0 ||
            lan96455_start_serdes(port - PORT_SFP_PRIMARY) != 0 ||
            lan_rmw(DEV_PCS1G_MODE_CFG(port), 3U, 0) != 0 ||
            lan96455_write_reg(DEV_PCS1G_ANEG_CFG(port), 0) != 0) return -1;
    }
    if (lan_rmw(DEV_PCS1G_CFG(port), 1U, 1U) != 0 ||
        lan_rmw(DEV_PCS1G_SD_CFG(port), 1U, 0) != 0) return -1;

    if (lan_rmw(DEV_MAC_MODE_CFG_PORT(port),
                DEV_MAC_MODE_CFG_GIGA_MODE_ENA | DEV_MAC_MODE_CFG_FDX_ENA,
                DEV_MAC_MODE_CFG_GIGA_MODE_ENA | DEV_MAC_MODE_CFG_FDX_ENA) != 0)
        return -1;
    if (lan_rmw(DEV_MAC_IFG_CFG_PORT(port),
                DEV_MAC_IFG_CFG_TX_IFG_MASK | DEV_MAC_IFG_CFG_RX_IFG1_MASK |
                DEV_MAC_IFG_CFG_RX_IFG2_MASK, DEV_MAC_IFG_CFG_1000) != 0)
        return -1;
    if (lan_rmw(DEV_MAC_TAGS_CFG(port), DEV_MAC_TAGS_CFG_TAG_ID_MASK | DEV_MAC_TAGS_CFG_LENGTH_FLAGS,
                (0x88A8U << DEV_MAC_TAGS_CFG_TAG_ID_POS) | DEV_MAC_TAGS_CFG_LENGTH_FLAGS) != 0)
        return -1;
    if (lan_rmw(DEV_MAC_MAXLEN_CFG_PORT(port), DEV_MAC_MAXLEN_CFG_MASK,
                MAC_MAXLEN_1518) != 0)
        return -1;
    if (lan_rmw(DEV_MAC_ENA_CFG_PORT(port),
                DEV_MAC_ENA_CFG_RX_ENA | DEV_MAC_ENA_CFG_TX_ENA,
                DEV_MAC_ENA_CFG_RX_ENA | DEV_MAC_ENA_CFG_TX_ENA) != 0)
        return -1;

    /* Release port reset before releasing MAC reset. */
    if (lan_rmw(DEV_CLOCK_CFG_PORT(port), DEV_CLOCK_CFG_PORT_RST, 0) != 0) return -1;
    return lan96455_write_reg(DEV_CLOCK_CFG_PORT(port), clock_base);
}
/* USER CODE END */
