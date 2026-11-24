# System Architecture

## Per-board functional blocks

```text
Laptop/RJ45
    |
Magnetics + protection
    |
LAN96455F managed Ethernet switch
    |-- 1000BASE-X/SGMII SerDes --> SFP0 primary
    |-- 1000BASE-X/SGMII SerDes --> SFP1 backup
    |-- RGMII0 -------------------> STM32N6 Ethernet MAC
    |-- SPI/MDIO/GPIO/IRQ --------> STM32N6 management
    `-- I2C configuration EEPROM

STM32N6
    |-- I2C mux/bus --> SFP0/SFP1 DOM telemetry
    |-- GPIO -------> TX_DISABLE and load-switch enables
    |-- GPIO <------- LOS, TX_FAULT, module-present/status
    |-- XSPI/QSPI --> external NOR flash
    `-- SWD/JTAG --> programming/debug connector
```

## Optical path

- Each SFP is full duplex and uses a duplex LC connector with separate transmit and receive fibers.
- Selected optics are 850 nm multimode 1000BASE-SX modules.
- Use duplex multimode fiber appropriate for 850 nm operation, such as OM3.
- A variable optical attenuator or fixed LC attenuator can be inserted inline on one fiber direction to demonstrate degrading received power.
- Attenuation testing must remain within the receiver absolute limits and the link budget.

## Management behavior

- SFP digital optical monitoring is read locally by each STM32 over I2C.
- Switch link state and counters are read through the selected LAN96455F management interface.
- Board-to-board telemetry is encapsulated in ordinary Ethernet frames and forwarded by the switch hardware alongside laptop traffic.
- Failover policy, hysteresis, timeout behavior, and synchronization remain firmware design items.

### 2026-11-02 — Executable demo behavioral model

The [PHOTON CLI/browser console](../demo_console/README.md) now implements a synthetic two-board model with persistent optical-fault detection, hysteresis, staged acknowledged switching, per-board VLAN state, watchdogs and traffic/DOM diagnostics. It is executable software, not STM32 firmware or measured hardware behavior.

**Proposed demo management-path refinement:** independently CPU-terminate management VLAN4090 on primary DEV5/DEV7 and VLAN4091 on backup DEV6/DEV7. Keep both management paths available while user VLAN100 uses DEV0 and only one optical uplink. This addresses loss of coordination after an active-fiber cut. It does not approve switch register configuration, and supersedes active-link-only management only within the simulation. Actual firmware must validate VLAN isolation, tagged CPU traffic, durable epochs, watchdogs, switch readback and reboot behavior. See [demo implementation notes](DEMO_CONSOLE.md).

## LAN96455F RGMII0 strap coexistence

- RGMII0 uses GPIO16 through GPIO27. GPIO22 through GPIO27 are also configuration-strap pins sampled when reset is released, then operate as `RGMII0_TX_CLK`, `TX_CTL`, and `TX_D[3:0]`.
- External strap networks therefore remain physically connected to six active RGMII0 transmit nets during runtime. This is intentional and is used by Microchip's LAN96459F EDS2 reference design.
- Place each shunt strap network at the LAN96455F pin/route with essentially no branch stub. The STM32 RGMII receive pins must remain inputs/high impedance while the switch is in reset and straps are being sampled.
- Exact fixed resistor populations remain tied to the final boot/management choice; do not omit these pulls merely because RGMII0 is enabled.

## Default and backup startup intent — 2026-09-16

- Recommended intended default: BOOT_MODE=000, MGMT_MODE=11; STM32 initializes and manages the switch through SPI. This remains contingent on proving the custom initialization firmware.
- Preserve an optional EEPROM and selectable BOOT_MODE=100 with SPI management retained for bring-up/recovery. It is not an automatic fallback and not yet validated as a full failover alternative.
- Hardware mix confirmed by DS00006065D Table 4-4, p. 54: CuPHY0/DEV0, CuPHY1/DEV1, SerDes0/DEV5, SerDes1/DEV6, RGMII0/DEV7. Copper population roles and exact initialization remain to be finalized.
- See LAN96455_CONTROL.md for mode comparison, ROM/STM32 ownership risks, and verification criteria.

## 2026-09-17 - Clarified optional copper debug port

For the current five-port interface mix, laptop copper is proposed on CuPHY0/DEV0 and optional debug copper on CuPHY1/DEV1. CuPHY2 is not active in this mix. Optical paths are DEV5/6; external STM32 RGMII0 is DEV7. Port9 is an internal injection/extraction/VRAP port, not a connector. Basis: DS00006065D Table 4-3 p. 53/Table 4-4 p. 54; LED candidate mapping and exact references are in LAN96455_CONTROL.md. This clarifies topology and does not freeze copper connector population.

## 2026-09-22 - STM32 interface tracking
[STM32_ETH_SW_INTERFACES.md](STM32_ETH_SW_INTERFACES.md) tracks19 currently shown logical MCU interfaces (12RGMII,4SPI,1supervisor reset request,2EEPROM programming) separately from proposed IRQ/power/SFP connections. Exact STM32 pins/AFs remain TBD; register is screenshot-backed, not compiled-netlist verification.


### 2026-10-05 ? Deployment review gates from full schematic export
[Full PDF/BOM audit](audits/2026-10-05/FULL_DESIGN_REVIEW.md) identifies confirmed hardware/assembly blockers and pending functional gates. The two-board architecture remains unchanged. Before deployment, demonstrate standalone MCU flash boot, SPI switch initialization, independently controlled SFP power, defined VLAN/CPU/debug-port policy, and coordinated loop-free primary/backup transitions including loss of the peer. EEPROM BOOT100 remains a bring-up/recovery option, not an independently powered automatic failover path. Block diagrams currently contain obsolete flash/regulator/USB-PD claims and an inconsistent input-power estimate; seeF12. No architecture change adopted in this review.
