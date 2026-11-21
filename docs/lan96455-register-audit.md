# PHOTON LAN96455F register audit

The reference is the local Downloads/linux checkout at `6ffda6116249fa9e9e892206d5df337821345c76`, under `drivers/`. PHOTON sends byte addresses divided by four on SPI. All addresses below are byte addresses. For replicated registers the table gives instance zero. The compiled address checks covered both initialization and path selection for ports 0, 1, 5, 6 and 7.

## Port policy

Both copper jacks send user frames over the selected optical path. The inactive optical ingress is isolated. Flood destinations include both copper jacks and the optical ports. Source masks restrict which of those destinations can receive a given frame. Optical traffic for the local STM32 resolves through a locked VID 0 MAC entry to destination PGID 7. Both copper jacks can therefore receive a remote unknown unicast. This is a shared uplink with learning disabled. The two jacks do not bridge directly to each other.

## Register decisions

- **IF_CTRL and IF_CFGSTAT.** Required for the selected SPI format. IF_CTRL is ORG+0x1C and IF_CFGSTAT is ORG+0x20. PHOTON retains the requested zero padding. The BSP calculates positive padding using a conservative 2 microsecond latency. A zero readback alone cannot detect MISO stuck low. Validate reads on hardware, including both SerDes status registers. [SPI initialization](C:/Users/auska/Downloads/linux/drivers/mfd/lan9645x-spi.c:389)
- **SYS_RESET_CFG and SYS_RAM_INIT.** Disable the core, start initialization with bit 1, wait for completion and enable the core. PHOTON additionally skips RAM reinitialization when CORE_ENA is already set. [main.c](C:/Users/auska/Downloads/linux/drivers/net/dsa/microchip/lan9645x/lan9645x_main.c:441)
- **ANA_MACACCESS, ANA_MACHDATA and ANA_MACLDATA.** Clear the table and poll command bits 3?0. Pack the runtime STM32 MAC with VID 0, learn it as a locked entry, and poll for idle before enabling forwarding. The destination field holds the PGID index, not a port bitmap. [mac.c](C:/Users/auska/Downloads/linux/drivers/net/dsa/microchip/lan9645x/lan9645x_mac.c:272)
- **ANA_VLANACCESS, ANA_VLANMASK, ANA_VLAN_CFG, REW_PORT_VLAN_CFG and REW_TAG_CFG.** Issue command 3 and poll for idle. Use zero port ingress-filter mask, VID 0, no ingress pop, and no egress tag insertion for transparent forwarding. Linux enables membership checks after installing VLAN memberships; PHOTON deliberately bypasses those checks. [vlan.c](C:/Users/auska/Downloads/linux/drivers/net/dsa/microchip/lan9645x/lan9645x_vlan.c:399)
- **QS_XTR_FLUSH.** Retained extraction-group flush with mask 3 and a one-millisecond delay. This cleans the CPU extraction FIFOs and is optional for external RGMII management. [main.c](C:/Users/auska/Downloads/linux/drivers/net/dsa/microchip/lan9645x/lan9645x_main.c:603)
- **ANA_PGID and ANA_PGID_CFG.** Use 89 DSA entries, 8-byte stride. Preserve aggregation and ordinary destination masks. Disable OBEY_VLAN for transparent forwarding. During init clear source masks; during selection isolate standby first and set the masks for both copper ports, DEV7 and the active SFP. [main.h](C:/Users/auska/Downloads/linux/drivers/net/dsa/microchip/lan9645x/lan9645x_main.h:114)
- **ANA_FLOODING, ANA_FLOODING_IPMC and destination PGIDs.** Program the destination selectors for all eight priorities. Reserve UC/BC/MC/IPv4MC/IPv6MC slots 59?63. Set flood destinations to user ports and explicitly set PGID 7 for management. This supports multicast user traffic with learning disabled. [main.c](C:/Users/auska/Downloads/linux/drivers/net/dsa/microchip/lan9645x/lan9645x_main.c:673)
- **QSYS_RES_CFG.** Replicas use 8-byte stride. Program indices 0?95 and 512?607 with 26 cells, following the DSA reservation loop. [main.c](C:/Users/auska/Downloads/linux/drivers/net/dsa/microchip/lan9645x/lan9645x_main.c:703)
- **SYS_ATOP and SYS_ATOP_TOT_CFG.** Use the BSP 160 KiB shared pool. Linux divides by nine physical slots plus the internal CPU slot. The resulting 16 KiB and 160 KiB thresholds encode as 0x110 and 0x1A0. [main.c](C:/Users/auska/Downloads/linux/drivers/net/dsa/microchip/lan9645x/lan9645x_main.c:471)
  Watermark encoding: [main.h](C:/Users/auska/Downloads/linux/drivers/net/dsa/microchip/lan9645x/lan9645x_main.h:772)
