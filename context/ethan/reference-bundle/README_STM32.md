# LAN96455F / STM32 public-source reference package

Prepared 2026-10-12. This is a collected reference-source package, NOT a ready-to-flash STM32 driver, complete Linux build tree, complete register manual, or verified board initialization sequence.

## Provenance

Repository: https://github.com/microchip-ung/linux
Branch at retrieval: bsp-6.18-2026
Pinned repository commit: 6ffda6116249fa9e9e892206d5df337821345c76
All upstream downloads use this immutable commit. SOURCE_MANIFEST.json records individual URLs, sizes and SHA-256 hashes. Original files and copyright/SPDX notices are preserved. GPL license text is included; derived driver code must retain applicable licensing. No October8 upstream patch definitions are mixed into this vendor snapshot.

## Start here

| Purpose | File under upstream/ |
|---|---|
| Main generated register definitions | drivers/net/dsa/microchip/lan9645x/lan9645x_regs.h |
| SerDes generated register definitions | drivers/phy/microchip/lan9645x_serdes_regs.h |
| SPI transfer, endianness, padding, reset and target base addresses | drivers/mfd/lan9645x-spi.c |
| Register tuple decoding / register read-write helpers | drivers/net/dsa/microchip/lan9645x/lan9645x_main.h |
| Switch initialization / device handling | drivers/net/dsa/microchip/lan9645x/lan9645x_main.c |
| Port / RGMII / phylink configuration | drivers/net/dsa/microchip/lan9645x/lan9645x_phylink.c and lan9645x_port.c |
| Optical SerDes setup | drivers/phy/microchip/lan9645x_serdes.c |
| GPIO alternate functions | drivers/pinctrl/pinctrl-ocelot.c (LAN9645x sections) |
| Copper PHY initialization / errata | drivers/net/phy/micrel.c (LAN9645x sections) |
| MDIO controller access | drivers/net/mdio/mdio-mscc-miim.c |
| VLAN and forwarding database | drivers/net/dsa/microchip/lan9645x/lan9645x_vlan.c and lan9645x_mac.c |
| CPU/NPI connection and Ethernet tag format | drivers/net/dsa/microchip/lan9645x/lan9645x_npi.c and net/dsa/tag_lan9645x.c |
| Linux evaluation-board configuration examples | arch/arm/boot/dts/microchip/*lan9645x* |

The complete LAN9645x switch-driver directory is included, including statistics, multicast, QoS and redundancy sources. These features are not all required by our project. Device-tree files describe evaluation boards, not the STM32 board; do not copy their port topology or GPIO ownership blindly. EVB rev-c DTS is a board configuration example, not DS50003971C user guide.

DEFINITION_INDEX.json provides a searchable index of every #define in the downloaded C/header sources, with original path/line and multiline definition. It includes non-register macros too. It is an index, not a flattened or independently verified register map.

## How register addresses work

Read lan9645x_main.h, functions __lan_rel_addr and lan_rel_addr. The generated __REG tuple contains target, target instance/count, group base/instance/count/stride and register offset/instance/count/stride. Relative byte address = group_base + group_instance*group_stride + register_offset + register_instance*register_stride. Add the selected target instance base from the driver resource mapping; never use the target enum value as an address.

Read lan9645x-spi.c resource arrays and lan9645x_spi_regmap_config: byte addresses are aligned and shifted by two for the SPI transfer. Preserve write-flag handling, address byte order, data byte order and read padding consistently. The source defines DEVCPU_GCB_IF_CTRL=(0x0007<<2) and IF_CFGSTAT=(0x0008<<2) as GCB-relative offsets, not complete chip addresses. Regmap selects the GCB base separately.

## SPI initialization references (source behavior, not a qualified STM32 recipe)

lan9645x-spi.c: lan9645x_spi_initialize writes IF_CTRL, configures read padding, waits, then checks IF_CFGSTAT readback. Probe initializes transport, calls lan9645x_spi_chip_reset and initializes transport again. Reset restores SPI settings; the reset routine handles default padding. Preserve this ordering when investigating a port. Source functions and manifest URLs are the exact references (software has no printed page numbers).

Do not treat SYS_RDYn as the managed BOOT000 ready signal. A working SPI readback does not prove PHY/SerDes/forwarding readiness.

## Our board: implementation work still required

Project intent: BOOT000, MGMT11; STM32 SPI host to LAN GPIO4 DI / GPIO5 DO / GPIO6 nCS / GPIO7 CLK. GPIO0-3 ALT2 is also listed in the datasheet, but alternate-function availability alone does not establish reset mapping. Verify reset-accessible mapping for exact silicon before pin changes.

Project port intent: CuPHY0/DEV0 laptop, CuPHY1/DEV1 optional debug, SerDes0/DEV5 primary SFP, SerDes1/DEV6 backup SFP, RGMII0/DEV7 STM32. Confirm against the final schematic and selected device's enabled ports; the EVB uses different hardware.

Porting work:
1. Supply STM32 SPI, chip-select, timing/delay, reset GPIO, locking, and error/timeout callbacks. Confirm actual STM32 pin assignments and Cube/HAL/RTOS configuration from final sources.
2. Implement transport and verify repeated nontrivial register readback, device/revision identity, padding and reset recovery on the bench.
3. Trace all switch, copper PHY, SerDes and pinctrl initialization dependencies; implement the required subset with register-level error handling. Linux includes/regmap/phylink/DSA/workqueues are not available on bare metal.
4. Qualify 1000BASE-X (not merely SGMII), reference clocks, polarity/swing, RGMII MAC-to-MAC mode and delay ownership for this exact board/silicon. Preserve applicable errata.
5. Select CPU/NPI tagging or untagged management operation deliberately and implement the matching STM32 Ethernet frame handling. Do not assume Linux DSA tagging works transparently with an ordinary STM32 MAC stack.
6. Implement VLAN membership, learning/flooding and CPU/debug-port isolation, then the two-board heartbeat/failover policy without creating a forwarding loop.
7. Keep STM-owned SFP power, TX_DISABLE, presence, LOS, TX_FAULT and DOM control consistent with fitted/DNP branches. These are board-level responsibilities, not supplied by the switch register header.
8. BOOT100 EEPROM configuration contents/tooling remain a separate unverified task. This package does not contain an approved EEPROM image or STM32 startup application.

Missing qualification: exact silicon support/errata closure; final STM32 pins; complete minimal init sequence; measurements; complete register access semantics/reset values for all fields. Public code provides substantial implementation evidence but does not guarantee every register is documented or every board requirement is covered.

## Datasheet

The included local DS00006065 PDF is the project's stored LAN9645xF datasheet, independently retained alongside the sources. Previously inspected revision D: Table3-5 p.30 GPIO mappings; section3.4.1.2 p.43 management strap; sections4.23.1.1 pp.164-167 SPI. PDF/printed pages match in that copy. This package does not assert that it is the newest public revision.

## Verification performed

All manifest-listed upstream files downloaded successfully and SHA-256 checked after packaging. Definition index points to existing source files/lines. ZIP integrity checked. No STM32 build, hardware execution or electrical validation performed. This is a source bundle, not a claim that all Linux transitive dependencies are included.