- **SYS_FRM_AGING.** Enable TX aging and use the BSP two-second count for its 165.625 MHz ASIC assumption. Removed ANA_AUTOAGE and ANA_ADVLEARN writes: PHOTON disables dynamic learning, and uses a locked management entry. [main.c](C:/Users/auska/Downloads/linux/drivers/net/dsa/microchip/lan9645x/lan9645x_main.c:614)
- **ANA_DROP_CFG and ANA_PORT_CFG.** Enable multicast-source drops, set the four-bit port ID and RECV_ENA. Leave learning disabled, differing intentionally from the Linux bridge. [main.c](C:/Users/auska/Downloads/linux/drivers/net/dsa/microchip/lan9645x/lan9645x_main.c:248)
- **DEV_CLOCK_CFG, DEV_MAC_MODE_CFG, DEV_MAC_IFG_CFG, DEV_MAC_ENA_CFG and QSYS_SW_PORT_MODE.** Use the gigabit full-duplex MAC mode on each wired port, set the IFG, enable TX/RX, release port reset before MAC reset, then admit forwarding after the masks are installed. This fixed mode assumes both copper partners negotiate gigabit full duplex. [phylink.c](C:/Users/auska/Downloads/linux/drivers/net/dsa/microchip/lan9645x/lan9645x_phylink.c:183)
- **DEV_MAC_TAGS_CFG and DEV_MAC_MAXLEN_CFG.** Recognize customer/provider tags for frame length accounting. Use 1518 bytes for a 1500-byte MTU; DEV7 is an external MAC and needs no NPI header allowance. [main.c](C:/Users/auska/Downloads/linux/drivers/net/dsa/microchip/lan9645x/lan9645x_main.c:159)
- **HSIO_HW_CFG, QSYS_MISC_CFG, HSIO_DLL_CFG and HSIO_RGMII_CFG.** Select RGMII0 on DEV7 by clearing the DEV4 selector and enabling DEV7 GMII. Clear the DEV4-specific QSYS override. Use RGMII instance 0 and DLLs 0/1. Keep the prior no-added-delay policy; the required PCB/STM32 skew is not established by the Linux sources. [phylink.c](C:/Users/auska/Downloads/linux/drivers/net/dsa/microchip/lan9645x/lan9645x_phylink.c:20)
- **CHIP_TOP_CUPHY_PORT_CFG.** Enable GTX clock on both gigabit copper ports. The PHY negotiation and partner-advertisement state must be confirmed independently. [phylink.c](C:/Users/auska/Downloads/linux/drivers/net/dsa/microchip/lan9645x/lan9645x_phylink.c:223)
- **DEV_PCS1G_MODE_CFG, DEV_PCS1G_CFG, DEV_PCS1G_SD_CFG and DEV_PCS1G_ANEG_CFG.** Enable PCS, use the BSP link-up signal-detect setting, and set the two optical endpoints for matching fixed 1000BASE-X without Clause 37 negotiation. Other peer configurations may require in-band negotiation. [phylink.c](C:/Users/auska/Downloads/linux/drivers/net/dsa/microchip/lan9645x/lan9645x_phylink.c:540)
- **CHIP_TOP strap and HSIO_SD_CFG, HSIO_MPLL_CFG, HSIO_SD_STAT.** For each lane read the latched reference clock, select the gigabit multiplier and divider, release resets in sequence and check PLL, common-mode, RX and TX readiness. Use direct lane polarity as in the default Linux configuration. [Strap and lane setup](C:/Users/auska/Downloads/linux/drivers/phy/microchip/lan9645x_serdes.c:144) ? [1000BASE-X lane selection](C:/Users/auska/Downloads/linux/drivers/phy/microchip/lan9645x_serdes.c:398)

## Address evidence

The MFD assigns absolute target bases. The generated DSA register header gives group offsets, instance counts, register offsets and strides. [MFD resources](C:/Users/auska/Downloads/linux/drivers/mfd/lan9645x-spi.c:117)

| Register | Instance-zero byte address | Linux definition |
|---|---:|---|
| `SYS_RESET_CFG` | `0x2d040` | [definition](C:/Users/auska/Downloads/linux/drivers/net/dsa/microchip/lan9645x/lan9645x_regs.h:14049) |
| `SYS_RAM_INIT` | `0x2d18c` | [definition](C:/Users/auska/Downloads/linux/drivers/net/dsa/microchip/lan9645x/lan9645x_regs.h:14409) |
| `SYS_FRM_AGING` | `0x2d0c0` | [definition](C:/Users/auska/Downloads/linux/drivers/net/dsa/microchip/lan9645x/lan9645x_regs.h:14149) |
| `SYS_ATOP_TOT_CFG` | `0x2d14c` | [definition](C:/Users/auska/Downloads/linux/drivers/net/dsa/microchip/lan9645x/lan9645x_regs.h:14271) |
| `ANA_MACACCESS` | `0x145cb4` | [definition](C:/Users/auska/Downloads/linux/drivers/net/dsa/microchip/lan9645x/lan9645x_regs.h:1565) |
| `ANA_MACHDATA` | `0x145cac` | [definition](C:/Users/auska/Downloads/linux/drivers/net/dsa/microchip/lan9645x/lan9645x_regs.h:1545) |
| `ANA_MACLDATA` | `0x145cb0` | [definition](C:/Users/auska/Downloads/linux/drivers/net/dsa/microchip/lan9645x/lan9645x_regs.h:1561) |
| `ANA_VLANACCESS` | `0x145cc0` | [definition](C:/Users/auska/Downloads/linux/drivers/net/dsa/microchip/lan9645x/lan9645x_regs.h:1649) |
| `ANA_VLANMASK` | `0x146a08` | [definition](C:/Users/auska/Downloads/linux/drivers/net/dsa/microchip/lan9645x/lan9645x_regs.h:681) |
| `ANA_FLOODING_IPMC` | `0x146a64` | [definition](C:/Users/auska/Downloads/linux/drivers/net/dsa/microchip/lan9645x/lan9645x_regs.h:1227) |
| `QS_XTR_FLUSH` | `0x8018` | [definition](C:/Users/auska/Downloads/linux/drivers/net/dsa/microchip/lan9645x/lan9645x_regs.h:12159) |
| `HSIO_HW_CFG` | `0x30048` | [definition](C:/Users/auska/Downloads/linux/drivers/net/dsa/microchip/lan9645x/lan9645x_regs.h:7101) |
| `QSYS_MISC_CFG` | `0x1038ec` | [definition](C:/Users/auska/Downloads/linux/drivers/net/dsa/microchip/lan9645x/lan9645x_regs.h:12571) |
| `ANA_PGID` | `0x146c00` | [definition](C:/Users/auska/Downloads/linux/drivers/net/dsa/microchip/lan9645x/lan9645x_regs.h:1503) |
| `ANA_PGID_CFG` | `0x146c04` | [definition](C:/Users/auska/Downloads/linux/drivers/net/dsa/microchip/lan9645x/lan9645x_regs.h:1513) |
| `ANA_FLOODING` | `0x146a44` | [definition](C:/Users/auska/Downloads/linux/drivers/net/dsa/microchip/lan9645x/lan9645x_regs.h:1205) |
| `SYS_ATOP` | `0x2d124` | [definition](C:/Users/auska/Downloads/linux/drivers/net/dsa/microchip/lan9645x/lan9645x_regs.h:14261) |
| `ANA_VLAN_CFG` | `0x146000` | [definition](C:/Users/auska/Downloads/linux/drivers/net/dsa/microchip/lan9645x/lan9645x_regs.h:2097) |
| `ANA_DROP_CFG` | `0x146004` | [definition](C:/Users/auska/Downloads/linux/drivers/net/dsa/microchip/lan9645x/lan9645x_regs.h:2149) |
| `ANA_PORT_CFG` | `0x146070` | [definition](C:/Users/auska/Downloads/linux/drivers/net/dsa/microchip/lan9645x/lan9645x_regs.h:2495) |
| `QSYS_SW_PORT_MODE` | `0x103858` | [definition](C:/Users/auska/Downloads/linux/drivers/net/dsa/microchip/lan9645x/lan9645x_regs.h:12329) |
| `DEV_CLOCK_CFG` | `0x38000` | [definition](C:/Users/auska/Downloads/linux/drivers/net/dsa/microchip/lan9645x/lan9645x_regs.h:4641) |
| `DEV_MAC_MODE_CFG` | `0x38018` | [definition](C:/Users/auska/Downloads/linux/drivers/net/dsa/microchip/lan9645x/lan9645x_regs.h:4833) |
| `DEV_MAC_ENA_CFG` | `0x38014` | [definition](C:/Users/auska/Downloads/linux/drivers/net/dsa/microchip/lan9645x/lan9645x_regs.h:4817) |
| `DEV_MAC_IFG_CFG` | `0x38028` | [definition](C:/Users/auska/Downloads/linux/drivers/net/dsa/microchip/lan9645x/lan9645x_regs.h:4903) |
| `DEV_MAC_MAXLEN_CFG` | `0x3801c` | [definition](C:/Users/auska/Downloads/linux/drivers/net/dsa/microchip/lan9645x/lan9645x_regs.h:4855) |
| `DEV_MAC_TAGS_CFG` | `0x38020` | [definition](C:/Users/auska/Downloads/linux/drivers/net/dsa/microchip/lan9645x/lan9645x_regs.h:4865) |
| `DEV_PCS1G_CFG` | `0x38040` | [definition](C:/Users/auska/Downloads/linux/drivers/net/dsa/microchip/lan9645x/lan9645x_regs.h:5089) |
| `DEV_PCS1G_SD_CFG` | `0x38048` | [definition](C:/Users/auska/Downloads/linux/drivers/net/dsa/microchip/lan9645x/lan9645x_regs.h:5133) |
| `REW_PORT_VLAN_CFG` | `0x19000` | [definition](C:/Users/auska/Downloads/linux/drivers/net/dsa/microchip/lan9645x/lan9645x_regs.h:13617) |
| `REW_TAG_CFG` | `0x19004` | [definition](C:/Users/auska/Downloads/linux/drivers/net/dsa/microchip/lan9645x/lan9645x_regs.h:13645) |
| `DEV_PCS1G_MODE_CFG` | `0x38044` | [definition](C:/Users/auska/Downloads/linux/drivers/net/dsa/microchip/lan9645x/lan9645x_regs.h:5111) |
| `DEV_PCS1G_ANEG_CFG` | `0x3804c` | [definition](C:/Users/auska/Downloads/linux/drivers/net/dsa/microchip/lan9645x/lan9645x_regs.h:5155) |
| `CHIP_TOP_CUPHY_PORT_CFG` | `0x10020` | [definition](C:/Users/auska/Downloads/linux/drivers/net/dsa/microchip/lan9645x/lan9645x_regs.h:3439) |
| `HSIO_DLL_CFG` | `0x30064` | [definition](C:/Users/auska/Downloads/linux/drivers/net/dsa/microchip/lan9645x/lan9645x_regs.h:7295) |
| `HSIO_RGMII_CFG` | `0x30054` | [definition](C:/Users/auska/Downloads/linux/drivers/net/dsa/microchip/lan9645x/lan9645x_regs.h:7167) |
| `QSYS_RES_CFG` | `0x104000` | [definition](C:/Users/auska/Downloads/linux/drivers/net/dsa/microchip/lan9645x/lan9645x_regs.h:12639) |

ORG addresses and SPI framing: [ORG](C:/Users/auska/Downloads/linux/drivers/mfd/lan9645x-spi.c:51) ? [regmap shift](C:/Users/auska/Downloads/linux/drivers/mfd/lan9645x-spi.c:246). SerDes addresses: [strap](C:/Users/auska/Downloads/linux/drivers/phy/microchip/lan9645x_serdes.c:141) ? [lane register bases](C:/Users/auska/Downloads/linux/drivers/phy/microchip/lan9645x_serdes.c:645) ? [field masks](C:/Users/auska/Downloads/linux/drivers/phy/microchip/lan9645x_serdes_regs.h:1).
