# LAN96455F control and configuration schematic

Status: proposed schematic guidance, not implemented or electrically verified in Altium.

## 2026-09-16 — Default and backup boot evaluation (latest guidance)

User clarification: EEPROM is a backup, not a request to replace the proposed STM32-initialized default. The preceding EEPROM clarification only describes what setting is needed to use that backup. Retain BOOT_MODE=000 / MGMT_MODE=11 as the recommended intended default, conditional on successful STM32 initialization proof; provide selectable BOOT_MODE=100 / MGMT_MODE=11 for EEPROM/ROM bring-up and recovery. These are manual reset-time choices, not automatic fallback.

### Architecture fit and evidence

- Hardware port mix verified in DS00006065D Table 4-4 and Figure 4-1, p. 54 (visually inspected): the five-port device supports CuPHY0/DEV0, CuPHY1/DEV1, SerDes0/DEV5, SerDes1/DEV6, and RGMII0/DEV7 together. Table 4-7, p. 55 lists 1G SFP SerDes operation. This confirms hardware resources, not a complete working initialization image.
- 000: STM32 owns switch initialization and link/forwarding policy. This suits coordinated primary/backup control and local SFP monitoring. Required firmware includes switch reset/core setup, GPIO/port mux, SerDes/PCS and PHY setup, fixed RGMII MAC link and delays, forwarding configuration, link handling, and failover updates. No working STM32 implementation has yet been demonstrated. Vendor support is Linux DSA over SPI (section 3.4.1.2 note, p. 43); SPI access alone is not proof of a complete driver. Do not assume a DSA driver's tagged host-port setup can be copied unchanged into an ordinary STM32 Ethernet stack.
- 100: internal ROM initializes from the EEPROM and keeps managing links (sections 4.1 and 4.1.2.1, p. 49). SPI client remains a valid parallel management interface, as shown by DS50003971A Table 3-5, p. 20. However, direct STM32-MAC RGMII configuration must be established because unmanaged firmware normally polls external PHYs (section 4.1.2.12, p. 51). EEPROM link-event instructions can run again after boot (section 4.1.2.15, pp. 51–52). Verify that runtime writes, SFP control/I2C ownership, and port enable/disable policy do not conflict with ROM behavior. Do not promise ROM-to-host ownership transfer without documentation/testing.
- Microchip Getting Started guide DS00006594A section 2.2 says the EEPROM Configurator is for static configuration; vendor-supported dynamic control/redundancy uses a managed Linux DSA system. The tool is supplied through SDE. This does not prove custom hybrid control impossible, but it means 100+SPI is not evidence of a supported STM32 failover solution. Source: https://onlinedocs.microchip.com/oxy/GUID-DD2492A7-AC89-4ABA-A1C1-22E9A6F7E1C2-en-US-1/GUID-5E7109FC-06C2-48A5-974A-8C5C0195BA5F.html

### Backup schematic and validation requirements

- Keep boot2 selectable: 000 default, 100 backup; boot1 and boot0 low, MGMT_MODE=11 in both. Sample changes through controlled reset; reset disrupts forwarding.
- Keep EEPROM on its ROM/I2C configuration bus and preserve GPIO4–7 SPI client management. Verify the full backup GPIO mode/table and all previously ignored unmanaged strap populations before calling the backup ready.
- In 000, attached EEPROM is not automatically consumed; STM32 must leave the EEPROM bus pins in safe states or deliberately configure them. Programming header/isolation and write protection remain to be designed.
- EEPROM contents must match our five-port map, fixed STM32 RGMII link, and 1G optical modules. Verify that 24AA014H's 128-byte capacity accommodates the actual image; do not freeze capacity from the reference BOM alone.
- Define a recovery configuration with only one optical forwarding path initially, to avoid a two-board Layer-2 loop. This is a proposed project policy, not a claim that the ROM defaults implement it. Backup mode need not provide automatic failover to be useful for basic hardware tests.
- Before making 000 the production default, demonstrate reset-to-SPI ID access, initialized copper/SFP forwarding, STM32 RGMII traffic, and controlled failover using the intended firmware. Independently test 100 cold boot and the safe forwarding image with STM32 initialization disabled. Both paths remain unvalidated on this board.

## Scope and sheet organization

Start with a `LAN96455 Control & Configuration` sheet: symbol parts G, H, and I, boot/management straps, reset, SPI management, interrupt/debug provisions, and optional configuration memory. Handle mandatory 25 MHz clock and reference resistors from part F in a subsequent support-circuit pass. Power design, RGMII wiring, SerDes wiring, and copper Ethernet wiring are deferred at the user's request. Their pin-multiplexing constraints still apply.

## Evidence

- Local `../eth_SW/docs/LAN9645xF-Data-Sheet-DS00006065.pdf`, revision D: Table 3-2 (pp. 21–22), Table 3-4 (pp. 28–29), Table 3-5 (pp. 30–31), section 3.4 (pp. 42–49), section 4.22, Table 5-17 (p. 200).
- Reviewed archived symbol screenshots G, H, and I. Their visible names match the relevant strap groupings; pin numbers are not visible, so these images do not verify library pad mapping.
- `../eth_SW/docs/LAN96459F_EDS2.pdf`, section 3.6: MIC2774 supervisor and host/manual reset arrangement. This is a family reference, not proof of the five-port device's topology.
- Microchip managed hardware checklist DS00006273B, sections 3.4 and 4: https://ww1.microchip.com/downloads/aemDocuments/documents/UNG/ProductDocuments/DesignChecklist/LAN96459F-Hardware-Design-Checklist-Managed-Mode-DS00006273.pdf
- Local revision D is newer than the revision C datasheet returned by web search; use the inspected local revision for this pass.

## Claim-by-claim reference guide

All datasheet locators below refer to the local **LAN9645xF Data Sheet, DS00006065D (revision D)**. Printed page numbers and PDF viewer page numbers match in this file.

| Guidance | Exact source | What it establishes |
|---|---|---|
| External strap resistors and reset sampling | Section 3.4, Configuration Straps, p. 42; Table 3-2 note, p. 22 | Straps latch on reset release; external pulls required; pull-ups use the associated I/O rail |
| BOOT_MODE = 000 | Section 3.4.1.1, Boot Mode Selector, p. 42; Table 3-12, Boot Mode Configuration Precedence, p. 43 | No internal-controller boot; unmanaged-specific straps ignored in mode 0 |
| MGMT_MODE = 11 | Section 3.4.1.2, Management Interface Mode Selector, p. 43 | Selects SPI client; accompanying note limits vendor managed-mode support to Linux DSA over SPI |
| JTAG_ENB = 1 | Section 3.4.1.3, JTAG Enable, pp. 43–44 | High disables JTAG; low enables a TAP selected by BOOT_MODE0 |
| SD_125M = 0 | Section 3.4.1.4, SerDes 125 MHz Selector, p. 44; section 4.22, Clocking and Reset, p. 163 | Optional external 125 MHz reference versus mandatory 25 MHz reference |
| Physical package pins | Table 3-2, 156-Pin VQFN-DR Assignments, pp. 21–22 | GPIO and reset physical pin numbers for this package |
| Pull-up rail assignments | Table 3-4, VDD_IO_x Power Domains by GPIO, pp. 28–29; explicit nRESET note, p. 29 | Domain C for GPIO23–28, E for GPIO34, A for nRESET |
| RGMII overlap and proposed SPI pins | Table 3-5, GPIO Alternate Function Assignments in Managed Mode, pp. 30–31 | GPIO multiplexing and SPI-client alternate-function names |
| Reset assertion and strap hold | Section 5.7.3, Figure 5-2, nRESET Timing Diagram, and Table 5-17, nRESET Timing Specifications, p. 200 | 2 ms minimum after supplies/clock stabilize, 1 us minimum reset pulse, 50 ns minimum strap hold |
| Recovery-time discrepancy | Section 4.22, p. 163 versus Table 5-17, p. 200 | Approximately 14 ms narrative versus 100 ms and 500 ns recovery entries; unresolved |
| Supervisor example | LAN96459F EDS2 guide DS50003971A, section 3.6 and Figure 3-4, LAN96459F EDS2 Daughter Card Reset Circuit, p. 21 | MIC2774 with host/manual reset and delayed release; reference implementation, not a required exact part |
| Proposed 10 kohm / 1% and opposite DNP footprints | Engineering proposal; no datasheet mandate | Must be checked against loads and electrical limits before freezing |

## Boot decision: proposed baseline, not frozen

For external STM32 control, propose BOOT_MODE[2:0] = 000 (internal controller does not boot) and MGMT_MODE[1:0] = 11 (SPI client). The host must initialize the switch; do not expect autonomous forwarding just because reset was released.

ROM + I2C table boot (100) instead runs internal unmanaged firmware with EEPROM configuration. It is a separate architecture option, even if SPI is also selected for management. SPI client management does not mean SPI table boot. Boot modes 001 and 010 are reserved.

Microchip states that managed-mode support is limited to Linux DSA over SPI. STM32 firmware therefore requires a custom initialization/control implementation; validate access to the required programming information and firmware feasibility before freezing the baseline. Keep resistor population options available while resolving this.

The earlier 24AA014H selection applies to the ROM/EEPROM option. It is not required for the proposed no-boot baseline, and must not be wired across SPI pins merely to retain the earlier block diagram.

## First seven strap connections (156-pin package)

Use one populated resistor per strap, to GND for 0 or the named I/O domain for 1. Proposed starting value: 10 kohm, 1%, with an opposite-polarity DNP resistor footprint for bring-up flexibility. This is an engineering choice, not a datasheet-mandated universal value; verify loading, internal pulls, voltage thresholds, and external device states, especially on shared signals or LED loads. Never hard-short a multiplexed GPIO to a rail.

| Signal | GPIO | Physical pin | Proposed level | Pull destination | Meaning |
|---|---|---:|---:|---|---|
| BOOT_MODE0 | 24 | 84 | 0 | GND | BOOT_MODE = 000 |
| BOOT_MODE1 | 25 | 82 | 0 | GND | BOOT_MODE = 000 |
| BOOT_MODE2 | 26 | 81 | 0 | GND | BOOT_MODE = 000 |
| MGMT_MODE0 | 27 | 80 | 1 | VDD_IO_C | SPI client |
| MGMT_MODE1 | 28 | 79 | 1 | VDD_IO_C | SPI client |
| JTAG_ENB | 34 | 52 | 1 | VDD_IO_E | Disable switch test JTAG |
| SD_125M | 23 | 85 | 0 | GND | No external 125 MHz reference |

SD_125M = 0 does not disable the SerDes data channels. It selects operation without the optional external 125 MHz reference. The mandatory 25 MHz reference remains required. Switch JTAG is separate from STM32 SWD.

## Remaining strap inventory

These are unmanaged-specific settings and are ignored as configuration in boot mode 000. The datasheet nevertheless instructs external pulls on all strap pins. Reserve explicit resistor options; finish population values alongside the runtime pin assignments. Do not interpret an ignored strap as a free/unconnected pin.

| Strap | GPIO | Physical pin | I/O domain |
|---|---:|---:|---|
| E2E_CLOCK_EN | 10 | 104 | B |
| FLOW_EN | 11 | 103 | B |
| EEE_EN | 12 | 102 | B |
| LED_MODE | 13 | 100 | B |
| CUPHY_MODE | 14 | 99 | B |
| SFP_EN | 15 | 98 | B |
| SGPIO_EN | 22 | 86 | C |
| EEPROM_SIZE | 33 | 74 | D |
| LOOP_EN | 35 | 51 | E |
| GPIO_MODE0 | 36 | 50 | E |
| GPIO_MODE1 | 37 | 49 | E |
| XMII_MODE1 | 38 | 47 | E |
| SD_MODE0 | 39 | 42 | E |
| SD_MODE1 | 40 | 40 | E |
| XMII_MODE0 | 41 | 41 | E |

GPIO4–15 can carry RGMII1 and GPIO16–27 can carry RGMII0. Reserve both groups until the five-port assignment is verified. All strap drivers must permit valid levels before reset release and for at least 50 ns afterward, including warm resets when the STM32 may already be running. An actively driven MCU pin can defeat a 10 kohm strap.

## RGMII0 and configuration-strap coexistence

RGMII0 is the selected STM32 Ethernet-MAC connection. Its receive group on GPIO16–21 has no configuration straps, but every signal in its transmit group on GPIO22–27 is also sampled as a strap:

| GPIO / pin | Runtime RGMII0 function | Reset-time strap | Proposed baseline level |
|---|---|---|---:|
| GPIO22 / 86 | `RGMII0_TX_CLK` | `SGPIO_EN` | 0, provisional; ignored as configuration in boot mode 000 |
| GPIO23 / 85 | `RGMII0_TX_CTL` | `SD_125M` | 0 |
| GPIO24 / 84 | `RGMII0_TX_D0` | `BOOT_MODE0` | 0 |
| GPIO25 / 82 | `RGMII0_TX_D1` | `BOOT_MODE1` | 0 |
| GPIO26 / 81 | `RGMII0_TX_D2` | `BOOT_MODE2` | 0 |
| GPIO27 / 80 | `RGMII0_TX_D3` | `MGMT_MODE0` | 1 |

`MGMT_MODE1` is GPIO28/pin 79 and is not part of RGMII0; the proposed SPI-client baseline also straps it high.

This multiplexing is intentional rather than a conflict. Section 3.4 states that straps are latched on reset release and that all straps require external pull-up or pull-down networks; Table 3-5 then assigns GPIO22–27 to the RGMII0 transmit signals in managed mode. The resistors remain connected after sampling and become shunt loads on the active outputs.

Microchip's EDS2 reference implements the same overlap in Figure A-5. Its selectable network uses a permanent 10 kohm pull-down per strap and, when the DIP switch is closed, a 1 kohm pull-up to VDDIO. The figure explicitly instructs layout to place the strapping resistors along the RGMII routes so there are no stubs. That 1 kohm/10 kohm switchable network is a reference-board implementation, not a universal resistor mandate.

Implementation constraints for this board:

- Do not delete the six strap networks when connecting RGMII0.
- Use fixed populations appropriate to the frozen boot/management mode, or retain opposite-polarity DNP options for bring-up. Final values are not yet frozen.
- Connect any high pull only to `VDD_IO_C`, the I/O domain for GPIO22–28.
- Place the shunt resistor pad directly adjacent to the LAN96455F route/pin and avoid a T-branch trace stub.
- GPIO22–27 are switch-to-STM32 RGMII transmit signals during runtime. Configure the corresponding STM32 pins as inputs and prevent any external device from driving them while `SW_nRESET` is asserted or being released.
- Validate the chosen resistor network against strap VIH/VIL margin and the RGMII output loading/edge-rate budget. A 10 kohm shunt is light; copying the reference board's enabled 1 kohm pull creates a stronger approximately 3.3 mA DC load when the output drives the opposite state and should not be generalized without review.

Exact references:

- LAN9645xF Data Sheet, DS00006065D, Table 3-5, "GPIO Alternate Function Assignments in Managed Mode," printed p. 30: GPIO16–27 map to RGMII0 and GPIO22–27 are its transmit group.
- LAN9645xF Data Sheet, DS00006065D, section 3.4, "Configuration Straps," printed p. 42: straps latch on reset release, all have internal pull-ups, external pulls are required, and high straps must use the associated `VDD_IO_x` rail.
- LAN96459F EDS2 Daughter Card User Guide, DS50003971A, section 3.5 and Table 3-4, printed p. 20: identifies the six selectable strap functions.
- LAN96459F EDS2 Daughter Card User Guide, DS50003971A, Figure A-5, "LAN96459F EDS2 Daughter Card RGMII and GPIO," printed p. 34: shows the 10 kohm pull-down/1 kohm selectable pull-up networks attached to RGMII0 and the no-stub layout note.

### Current Windows schematic review — 2026-09-16

The captured Altium strap block correctly expresses the proposed no-boot/SPI-client levels for `SD_125M`, `BOOT_MODE[2:0]`, and `MGMT_MODE[1:0]`, but it omits the `SGPIO_EN` strap on GPIO22/pin 86 (`RGMII0_TX_CLK`). Revision D states that every strap needs an external pull despite the internal pull-up, so add a seventh strap row on `ETH_SW_RGMII0_TX_CLK`. The proposed default is `SGPIO_EN=0`, implemented by the default pull-down; an optional selectable pull-up may use DIP position 7, leaving position 8 unused.

Recommended schematic notes:

- `CONFIGURATION STRAPS — sampled on SW_nRESET rising edge; valid before release and held >= 50 ns afterward. Pull-ups connect only to +3V3_ETH_SW_VDD_IO_C.`
- `RGMII0 SHARED STRAP PINS — GPIO22–27 become TX_CLK/TX_CTL/TXD[3:0] after strap sampling. Place strap resistors at LAN96455F pins with no trace stubs. STM32 RGMII RX pins must remain inputs during SW_nRESET.`
- `DEFAULT — SGPIO_EN=0; SD_125M=0; BOOT_MODE[2:0]=000 (NO BOOT); MGMT_MODE[1:0]=11 (SPI CLIENT). STM32 must initialize LAN96455F before forwarding.`
- `SW OFF = 0 via 10k pulldown; SW ON = 1 via 1k pull-up plus 10k pulldown. Debug/bring-up configuration; freeze production populations.`
- `SD_125M=0 disables the optional external 125 MHz SerDes reference; the mandatory 25 MHz reference is still required.`

Rename the schematic's bare `PLL1` annotation to `SD_125M / optional 125 MHz SerDes reference` (or `PLL1 (SD_125M)`) to avoid implying that it controls the mandatory 25 MHz reference. If GPIO28 is used only as a strap, label it `GPIO28/MGMT_MODE1` rather than `MIIM0_MDC`; retain the MIIM name only if that runtime interface will actually be used.

A screenshot-ready companion workbook is stored at `../eth_SW/docs/LAN96455_Project_Reference_Tables.xlsx`. It contains separate formatted views for the project configuration, boot/management matrix, RGMII0 strap mapping, and current GPIO allocation. Green rows are selected connections, yellow rows are proposals not yet frozen, and gray rows are reserved/TBD.

## Reset block

- Part I: physical pin 123, nRESET; proposed net name `SW_nRESET`.
- Pull up only to VDD_IO_A. Do not connect to a generic upstream 3.3 V rail without checking domain identity and switch-off behavior.
- Use a supervisor-controlled reset with a host reset-request/manual-reset path. Preserve an accessible reset test point. Exact supervisor, pull-up, polarity, voltage monitoring, and output type remain to be selected when power sequencing is resolved.
- Keep reset asserted at least 2 ms after required supplies and clock stabilize (Table 5-17 minimum).
- Subsequent low reset pulse: at least 1 us. Straps must be valid before release and held at least 50 ns afterward.
- Do not directly parallel push-pull reset outputs. Determine whether reset requests go through the supervisor's manual input or a compatible open-drain arrangement after selecting the supervisor.
- Recovery-time conflict: section 4.22 says approximately 14 ms to operational; Table 5-17 gives 100 ms maximum to start PROM reads and an apparently inconsistent 500 ns typical to fully active in unmanaged mode. Visually confirmed on p. 200. Do not use 500 ns as firmware readiness timing; resolve with manufacturer clarification and device-status checks during bring-up.

## Following support-circuit passes

1. Freeze boot architecture with firmware feasibility, then complete the resistor population table for all 22 straps.
2. SPI client proposal: GPIO4/pin112 = SI_SLV_DI (STM32 MOSI), GPIO5/pin111 = SI_SLV_DO (STM32 MISO), GPIO6/pin110 = SI_SLV_nCS, and GPIO7/pin108 = SI_SLV_CLK; domain B. `MGMT_MODE[1:0]=11` exposes the strap-selected SPI client on GPIO[7:4], as shown by LAN96459F EDS2 Figure A-5/boot table and the ALT6 entries in LAN9645xF Data Sheet revision D Table 3-5, printed p. 30. The earlier GPIO[3:0] proposal was incorrect; GPIO0–3 also list manually selectable `SI_SLV_*` ALT2 functions, but they are not the strap-selected management-interface pins used by this project baseline. Confirm electrical details and timing before capture sign-off.
3. Reserve an interrupt candidate: GPIO33/pin74, IRQ0 alternate function, domain D. Confirm firmware routing/polarity and electrical behavior before adding pulls. It overlaps EEPROM_SIZE.
4. Complete mandatory clock/reference-resistor circuitry, LEDs/debug/test points, and unused-pin treatment, with five-port mapping checked before assigning alternate functions.
5. Review an exported schematic or saved screenshots against this document; no Altium changes have been made in this task.

## 2026-09-16 — EEPROM boot clarification (current guidance)

The earlier BOOT_MODE=000 baseline is an externally initialized managed proposal only; it must not be used as the default for autonomous unmanaged EEPROM startup. For the user's current EEPROM schematic, the applicable candidate is BOOT_MODE[2:0]=100 (ROM + I2C table), with MGMT_MODE[1:0]=11 if retaining SPI client management. Final architecture selection remains pending; no Altium strap change is claimed.

- DS00006065D section 3.4.1.1, p. 42 and Table 3-12, p. 43: 000 does not boot the internal controller or autonomously read the configuration EEPROM; 100 runs ROM with the I2C configuration table.
- DS00006065D section 3.4.1.2, p. 43: management interface selection is separate from boot selection. DS50003971A section 2.2/Figure 2-2, p. 13 and Table 3-5, p. 20 explicitly demonstrate ROM + I2C table with SPI client on EDS2.
- For 100, BOOT_MODE2/GPIO26/pin81 must be high; BOOT_MODE1/GPIO25/pin82 and BOOT_MODE0/GPIO24/pin84 low. Both management bits remain high. Pull-up domain is VDD_IO_C (Table 3-2, pp. 21–22; Table 3-4, pp. 28–29).
- Table 3-6, p. 32 shows I2C1_SDA on GPIO2/pin119 and I2C1_SCL on GPIO3/pin118 for GPIO mode A ROM/I2C configuration. GPIO mode/topology must be checked before final wiring. Keep the previously corrected SPI-client mapping on GPIO4–7; do not revert to the old GPIO0–3 proposal.
- Section 3.4.2.8, p. 46 defines EEPROM_SIZE=1 for 8-bit addressing and 0 for 16-bit. Section 4.1.2.15, pp. 51–52 requires I2C base address 0x50 and at least 400 kHz support. Verify the exact EEPROM ordering code, addressing, capacity for the intended table, and programming/write-protection circuit before capture sign-off.
- Having an STM32 attached does not by itself require no-boot mode. However, end-to-end compatibility is not yet confirmed: verify five-port mapping, RGMII0 direct-MAC/fixed-link configuration, two 1000BASE-X channels, and ownership of runtime port control. Section 4.1.2.12, p. 51 describes internal PHY polling; section 4.1.2.15, p. 52 describes EEPROM instructions executing at initialization and link changes. These behaviors must be reconciled with STM32 failover commands.
## 2026-09-16 — 24CSM01 pull-up calculation explanation

The 24CSM01 is under discussion; this explanation does not select it as a replacement for the previously listed 24AA014H or verify boot compatibility.

Reference: Microchip, *24CSM01 — 1-Mbit, 3.4 MHz I2C Serial EEPROM with 128-Bit Serial Number and Enhanced Software Write Protection*, DS20006781D (revision D, January 2025), [manufacturer PDF](https://ww1.microchip.com/downloads/aemDocuments/documents/MPD/ProductDocuments/DataSheets/24CSM01-1-Mbit-3.4MHz-I2C-Serial-EEPROM-DS20006781.pdf). Printed and viewer page numbers agree.

- Unnumbered diagram “System Configuration Using Serial EEPROMs,” p. 2: Rmax = tr(max)/(0.8473 CL); Rmin = (VCC - VOL(max))/IOL. The upper bound limits RC rise time; the lower bound ensures the low-driving output can sink the pull-up current.
- Engineering derivation: for exponential charging, time from 0.3 VCC to 0.7 VCC is RC ln(0.7/0.3) = 0.8473 RC. CL is the total capacitance of the individual bus net, including attached pins, routing, connectors, and probes.
- Table 1-1 “DC Characteristics,” D3, p. 4: EEPROM VOL(max) = 0.4 V at IOL = 2.1 mA for VCC >= 2.5 V. This is a guaranteed output test point, not EEPROM supply current or an absolute-maximum sink-current rating. At nominal 3.3 V, the EEPROM-derived minimum is 1.381 kohm.
- Table 1-2 “AC Characteristics,” parameter 4 and Note 1, p. 5: lists 1000 ns and explicitly directs use of applicable I2C timings. Choose the tightest requirement of the operating mode and attached devices, not 1000 ns unconditionally.
- Illustrative assumptions only: CL = 100 pF and allowed rise time = 300 ns give Rmax = 3.541 kohm. A 2.2 kohm resistor gives calculated rise time 186 ns and current 1.318 mA at VOL = 0.4 V. No resistor value is frozen; verify controller characteristics, supply/resistor tolerance, actual capacitance, and existing parallel pull-ups. Evaluate SDA and SCL separately; EEPROM SDA sink strength is not a specification for the controller's SCL output.

## 2026-09-16 - Mode-specific pull-up table

Created [Consolas-ready Altium text](24CSM01_PULLUP_ALTIUM.txt) for 3.3 V and assumed 100 pF per line. NXP UM10204 Rev. 7.0, Table 11 p. 44 establishes 1000/300/120 ns rise-time limits; Table 13 p. 47 establishes 80 ns Hs SDA limit at 100 pF. Printed/viewer pages agree; source links are in the text file. Calculated Rmax = 11.802/3.541/1.416/0.944 kohm, respectively, against EEPROM-derived Rmin = 1.381 kohm. Fast Plus has little tolerance margin; Hs SDA has no passive-resistor overlap under these assumptions. Hs SCL uses current-source assistance (section 5.3.1 pp. 34-35). No final component choice or Altium edit made.

## 2026-09-16 - EEPROM I2C pull-up placement guidance

Proposed layout guidance for a short, unbuffered on-board bus: place one SDA/SCL pull-up pair near the controller if convenient, with short branches to the routed nets. For the ROM EEPROM bus this is the LAN96455F controller end. Placement near the EEPROM is also acceptable when it gives cleaner routing; controller-side placement is a convention, not a manufacturer requirement. Do not duplicate populated pairs at both ends without recalculating parallel resistance. Use a pull-up rail compatible with both endpoints and their power sequencing. Buffers/level translators require their own segment-specific review.

Evidence: NXP technical support, “I2C pull up resistors location,” accepted answer by lukaszadrapa, 2021-10-13 (web response, no revision/page numbering), says placement makes little difference in the short connection discussed and controller-side placement is usual: https://community.nxp.com/t5/Other-NXP-Products/I2C-pull-up-resistors-location/m-p/1354940 . NXP UM10204 Rev. 7.0, section 7.1 “Pull-up resistor sizing,” Figure 41 and Equation 1, printed/PDF p. 50, establishes total wiring/connection/pin capacitance as the rise-time sizing basis: https://www.nxp.com/docs/en/user-guide/UM10204.pdf . Short-branch routing is our engineering recommendation. No placement or schematic file was changed.

## 2026-09-16 - Long-bus placement reasoning

Clarification: near-controller placement is a short-board convenience, not a universal signal-integrity optimum. In a lumped RC model, moving an unchanged pull-up does not remove any bus capacitance. For long wiring, series resistance/inductance and propagation delay make the endpoint waveforms differ; fast falling edges can require transmission-line analysis even at a low clock rate.

Circuit reasoning (assume common ground, negligible leakage, one pull-up, settled LOW): when pull-up and active sink are at opposite ends, current traverses the signal conductor and the pull-up-end receiver sees approximately VOL(sink) + I*Rwire. When pull-up and sink share an end, the remote high-impedance input draws negligible DC signal-wire current. Thus master-end pulls favor the LOW-level margin for master-driven traffic, while slave-end pulls favor slave-driven traffic. SDA direction reverses for reads/ACKs; SCL can reverse ownership with clock stretching. Ground offsets and transient behavior require separate checks. This is a circuit-model deduction, not a manufacturer placement mandate.

NXP AN10658 Rev. 01 (2008-02-26), section 4.1, Figure 8 and explanation, printed/PDF pp. 12-13, demonstrates receiver-end 150-ohm pulls with dedicated P82B96 split-direction cable drivers, approximately 100-ohm cable impedance and 30 mA drive. It establishes a specific termination/timing advantage, not a rule to put ordinary EEPROM pull-ups at the slave. Typical kohm EEPROM pulls are not matched cable terminations. Source: https://www.nxp.com/docs/en/application-note/AN10658.pdf . NXP UM10204 Rev. 7.0 section 7.5, printed/PDF p. 54, establishes wiring/crosstalk guidance including signal/ground twisted pairs; section 7.1 p. 50 establishes total-capacitance sizing. Source: https://www.nxp.com/docs/en/user-guide/UM10204.pdf .

Engineering recommendation: for actual long-cable operation inspect waveforms at both ends for both SDA transmitters, assess cable/return path, rise time, low margin and ringing, and select an appropriate buffer/extender if needed. Splitting one effective resistance into two endpoint pulls changes distributed behavior but does not eliminate total charging current/capacitance; account for combined sink current and supply compatibility. No project layout decision changed.

## 2026-09-16 - Backup EEPROM programming modularity

Reviewed the user-provided TWI Boot crop against local LAN96459F EDS2 Daughter Card User Guide DS50003971A (revision A), Figure A-5 “LAN96459F EDS2 Daughter Card RGMII and GPIO,” printed/PDF p. 34, and Table 3-5 “Boot Options,” p. 20. Local reference: ../eth_SW/docs/LAN96459F_EDS2.pdf. The depicted U5 is 24LC64, not the proposed 24CSM01; pinout, capacity, addressing and pull values must be rechecked when adapting it.

- U5 stays directly connected to switch TWI_SCL/TWI_SDA; the figure maps these to GPIO3/pin118 and GPIO2/pin119. C71 provides local decoupling, R79/R80 are reference-board bus pulls, and R58 pulls WP low for writable operation. These values are reference examples, not frozen project choices. In particular, the depicted 1 kohm pull-ups are below our calculated 1.381 kohm minimum for 24CSM01 at 3.3 V using its specified 2.1 mA/0.4 V guarantee.
- J1 BOOT_PROG exposes SCL, SDA and GND directly for an external programmer. It does not power the EEPROM, select boot mode, or isolate the switch. Arrange valid target power and hold the switch in reset while programming, following the bus-ownership principle shown by the reference.
- R83 (0 ohm) joins EEPROM SDA to host SODIMM_SDA while populated. J4 BPROG adds/removes the host SCL connection. The figure explicitly specifies holding the switch in reset with SW2 and fitting J4 for CPU-host programming. J4 open does not isolate SDA; there is no active bus mux or automatic arbitration in this block. Any host sharing SDA must avoid driving it incompatibly, even with J4 open.
- BOOT_MODE=000 leaves initialization to the external host; BOOT_MODE=100 starts internal ROM and reads the EEPROM table. Boot choice and programming access are independent. LAN9645xF Data Sheet DS00006065D, section 3.4.1.1 p. 42, Table 3-12 p. 43; local ../eth_SW/docs/LAN9645xF-Data-Sheet-DS00006065.pdf. MGMT_MODE=11 can remain selected for SPI access (EDS2 Table 3-5 p. 20), but ROM/STM32 runtime register ownership still needs validation.
- Our proposed adaptation can retain a direct programming header and optionally a STM32 I2C programming connection; SODIMM is just the evaluation-board host connector. Two disconnectable host links are an optional clearer-isolation alternative to the reference's single clock jumper. This is an option, not a final design decision. Neither method replaces separate BOOT_MODE straps or resets.

The backup preserves an alternate initialization/configuration path, not automatic MCU-failure detection or guaranteed replacement of STM32 optical failover control. No Altium circuit was changed or verified as implemented.

### BPROG naming clarification - 2026-09-16

DS50003971A Table 3-1, LAN96459F EDS2 Daughter Card Connectors, printed/PDF p. 18, explicitly lists J4 as BPROG and describes its I2C-clock connection for CPU-host EEPROM programming with the switch held in reset using SW2. It also lists J1 BOOT_PROG for an external programmer. The guide does not spell out BPROG; do not expand it as 'backup programming' or treat it as a standard I2C term. It is the reference board's connector name. Suggested project label EEPROM_PROG_EN would describe its purpose, but no rename was made.


## 2026-09-16 - EEPROM screenshot review: corrections required

Status: user-provided schematic/power-tree drafts reviewed; NOT approved. No native Altium source or compiled connectivity/ERC inspected or changed.

User intent: +3V3_ETH_SW_EEPROM aliases +3V3_MAIN upstream of the Ethernet load switch so EEPROM remains powered while switch rails are off. This alias is user-declared; the screenshots do not prove compiled net connectivity. The EEPROM selected in the draft is 24CSM01-I/SN, replacing the earlier 24AA014H working selection.

Findings and evidence:
1. Powered-off switch I/O violation: always-on SDA/SCL pulls remain connected through populated 0-ohm links to GPIO2/3. LAN9645xF DS00006065D section 5.2, printed/PDF p. 189 explicitly prohibits driving inputs without device power. Reset alone does not isolate an unpowered device. Preserve always-on EEPROM intent only with SDA AND SCL disconnected from the switch while its supplies are off (suitable isolation circuit with verified off-state behavior, or manual removal/opening of both switch-side links for controlled programming). A permanently populated zero-ohm resistor provides no protection. Recheck host pins if STM32 can be unpowered separately.
2. Address: both depicted 0-ohm straps to EEPROM VCC appear populated; 10 kohm pulldowns cannot override them. A2=A1=1 selects 7-bit 0x56/0x57, not required base 0x50. Mark high straps DNP or remove them, retaining A1=A2=0. EEPROM DS20006781D section 3.3/Tables 3-1 and 3-2, p. 10 gives 1010 A2 A1 A16; LAN datasheet section 4.1.2.15 p. 52 requires base 0x50. Bank addresses become 0x50/0x51. Verify EEPROM_SIZE=0 for 16-bit word addressing (LAN section 3.4.2.8 p. 46; EEPROM section 3.3.2 p. 11).
3. Pull-ups: 8.25 kohm at assumed 100 pF gives 699 ns rise time; this meets 1000 ns but exceeds Fast-mode's 300 ns. Rmax=3.541 kohm at 100 pF and 300 ns; EEPROM-based Rmin=1.381 kohm at nominal 3.3 V. Proposed 2.2 kohm gives 186 ns and 1.318 mA at VOL=0.4 V; final value must include all parallel pulls, tolerances, capacitance and both drivers. NXP UM10204 Rev. 7.0 Table 11 p. 44; EEPROM DS20006781D Table 1-1 D3 p. 4; LAN section 4.1.2.15 p. 52 requires EEPROM support of at least 400 kHz. 8.25 kohm permits only approximately 42.9 pF for 300 ns, before tolerance margin.
4. Pin review: EEPROM SOIC pin map matches DS20006781D Table 2-1 p. 8. LAN GPIO2/pin119 SDA and GPIO3/pin118 SCL agree with EDS2 DS50003971A Figure A-5 p. 34 and LAN Table 3-6 p. 32 for ROM/I2C mapping. Global net-label scope/ports must be checked in the actual project; screenshot labels alone cannot establish intersheet connectivity or rail aliases.
5. WP: open jumper plus 10 kohm pulldown allows writes in default legacy protection mode; closed jumper asserts WP, so EEPROM is not always writable. Enhanced software protection changes WP behavior (DS20006781D section 2.4 p. 8). BOOT_PROG is SCL/SDA/GND only; target must be powered. Host BPROG opens only SCL; SDA stays connected. Replace copied SW2 reference if not the real project reset control.
6. Restore the omitted table caveats: 100 pF assumption, very narrow Fast Plus range, and no passive Hs SDA overlap at that capacitance. Final unique designators, DNP population, no-connect marker on EEPROM NC, and ERC/netlist inspection remain pending.

Power-tree screenshot shows TPS54J061RPGR for both main bucks and W25Q256JVEIM TR for NOR, differing from prior context. Recorded as user-shown working candidates only; regulator circuits and exact NOR suffix were not verified in this EEPROM review. EEPROM 3 mA allocation is not the whole branch budget: include pull-up current and strap/WP resistor current. Broader power tree is not signed off by this review.

References: [LAN datasheet](../eth_SW/docs/LAN9645xF-Data-Sheet-DS00006065.pdf), [EDS2 guide](../eth_SW/docs/LAN96459F_EDS2.pdf), [24CSM01 DS20006781D](https://ww1.microchip.com/downloads/aemDocuments/documents/MPD/ProductDocuments/DataSheets/24CSM01-1-Mbit-3.4MHz-I2C-Serial-EEPROM-DS20006781.pdf), [NXP UM10204 Rev. 7.0](https://www.nxp.com/docs/en/user-guide/UM10204.pdf). Printed/viewer pages agree.


## 2026-09-16 - Address DNP clarification and switched-supply programming proposal

User clarified A1/A2 upper 0-ohm pulls are intended DNP, though not yet marked. Thus intended address is correct (0x50/0x51); prior address finding is a population-documentation issue, not an intended electrical error. Actual variant/BOM marking remains pending.

8.25 kohm is intentionally for Standard-mode programming. At assumed 100 pF, calculated 699 ns meets the 1000 ns Standard-mode rise-time limit (NXP UM10204 Rev. 7.0 Table 11 p. 44). It is not approved for the complete backup-boot bus: LAN DS00006065D section 4.1.2.15 p. 52 requires EEPROM support of at least 400 kHz. This requirement does not itself establish an exact ROM clock waveform; no verified way to force ROM boot to 100 kHz was found. Controller/programmer 100 kHz settings do not set ROM timing. Recommend 2.2 kohm as a candidate suitable for both 100/400 kHz under current capacitance assumptions, pending full electrical checks.

User asks about moving EEPROM supply to +3V3_ETH_SW_VDD_IO_A and powering the switch while holding reset for programming. Recommended proposal: put EEPROM VCC, its SDA/SCL pull-ups, and address/WP high-option supply on the same switched 3.3 V I/O-A rail. Power the full switch normally (including 1.15 V core), hold nRESET asserted, wait for rail/EEPROM readiness, use exactly one external or STM32 programmer, WP low in legacy mode, then complete/verify writes and release the programmer before switch reset release. EDS2 DS50003971A Table 3-1 p. 18 and Figure A-5 p. 34 explicitly document powered host programming with switch held in reset. LAN DS00006065D section 5.2 p. 189 prohibits signals into an unpowered switch. Both local PDFs are in ../eth_SW/docs/; prior source links apply.

This proposal eliminates the specific always-on EEPROM pull-up source when the switch is off, but an external programmer or independently powered STM32 must also leave SDA/SCL undriven and disable/isolate any of its always-on pull-ups then. Reset does not provide power-off isolation. No Altium change or acceptance of the hypothetical rail migration is claimed; the independent-power intent is under reconsideration.


## 2026-09-16 - Revised EEPROM screenshot: prior three findings addressed

Latest user screenshot shows 24CSM01-I/SN VCC, both 2.5 kohm 0.1% bus pulls, WP jumper high side, and optional address high straps on +3V3_ETH_SW_VDD_IO_A. Both A1/A2 upper zero-ohm options visibly say DNP; 10 kohm pulldowns give base 0x50 (upper bank 0x51). These resolve the previous schematic-level rail, address-population, and 100 pF/400 kHz rise-time findings, subject to actual Altium variant/netlist verification. This supersedes the prior upstream +3V3_MAIN EEPROM supply intent for the latest displayed revision.

Calculated at 3.3 V, 100 pF: 2.5 kohm gives 211.825 ns rise time, 1.16 mA sink current at VOL=0.4 V, and approximately 141.6 pF maximum for 300 ns before margins. At +5% rail and -0.1% resistance, current at VOL=0.4 V is approximately 1.227 mA. EEPROM 2.1 mA guarantee is met; verify switch/STM32 drivers and all parallel pulls. Choice supports Standard/Fast-mode at assumed capacitance, not 1 MHz/100 pF. Basis: DS20006781D Table 1-1 D3 p. 4; NXP UM10204 Rev. 7.0 Table 11 p. 44. References linked in preceding review.

Visible EEPROM pin map, grounded VSS, 100 nF bypass, open NC, programming header and WP topology remain consistent. Remaining release checks: actual DNP/BOM status, unique designators/ERC/net aliases, actual reset control replacing copied SW2 text, EEPROM_SIZE=0 in backup configuration, no external/STM32 pull-up or push-pull back-drive while switched domain is off. Program with complete switch supplies stable and nRESET held low; release programmer after write completion before reset release. WP high inhibits writes in legacy mode. Sources: DS20006781D Table 2-1 and section 2.4 p. 8, Tables 3-1/3-2 p. 10; DS00006065D section 3.4.2.8 p. 46, section 4.1.2.15 p. 52, section 5.2 p. 189; EDS2 DS50003971A Table 3-1 p. 18/Figure A-5 p. 34. Printed/PDF pages agree.

Review status: acceptable schematic draft under stated conditions; no native source changes, compiled verification, hardware validation or full-board sign-off. This screenshot does not show an updated power-tree drawing; update that drawing to move EEPROM branch downstream if not already done.

## 2026-09-16 - LOOP_EN strap versus runtime GPIO

Verified LAN9645xF DS00006065D section 3.4.2.1 p. 44 and section 4.1.2.10 p. 51: LOOP_EN=1 enables unmanaged MAC-learning-based loop prevention; repeated learning of one MAC on different ports can disable involved ports for 10 seconds, then retry. LOOP_EN=0 disables it. It applies to ROM/EEPROM backup 100; unmanaged straps are ignored for no-boot 000 (Table 3-12 p. 43). It is not the project's coordinated optical failover policy.

Pin is GPIO35/pin51, VDD_IO_E (Table 3-2 p. 22, Table 3-4 p. 29). Reset-time strap sampling is independent of subsequent GPIO alternate function (section 3.4 p. 42). EDS2 DS50003971A Figure A-5 p. 34 GPIO mapping lists GPIO35 as ALT2 MagJack P4.LED1, without a LOOP_EN label; this is runtime labeling, not evidence that strap sampling is absent or disabled. Exact reset voltage of its LED network was not established here. Internal strap pulls exist, but revision D requires explicit external pulls (Figure 3-2 p. 20 note; section 3.4 p. 42); do not copy an omitted reference strap as a requirement.

Project guidance: reserve an explicit selectable LOOP_EN strap using its associated switched VDD_IO_E rail for a high choice; choose backup policy deliberately. An automatic 10-second port shutdown may conflict with fast failover objectives, while disabling prevention does not make parallel forwarding paths safe. No final 0/1 value or resistor value selected and no Altium edits. Local references: ../eth_SW/docs/LAN9645xF-Data-Sheet-DS00006065.pdf and ../eth_SW/docs/LAN96459F_EDS2.pdf; printed/PDF pages agree.

## 2026-09-16 - Internal strap pulls and E2E_CLOCK_EN

DS00006065D section 3.4 p. 42 states all straps have internal pull-ups and are sampled at nRESET release; it also explicitly requires external PCB pulls. Missing visible reference pulls do not establish firmware configuration: an unloaded pin can be biased high by reset-default hardware without firmware, and connected LED/host circuits may affect its sampled voltage. Exact EDS2 LOOP_EN reset state remains unverified. Follow revision D external-pull requirement rather than infer a firmware workaround.

User's E2D_CLOCK_EN refers to E2E_CLOCK_EN. Section 3.4.2.2 p. 44 defines 0=disabled, 1=enabled, unmanaged-only stateless one-step end-to-end PTP transparent-clock processing: updates correction fields in Sync/Delay_Req frames for switch residence delay. It is not a reference-oscillator enable or a requirement for ordinary Ethernet/telemetry. Table 3-4 p. 28 assigns GPIO10 to VDD_IO_B. Recommend default low for our current non-PTP backup configuration; this remains a proposal, no resistor value or Altium population finalized. No-boot 000 ignores this unmanaged strap; ROM+EEPROM 100 uses unmanaged configuration (section 3.4 p. 42/Table 3-12 p. 43).

Also preserve the prior response's provisional LOOP_EN=1 recommendation for backup 100: a safety net against accidental parallel-path forwarding, not the normal optical failover algorithm. Backup should still have one optical forwarding path; 10-second loop response requires validation. Earlier note saying no proposed polarity is superseded by this provisional recommendation, not a final design freeze.

All references are to local ../eth_SW/docs/LAN9645xF-Data-Sheet-DS00006065.pdf revision D; printed/PDF pages agree.

## 2026-09-17 - EEE_EN backup recommendation

Verified DS00006065D section 3.4.2.4 p. 45: EEE_EN=0 disables, 1 enables Energy-Efficient Ethernet on CuPHY ports. Section 4.1.2.9 p. 50 says this applies to all internal CuPHY ports; it is not SFP optical power control. GPIO12 uses VDD_IO_B (Table 3-4 p. 28). Unmanaged strap applies to backup 100, ignored in no-boot 000 (section 3.4 p. 42/Table 3-12 p. 43). Recommend provisional default low for simpler latency/bring-up behavior; no measured latency penalty or power savings claimed, no final resistor selected. Local reference: ../eth_SW/docs/LAN9645xF-Data-Sheet-DS00006065.pdf; printed/PDF pages agree.

Also record previous FLOW_EN explanation: provisional enabled for backup advertisement, subject to partner negotiation and congestion/latency validation (section 3.4.2.3 p. 44). Neither recommendation is an implemented strap change.


## 2026-09-17 - Switchable 1 kohm/10 kohm strap DC margin check

Reviewed user screenshot: each pin net is at junction of permanent 10 kohm pulldown and 1 kohm path to DIP switch/associated 3.3 V rail. Closed switch gives nominal 3.000 V ignoring internal pull-up. LAN DS00006065D Table 5-7 p. 192 VIS thresholds: VIH(min)=0.63 VARIO (2.079 V at 3.3 V), VIL(max)=0.39 VARIO (1.287 V). HS_I rows p. 194 specify 0.6/0.4 VARIO in RGMII mode. Using stricter VIS values gives adequate margin for displayed network and proposed ordinary VIS/IS strap networks.

Conservative closed-switch calculation at 3.3 V: 1.01 kohm upper, 9.9 kohm lower, 10 uA leakage sinking, and ignoring helpful internal pull-up yields 2.9853 V. Open-switch calculation with strongest internal pull-up 58.9 kohm, 10.1 kohm external pulldown and 10 uA leakage sourcing yields 0.5693 V. Table 5-7 pp. 192-194 gives internal pull-up 58.9-83.7 kohm and leakage; Note 5-6 p. 194 says leakage excludes pull-resistor current. Thus DC high/low pass under stated model, not full timing/layout sign-off. Pull rail must track the pin supply; checks also pass across +/-5% common supply using these models. No attached external output is assumed driving at reset.

For extension to other straps, use each pin's domain (Table 3-4 pp. 28-29): GPIO10/11/12 E2E/FLOW/EEE use B; GPIO34/35 JTAG/LOOP use E; GPIO33 EEPROM_SIZE uses D. Do not indiscriminately reuse C across switched/independent domains. Same-voltage names do not prove same physical net. Runtime alternate-function loads, reset setup/hold and other connected outputs require review. RGMII straps remain shunt loads; a closed 1 kohm pull can draw about 3.3 mA when its output drives low. Keep shunt branches short as reference Figure A-5 notes. No resistor is to be inserted in series with the RGMII signal itself. Local source: ../eth_SW/docs/LAN9645xF-Data-Sheet-DS00006065.pdf revision D, printed/PDF pages agree.

Screenshot default 000/SPI and selectable 100 backup are consistent with prior intent. No new final strap populations or native Altium changes claimed.

## 2026-09-17 - LED_MODE explanation and provisional default

DS00006065D section 3.4.2.5 and Tables 3-13/3-14 p. 45: with two LED outputs per CuPHY, 0 selects separate link/activity indications, 1 selects speed-encoded combinations (Table 3-15 p. 46). With only one configured LED output per port, single-LED link/activity behavior is automatic regardless of strap; physically fitting two LEDs does not ensure two GPIO outputs are configured. Tri-color means combinations; physical colors depend on wiring. Recommend provisional 0 for easy bring-up interpretation, pending RJ45/LED and GPIO selections. GPIO13/pin100, domain B (section 3.4.2.5 p. 45, Table 3-4 p. 28). Use 10 kohm pulldown and optional switch/1 kohm pull to corresponding VDD_IO_B as previously reviewed, subject to runtime load checks. Strap is unmanaged-only, sampled at reset/boot, and not changeable live through DIP switch; 000 uses software configuration. Table 3-14 column names LED1/LED2 conflict with notes LED1/LED0: physical activity/link pin mapping must be resolved before wiring, rather than inferred from the table headings. Local source: ../eth_SW/docs/LAN9645xF-Data-Sheet-DS00006065.pdf; printed/PDF numbering agrees. No final population or native source edits.

## 2026-09-17 - Project port numbering and candidate copper LED pins

Verified DS00006065D Table 4-4 p. 54: our 2x CuPHY + 2x SerDes + 1x RGMII LAN96455 combination activates DEV0/CuPHY0, DEV1/CuPHY1, DEV5/SerDes0, DEV6/SerDes1 and DEV7/RGMII0. Proposed role mapping is laptop on CuPHY0 and optional debug RJ45 on CuPHY1, not CuPHY2. A label 'second connector' or J2 does not mean switch port 2. Final copper population remains pending. Section 4.2/Table 4-3 p. 53 defines port 9 as internal CPU injection/extraction/VRAP port with no physical port module; it is neither a copper jack nor the external STM32 RGMII connection (DEV7 here).

Candidate LED mapping suitable for managed ALT2 and unmanaged GPIO Mode A ROM+I2C table: laptop PHY0_LED0 = EGPIO49 pin44, PHY0_LED1 = EGPIO50 pin45; debug PHY1_LED0 = EGPIO47 pin113, PHY1_LED1 = EGPIO48 pin46. Sources: managed Table 3-5 p. 31, unmanaged Table 3-6 p. 33, package Table 3-2 pp. 21-22. EGPIO47 domain A; EGPIO48/49/50 domain E (Table 3-4 p. 29). MGMT_MODE=11 avoids the MIIM-client conflict on EGPIO48-50, but actual firmware/GPIO/EEPROM pin setup still needs verification. Do not label physical LED0/LED1 as definitive link/activity yet: Table 3-14 numbering inconsistency remains open. LED_MODE GPIO13 is a selection input at reset, not an LED drive output. LED wiring/polarity/current resistance remains unreviewed. Local source ../eth_SW/docs/LAN9645xF-Data-Sheet-DS00006065.pdf revision D; printed/PDF pages agree.

## 2026-09-17 - One physical copper status LED per populated port

User chooses simple single-LED indication: off=no link, on=link, blink=activity. Proposed hardware uses one active-low LED and series resistor per populated copper port: associated switched VDD_IO rail -> resistor -> LED anode/cathode -> selected LED output. Retain PHY0_LED0 on EGPIO49/pin44/domain E and optional PHY1_LED0 on EGPIO47/pin113/domain A as candidate routes from earlier mapping. Exact combined link/activity register programming remains to verify; ALT2 routing by itself does not select LED behavior. Leave unused PHY_LED1 outputs unconnected unless another use is explicitly configured. Do not join LED0 and LED1 outputs.

Illustrative resistor: 1 kohm gives about 1.1 mA for 3.3 V, assumed LED Vf=2.0 V and output low=0.2 V. Final resistor and LED MPN depend on Vf/brightness and output drive setting; not a manufacturer-mandated value. Use switched domain E for EGPIO49 LED supply, A for EGPIO47, to avoid always-on LED sourcing into off pins. GPIO domains: DS00006065D Table 3-4 p. 29; functions: Table 3-5 p. 31/Table 3-6 p. 33; pin numbers Table 3-2 pp. 21-22.

Critical backup caveat: section 3.4.2.5/Table 3-13 p. 45 specifies automatic single-LED behavior only when one LED output is available in the GPIO configuration. Our 156-pin GPIO Mode A Table 3-6 p. 33 exposes two PHY LED outputs per copper port. Simply omitting one LED or strapping LED_MODE=0 does not establish combined behavior. Section 4.1.1 p. 49 generically describes one-status-LED Mode A while package-specific table lists pairs; treat the specific mapping and explicit p. 45 condition as limiting assumptions. Keep LED_MODE low as provisional default, but verify/configure PHY combined LED behavior in STM32 initialization and EEPROM backup table before claiming matching behavior for both boot paths. No exact PHY register sequence validated here. Local reference ../eth_SW/docs/LAN9645xF-Data-Sheet-DS00006065.pdf revision D; printed/PDF pages agree.

## 2026-09-18 - CUPHY_MODE recommendation

Recommend CUPHY_MODE=0 for gigabit laptop and optional debug copper ports. DS00006065D section 3.4.2.6 p. 46 specifies 0=auto-negotiation/Auto MDI-X, all speeds advertised; 1=only 100FD/HD and 10FD/HD advertised. Applies to integrated CuPHYs, not external PHY advertising or SFP speed. Low permits gigabit negotiation rather than forcing gigabit. Pin GPIO14/pin99 per Table 3-2 p. 21; domain VDD_IO_B per Table 3-4 p. 28. Proposed 10 kohm external pulldown, optionally existing switch/1 kohm path to associated switched VDD_IO_B for testing 10/100-only. This is an engineering choice, not a manufacturer-mandated resistance. Section 3.4 p. 42 requires external strap bias and reset-release sampling. Table 3-12 p. 43: unmanaged strap ignored in normal no-boot 000, so STM32 must separately configure PHY advertisement; relevant to backup ROM+I2C 100. No Altium change or final population claimed. Reference: ../eth_SW/docs/LAN9645xF-Data-Sheet-DS00006065.pdf (revision D, printed/PDF pages agree).

## 2026-09-18 - SFP_EN recommendation

Recommend SFP_EN=0 for the selected fixed 1000BASE-SX optical paths. DS00006065D section 3.4.2.7 p. 46 defines this as 2.5G speed detection: 0 disables detection, 1 enables retry at lower speeds when 2.5G link fails (1000BASE-X for optical, SGMII for copper SFPs). It is not module power or TX_DISABLE control and setting it low alone does not configure 1000BASE-X. Configure intended SerDes speed/mode separately in STM32 initialization and backup EEPROM. Section 4.1.2.5 p. 50 summarizes retry behavior, with wording about termination differing from p. 46; use p. 46 for the definition and do not rely on retry behavior for our fixed 1G choice. GPIO15/pin98 (Table 3-2 p. 21), VDD_IO_B (Table 3-4 p. 28). Proposed external 10 kohm pulldown, optional existing 1 kohm/DIP high path to switched VDD_IO_B; switch off by default. Section 3.4 p. 42 and Table 3-12 p. 43: reset sampled, unmanaged strap used for backup 100 and ignored in no-boot 000. No native schematic change or final population claimed. Source: ../eth_SW/docs/LAN9645xF-Data-Sheet-DS00006065.pdf revision D; printed/PDF pages agree.

## 2026-09-18 - EEPROM_SIZE for 24CSM01

Retain EEPROM_SIZE=0 (16-bit word addressing) for backup BOOT_MODE=100. LAN DS00006065D section 3.4.2.8 p. 46 defines 0=16-bit, 1=8-bit; this is the word-address byte count, not I2C device address width or EEPROM capacity. 24CSM01 DS20006781D Figure 7-2 p. 20 shows two word-address bytes A15:A0; A16 is in the device-address byte (Table 3-2 p. 10). Full 128 KiB bank access by ROM has not been validated; choosing this strap alone does not establish it. EEPROM reference: https://ww1.microchip.com/downloads/aemDocuments/documents/MPD/ProductDocuments/DataSheets/24CSM01-1-Mbit-3.4MHz-I2C-Serial-EEPROM-DS20006781.pdf

Pin GPIO33/pin74, domain VDD_IO_D (LAN Tables 3-2 p. 22 and 3-4 p. 29). Proposed fixed 10 kohm pulldown; optional existing switch high path must use associated VDD_IO_D, not domain B. Ignored in no-boot 000 (Table 3-12 p. 43). Section 3.4.2.8 includes an unresolved contradictory sentence that configuration is unnecessary when EEPROM is only used for booting, while section 3.4.1.1 p. 42 explicitly requires setting addressing mode for ROM+I2C boot. Preserve explicit low per address-format definition; do not interpret the sentence as permission to float. No native edits. LAN local source: ../eth_SW/docs/LAN9645xF-Data-Sheet-DS00006065.pdf. Printed/PDF pages agree.

## 2026-09-18 - GPIO_MODE recommendation

Recommend GPIO_MODE[1:0]=00 (Mode A) as the backup strap baseline consistent with candidate RGMII0/LED mapping, not a complete validated EEPROM configuration. DS00006065D section 3.4.2.9 and Table 3-16 p. 47: 00=A (2 RGMII, 1-2 SGMII), 01=B (0-1 RGMII, QSGMII), 10=C (2 RGMII, no SGMII), 11=D (0-1 RGMII, 0-2 SGMII). These are family interface options, not permission to exceed LAN96455F five-port combinations. SFP SerDes usage is selected/configured separately (section 3.4.2.11 p. 47). Mode A Table 3-6 pp. 32-33 supplies RGMII0 and previously proposed copper LED mapping; changing to D requires a fresh pin-mapping review.

GPIO_MODE0=GPIO36/pin50 and GPIO_MODE1=GPIO37/pin49, both VDD_IO_E (Tables 3-2 p. 22 and 3-4 p. 29). Proposed 10 kohm pulldown on each, optional existing switched 1 kohm high paths to associated VDD_IO_E, both switches off. GPIO mode selects a bundle of alternate functions, not a per-pin input/output direction setting. Runtime functions still require review before schematic release.

Crucial boot precedence: Table 3-12 p. 43 states normal no-boot 000 ignores unmanaged straps; in backup ROM+I2C 100, EEPROM overrides GPIO_MODE for port topology, while ROM still uses GPIO_MODE for LED mapping/runtime mode. Explicitly encode valid 2CuPHY+2SerDes+RGMII0 topology in backup table (Table 4-4 p. 54); 00 alone does not implement it. No changed frozen decision or native Altium edits. Reference: ../eth_SW/docs/LAN9645xF-Data-Sheet-DS00006065.pdf revision D; printed/PDF pages agree.

## 2026-09-18 - Mode D comparison and conditional revised recommendation

User proposes Mode D to match one RGMII plus two SerDes. Revise prior Mode A preference: Mode D (GPIO_MODE=11) is a natural candidate, conditional on resolving SPI/SFP control overlap and backup ownership; interface count alone does not make A invalid. Optical links use 1000BASE-X, not SGMII protocol. DS00006065D section 3.4.2.9/Table 3-16 p. 47 and Table 4-4 p. 54 support interface/topology distinction.

Table 3-9 pp. 35-36 compared with Mode A Table 3-6 pp. 32-33 (156-pin ROM+I2C):
- PHY0_LED0: EGPIO49/pin44 -> GPIO42/pin39; PHY0_LED1: EGPIO50/pin45 -> GPIO43/pin36.
- PHY1_LED0: EGPIO47/pin113 -> GPIO40/pin40; PHY1_LED1: EGPIO48/pin46 -> GPIO41/pin41.
- All proposed Mode D copper LED pins are domain E (Tables 3-2 pp. 21-22/3-4 p. 29). GPIO40 also SD_MODE1, GPIO41 XMII_MODE0; reset LED loading must preserve strap levels. These pins overlap JTAG functions (Table 3-5 p. 31); reserve JTAG_ENB=1 for LED use or explicitly isolate/remap for JTAG debug. Mode D EGPIO44-50 have no default assignment. Managed firmware can route new copper pins through ALT2 (Table 3-5 p. 31). Single physical LED still does not prove combined indication with two mapped outputs.
- RGMII0 stays GPIO16-27; EEPROM I2C1 stays GPIO2/3; SYS_RDYn remains GPIO33. Mode D has no RGMII1. XMII_MODE0=0 selects RGMII0; XMII_MODE1 has no Mode D assignment in Table 3-17 p. 48 (retain explicit strap bias).
- Mode D default SFP sidebands: GPIO4 MOD_DET_1, GPIO5 MOD_DET_0, GPIO6 TX_DIS_1, GPIO7 SD0, GPIO8 SD1, GPIO11 TX_DIS_0. GPIO13 MAC_LED6 and GPIO14 MAC_LED5; UART GPIO9/10. I2C0 SDA GPIO30, SCL0 GPIO31, SCL1 GPIO32. Table 3-9 p. 36. SD signal polarity versus SFP LOS requires separate verification.
- SPI client ALT6 on GPIO4-7 (Table 3-5 p. 30) overlaps Mode D sidebands. These roles cannot coexist on those same pins; do not blindly wire table and SPI together. Exact ROM+I2C+MGMT11 precedence/remapping not established. Preserve STM32 SPI requirement and decide which sidebands are omitted, isolated, or remapped with verified initialization. Also reconcile switch I2C0/ROM SFP probing with STM32 DOM mux and TX_DISABLE ownership; no two uncontrolled output drivers. Existing architecture does not automatically give ROM access to SFP EEPROMs.
- SGPIO_EN unused in D (section 3.4.2.10 p. 47). SD_MODE=11 is SFP strap intent (section 3.4.2.11 p. 47), not enough for boot100: Table 3-12 p. 43 gives EEPROM precedence for port topology/selected SerDes exposure but retains GPIO_MODE influence on LEDs. Need explicit DEV0/1/5/6/7 topology and 1000BASE-X settings, GPIO/ownership initialization, then bench validation. No-boot000 must explicitly configure matching runtime pins.

If adopted, drive GPIO36/pin50 and GPIO37/pin49 high from associated VDD_IO_E; existing 10k-down/1k-switch-high proposal means both switches ON. This is a proposed change, not native implementation or complete backup approval. Source: ../eth_SW/docs/LAN9645xF-Data-Sheet-DS00006065.pdf revision D, printed/PDF pages agree.

## 2026-09-18 - Strap sampling versus ROM/table/runtime ownership clarification

DS00006065D section 3.4 p. 42/Table 3-12 p. 43 distinguish latch sampling from use. Boot0 ignores unmanaged straps including GPIO_MODE, but BOOT_MODE, MGMT_MODE, JTAG_ENB and SD_125M remain applicable. Managed firmware programs available per-pin ALT selections (section 3.3.1/Table 3-5 pp. 29-31), not arbitrary signals on arbitrary GPIOs. Ignored functional strap does not waive explicit all-strap external bias requirement in section 3.4 application note p. 42 and Figure 3-2 note p. 20; retain pulls especially for supported boot4 backup. VDD_IO domains specify electrical supply, not which settings apply in a boot mode.

Boot4 executes ROM continuously with external configuration instructions; it is not simply table initialization followed by STM32 ownership. GPIO_MODE is overridden for topology but retained for LED mapping/runtime mode (Table 3-12 p. 43). Other straps are not blanket ignored; EEPROM_SIZE outside domain C is required to read the table (section 3.4.1.1 p. 42). Section 4.1.2.15/Table 4-2 p. 52 explicitly permits switch/PHY register read-modify-write commands and GPIO alterations, executed at initialization OR link-up/down. Thus prior suggestions of start-up-only table execution would be wrong. Runtime PHY monitoring continues (section 4.1.2.12 p. 51). External SPI writes may be possible but cannot be assumed to take persistent precedence over ROM/event table reconfiguration; define ownership, ordering, supported writable fields, safe sequencing and avoid remapping the active management interface.

This also qualifies prior Mode D LED migration: GPIO40-43 are default Mode D ROM mappings, not necessarily immutable PCB requirements if a verified custom table/firmware configuration safely maps otherwise. Exact ROM mapping order and SPI/SFP sideband coexistence remain unverified; cannot assume later EEPROM writes erase reset/initialization conflicts.

Correction to earlier EEPROM capacity uncertainty: section 4.1.2.15 p. 52 explicitly documents up to 128 KB using incremented I2C addresses from base0x50. ROM bank access is therefore documented; actual 24CSM01 table/programming path still untested. Earlier 'full-bank ROM support unverified' should be understood only as no bench validation, not absence of documented support.

All sources: ../eth_SW/docs/LAN9645xF-Data-Sheet-DS00006065.pdf revision D; printed/PDF pages agree. No circuit changes or finalized register/table sequence.

## 2026-09-18 - EDS2 ALT table does not identify boot ownership

Verified EDS2 User Guide DS50003971A Figure A-5 p. 34 (../eth_SW/docs/LAN96459F_EDS2.pdf): same sheet lists boot0 and boot4 with external client interfaces and a GPIO#/Alt_mode/Connected_to/Signal wiring table. ALT numbers are per-pin mux choices; they do not prove managed boot. DS00006065D section 3.4.1.1 p. 42 categorizes boot4 as unmanaged ROM+I2C; section 4.1.2.15/Table 4-2 p. 52 permits EEPROM register/GPIO alterations and event execution. Therefore board supports host access with ROM boot, and a custom mapping can be established by ROM/table/host; schematic alone does not identify exact writer, timing or flashed table. Register-configurable GPIO hardware is common to both use cases.

Reference schematic avoids the literal Mode D overlap by wiring GPIO4-7 for SPI, module detects GPIO28/29, SFP1 TXDIS EGPIO44, SFP0 SD GPIO42 ALT6 (Figure A-5 p. 34). These demonstrate physical routing choices, not a verified ROM initialization recipe. Need matching table/firmware to adopt them, and verify pre-override behavior. EGPIO49 row shows ALT1 P0.LED0/(MIIM MDC); revision D Table 3-5 p. 31 differentiates ALT1 MIIM MDC versus ALT2 PHY0_LED0, so do not copy row as LED ALT1. Reference is nine-port LAN96459F; retain LAN96455F topology constraints. No Altium changes.

## 2026-09-18 - Night-session schematic snapshot and handoff

User supplied a full-sheet Altium screenshot at session close showing the ETH SW Config Sheet: boot/management table, selectable main straps, RGMII0/SPI/I2C signal labels, EEPROM/programming circuitry, and LAN96455F GPIO symbol groups. Additional strap-network alternatives are visible outside the page boundary on the right; treat these as working alternatives, not a frozen population or proof they are excluded from electrical connectivity. The image is archived in images/2026-09-18/windows-altium-control-sheet-session-close-5e9c8b78.png. Full-sheet screenshot permits documentation of progress but does not substitute for native connectivity, DNP/variant, or ERC inspection. Unannotated designators and unfinished pin assignments remain visible; no schematic release claimed.

Resume with a single pin-allocation table showing physical pin/domain, reset strap and intended value, normal boot0 runtime function, backup boot4 runtime function, and signal owner. Resolve Mode D/custom reference-style mapping, SPI GPIO4-7 sideband overlap, ROM versus STM32 SFP I2C/TX_DISABLE ownership, and copper LED behavior before final routing. Existing component recommendations and source references above remain in force; Mode D is a candidate, not a verified final EEPROM configuration. No Altium file was changed by Codex in this logging task.

## 2026-09-20 - Plain-text GPIO allocation table for Altium

Created [LAN96455_GPIO_ALTIUM.txt](LAN96455_GPIO_ALTIUM.txt), a Consolas-friendly table covering GPIO0-43 and EGPIO44-50. It is reconstructed from shared project notes rather than claimed as the exact lost side-chat table. EEPROM ALT3, SPI ALT6 and RGMII0 ALT1 reflect current working assignments; candidate SFP bus/IRQ/earlier LED routes are explicitly starred, and unresolved runtime pins are TBD. Rechecked ALT availability against DS00006065D Table 3-5 pp. 30-31. Included separate Mode D LED alternatives and boot4 caveats; no new routing or population is frozen. Source links/references are inside the text file. No native Altium changes.

## 2026-09-20 - SD_MODE explanation and proposed values

Recommend SD_MODE[1:0]=11 as SFP strap intent for our two optical paths. These are independent per-SerDes selections, not a four-mode code: SD_MODE0 controls SerDes0 and SD_MODE1 controls SerDes1; 0=SGMII/external copper PHY via MIIM, 1=SFP (-X/-KX) via I2C. Applicable to GPIO modes A/D, unmanaged operation. Source: DS00006065D section 3.4.2.11 p. 47. Our 1000BASE-SX modules use a 1000BASE-X host interface; this setting alone does not establish speed, and the section notes SFP module EEPROM reading determines mode/speed. Distinguish SFP module EEPROM from board boot EEPROM.

GPIO39/pin42=SD_MODE0 and GPIO40/pin40=SD_MODE1, both VDD_IO_E (Tables 3-2 p. 21 and 3-4 p. 29). Existing 10k-down/1k-switched-high proposal: both switches ON for 11, using associated switched VDD_IO_E. This is an engineering proposal pending runtime loading checks, particularly GPIO40 if used as a copper LED output. Table 3-12 p. 43: boot0 ignores this unmanaged strap; boot4 table overrides GPIO exposure/topology-related effects, so explicitly configure intended SerDes topology/behavior in EEPROM rather than rely on 11 alone. SFP access/ownership unresolved as previously recorded. No native schematic edits or final strap-population freeze. Local source: ../eth_SW/docs/LAN9645xF-Data-Sheet-DS00006065.pdf, revision D; printed/PDF pages agree.

## 2026-09-20 - XMII_MODE for STM32 RGMII0

Recommend XMII_MODE0=0 for the STM32 RGMII0 interface. DS00006065D section 3.4.2.12 pp. 47-48/Table 3-17 p. 48: mode0 bit selects 0=RGMII0, 1=RMII0. In GPIO A/C, XMII_MODE1 selects RGMII1/RMII1; in B it instead selects CuPHY4 versus RGMII0; in D no XMII_MODE1 function is listed and RGMII1 is not used. For proposed D, suggest external low bias on XMII_MODE1 as a defined unused strap, not a required functional selection. GPIO41/pin41=XMII_MODE0 and GPIO38/pin47=XMII_MODE1, both domain E (Tables 3-2 pp. 21-22/3-4 p. 29). Proposed 10 kohm pulldown each / both existing DIP switches OFF, subject to runtime loading if GPIO41 becomes PHY1_LED1. Unmanaged-only per Table 3-17; boot0 requires software RGMII setup; boot4 needs matching EEPROM topology and interface configuration. These straps alone do not establish STM32 MAC-to-MAC fixed-link compatibility or clock-delay settings. Reference ../eth_SW/docs/LAN9645xF-Data-Sheet-DS00006065.pdf revision D, printed/PDF numbering agrees. No Altium edits or frozen population decision.

## 2026-09-20 - Runtime SD0/SD1 versus SD_MODE straps

SD0/SD1 are runtime signal-detect inputs for corresponding SerDes/SFP paths, not reset configuration bits (DS00006065D Table 3-11 p. 41). Table 3-5 pp. 30-31 permits SD0 on GPIO7 ALT3 or GPIO42 ALT6; SD1 on GPIO8 ALT3 or GPIO43 ALT6. For our SPI GPIO4-7 allocation, reference-style candidates are SD0=GPIO42/pin39/domain E ALT6 and SD1=GPIO8/pin107/domain B ALT3; do not use GPIO7 for SD0 while it carries SPI clock. GPIO42 would then conflict with default Mode D PHY0_LED0; retain custom LED/mapping review. EDS2 DS50003971A Fig. A-5 p. 34 labels these paths S0.LOS and S1.LOS. Mapping proposed, not frozen; standalone GPIO table remains TBD pending acceptance.

Selected Finisar FTLF8519P3xyL specification Rev C1 (2014-08-14), section I p. 2 pin8/Note4 states LOS open-collector, 0 normal and 1 loss, with host 4.7k-10k pullup to 2.0-3.6V. Do not infer LAN SD active polarity from signal name: exact switch inversion/configuration still unverified. Proposed 3.3V pullup must suit receiving domain and off-state sequencing; no unconditional wire/rail release. LOS represents received optical signal indication, not proof of a valid Ethernet link or remote health. Sources: ../eth_SW/docs/LAN9645xF-Data-Sheet-DS00006065.pdf; ../eth_SW/docs/LAN96459F_EDS2.pdf; ../sfp/FTLF8519P3xyL_RevC1_8-14-14.pdf. Printed/PDF pages agree. No native edits.

## 2026-09-20 - Six-screenshot Section 3 review

Reviewed user-supplied RGMII, main straps, EEPROM, GPIO30-50, SPI, and E/B strap screenshots. GPIO16-27 RGMII labels, GPIO4-7 SPI client, GPIO2/3 EEPROM, EGPIO47-50 PHY1/0 LED labels and GPIO33 EEPROM_SIZE mapping are consistent with DS00006065D Tables 3-2 pp.21-22 and 3-5 pp.30-31. Screenshot net-label matching appears consistent, not native netlist validation. Boot0 per-pin configuration is feasible; boot4/custom GPIO mapping is not validated. No full-board signoff.

Pull domains correct as drawn: main switch positions1-7 domain C; position8 separate domain D; GPIO34-41 E; GPIO10-15 B; EEPROM VCC/bus/address/WP high options A. Ensure main position8 supply wire really remains isolated from C in native netlist. Basis Table 3-4 pp.28-29 and section3.4 p.42. Shunt 1k/10k networks remain runtime loads; validate output drive/timing/layout. No shorted GPIO straps shown.

Corrections / qualifications:
- Main default note omits position8; use ON5,6 / OFF1-4,7,8. Backup ON4-6 / OFF1-3,7,8. GPIO_MODE D E-block proposal ON1-6/OFF7,8 corresponds JTAG disabled, LOOP enabled, GPIO_MODE11, SD_MODE11, XMII00; B-block ON2/OFF1,3-6,7/8 unused. These are proposed backup-intent settings, not ROM validation; boot0 ignores all B straps and E straps other than JTAG_ENB. Paste text saved in LAN96455_SWITCH_DEFAULTS_ALTIUM.txt.
- Two-pin MIIM MDC/MDIO header is a signal-access header, not a shunt jumper. Shorting it joins clock and data. MIIM0 is switch host/controller interface, not MIIM client management; keep runtime ALT2 only if intentionally using that controller. If no MIIM0 use, GPIO28 is strap-only and name should reflect that. Do not force a rename if interface is retained. Table3-5 pp.30-31/Table3-11 p.40.
- SPI SI_DI/DO names are switch-perspective: DI=STM32 MOSI, DO=STM32 MISO. Optional SI_SLV naming improves clarity. GPIO4-7 cannot also be default Mode D SFP signals. RGMII switch RX goes to STM32 TX and vice versa.
- GPIO33 EEPROM_SIZE at reset may be IRQ0 ALT2 in boot0 or SYS_RDYn in ROM mode; not both simultaneous outputs. GPIO40/41 similarly select a single runtime output, not combined copper+SFP LED functions. Table3-5 p.31/Table3-11 p.41.
- EGPIO47/48 PHY1_LED0/1 and EGPIO49/50 PHY0_LED0/1 all ALT2; EGPIO47 pull/LED supply A, others E. Labels do not prove LED hardware population or combined behavior. Single-LED decision not superseded by four net labels. Section3.4.2.5 p.45.
- EEPROM high address options changed from prior 0R to 10k DNP. DNP low-address arrangement remains consistent; fitting a 10k high option while keeping 10k low creates half-supply, not a valid intentional high. Opposing pull must be depopulated/reworked for high address. WP note assumes legacy protection mode. Existing 2.5k/100pF calculation remains conditional. BPROG remains SCL-only; update SW2 placeholder to actual reset reference. nRESET circuit not shown here.
- _R names on opposite sides of resistors appear distinct; do not merge labels around isolation resistors. SYS_RDYn/IRQ0, SFP I2C bus sharing, unshown reset, unique annotation and native ERC remain open.

Sources: ../eth_SW/docs/LAN9645xF-Data-Sheet-DS00006065.pdf revision D sections3.3-3.4 pp.29-48 (printed/PDF pages agree); existing EEPROM references above. Native Altium not edited or compiled. Original evidence and captions in notebook.

## 2026-09-20 - Revised three-screenshot progress review

Latest user screenshots show the horizontal 0/1 switch tables applied, including main position8 default0; domain E 11111100 and B 010000-- match proposed settings. Main default00001100 / backup00011100 are ordered by switch positions1-8, not binary boot-field order. Domain C/D split and E/B/A pulls remain visually consistent with DS00006065D Table3-4 pp.28-29. GPIO28 is now ETH_SW_MGMT_MODE1 with the former MIIM header absent, addressing prior ambiguity. SPI, RGMII and EEPROM ports plus 0R series links and EGPIO47-50 LED ports are now shown. Net names match visible endpoints; global sheet connectivity is not verified. Runtime ALT mapping remains boot0-feasible per Table3-5 pp.30-31, and boot4 table/ownership unresolved per Table3-12 p.43.

One new annotation correction: EEPROM note says standard placement is near master. Change to 'Place pull-ups on the shared bus with short connections; verify total bus capacitance and rise time.' Master-end placement is not a universal I2C requirement; our master also changes during programming. Prior RC calculation (100pF,2.5k ->212ns) supports the electrical check, not a mandated physical location.

Outstanding: actual reset reference still SWx and reset circuit absent from crop; Orange IRQ/SYS_RDYn and SFP bus/function comments remain questions, not decisions. A1/A2 upper10k DNP with lower10k must not both be populated to select high. GPIO40/41 alternatives and GPIO33 IRQ/ready must each resolve to one runtime function at a time. Two LED net exports do not establish changed single-indicator population. JTAG=1 applies in boot0 despite neighboring unmanaged strap note. No native Altium edits/ERC or final board approval. Original screenshots archived and captioned in notebook.

## 2026-09-20 - Manufacturer review questions prepared

Ethan reports contact established with a Microchip FAE. [Draft review email](correspondence/2026-09-20_microchip_fae_design_review_draft.md) consolidates outstanding firmware-dependent schematic questions and a complete grouped GPIO proposal. Prioritizes supported non-Linux initialization and hardware forwarding under boot0, possible customer firmware on internal controller, boot4 ROM/host ownership, custom ALT mapping, fixed-link RGMII0 and SFP bus/control ownership. No email sent or answers received in this task; assignments remain provisional.

## 2026-09-21 - Reset supervisor reference investigation

Microchip LAN96459F EDS2 Daughter Card User Guide DS50003971A Figure A-4, printed/PDF p.33 (../eth_SW/docs/LAN96459F_EDS2.pdf), visually checked: U3 is MIC2774N/2.9V. VDD pin5 monitors 3V3; IN pin4 monitors core net 1V1 through R25+R26=20k upper and R27=10k lower, yielding nominal 0.9V core trip at 300mV IN. /RST pin1 connects LAN nRESET, 10k pullup R23 to VDDIO, red LED+1k, and test point. Host nSYSRESET drives /MR pin3. SW2 instead pulls /RST directly to ground through 100R; it is not connected to /MR and does not inherently receive the supervisor's post-MR release delay.

MIC2774 datasheet DS20006527A, Electrical Characteristics p.3, Table2-1 p.5 and section3 p.6 (https://ww1.microchip.com/downloads/en/DeviceDoc/MIC2774-Dual-Micropower-Low-Voltage-Supervisor-DS20006527A.pdf): N output is active-low open drain; IN threshold 295/300/305mV; reset timeout 140ms minimum, 280ms maximum; /MR has internal pullup. Fixed 2.9V option is nominal 2.93V. Supervisor asserts reset for either monitored undervoltage or /MR low, releasing after valid conditions and timeout.

Candidate adaptation, not frozen: monitor switched switch 3.3V and 1.15V, pull /RST to VDD_IO_A, use STM32 open-drain /MR request and optionally button on /MR for delayed release. Recalculate core divider from rail limits/tolerances rather than adopting reference 0.9V trip blindly. LAN9645xF DS00006065D section5.7.3 Figure5-2/Table5-17 p.200 requires reset held at least2ms after supplies AND clock stabilize; voltage-only supervision does not prove clock startup timing. Table3-4 p.29 identifies nRESET supply as VDD_IO_A. Check cross-domain unpowered behavior, timing, thresholds, decoupling and reset output load before freezing values. No Altium edits or final component selection.

### 2026-09-21 - Manual reset input versus output clarification

Either button location can assert reset with the MIC2774N open-drain output. /MR restarts the 140-280ms release timeout; direct /RST grounding does not notify the timer, so release follows button opening/pullup rise if no other reset cause remains. Contact bounce can therefore create repeated reset edges at /RST; /MR holds reset through ordinary bounce because its release gaps are shorter than the timeout. Direct output button does not disable voltage supervision: the IC can still hold reset low. This is a release-quality preference, not a finding that Microchip's reference is invalid. Proposed /MR button retained as recommendation only. Sources: MIC2774 DS20006527A Electrical Characteristics pp.3-4, Fig.1-1 p.4 and Table2-1 p.5; reference DS50003971A Fig.A-4 p.33. Source links in preceding entry.

## 2026-09-22 - Reset indicator resistor proposal

Assuming user-linked LTST-C191KRKT is the LED from switched VDD_IO_A to MIC2774N /RST, reject100ohm direct-drive choice: with illustrative Vf2.0V,3.3V supply, near-zero output, current13mA plus reset pullup. MIC2774 DS20006527A Electrical Characteristics p.3 guarantees VOL<=0.3V at1.6mA,VDD>=1.6V;1.6mA is guarantee test condition, not absolute current rating. Recommend conservative3.3kohm1%0603>=0.1W LED resistor, retaining separate10kohm reset pullup. Approx LED0.39mA and total0.72mA at nominal assumptions, brightness to verify. At illustrative3.465V rail,1% low resistors, even Vf=0 and VOL=0 gives total3.465/3267+3.465/9900=1.411mA before other reset-net loads. Design assumption3.3V+5%, not newly verified rail tolerance. LED anode faces rail, cathode reset. If more brightness required use separate buffer; do not load reset excessively. Source supervisor https://ww1.microchip.com/downloads/en/DeviceDoc/MIC2774-Dual-Micropower-Low-Voltage-Supervisor-DS20006527A.pdf p.3. LED manufacturer PDF fetch failed; exact low-current brightness/Vf not verified. User listing https://www.digikey.com/en/products/detail/liteon/LTST-C191KRKT/386837 . Recommendation only, no implementation/selection change claimed.

## 2026-09-22 - First reset circuit screenshot review

Visible topology: MIC2774 VDD fed+3V3_ETH_SW_LS; output10k pullup and LED from switched VDD_IO_A; nRST through0R to LAN pin123;20k upper/10k lower core divider=>0.900V nominal; STM reset through0R to /MR; bankB switch8 grounds same STM reset net through100R; /MR100nF marked DNP. Appropriate topology conditional on rails and MCU electrical state. Switch8 is active-low maintained reset:0/open normal,1/closed hold reset. Update old '-- unused' switch8 note. MCU must pull low/release (open drain), never drive high against closed switch; switch power-off/MCU reset pin state needs verification.

Corrections/recommendations: LED resistor shown1k, change to proposed3.3k for current margin, retain10k output pullup. Add local100nF VDD-GND decoupling (engineering recommendation). Recommend10k /MR pullup to supervisor switched VDD for robust board-level bias; internal pullup is only100nA typical (DS20006527A p.4); external pullup optional per manufacturer, not mandated. Keep /MR capacitor DNP pending intentional RC design. Divider matches reference0.9V but is not verification of full1.15V valid-range protection; fixed2.93V likewise not exact3.3V valid-range supervision. Thresholds remain provisional. Supply/clock readiness: LAN DS00006065D section5.7.3 Fig5-2/Table5-17 p.200 requires2ms after supplies/clock stable. Supervisor timeout140-280ms after monitored conditions/MR recover does not sense clock. Sources: MIC2774 DS20006527A pp.3-6, Eq4-1 p.7, Table5-1 p.11; reference DS50003971A FigA-4 p.33. Manufacturer MIC2774 URL in prior entries. Screenshot-only review, no native connectivity/Altium edits or complete approval.

## 2026-09-22 - Reset circuit second screenshot review

VDD100nF bypass now visibly added. External nMR10k pullup added but marked DNP, so internal pullup still provides bias; recommended populate for board-level noise/leakage margin, not datasheet mandatory. LED series remains1k, not proposed3.3k: illustrative3.3V,Vf2V,VOL0 gives1.3mA LED+0.33mA pullup=1.63mA, so no margin against1.6mA VOL test condition. Recommend3.3k unchanged; do not claim current1k inherently fails. Input100nF and newly shown LAN-side nRESET100nF both DNP; retain DNP for baseline. Position8 physically wired to manual reset but table still '--'; replace function with RESETn and default0/open. Updated explanatory text must state actual DNP population, not silently treat pullup as fitted. Host open-drain/power-off behavior and0.9V core threshold/clock timing review remain open. Sources unchanged MIC2774 DS20006527A pp.3-7,11 and LAN DS00006065D section5.7.3 p.200. Screenshot review only, no native changes.

## 2026-09-22 - Latest reset sheet final-review request (conditional)

Latest full-sheet screenshot confirms populated10k nMR pullup to switched3V3_ETH_SW_LS (DNP marking removed),100nF supervisor VDD bypass,10k output pullup to VDD_IO_A, both nMR/nRESET capacitors DNP, and switch8 labelled NRESET/default0 with open=normal/closed=hold-reset note. Host open-drain notes added. Visible wiring/names consistent with earlier reset topology; actual native connectivity not checked. LED resistor remains1k, verified with enlarged view.3.3k recommendation remains outstanding; do not log as implemented. Core20k/10k remains0.900V nominal. Latest note omits threshold-review caveat and >=2ms supplies/clock-stable requirement; recommend retain these as pending verification. Change 'DNP for delay tuning' to 'DNP; populate only after reset timing/edge review' since capacitors change edge timing, not supervisor internal timeout. Current block pass for draft topology only; no unconditional hardware signoff. MIC2774 DS20006527A pp.3-7/Table5-1 p.11 and LAN DS00006065D section5.7.3 Fig5-2/Table5-17 p.200 remain reference basis. Native annotation/ERC and firmware-state checks pending.

## 2026-09-22 - MIIM0/MIIM1 meaning for our topology

Verified local DS00006065D section4.1 p.49 places section4.1.2.12 p.51 in unmanaged/internal-controller operation. MIIM0 is switch-controller-to-external-PHY management, not the chosen STM SPI host-management interface. Mandatory external PHY addresses in section4.1.2.12.1 are ROM topology expectations; do not assign STM32 MAC a PHY address0 simply because connected to RGMII0. Our intended RGMII0 link is MAC-to-MAC, so fixed-link/no-external-PHY configuration must be supported and validated. No extra MIIM0 wires to STM required by this excerpt; boot100 ROM polling/defaults on that port need FAE confirmation or documented override.

MIIM1 is internal to switch (section4.1.2.12.2 p.51); CuPHY0 physical laptop interface uses internal MIIM address1; optional CuPHY1 debug uses2. These addresses are neither DEV numbers nor I2C addresses; no STM PCB wires for MIIM1. Our optical SFP paths are intended1000BASE-X rather than external SGMII copper PHYs; MIIM0 addresses2/3 are not SFP DOM I2C addresses. Link/SerDes setup remains distinct from SFP EEPROM telemetry.

Boot000/SPI baseline must initialize switch/PHY management and link handling via appropriate register/SDK mechanisms; cannot assume ROM10ms polling described on p.51 runs under no-boot. Boot100 internal ROM runs and can read config table (section4.1.2.15 pp.51-52); external host ownership and MAC-to-MAC adaptation unresolved. No new STM pins or hardware route selected. Sources: ../eth_SW/docs/LAN9645xF-Data-Sheet-DS00006065.pdf revD sections4.1 p.49,4.1.2.12.1/.2 p.51,4.1.2.15 pp.51-52; existing Table4-4 p.54 topology mapping.

## 2026-09-22 - SerDes basics and section4.4 project interpretation

Verified DS00006065D section4.4 pp.56-57 and Table4-7 pp.55-56 locally. SerDes converts parallel/internal data to/from differential serial streams; our two optical paths use switch SerDes0/DEV5 and SerDes1/DEV6 at1.25Gbps line rate for1Gbps1000BASE-X (8b/10b ratio10/8). Each SFP path has independent transmit and receive differential pairs; module converts electrical to optical and back. SGMII and1000BASE-X share nominal line rate but are distinct protocol configurations, not interchangeable just from wiring.3.125Gbps2.5G and5GbpsQSGMII are family options, not our selected optics mode; Table4-4 p.54 establishes LAN96455 five-port mix.

Section4.4.2/.3 p.56 RX equalizer/TX emphasis address electrical channel losses; cannot restore lost optical power from fiber. Defaults/SDK setup and electrical signal-integrity verification needed; no amplitudes/equalizer values selected. Section4.4.4 pp.56-57 recommends external SFP LOS for optical connections; internal electrical activity detector is not equivalent and must be disabled above1.25Gbps. At our1.25Gbps this prohibition does not apply, but optical recommendation does. SFP LOS routing/polarity/ownership remains open; don't assume firmware DOM polling substitutes for switch signal-detect requirements.

Section4.4.5 p.57 eye monitor is intrusive (receiver reconfigured), unsuitable as live failover telemetry. Section4.4.6 near-pin TX/RX loopback is diagnostic, distinct from network LOOP_EN. Section4.4.7 PRBS7/15 are controlled bit-test patterns, not normal traffic; test topology/checker placement determines coverage, no claim of complete fiber verification from local loopback. Section4.4.8 permits TX/RX differential-polarity inversion for PCB routing; not swapping transmit and receive or removing timing/impedance constraints. Our planned health algorithm should use DOM optical levels, appropriate LOS/link indications, counters/heartbeats; spare-path intrusive tests only with deliberate coordination, not adopted design behavior. No new schematic edits/ALT choices. Source ../eth_SW/docs/LAN9645xF-Data-Sheet-DS00006065.pdf revisionD sections cited above, printed/PDF pages agree.

## 2026-09-22 - Boot100 plus custom GPIO and STM SPI feasibility revisited

Verified DS00006065D section3.4.1.1 p.42, Table3-12/section3.4.1.2 p.43, section4.1.2.15/Table4-2 pp.51-52, and EDS2 DS50003971A Table3-5 p.20. Boot100+MGMT11 explicitly exposes SPI client while ROM reads I2C table. EEPROM can alter GPIO configuration and switch/PHY registers; GPIO_MODE not wholly discarded, retaining firmware LED mapping/runtime meaning. Thus hybrid is technically plausible and worth prototyping, not verified drop-in replacement for boot000.

Critical uncertainty is runtime ownership, not mere register reachability: ROM continues link management and table can run at initialization AND link up/down. Custom ALT/forwarding writes can conflict with ROM/event writes. Need exact EEPROM format/tooling, safe SPI-ready criterion, transient mux behavior (especially GPIO4-7 SPI vs SFP), fixed MAC-to-MAC RGMII0 without MIIM PHY0, SFP LOS/I2C ownership, GPIO33 SYS_RDYn vs IRQ0, and supported way to keep forwarding/VLAN/port policy host-owned. Do not invent register sequence or claim that halting internal controller is supported/sufficient. Consider ROM ownership of PHY/link and STM ownership of forwarding policy only after Microchip confirms separation.

Prototype criteria: cold boot table/custom mux validation; SPI register reads after documented ready; RGMII0 fixed link; forwarding initially restricted to one optical path to avoid loop; STM failover followed by link flap/SFP removal/reset/reboot to confirm policy and mux persist or reapply correctly; review LOOP_EN10s shutdown interaction. Boot100 may reduce initialization firmware effort but introduces shared ownership; boot000 has more bring-up burden but simpler single software owner. Datasheet p.43 restricts Microchip managed-mode support to Linux DSA over SPI; bare-metal STM/hybrid support must be confirmed. Preserve boot000 default/boot100 candidate backup pending validation, no schematic or final decision change.

## 2026-09-22 - FAE review brief created

Prepared nine-page [PDF](../output/pdf/ECE4015_LAN96455F_FAE_Design_Review.pdf) and [editable source](correspondence/2026-09-22_LAN96455F_FAE_review.md). Includes original one/two-board diagrams with explicit correction of older LAN96445F/SFP suffix/MDIO/MCU labels; all51GPIOs in grouped side-by-side boot000/100 proposals; STM failover policy, ROM ownership, fixed RGMII, reset/EEPROM support and tooling questions. User proposes STM-owned SFP sidebands with potential0R links to LAN when GPIOs permit; distinguish input fanout from mutually exclusive driver options and unresolved I2C arbitration. New options remain proposals, not frozen assignments. Request includes inability to locate Secure Document Extract for NDA codes, no codes reproduced or NDA completion claimed. No external email sent.

## 2026-09-22 - Short introductory FAE document

Prepared a separate two-page [project introduction PDF](../output/pdf/ECE4015_Microchip_FAE_Project_Introduction.pdf) and [editable source](correspondence/2026-09-22_Microchip_FAE_introduction.md) at the user's request for a gentler initial contact. Includes project background, original two-board and single-board diagrams with outdated-label caveats, high-level boot000 versus boot100 comparison, and a broad request for Microchip architecture/software guidance. Detailed nine-page brief retained for follow-up. No new GPIO assignments or boot decision; no email sent.

## 2026-09-22 - Section5.8 crystal learning notes

Reviewed meanings of all DS00006065D section5.8.1/Table5-28 p.209 entries: fundamental25MHz; load6-12.5pF; shunt<=3pF; ESR<=100ohm; motional1.3-3.2fF; drive300uW MIN. Frequency row -100/+2000ppm retained verbatim and flagged for clarification, not treated as Ethernet compliance budget. At25MHz,1ppm=25Hz and20ppm=500Hz. Initial tolerance, thermal drift, aging and load error are separate contributions.

Crystal is passive resonator; LAN amplifier sustains vibration. Motional RLC models mechanical resonance, C0 is crystal electrode/package capacitance, CL is external effective load. Approximate CL=CA*CB/(CA+CB)+Cparasitic. Reference two10pF capacitors contribute5pF effective; reaching BOM10pF would imply about5pF effective additional parasitics, an inference not measured LAN pin capacitance. Do not choose each capacitor equal to CL automatically or add C0 as though it were external stray capacitance. ESR represents loss, not PCB termination; Cm is equivalent-model parameter, not a capacitor to populate. Drive is power dissipated in crystal, separate from VDD or IC total consumption. Higher maximum drive rating does not imply higher minimum startup power.

Sources for explanation: Microchip ANTC207, Quartz Crystals and Microchip ICs, DS00002187A(2016), Figure3 p.2, Explanation of Parameters p.3, Drive Level p.4, Frequency Accuracy/Stability p.5: https://ww1.microchip.com/downloads/en/Appnotes/00002187A.pdf . Microchip AN826 DS00826A(2002), Drive Level p.6, Pierce load equation p.9, Specifying a Crystal/Product Testing p.10: https://ww1.microchip.com/downloads/en/appnotes/00826a.pdf . Exact VXM7-9032 vs generic family qualification gap recorded in COMPONENT_SELECTIONS.md; need FAE clarification before claiming full datasheet match. No native schematic changes.

## 2026-09-23 - Crystal circuit screenshot review

Visible draft: XI153 through populated 0-ohm link to crystal1; XO152 through populated 0-ohm link to crystal3; each crystal terminal has 10pF/1%/50V/0402 to GND; cover2/4 grounded. Matches intended connectivity; native netlist not checked. C0G dielectric/exact capacitor MPN not visible. Two series links are project isolation provisions, not oscillator terminations or reference requirements; keep 0 ohm populated for crystal operation. Engineering layout advice: compact loop, short traces, local ground returns, no long test stubs. Load remains provisional; no claimed measured parasitics. Other shown block pins RESREF/ISET/S0_CLKP are unwired and outside this crystal-only review; block not fully approved.

Pasteable note:
25 MHz CRYSTAL REFERENCE
Y: VXM7-9032-25M0000000; specified CL = 10 pF.
Ca = Cb = 10 pF, C0G/NP0, 1%, 50 V, 0402.
Preferred capacitor: GRM1555C1H100FA01D.
Ground crystal case pins 2 and 4.
Populate both XI/XO isolation links with 0 ohm.
Layout (project guidance): place crystal/caps close to XI/XO;
keep traces short, ground returns local, and avoid test stubs.
10 pF values follow reference design; confirm final load,
frequency and startup on assembled PCB. IC/PCB parasitics TBD.
Refs: EDS2 DS50003971A Fig. A-4 p.33, Table B-1 p.39;
VXM7 datasheet rev.30-Apr-2020 Table 2 p.1;
LAN DS00006065D sec.5.8.1/Table 5-28 p.209.

Reference sources: ../eth_SW/docs/LAN96459F_EDS2.pdf ; ../eth_SW/docs/LAN9645xF-Data-Sheet-DS00006065.pdf ; https://www.microchip.com/content/dam/mchp/documents/TCG/ProductDocuments/DataSheets/VXM7-Website-April-2020.pdf . EDS2 establishes reference network/exact crystal, VXM7 establishes cover connections, LAN table establishes crystal limits rather than pin capacitance. Murata capacitor specifications sourced in COMPONENT_SELECTIONS.md. No Altium edits or final qualification claimed.

## 2026-09-23 - Crystal notes implemented visually; calibration next

Latest screenshot confirms previous crystal connectivity, both capacitors explicitly C0G, 10 pF/1%/50 V/0402, and reference/layout/provisional-load notes placed. No visible wiring error; exact capacitor MPN and compiled connectivity not checked. Suggest retain explicit populate-both-0-ohm-links note and move dielectric text clear of net labels. Crystal draft ready to proceed; oscillator electrical qualification still pending.

Next: RESREF pin64 to GND through200 ohm1%; ISET pin156 to GND through6.0 kohm1%. Manufacturer requirements: LAN9645xF DS00006065D section3.2 Table3-3 printed/PDF p.24 RESREF and p.23 ISET; 156-pin mapping Table3-2 p.22. Source ../eth_SW/docs/LAN9645xF-Data-Sheet-DS00006065.pdf . Reference EDS2 BOM DS50003971A TableB-1 p.39 uses6.04 kohm1% for R8; this differs from literal datasheet6.0 kohm1%. Prefer6.00 kohm1% for proposed circuit; do not silently treat6.04 kohm as identical. Close placement/local ground return is project layout advice. Neither calibration resistor shown placed yet. S0_CLKP/SD_125M configuration and unused CK25OUT disposition remain subsequent tasks.

## 2026-09-23 - Calibration placement review and clock next step

Screenshot shows RESREF64 grounded through200 ohm0.1% (meets200 ohm1% requirement) and ISET156 grounded through6.04 kohm0.5%. ISET note still says6.00 kohm1%. Literal6.00 kohm+/-1% interval5940-6060ohm; shown6.04kohm+/-0.5% interval6009.8-6070.2ohm, not fully contained. Recommend actual6.00kohm1% or tighter; this is conservative compliance with DS00006065D Table3-3 p.23, not claim shown circuit cannot work (EDS2 reference uses6.04kohm1%). No correction yet shown. Next review SD_125M strap (GPIO23, also RGMII0 TX_CTL): intended25MHz crystal architecture should disable external125MHz selection. DS00006065D section3.4.1.4 p.44 explains external125MHz enable and section3.4 p.42 applies strap to both boot modes. Verify existing strap screenshot before changes. CK25OUT is output per Table3-3 p.25; leave unconnected if unused (project choice). S0_CLKP unused-input termination not yet resolved here; do not assume no-connect based on output rule.

## 2026-09-23 - Existing SD_125M low strap confirmed visually

User supplied existing strap/RGMII screenshots: ETH_SW_RGMII0_TX_CTL connects GPIO23/SD_125M pin85; 10kohm pulldown to GND and1kohm series path to DIP position1 high option supplied by VDD_IO_C. Position1 open yields SD_125M=0; default and backup tables both specify0. Confirms prior intended25MHz-clock selection is already represented; no strap redesign needed. Basis DS00006065D section3.4.1.4 p.44; matched net labels are screenshot evidence, not compiled netlist. DIP default table specifies intended settings, not verified physical switch state. Red compiler markers visible on RGMII harness entries; exact diagnostics not readable and not inferred. Next resolve unused S0_CLKP disposition, then CK25OUT and SerDes signal wiring. ISET reconciliation remains a separate prior open item; no new resistor evidence.

## 2026-09-23 - S0_CLKP and CK25OUT disposition resolved

For project25MHz crystal/SD_125M=0 configuration, leave S0_CLKP pin54 electrically unconnected, with no pull/termination. Manufacturer basis: LAN9645xF DS00006065D section4.22 Clocking and Reset printed/PDF p.163 explicitly requires125MHz clock unconnected when25MHz mode selected for CuPHY and SerDes. Local source ../eth_SW/docs/LAN9645xF-Data-Sheet-DS00006065.pdf . Corroboration LAN96459F Hardware Design Checklist Managed Mode DS00006273B section5.0 p.10: https://ww1.microchip.com/downloads/aemDocuments/documents/UNG/ProductDocuments/DesignChecklist/LAN96459F-Hardware-Design-Checklist-Managed-Mode-DS00006273.pdf . CK25OUT pin151 is PLL-derived25MHz output per DS00006065D Table3-3 p.25; project has no consumer, so leave open (project choice, not dedicated NC pin requirement). Place targeted Altium No ERC markers on these intentionally unused pins; do not ground either or join them together. Proposed connections only; user implementation not yet shown. This closes prior research uncertainty about S0_CLKP disposition.

## 2026-09-23 - S0_CLKP marker and CK25OUT test point shown

User screenshot shows No ERC cross at S0_CLKP54 outer pin end, no wire: matches DS00006065D section4.22 p.163. CK25OUT151 instead routed to ETH_SW_CK25OUT and TP?, an intentional monitoring connection superseding proposed unused-output NC. No No ERC needed on this connected output merely for being unused by logic. Clock-output identity per DS00006065D Table3-3 p.25; test point is project choice, not manufacturer requirement. Recommend small pad/short route and high-impedance low-capacitance probe (no direct50ohm scope termination); presence/amplitude/frequency measurement at CK25OUT does not independently qualify crystal drive or startup margin. Native connection/drive/load budget not verified. ISET still6.04kohm0.5%; prior6.00kohm1% reconciliation remains open. Source ../eth_SW/docs/LAN9645xF-Data-Sheet-DS00006065.pdf .

## 2026-09-23 - XO series-resistor tuning provision

Refined recommendation: retain compact XO series footprint populated0ohm as potential drive-limiting adjustment; XI0ohm optional for isolation, not normally the drive-control location. Supersedes conversational preference to omit both links. A nonzero XO resistor may be useful if measured crystal drive exceeds exact crystal limit; no such overdrive measured here. Existing topology XO152 -> resistor -> node shared by crystal3 and Cb is appropriate generic output-side Rs arrangement. Microchip AN849 Basic PICmicro Oscillator Design DS00849A (2002), Figure1 p.1 and Purpose of Resistors p.5, explains output-side Rs for overdrive control: https://ww1.microchip.com/downloads/en/appnotes/00849a.pdf . General oscillator principle, not LAN-specific resistor-value authorization. AN826 DS00826A (2002), Drive Level p.6 and Product Testing p.10, supports limiting crystal dissipation and testing voltage/temperature/variation: https://ww1.microchip.com/downloads/en/appnotes/00826a.pdf . Any nonzero value requires FAE/measurement-based choice and startup-margin, frequency and drive validation. No guessed resistance or inference of crystal drive from CK25OUT. Exact9032 drive specification remains unresolved. No native change claimed.

## 2026-09-23 - SerDes differential pair directives proposed

Screenshot has ETH_SW_SD0/1_RXP/RXN/TXP/TXN labels and8test points; XI now direct with added TP, XO retains0ohm and TP. Recommend rename to ETH_SW_SD0_RX_P/_N, ETH_SW_SD0_TX_P/_N, ETH_SW_SD1_RX_P/_N, ETH_SW_SD1_TX_P/_N everywhere those nets connect. Four distinct pairs. Place differential pair directive on each of8wires; assign Diff Pair Class ETH_SW_SERDES_100R to all four pairs, optionally Net Class ETH_SW_SERDES to8nets. Project names/organization choices. Altium Defining Differential Pairs in Your Schematics: https://www.altium.com/documentation/altium-designer/schematic/defining-differential-pairs establishes common root with_P/_N and directive per net. Classes alone do not set impedance; PCB rule/profile required, geometry from actual fabrication stackup. Microchip LAN96459F Hardware Design Checklist Managed Mode DS00006273B section12 p.28 establishes100ohm differential routing, minimal intra-pair skew and avoiding plane splits: https://ww1.microchip.com/downloads/aemDocuments/documents/UNG/ProductDocuments/DesignChecklist/LAN96459F-Hardware-Design-Checklist-Managed-Mode-DS00006273.pdf . No arbitrary trace width/gap/skew tolerance frozen; do not length-match unrelated TX/RX/ports together. If board series components split nets, define pairs on each segment and end-to-end xSignals as needed later; AC-coupling population remains separate review. Recommend remove ordinary SerDes TP components/branches and XI/XO test pads from baseline to avoid added discontinuities/loading; engineered probing provisions can be reviewed separately. Retain CK25OUT monitoring. XI/XO are not a differential pair. No directives/rules/native changes verified.

## 2026-09-23 - SerDes directive screenshot review

User screenshot now shows ETH_SW_SD0_RX_P/_N on62/60, SD0_TX_P/_N on58/56, SD1_RX_P/_N on72/70 and SD1_TX_P/_N on68/66. Eight differential-pair directive symbols visibly attached to corresponding wires. ETH_SW_SERDES_100R and ETH_SW_SERDES labels displayed on both sides; individual class types/membership on every directive not shown, so do not claim all four pairs assigned/compiled. Properties or PCB ECO/pair list remains verification step. S0_CLKP54 No ERC remains present. All8SerDes TP components and XI/XO TPs remain; recommendation to omit ordinary probing pads absent reviewed SI arrangement remains open. ISET remains6.04kohm0.5%, prior note/spec reconciliation open. No PCB impedance enforcement, stackup, cross-sheet rename, end-to-end SFP connectivity or native edits verified. Altium naming/directive reference and Microchip100ohm layout reference retained in preceding entry.

## 2026-09-23 - Microchip concept support case opened

User screenshot confirms Microchip support case01896570, subject LAN96455F ETH SW Project, status New, with ECE4015_Microchip_FAE_Project_Introduction.pdf attached (event by Ethan Su, portal timestamp9/23/2026 22:09, timezone unspecified). Supersedes earlier submission-unconfirmed state. Exact submitted form text not shown; no Microchip technical response or design approval yet evidenced. Case tracking details in correspondence/2026-09-23_Microchip_concept_check_form.md .

## 2026-09-28 — Boot4 firmware/hardware sequence clarified
Windows/Codex: verified localDS00006065D section3.4/3.4.1.1p.42 strapslatchedonresetrelease,BOOT100internalROM+I2C1configurationtable; section3.4.1.2p.43 MGMT11SPIclient; Table3-11p.41 SYS_RDYnactiveLOWinitialboot/configdone,ROMcontinuesportmonitoring; section4.1.2.15/Table4-2p.52 EEPROMbase0x50,>=400kHz,addresswidthstrap andtableexecutionatinit/linkevents. Correctionto proposedsequence:strapslatchatresetrelease,notafterROMinitialization. ROMisfactorycode,EEPROMtableiscustomerconfigurationinstructions,notarbitraryfirmwareimage. MerelyconnectingblankEEPROMdoesnotestablishsuccessfulconfiguredboot; validcorrectformat/tableprogrammingrequired,blank/invalidbehaviornotverified. Hardwareincludespower/clock/reset,straps,correctI2C1pins/pulls/addressing,EEPROMpowered/readable,noexternalprogrammercontention,MGMT11SPIwiringandobservableSYS_RDYnGPIO33ifretained; GPIO33sharedEEPROM_SIZE/IRQfunctions requireownershipreview. ProposedSTMflowholdreset/stablepowerclock->release->waitSYS_RDYnLOWwithtimeout->verifySPIidentity/status->applyapprovedhostpolicy. ThisisnotROM-to-STMhandoff:ROMstillrunsandmayreapplytableonlinkevents; safeownershipforcustomSPIwrites/fixedRGMII/SFPmapping remainsFAEitem. Existingboot0baseline/boot4backup not changed; nohardware/firmwareexecution.

## 2026-09-29 — Support reply and register-documentation clarification
Windows/Codex: archived Vishnu's pasted reply and prepared an unsent follow-up in correspondence/2026-09-29_Microchip_reply_draft.md. Support states Linux DSA only, recommends STM32 SPI initialization before EEPROM, and says complete register map is in DS00006065. Local DS00006065D section4.23.1.1/Table4-57 printed/PDF pp.164–167 provides transport format and symbolic IF_CTRL/IF_CFGSTAT names but Table4-57 does not give numeric register addresses; complete map location remains unresolved. Requested numeric addresses/bitfields or headers and minimum boot0 initialization/ready procedure. Do not interpret prior unmanaged SYS_RDYn flow as verified boot0 readiness. Source: ../eth_SW/docs/LAN9645xF-Data-Sheet-DS00006065.pdf . Official indexed EV03E14A guide DS50004036B section3.3 printed p.20 explicitly describes a LAN9645x configuration-table tool; full PDF web fetch exceeded tool size limit. Requested software/guide/examples and custom-board compatibility; access not obtained. Source: https://www.microchip.com/content/dam/mchp/documents/NCS/ProductDocuments/UserGuides/EV03E14A-LAN96459-Unmanaged-Eval-Board-User-Guide-DS50004036.pdf . Exact selected MPN LAN96455F-I/8NW; silicon revision unconfirmed. No boot architecture decision changed or email sent.

## 2026-09-29 — Follow-up sent (user confirmation)
User confirmed sending the exact message archived in [sent correspondence](correspondence/2026-09-29_Microchip_reply_sent.md), superseding the earlier unsent status for this follow-up. Requests register addresses/bitfields/headers, boot0 initialization/readiness guidance, and configurator access/custom-board applicability. Selected MPN supplied; silicon revision still pending. Initial high-level block diagram attached per user; exact attachment version not provided here. Updated schematic PDF promised later this week. Actual sent text thanks support for errata without requesting an attachment; promised sheet list is power, reset/straps, SPI and RGMII. Awaiting response; no architecture change or technical approval implied.


### 2026-10-01 — Minor reset schematic update archived
Windows/Altium screenshot supplied by Ethan. Visible MCU hierarchical port remains STM_ETH_SW_NRESET, while its wire label is now STM_ETH_SW_RESET_N; 0ohm series link leads to STM_ETH_SW_RESET_N_R at MIC2774 nMR pin3 with a test point. Supervisor nRST output is labeled ETH_SW_RESET_N; 0ohm link leads to ETH_SW_RESET_N_R at LAN nRESET pin123, with a test point. These supersede earlier wire-label spellings STM_ETH_SW_NRESET_R / ETH_SW_NRESET / ETH_SW_NRESET_R in the visible crop; cross-sheet/global rename not verified. The MCU port name itself is unchanged, so prior pseudocode using STM_ETH_SW_NRESET still matches the shown port.

Visible retained circuitry: 10kohm nMR pull-up to +3V3_ETH_SW_LS; supervisor VDD on that switched rail with100nF bypass; core sense uses two series10kohm upper resistors and10kohm lower toGND (20k/10k total, previously recorded nominal0.9V trip). Output10kohm pull-up and LED anode feed from +3V3_ETH_SW_VDD_IO_A; LED series resistor remains1kohm. LED annotation now shows30mA/2.4V, which does not establish actual circuit current or an exact part specification. Both reset-related100nF capacitors remainDNP. Supervisor symbol caption MIC2774N-29YM is abbreviated; exact ordering-code/library association not reverified.

Status: minor schematic changes recorded from image, not a new complete electrical review or native-file edit. Existing supervisor-threshold, reset LED loading and MCU power-off/timing checks remain open. Manual reset switch path is outside this crop and is not reverified.

![Updated LAN reset supervisor circuit](images/2026-10-01/lan-reset-circuit-update-fbbbefc6.png)
*Original user Windows/Altium screenshot. Records revised reset wire labels, test points and retained values; saved unchanged.*


### 2026-10-01 — RJ45 individual link/activity LED plan
Windows/design review. Ethan clarified that each populated RJ45 should use both its green and yellow LEDs, superseding the September 17 single combined copper LED plan. Proposed color assignment: green steady for link; yellow blinking for RX/TX activity. Both off with no link; with an idle link green on/yellow off. Colors are a board-level choice, not mandated by the LAN.

Normal BOOT_MODE=000: STM32 must configure both PHY LED outputs and individual link/activity behavior over SPI; unmanaged LED_MODE strap is ignored. Backup BOOT_MODE=100: retain proposed LED_MODE=0 for individual mode with two configured outputs. Existing EGPIO49/50 (PHY0) and EGPIO47/48 (PHY1) routes differ from default GPIO Mode D GPIO42/43 and GPIO40/41 routes, respectively; custom EEPROM mapping/ownership remains unresolved. This decision does not change MAC/SerDes LED plans.

Source: [LAN9645xF datasheet DS00006065D](../eth_SW/docs/LAN9645xF-Data-Sheet-DS00006065.pdf), section 3.4.2.5 and Table 3-14, printed/PDF p.45, establish individual link/activity behavior. Table headings LED1/LED2 conflict with notes LED1=link and LED0=activity: do not freeze physical output-to-color assignment until clarified or tested. Table 3-5 pp.30–31 establishes managed alternate functions; Table 3-9 p.36 establishes default Mode D mapping; Table 3-12 p.43 establishes boot precedence. No native Altium edit or bench validation performed; OneDrive sync not checked.

- [ ] Ethan: resolve PHY_LED0/1 behavior and RJ45 LED pinout/polarity before schematic release; verify both boot paths during bring-up. Deadline: before schematic release / bring-up respectively.


### 2026-10-01 — Dual RJ45 LED wiring screenshot review
Windows/Altium review. Archived two original user screenshots. LAN-side PHY1_LED0/1 route through separate 1kohm 1% 0603 resistors to EGPIO47/pin113 and EGPIO48/pin46; PHY0_LED0/1 similarly to EGPIO49/pin44 and EGPIO50/pin45. IC-side nets carry _R suffixes. These match managed ALT2 functions (LAN9645xF DS00006065D Table3-5 p.31). LED symbol shows switched3.3V feeding green/yellow anodes (pins11/13), green cathode12 on LED1 and yellow cathode14 on LED0; each complete path must include its own shown resistor once. Illustrative current (3.3-2.0-0.2)/1k=1.1mA, not verified brightness/current for the unidentified jack.

Finding requiring correction: both RJ45 hierarchical ports are named ETH_SW_PHYX_LED0, while the lower wire label is ETH_SW_PHYX_LED1. Change lower port to ETH_SW_PHYX_LED1 and verify compiled hierarchy maps each instance to the correct PHY0/PHY1 pair; do not assume the wire label overrides a duplicated port. Potential merge/misconnection depends on hierarchy/net naming settings. EGPIO47 uses domainA; other three use domainE (Table3-4 p.29); switched LED supply must track both actual IO feeds. Pin/color interpretation is based on displayed symbol only: exact RJ45 MPN is absent and this 11–14 LED numbering differs from previously discussed 7499111614A, so manufacturer symbol/footprint confirmation remains required.

Individual green-link/yellow-activity intent retained. DS00006065D section3.4.2.5/Table3-14 p.45 numbering inconsistency remains; boot000 needs runtime configuration, boot100/ModeD requires resolving EGPIO versus GPIO40–43 mapping (Tables3-9 p.36 and3-12 p.43). Reference: [LAN datasheet revision D](../eth_SW/docs/LAN9645xF-Data-Sheet-DS00006065.pdf); printed/PDF pages agree. Review limited to shown LED circuit; magnetics/capacitor connections not signed off. No native edits or compiled ERC verification; OneDrive sync not checked.

![LAN-side individual LED series resistors](images/2026-10-01/rj45-phy-led-series-resistors-a0f57d42.png)
*Original Windows/Altium screenshot: four independent 1kohm LED paths and EGPIO assignments.*

![RJ45 LED wiring with duplicate hierarchical port](images/2026-10-01/rj45-led-wiring-port-typo-f7ddefa3.png)
*Original Windows/Altium screenshot: lower port repeats LED0 despite LED1 wire label; correction pending.*

- [ ] Ethan: correct lower LED port, verify instance mapping and exact jack LED pinout before schematic release; test both boot configurations during bring-up.


### 2026-10-01 — Corrected RJ45 LED port and MAC LED planning
Windows/design review. User screenshot now shows distinct ETH_SW_PHYX_LED0 and ETH_SW_PHYX_LED1 hierarchical ports matching wire labels, resolving the prior visible duplicate-port finding. Native compiled hierarchy remains unverified.

GPIO13 also samples LED_MODE (proposed0 individual), GPIO14 CUPHY_MODE (proposed0 all-speed copper auto-negotiation); both domainB. Runtime ALT5 maps GPIO13 to MAC_LED6 (SerDes1/backup DEV6), GPIO14 to MAC_LED5 (SerDes0/primary DEV5); ModeD defaults use those same MAC mappings. No RGMII1 is used in our architecture, and neither pin belongs to existing SPI4–7, EEPROM2/3, or RGMII0 16–27 allocation. Preserve strap loading: direct LED+resistor to3.3V can bias a low strap high during reset; recommend reviewing a high-impedance buffer/driver before adding indicators. Do not adopt an unverified direct LED connection or infer MAC LED polarity/blink behavior from the PHY LED table.

MAC_LED is an electrical MAC-status output, not an optical laser drive or module-present/LOS/TX_FAULT signal. A visible optical-port indicator would be a separate board LED near the SFP cage; no LED drive contact exists in the shown standard20-pin SFP interface. Exact MAC status/polarity/blink configuration remains to verify from register documentation. No final LED driver/component selected.

Reference: [LAN9645xF DS00006065D](../eth_SW/docs/LAN9645xF-Data-Sheet-DS00006065.pdf), Table3-4 p.28 domains/straps; Table3-5 p.30 managed ALT5; Table3-9 p.36 ModeD; Table3-11 p.41 defines MAC-status output; section3.4 p.42 reset strap sampling; sections3.4.2.5–6 pp.45–46 strap meanings. Printed/PDF pages agree. No native edits, measurements or OneDrive sync verification.

![Corrected separate RJ45 LED ports](images/2026-10-01/rj45-led-port-corrected-912d3a29.png)
*Original user Windows/Altium screenshot; lower port now correctly identifies LED1.*

- [ ] Ethan: validate MAC LED programming and a strap-compatible indicator driver before schematic release.


### 2026-10-01 — KT-0805G MAC LED strap resistor proposal
Windows/reference review. User supplied LED strap diagram matches local LAN9645XS/F Hardware Design Checklist DS00006273C Figure4-1 p.9 (revision C verified, not older B). Manufacturer illustrates470ohm LED series with1kohm pulldown, or470ohm series with10kohm pullup. Propose470ohm1% series and1kohm1% pulldown,0603>=0.1W, for fixed-low GPIO13 LED_MODE and GPIO14 CUPHY_MODE, supplied from associated switched VDD_IO_B. This qualifies earlier blanket preference for a buffer: direct drive is a manufacturer example if strap loading is checked. Checklist Table4-4 p.8 omits GPIO13/14; use LAN DS00006065D Tables3-4 p.28 and3-5 p.30 for their functions, not checklist table as proof of those particular pins.

Kento KT-0805G manufacturer sheet A3 dated2017-05-16 (cover A.2 discrepancy), Electrical/Optical Characteristics printed/PDF p.3 gives Vf2.5–3.0V at5mA: https://www.rcscomponents.kiev.ua/datasheets/KT-0805-G.pdf . At assumedVf2.7V andVOL0.2V,470ohm yields0.85mA; low-currentVf and actual brightness require measurement, not guaranteed by5mA limits. LED can glow during reset through1k pulldown. Verify reset pin voltage versus LAN VIL over supply/temperature; MAC runtime polarity/configuration still pending. Existing switchable1k-high/10k-low straps cannot simply have pulldown changed to1k: closed1k/1k divider is nearhalf-rail. Rework/qualify selectable strap if retained; values here assume fixed-low network. No native edits or final sign-off.

![Manufacturer LED strap circuit supplied by user](images/2026-10-01/lan-led-strap-reference-8ad1f347.png)
*Original user reference screenshot; manufacturer example, not implemented circuitry.*


### 2026-10-01 — MAC LED fixed-low strap schematic review
Windows/Altium screenshot review. GPIO14/pin99 ETH_SW_CUPHY_MODE sinks upper KT-0805G LED (MAC_LED5/primary); GPIO13/pin100 ETH_SW_LED_MODE sinks lower LED (MAC_LED6/backup). Each anode fed by470ohm1%0402 from+3V3_ETH_SW_LS; each cathode/GPIO node has1kohm1%0603 toGND. Original10kohm pulldowns AND1kohm DIP pullup links for both straps are visiblyDNP. This implements the proposed fixed-low topology in the screenshot and avoids the earlier1k/1k switch-divider problem; DIP positions4/5 are now inactive. Update switch note to say fixed0/DNP, not selectable. Supply must be physically same as VDD_IO_B. No native connectivity or BOM variant verification performed.

Suggested naming cleanup: ETH_SW_MAC_LED5/6 currently label LED anode-side nets, not the GPIO outputs. Use *_A or *_ANODE there to avoid implying those labels are raw MAC outputs; cathode nets retain strap names.30mA/2.6V symbol text is not operating current and conflicts with reviewed Kento A3 p.3 continuous maximum20mA; reconcile library to purchased part (cover A.2/body A3 inconsistency already recorded).470ohm is project/example value; assumedVf2.7,VOL0.2 gives0.85mA, actual low-current Vf/brightness not verified.

Runtime HIGH must source approximately3.3mA into each1k pulldown; configure/verify output drive sufficient (VO4mA or greater where applicable), not an assumed2mA default. Open-drain/high-Z off would leave reset-bias path and possible glow; verify MAC output type/polarity/configuration. At reset check Vpin<=0.39*VDD_IO_B (1.287V at3.3V) including LED low-current/temperature spread, internal pullup and leakage. Reference topology alone is not a measured or worst-case pass. BOOT000 ALT5 configuration required; BOOT100 ModeD lists GPIO13/14MAC mappings. Copper EGPIO backup mismatch remains independent.

References: [LAN9645xF DS00006065D](../eth_SW/docs/LAN9645xF-Data-Sheet-DS00006065.pdf), Tables3-4 p.28,3-5 p.30,3-9 p.36 establish mapping/domain; Table5-7 pp.192–194 establishes input thresholds, output-drive test conditions and pull/leakage. [Hardware Design Checklist DS00006273C](../eth_SW/docs/LAN9645XSF-Hardware-Design-Checklist-Managed-Mode-DS00006273.pdf), Fig4-1 p.9 illustrates470ohm/1kohm topology. [Kento KT-0805G A3](https://www.rcscomponents.kiev.ua/datasheets/KT-0805-G.pdf), Electrical/Optical Characteristics and Absolute Maximum Ratings p.3 establish Vf at5mA and20mA continuous limit. Printed/PDF pages agree. No native edits, bench results or OneDrive sync verification.

![MAC LEDs with fixed-low straps](images/2026-10-01/mac-led-fixed-low-straps-0dc22e55.png)
*Original user Windows/Altium screenshot; DNP selectable paths and new LED/pulldown networks visibly shown.*


### 2026-10-02 — GPIO drive current versus IO supply domains
Windows/design review. Explained LAN source current (HIGH output supplies external load) versus sink current (LOW output takes external current to ground). Table5-7 drive2/4/8/10mA rows characterize configurable output strength at VOL0.4V or VOH=VARIO-0.4V; they are not constant-current settings or absolute-maximum permitted load ratings. Use minimum capability for guaranteed design, not max-column process spread. At3.3V,4mA setting sources at least3.9mA at2.9V;2mA minimum1.9mA is insufficient to guarantee HIGH with1k pulldown (2.9mA at2.9V; approximately3.3mA nearrail). Recommend verify/select4mA or stronger appropriate drive for the two MAC LEDs; register/default/output type remain unverified.

User power crop visibly ties IO domainsA/B/C/D/E to+3V3_ETH_SW_LS, supporting the prior supply-tracking condition for the LED anode supply and domainB GPIO13/14. VARIO here is3.3V; bank labels list supported alternative supplies, not simultaneous rails or pin-output voltages. Domains are supply groups, not per-pin current allocations. Common3.3V supply does not set output drive strength. Screenshot-level observation only, no native netlist or measurements. Reference: [LAN9645xF DS00006065D](../eth_SW/docs/LAN9645xF-Data-Sheet-DS00006065.pdf), Table3-4 pp.28–29 defines pin domains; section5.6 Table5-7 pp.192–194 defines electrical test conditions and current capability. Printed/PDF pages agree.

![LAN GPIO drive current specifications](images/2026-10-02/lan-io-drive-current-table-7fff2d16.png)
*Original user datasheet crop; source/sink rows at specified output voltages.*

![Shared switched3.3V supply for LAN IO domains](images/2026-10-02/lan-shared-3v3-io-domains-0e320972.png)
*Original user Windows/Altium crop; domainsA–E visibly share switched3.3V.*

Next: verify output-drive register settings and MAC LED polarity in both boot paths before firmware bring-up. No native edits; OneDrive sync not verified.


### 2026-10-02 — Six port LEDs consolidated
Windows/Altium review. Latest MAC LED screenshot now labels anode nets ETH_SW_MAC_LED5_A / ETH_SW_MAC_LED6_A and annotates20mA instead of30mA. These resolve the previous visible naming/rating annotation findings;2.6V remains a symbol annotation, not a guaranteed low-current Vf. Screenshot does not establish native netlist changes.

| Indicator | Intended behavior | LAN output / package pin | IO domain | Managed ALT | Series R | External pulldown |
|---|---|---|---|---|---|---|
| Laptop copper yellow | Activity* | PHY0_LED0 / EGPIO49 /44 | E |2|1k|None shown|
| Laptop copper green | Link* | PHY0_LED1 / EGPIO50 /45 | E |2|1k|None shown|
| Debug copper yellow | Activity* | PHY1_LED0 / EGPIO47 /113 | A |2|1k|None shown|
| Debug copper green | Link* | PHY1_LED1 / EGPIO48 /46 | E |2|1k|None shown|
| Primary optical green KT-0805G | MAC5 status; exact pattern TBD | MAC_LED5 / GPIO14 /99 | B |5|470ohm|1k; CUPHY_MODE=0|
| Backup optical green KT-0805G | MAC6 status; exact pattern TBD | MAC_LED6 / GPIO13 /100 | B |5|470ohm|1k; LED_MODE=0|

All shown IO domains and LED anodes share switched+3V3_ETH_SW_LS. Copper series resistors1%0603; MAC series470ohm1%0402 and pulldowns1%0603. *Green-link/yellow-activity assignment follows Table3-14 footnotes but conflicting headings require confirmation. Actual RJ45 MPN/polarity verification pending. Copper current approximately(3.3-Vf-VOL)/1k; optical approximately(3.3-Vf-VOL)/470, illustrative0.85mA atVf2.7/VOL0.2. MAC HIGH sources approximately3.3mA per pulldown, so verify4mA-or-stronger applicable output setting; exact MAC polarity/default/registers unverified.20mA LED annotation is maximum continuous rating in reviewed Kento sheet, not circuit current.

BOOT000 requires STM configuration of alternate functions and indication behavior. BOOT100 proposedModeD defaults copper LED0/1 toGPIO42/43(PHY0) andGPIO40/41(PHY1), not currently routedEGPIOs; custom EEPROM mapping unresolved. ModeD MAC13/14 mapping matches shown outputs. OldDIP links/10k pulldowns shownDNP in preceding image; unchanged assumed, not visible here.

References: [LAN9645xF DS00006065D](../eth_SW/docs/LAN9645xF-Data-Sheet-DS00006065.pdf), Table3-2 pp.21–22 pin numbers, Table3-4 pp.28–29 domains, Table3-5 pp.30–31 managedALT, Table3-9 p.36 ModeD, Table3-12 p.43 bootprecedence, section3.4.2.5/Table3-14 p.45 PHYbehavior, Table5-7 pp.192–194 drive. [Hardware checklist DS00006273C](../eth_SW/docs/LAN9645XSF-Hardware-Design-Checklist-Managed-Mode-DS00006273.pdf),Fig4-1 p.9 strapLEDnetwork. Printed/PDF pages agree. No new full electrical sign-off, native edits, or sync verification.

![MAC LED annotations corrected](images/2026-10-02/mac-led-anode-labels-updated-5c0396d0.png)
*Original user Windows/Altium screenshot; anode suffixes and20mA annotations now visible.*


### 2026-10-02 — MAC LED low-strap margin calculation
Windows/calculation. Prepared concise Altium MAC note and six-LED status table. Fixed-low network:470ohm series LED,1k pulldown, both1%; internal pullup58.9k minimum and10uA sourcing leakage included. AtVDD3.3V,VILmax=1.287V; boundary LED current budget1.230mA and required LED Vf>=1.441V. AtassumedVDDmax3.465V(+5%, engineering assumption),VILmax1.35135V; boundary current1.292mA and requiredVf>=1.513V. Uses worst resistor directions RPD1010ohm,Rseries465.3ohm. With illustrativeVf2.6V at3.465V, calculatedVpin approximately0.630V, below1.351V. This is an estimate: Kento A3 Vfmin2.5V is specified at5mA and cannot be asserted as a guaranteed minimum atapproximately1mA/across temperature. Low strap expected to pass, but low-current hotVf data or validation needed for guaranteed margin. Exact rail tolerance not newly verified. LAN DS00006065D Table5-7 pp.192–194 establishes thresholds,pullup,leakage; DS00006273C Fig4-1 p.9 establishes reference topology. Existing MAC runtime pattern and PHY LED numbering caveats retained. No schematic edits.


### 2026-10-02 — LED schematic status and strap notes shown
Windows/Altium screenshot supplied by Ethan. Screenshot shows six-LED intended-status table: laptop/debug RJ45 green PHYx_LED1 link and yellow PHYx_LED0 activity; primarySFP0 MAC_LED5 and backupSFP1 MAC_LED6 patterns remainTBD. MAC circuits retain470ohm series/1kohm pulldowns,KT-0805G20mA/2.6V annotations and *_A anode net names. Added MAC note records GPIO14/pin99 andGPIO13/pin100,ALT5,domainB on switched3.3V,approximately3.3mA HIGH pulldown load and proposed>=4mA drive verification. Purple note explicitly marks drive-strength configurabilityTBD; no register setting/default established.

Low-strap annotation includes assumed3.465V rail,VIL1.351V,estimatedVpin0.63V atassumedVf2.6V and requiredVf>=1.513V atapproximately1.29mA overtemperature. This remains a conditional calculation, not measured or guaranteed sign-off. Visible references: LAN9645xF DS00006065D Tables3-4/3-5 pp.28–30,Table5-7 pp.192–194,Table3-11 p.41,Table3-12 p.43,section3.4.2.5/Table3-14 p.45; DS00006273C Fig4-1 p.9; KT-0805G A3 characteristics p.3. Source files linked in preceding entries. Prior PHY LED numbering inconsistency and boot100 ModeD/copper EGPIO routing issue remain open even though absent from visible status box. Records annotations shown in screenshot only; no native-file edits, firmware implementation, bench tests or OneDrive sync verification.

![Ethernet LED status table and MAC strap annotations](images/2026-10-02/ethernet-led-status-and-strap-notes-39f0fdaa.png)
*Original user Windows/Altium screenshot; six-LED legend, MAC circuitry, conditional strap check and drive-strength TBD note.*


### 2026-10-02 — JD1-0001NL magnetics and protection review
Windows/document review. Downloaded manufacturer JD1-0001NL RevB (2014-11-14), sheet2/5 schematic and electrical specifications to ../eth_SW/docs/JD1-0001NL.pdf; inspected rendered drawing. Important correction to initial tentative chat interpretation: pin1 is common return for four internal100nF capacitors; pin10 is the common PHY-side center-tap node. With pin1 atGND as shown, internal100nF capacitors already connect center taps to signalGND. External pin10 100nF is parallel additional capacitance (400nF internal aggregate ->500nF nominal), not a missing bypass or series return. Recommend DNP external pin10 cap initially/leave pin10 externallyNC; do NOT ground pin10 directly, which would DC-ground PHY winding centers. Internal common CT differs from separately shown CTs in generic reference; full compatibility remains to qualify.

Four1:1 transformers provide galvanic isolation and differential AC transfer; common-mode chokes impede equal-direction interference while passing differential currents. Four75ohm cable CT resistors plus internal1000pF/2kV toshield provide common-mode termination/AC noise path, not100ohm differential data termination. SignalGND belongs to PHY/board; chassisGND to shield/enclosure, not automatically earth. External1uF100V betweenGND andchassis is separate intentional AC bond, not part of required CT network; keep asDNP option pending enclosure/EMC/isolation requirements.100V ordinaryMLCC does not establish suitable isolation rating. No automatic replacement value or safety class selected.

No eight series AC caps shown in Microchip DS00006273C Fig6-3 printed/PDFp16: transformers supply galvanic/DC separation, CT capacitors provide common-mode bypass. Section6.2 p14 wording says couplingcapacitors required, ambiguous againstfigure; record conflict rather than infer eightseriescaps. Previous B revision citations p15 must not be reused for localC figurep16. TI ESDS30x SLVSEG8B Jan2024 section7.2 p9/section7.4 p10 andTable4-2 p3 support PHY-sideESD arrays andGNDpin2; lowcap shunt clamps transients, not3.6Videalclamp nor12Acontinuous current. Exact LAN transient survivability and routing unqualified. In screenshot fourchannels perarray spanall8MDI nets; geometry/nativepin mapping still needsERC.

New LED1k series resistors onRJ45sheet may duplicate earlierLAN-side1k screenshot; ifbothfitted eachLED has2k total. Same ETH_SW_PHYX_LED0/1 labels onboth sides of newseriesresistors appear to bypass them electrically; correct withdistinctnets/removefar-side label andverify compilednetlist. Do not claim corrected. Nativefiles untouched. No bench orsyncvalidation.

References: [Pulse JD1-0001NL RevB sheet2](../eth_SW/docs/JD1-0001NL.pdf); [Microchip DS00006273C section6.2 pp14–16 Fig6-3p16](../eth_SW/docs/LAN9645XSF-Hardware-Design-Checklist-Managed-Mode-DS00006273.pdf); [TI SLVSEG8B](https://www.ti.com/lit/ds/symlink/esds304.pdf).

![RJ45 integrated magnetics and TVS draft](images/2026-10-02/rj45-magnetics-review-3efeaa94.png)
*Original user Altium screenshot; review findings above remain pending edits.*


### 2026-10-02 — TVS optionality and center-tap bypass clarified
Windows/design discussion. TVS arrays are not needed for basic Ethernet signaling, but retaining PHY-side protection is recommended for externally accessible RJ45 ports; transformers provide galvanic isolation but parasitic capacitance and differential transients can still couple disturbances. Array ratings do not prove assembled-boardESD immunity; omission should be based on a validated protection/test strategy, not transformer presence alone. TI ESDS30x SLVSEG8B section7.2 p.9 provides Ethernet application example, not LAN-specific mandate.

For JD1-0001NL RevB sheet2, pin1 ties one side of all four internal100nF caps; pin10 ties all PHY-side CTs and the other sides. Pin1 grounded gives400nF nominal aggregate CT-to-GND, with internal branch geometry still relevant atHF. Pin10 external100nF with pin1 floating instead gives only one100nF common bypass; floating pin1 does not create four independently grounded caps. Recommended baseline remains pin1 directlysignalGND, pin10 externallyNC / extra capDNP. Microchip DS00006273C Fig6-3 p16 shows100nF pertransformer tosignalGND; four externalCT-specific caps cannot be independently implemented with this jack because CTs are internallyjoined. SharedCT compatibility remains to qualify; do not advertise this jack as fully equivalent to fourindependentCTs. No schematic edits.


### 2026-10-02 — Revised RJ45 sheet reviewed and archived
Windows/Altium screenshot supplied by Ethan. External pin10 100nF is nowDNP; pin1 remains directlysignalGND, using internal CT bypass. RJ45-sheet LED series resistors removed, resolving the prior duplicate-resistor/same-label bypass concern on this sheet. LED1 routesgreen cathode12; LED0 yellow cathode14; anodes11/13 switched3.3V. Existing LAN-side1k resistors must remain fitted and be verified through compiledhierarchy; no series resistance is visible on this sheet itself.

MDI routing visually maps A+/A- to9/8,B+/B- to7/6,C+/C- to4/5,D+/D- to3/2, agreeing with PulseRevB sheet2 winding polarity. TwoESDS304 arrays retain8shuntchannels with pin2 tosignalGND; placement and actualclamp margin not verified by schematic. SharedCT compatibility with LAN96455 remains open. External1uF100V signalGND-tochassis capacitor still appears fitted (noDNP marking); prior recommendation to leaveoptionalDNP pending enclosure/EMC/isolation review remains unresolved. Red/orange dashed markers visible; cause cannot be determined from image, review AltiumMessages/ERC before release. Designators remainunannotated. No fullsign-off/nativeedit/syncverification.

References: [Pulse JD1-0001NL RevB, sheet2/5 schematic](../eth_SW/docs/JD1-0001NL.pdf) establishes pinout/internalcaps; [Microchip DS00006273C Fig6-3 p16](../eth_SW/docs/LAN9645XSF-Hardware-Design-Checklist-Managed-Mode-DS00006273.pdf) gives copper reference topology; [TI ESDS30x SLVSEG8B Table4-2 p3 and section7.2 p9](https://www.ti.com/lit/ds/symlink/esds304.pdf) establishes arraypinout/application.

![Revised RJ45 sheet with optional pin10 capacitor DNP](images/2026-10-02/rj45-pin10-dnp-led-resistors-removed-cabef7e7.png)
*Original user Windows/Altium screenshot; pin10 capacitorDNP andlocalLEDresistorsremoved; chassis-bondcapacitor remains shownfitted.*

- [ ] Ethan: confirm LAN-sideLEDresistors and hierarchy; resolve chassis bond, sharedCT compatibility andERC before schematicrelease.


### 2026-10-02 — PHY0/1 MDI pin review and unused-port markers
Windows/Altium screenshot review. PHY0A122/124,B126/128,C130/132,D134/135; PHY1A136/138,B140/142,C143/144,D146/148 (+/- respectively) visually match DS00006273C Table6-3 p14. Allfourpairs perusedport arepresent; same-namedwire/port validhere because no seriescomponent isbetween them. Nativehierarchyconnectivity,100ohmdifferentialrules and warningmarkers notverified. Keep MDI symbolpins/ports bidirectional/passive asappropriate; these arebidirectional Ethernetpairs, notordinarydigitalinputs.

Our2CuPHY+2SerDes+RGMII0 topology usesDEV0/1/5/6/7, leavesCuPHY2/3/4unused (LAN DS00006065D Table4-4 p54). Recommend leaveunusedMDI pins individuallyopen andmark intentionalunused withtargetedNoERC atpinends; no joinedNCnet,groundties orresistorsproposed. No explicitmanufacturerunusedMDI terminationparagraph located inreviewedLAN/checklist, so openpin treatment is projectproposal pendingqualification, notquotedrequirement. NC/NoERC doesnotdisablehardware; configureunusedPHYs appropriately inbothbootpaths. Do notapplythisblanketpolicy tootherunusedGPIOs, straps orsupplies.

Altium officialDesignValidation documentation explains rightclickMessages violation -> PlaceSpecificNoERCforthisviolation, preferred overblanketsuppression: https://www.altium.com/documentation/altium-designer/schematic/design-validation . Checkred/orangemarkersonusedPHY0/1 ratherthanhide. OnlyPHY2upperpartvisible; noassertionremainingunusedunitsreviewed.

References: [LAN9645xF DS00006065D Table4-4 p54](../eth_SW/docs/LAN9645xF-Data-Sheet-DS00006065.pdf); [DS00006273C Table6-3 p14](../eth_SW/docs/LAN9645XSF-Hardware-Design-Checklist-Managed-Mode-DS00006273.pdf). No nativeedits/bench/syncverification.

![PHY0 and PHY1 MDI port wiring](images/2026-10-02/lan-phy01-mdi-ports-b11d051f.png)
*OriginaluserWindows/Altiumscreenshot; usedportsvisuallychecked, PHY2partiallyvisibleunused.*


### 2026-10-02 — SFP/LAN SerDes direction mismatch review
Windows/screenshot review. Connectorpage SFP_TXD_P/N feeds SFP TD18/19(inputs), SFP_RXD_P/N comesfrom RD13/12(outputs). LANpage currentlylabels RX0pins62/60 SFPA_TXD_P/N andTX0pins58/56 SFPA_RXD_P/N; similarlyRX1pins72/70 SFPB_TXD andTX1pins68/66 SFPB_RXD. If repeatedsheet mapsbybasename, this connectsinput-input andoutput-output. Correct LAN labels soTX0->SFPA_TXD, RX0<-SFPA_RXD; TX1->SFPB_TXD,RX1<-SFPB_RXD, preserveP/N. Or verify explicitcrossmapping ifalreadyimplemented; nativehierarchy notavailable, no confirmedphysicalshort asserted. Requiredcompiledmapping LAN58/56->SFPA18/19; SFPA13/12->LAN62/60; LAN68/66->SFPB18/19; SFPB13/12->LAN72/70.

ConnectorVCCT16/VCCR15, ground1/9/10/11/14/17/20 andopenRATE_SELECT7 match selectedFinisar pinlist. ModuleinternallyACcouples TDandRD; noadditionalseriescaps required solelyforDCblocking. TX0ohmlinkspresent needmatchedshortlayout, downstream_Rnetpairrulecoverage verifiedinPCB. LANSerDes testpoints needminimalbalancedpad/no branchingstub; pads themselves addloading. RESREF200ohm0.1% meetsdocument200ohm1% requirement. S0_CLKP54openconsistentpreviousclockplan; no fullclock/ISETreview. LOS/TX_FAULTpullups andI2Cbusconnections outsidecrop notfullychecked. SerDesamplitude remainspreviousopenitem, directionfixdoesnotcloseit.

Sources: [LAN9645xF DS00006065D](../eth_SW/docs/LAN9645xF-Data-Sheet-DS00006065.pdf),Table3-2 p22 physicalpins andTable3-3 pp23–24 RXinput/TXoutput/RESREF; [Finisar FTLF8519P3xyL RevC1 Aug14,2014](../sfp/FTLF8519P3xyL_RevC1_8-14-14.pdf),sectionI p2 directions,pinout,internalACcoupling andRATE_SELECT. No nativeedits,ERC,bench orsyncverification.

![SFP connector data paths](images/2026-10-02/sfp-data-connector-review-998a668f.png)
*OriginaluserAltiumscreenshot; TDinputs andRDoutputs asnamed.*

![LAN SerDes labels requiring direction review](images/2026-10-02/lan-serdes-tx-rx-label-review-63ab53b9.png)
*OriginaluserAltiumscreenshot; TXDlabels currentlyonLANRX andRXDlabelsonLANTX.*


### 2026-10-02 — SerDes direction corrected; duplicated port labels found
Windows/Altium screenshotreview. RXlabels nowonLANRX andTXlabels onLANTX, resolving prior directionintent; naming changes SFPA/B toSFP0/1 shown. Criticalvisibletypos: bothS0RXports readSFP0_RXD_N; pin62positive mustSFP0_RXD_P, pin60negative remainsSFP0_RXD_N. BothS1RXports readSFP1_RXD_N; pin72positive mustSFP1_RXD_P, pin70negative remainsSFP1_RXD_N. S1TXrightports stillSFP0_TXD_P/N; mustSFP1_TXD_P/N forpins68/66, matchingSFP1_TXD_P_R/N_R labels. Same-namedports riskshorting RXpair legs andjoining bothTXchannels dependingcompiledhierarchy; notapproved until corrected/netlistchecked.

S0TX58/56rightportsSFP0_TXD_P/N agree; new0ohmTXlinks onLANsheet needreconciliation with earlier0ohmTXlinks onconnectorpage (avoidunintentionalduplicatefootprints). Preservecorrectdifferentialpairdefinitionacross_Rsegmentednets andconsistent0/1 versusA/B sheetentrymapping. S0_CLKP54 nowshowsintentionalunusedmarker. No fullclock/ISETreview ornativeverification.

Requiredmapping: LAN58/56 ->SFP0TD18/19; SFP0RD13/12 ->LAN62/60; LAN68/66 ->SFP1TD18/19; SFP1RD13/12 ->LAN72/70. Sources: [LAN9645xF DS00006065D Tables3-2/3-3 pp22–24](../eth_SW/docs/LAN9645xF-Data-Sheet-DS00006065.pdf); [Finisar FTLF8519P3xyL RevC1 sectionI p2](../sfp/FTLF8519P3xyL_RevC1_8-14-14.pdf). No nativeedits,ERCresults,bench orsyncverification.

![SerDes revised directions with remaining port-name errors](images/2026-10-02/serdes-direction-corrected-port-typos-0a984fbc.png)
*Original userWindows/Altiumscreenshot; positiveRXportnames andSFP1TXportnames needcorrection.*


### 2026-10-02 — SerDes port-name corrections confirmed in screenshot
Windows/Altium screenshot review. All four previously reported port-name errors are visibly corrected: S0_RXP62 now SFP0_RXD_P, S0_RXN60 SFP0_RXD_N; S1_RXP72 now SFP1_RXD_P, S1_RXN70 SFP1_RXD_N. S0_TX58/56 uses SFP0_TXD_P/N through separate 0ohm links with matching _R nets; S1_TX68/66 now uses SFP1_TXD_P/N with matching _R nets. RX/TX direction, channel identity and P/N labels are consistent on this sheet. Prior duplicate-name finding is closed at screenshot level only.

Required end-to-end mapping remains LAN58/56 -> SFP0 TD18/19, SFP0 RD13/12 -> LAN62/60; LAN68/66 -> SFP1 TD18/19, SFP1 RD13/12 -> LAN72/70. Verify compiled hierarchy, A/B-to-0/1 sheet entries, possible duplicate TX0ohm links on connector sheet, and differential rules on resistor-separated segments. Keep SerDes test points small and without branch stubs. S0_CLKP54 intentional-unused marker retained. Clock and ISET circuitry not newly requalified.

References: [LAN9645xF DS00006065D](../eth_SW/docs/LAN9645xF-Data-Sheet-DS00006065.pdf), Table3-2 p22 gives pin assignments and Table3-3 pp23–24 gives directions; [Finisar FTLF8519P3xyL RevC1](../sfp/FTLF8519P3xyL_RevC1_8-14-14.pdf), sectionI p2 defines TD inputs/RD outputs. No native edits, compiled ERC, bench test or OneDrive sync verification.

![Corrected LAN SerDes port names](images/2026-10-02/serdes-port-names-corrected-0e116a61.png)
*Original user Windows/Altium screenshot, archived unchanged; all four previous port-name errors visibly corrected.*


### 2026-10-02 — Five-crop LAN control high-level review
Windows/Altium screenshot review. Images readable for architecture/connectivity and mostvalues; smallannotations insufficient for fullERC/componentaudit. Majorblocks present:boot/managementstraps,SPI,RGMII0,EEPROM/programming,supervisor/hostreset,25MHzcrystal/reference resistors,2SerDes,2copperPHY,LEDs. CorrectedSerDesdirectionandnames retained. PartialunusedPHY2 hasintentionalunusedmarkers; remainder notshown.

Priorityfinding: EEPROMVCC and2.5kI2Cpullups visiblyuse+3V3, unlike earlierdocumentedswitchedVDD_IO_A baseline; LANuses+3V3_ETH_SW_LS. If+3V3alwayson, EEPROMpullupsdrive unpoweredLAN GPIO2/3. Preferredsame-switchedsupply/pullups asLANdomainA, orqualifiedpoweroffisolatingI2Cpath ifoff-stateEEPROMprogrammingneeded. STM SDA0ohmconnection remains whileSCL gated byBPROG: SCLjumperalone notcompletepowerdomainisolation. ConfirmSTMpinpowered-offtoleranceandhighZ,programmerpowerstate andbusownership. LANDS00006065Dsection5.2 p189 explicitlyprohibitsdrivinginputswithoutpower. Noassertionprotectiondiodecurrentlimitallowsit.

Otheropenitems: boot100ModeD defaultcopperLEDmapping conflictsEGPIO47–50; ROMSFPsidebands onGPIO4–7 conflictSPI. No LANSD0/SD1paths shown: decide whetherROMrequiresSDinputs, invert/configureLOS asneeded, andresolveSTMvsROMTX_DISABLE/moduleI2Cownership. Do notblindlywireSDonGPIO42/43 whileassumingModeDLEDfunctions there. SYS_RDYn notexportedtoSTM here; optionalforbackupobservability, notrequirement forboot000. GPIO33alsoEEPROM_SIZEpulldown; initialLOW notproofready. SPIpolledstatus canavoidIRQ; no mandatoryPTP/JTAGaddition.

Resetdividerremains20k/10k nominal0.9V, supervisor3V3tripapprox2.93V; bothbelowLANnormal minima1.0925/3.135V. Thereforecircuit notyetguaranteedbrownout-reset beforeout-of-specoperation; refine thresholds orjustify additionalmonitoring. Clockstartupbound remainsTBD; resetheld>=2msafterrailsandclockstable (DS00006065D Table5-17p200). Supervisor delayalone notproofclockstable. Crystal2x10pF and0ohmXO tuningfootprint areinitialvalues, notvalidatedfrequency/drivemargin. RGMII1k/10kstraps and33ohmoutputlinks needcompilednet andresetlevels verified withSTMhighZ; rise/fall/timing/clockskewdelaysneedbothenddesign, do not simplyenablebothTX/RXinternaldelaysblindly. No obviouspinmissingidentified onvisibleRGMIIblock.

Smallnoteedits: strap-timingreference shouldpointsection5.7.3Table5-17p200 ratherthan5.3; LED_MODE/CUPHYDIPpositions4/5 inactivewithDNP links, reflectfixedLOW. Existingpower,decoupling,connectorTVS/chassis/sharedCTqualification notrechecked bythesecontrolcrops. FullPDF/nativecompilednetlistandERC neededforsign-off; red/orangemarkers unresolved; nofirmwareorhardwaretests.

References: [LAN9645xF DS00006065D](../eth_SW/docs/LAN9645xF-Data-Sheet-DS00006065.pdf),Table3-5 pp30–31,Table3-9 pp35–36,Table3-11p41,Table3-12p43,section5.2p189,Table5-17p200,Table5-28p209. MIC2774 DS20006527A ElectricalCharacteristics p3 (previouslyverified2.93Voption and0.300VINnominal). Findingsareengineeringreview,notnativeedits;OneDrivesyncnotverified.

![LAN control review evidence](images/2026-10-02/lan-overview-management-reset-f4d5c5db.png)
*Original user Windows/Altium crop, archived unchanged; review scope and open findings above.*

![LAN control review evidence](images/2026-10-02/lan-overview-boot-rgmii-41d58043.png)
*Original user Windows/Altium crop, archived unchanged; review scope and open findings above.*

![LAN control review evidence](images/2026-10-02/lan-overview-eeprom-947409b1.png)
*Original user Windows/Altium crop, archived unchanged; review scope and open findings above.*

![LAN control review evidence](images/2026-10-02/lan-overview-serdes-crystal-77e9f7f3.png)
*Original user Windows/Altium crop, archived unchanged; review scope and open findings above.*

![LAN control review evidence](images/2026-10-02/lan-overview-copper-phy-832c71c9.png)
*Original user Windows/Altium crop, archived unchanged; review scope and open findings above.*


### 2026-10-04 — Boot 100 GPIO overrides clarified
Windows/datasheet review. BOOT_MODE=100 (decimal 4) supports EEPROM GPIO configuration; Mode D assignments are defaults, not an immutable pin map. LAN9645xF DS00006065D section 3.4.1.1 p.42 explicitly permits selected GPIO mapping overrides. Section 4.1.2.15 pp.51–52 and Table 4-2 p.52 specify configuration-table register read-modify-write access and GPIO configuration changes. This qualifies earlier backup-mode warnings: custom mapping is supported, but the exact EEPROM image and ROM runtime compatibility have not been verified.

Proposed target pin map, not programmed: GPIO2/3 ALT3 I2C1 SDA/SCL; GPIO4–7 ALT6 SPI client DI/DO/nCS/CLK; GPIO13/14 ALT5 MAC_LED6/5; GPIO16–27 ALT1 RGMII0; EGPIO47/48 ALT2 PHY1_LED0/1; EGPIO49/50 ALT2 PHY0_LED0/1. Hardware alternate functions verified in Table 3-5 pp.30–31. Table 3-12 p.43 retains GPIO_MODE use for ROM LED mapping/runtime, so pin mux writes alone do not establish correct ROM-driven LED behavior. Table 3-9 pp.35–36 gives Mode D defaults; SFP ownership and any ROM sideband accesses must match the overrides, including initial boot before table execution. Exact command format, register writes, timing, and hardware validation remain pending. No rewiring, EEPROM programming, or Altium edits performed.

Source: [LAN9645xF Data Sheet DS00006065D](../eth_SW/docs/LAN9645xF-Data-Sheet-DS00006065.pdf); printed pages equal PDF pages for cited sections.


### 2026-10-04 — GPIO assignment table for BOOT 000 and 100
Windows/Altium screenshot and datasheet review. Shown strap intent is BOOT000 baseline/100 backup, MGMT11 SPI, JTAG_ENB1, GPIO_MODE11, SD_MODE11, XMII_MODE00, SD_125M0, SGPIO_EN0, EEPROM_SIZE0, LOOP1, FLOW1, E2E0, EEE0, LED_MODE0, CUPHY_MODE0, SFP_EN0. Logical settings are consistent with stated intent; electrical margins and actual switch positions/netlist not verified. In 000 unmanaged straps are ignored. In 100 topology must be explicitly set by EEPROM; do not infer standalone operation from straps alone.

Prepared target table: GPIO4–7 ALT6 SPI client; GPIO13/14 ALT5 MAC_LED6/5; GPIO16–27 ALT1 RGMII0; EGPIO47–50 ALT2 PHY1_LED0/1 and PHY0_LED0/1. GPIO2/3 may remain inputs in 000 if STM directly programs EEPROM while LAN reset is held, or use ALT3 only when LAN owns that bus; ALT3 required for 100 ROM I2C boot. Unused connected strap pins should be inputs after sampling under managed firmware. GPIO33 stays input in 000 unless intentionally assigned; SYS_RDYn in 100 is a ROM-controlled function, not an assigned ALT number here. Unwired pins are proposed inputs; suppress unused ROM functions in 100 only with verified configuration support. SFP sideband ownership, ROM LED runtime mapping, and RGMII direct-MAC configuration remain dependencies. No EEPROM generated/programmed and no native edits.

References verified locally: LAN9645xF DS00006065D Table3-5 pp.30–31 (hardware mux), Table3-9 pp.35–36 (Mode D ROM defaults), section3.4.1/Table3-12 pp.42–43 (boot and precedence), sections3.4.2.1–12 pp.44–48 (straps), section4.1.2.15/Table4-2 pp.51–52 (EEPROM override support). LAN96459F EDS2 DS50003971A Fig.A-5 p.34 is a reference table format, not this board's assignment: E49 ALT1 selects MIIM MDC under DS00006065D, while our PHY0 LED0 uses ALT2. Printed and PDF page numbers coincide.

![GPIO straps, SPI and LED wiring](images/2026-10-04/windows-altium-gpio-straps-led-spi-34c63d44.png)
*User-pasted Altium draft, reviewed visually; wiring and configuration not validated in hardware.*

![Boot straps and RGMII wiring](images/2026-10-04/windows-altium-gpio-boot-rgmii-6f2a0766.png)
*User-pasted Altium draft documenting BOOT000/100 intent and RGMII strap sharing.*


### 2026-10-04 — Proposed shared SFP sideband routing
Windows/research. User confirms existing sidebands route to STM32 and asks how to also connect LAN. Proposal only: LOS, module-present and TX_FAULT may fan out to both controllers configured as inputs, with powered-off isolation as required. Candidate LOS routes GPIO42 ALT6 SD0 and GPIO43 ALT6 SD1 (pins39/36); polarity/configuration must match active-high LOS. Candidate module-present inputs GPIO29/28 follow EDS2 example, not guaranteed ROM Mode D remapping. TX_FAULT can remain STM-only unless software requires LAN access. For TX_DISABLE select one controller with a 2:1 selector or mutually exclusive assembly links, defaulting to disabled; never tie push-pull outputs. Two open-drain sinks implement enable-by-either, not disable-by-either. I2C requires explicit bus ownership or controller selection, not an assumed safe parallel connection. Keep identical-address SFP modules on separate selected branches; mux control must work without STM for a truly autonomous fallback. LAN candidates GPIO30 ALT2 SDA, GPIO31/32 ALT2 SCL0/1. Boot EEPROM stays on GPIO2/3 I2C1. Reserve TX_DISABLE GPIO only after confirming ROM configuration tool supports chosen mapping. LAN-owned fallback requires SFP supply enables to work independently of STM as well.

References: LAN9645xF DS00006065D Table3-5 pp.30–31 (mux), Table3-9 pp.35–36 (Mode D SPI conflicts), Table3-11 p.41 (SFP controls), section5.2 p.189 (do not drive unpowered device); LAN96459F EDS2 DS50003971A Fig.A-5 p.34 (GPIO42 SD0, GPIO28/29 module detection and GPIO30–32 SFP I2C reference example); FTLF8519P3xyL Rev.C1 Aug14 2014 sectionI p.2 (LOS active high/open collector, MOD_DEF0 grounded when present, 4.7–10k host pulls, TX_DISABLE high/open disables, rate-select NC), sectionIII p.3 (TX disable >=2V, enable <=0.8V). PDFs in eth_SW/docs and sfp. No part selected, firmware/image validation, or Altium edits.


### 2026-10-04 — SFP DNP forks reviewed; GPIO28 strap conflict
Windows/Altium screenshot review. User intends additional DNP fork links at SFP end; switch-end screenshots show DNP0R for SFP I2C0 SDA GPIO30, SCL0 GPIO31, SCL1 GPIO32, LOS0 GPIO42 and LOS1 GPIO43. These hardware functions match DS00006065D Table3-5 pp.30–31 (ALT2 I2C, ALT6 SD). SFP-side forks not yet shown; both endpoint links must be populated for any enabled branch. Fit one I2C controller path per assembly and account for parallel pullups; maintain correct two-module bus separation. TX_DISABLE ownership is not shown or resolved.

Critical correction to previous GPIO28 recommendation: screenshot connects SFP0 MOD_DEF0 through DNP0R to GPIO28/MGMT_MODE1 (pin79), and SFP1 to GPIO29 (pin78). An installed SFP grounds MOD_DEF0, so populating the GPIO28 path at reset overrides MGMT_MODE1 high and can change intended MGMT11 SPI to MGMT01 I2C. DNP is safe only while open; a larger pullup is not a fix. Do not populate GPIO28 branch as drawn. Prefer another non-strap GPIO input (e.g. spare EGPIO44 or45, subject to ROM remapping support), or verified reset-time isolation that opens for every LAN reset. Earlier advice describing the strap as merely a pullup-compatibility issue was insufficient and is superseded. Module indices also differ from EDS2 example (SFP0 GPIO29, SFP1 GPIO28); custom mapping must explicitly match chosen hardware.

Evidence: LAN9645xF DS00006065D section3.4 pp.42–43 establishes reset-latched straps and MGMT01 I2C /11 SPI; Table3-5 pp.30–31 muxes, section5.2 p.189 unpowered-input prohibition. Finisar FTLF8519P3xyL Rev.C1 Aug14 2014 sectionI p.2 states MOD_DEF0 grounded inside module. Local sources eth_SW/docs/LAN9645xF-Data-Sheet-DS00006065.pdf and sfp/FTLF8519P3xyL_RevC1_8-14-14.pdf. No native edit or hardware validation.

![DNP module detection branches](images/2026-10-04/windows-altium-sfp-moddef-dnp-bb459ad0.png)
*User Altium screenshot: optional SFP0 module-present path conflicts with GPIO28 management strap when fitted.*

![DNP SFP I2C and LOS branches](images/2026-10-04/windows-altium-sfp-i2c-los-dnp-c38b25f5.png)
*User Altium screenshot: proposed optional LAN sideband branches; SFP-end selection not shown.*


### 2026-10-04 — Module detection moved to GPIO8/9; optional breakout recommendations
Windows/Altium screenshot review. SFP0 MOD_DEF0 now connects through DNP0R to GPIO8 pin107, SFP1 to GPIO9 pin106. GPIO28 pin79 retains MGMT_MODE1 without module-present branch; this removes the identified reset strap conflict. Screenshot confirms draft change only, not compiled connectivity. GPIO8/9 must use generic input mode for module detection; stock Mode D SD1/UART_RX roles are not automatic MOD_DET remaps. ROM EEPROM support remains unverified.

Proposed additions, not selected/implemented: highest priority GPIO33 pin74 to STM input/test point, ALT2 IRQ0 in managed000 versus ROM SYS_RDYn in100. Keep EEPROM_SIZE0 strap and STM high impedance during reset; initial strap-low is not proof of ready. Reserve spare EGPIO44/45 (pins116/115) for optional TX_DISABLE control through mutually exclusive branches; generic outputs do not prove ROM remapping support. Optional GPIO0/1 (pins121/120) pads plus GND for ALT5 UART_RX/TX, configurable debug only, not guaranteed ROM console or any UART in no-boot000 without executing code. One EGPIO46 or GPIO29 spare pad is sufficient; no present need for dedicated PTP/recovered-clock headers.

Sources verified: local LAN9645xF DS00006065D Table3-5 pp.30–31 establishes mux options, Table3-9 pp.35–36 Mode D defaults, Table3-11 p.41 SYS_RDYn active-low unmanaged-only, section3.4 pp.42–43 reset straps, section3.4.2.8 p.46 EEPROM_SIZE0 16-bit addressing. Physical pins cross-checked screenshot/reference EDS2 DS50003971A Fig.A-5 p.34. Web search surfaced older RevC indexing; local RevD remains controlling source. No native edits or hardware tests.
![sfp-moddef-gpio8-9](images/2026-10-04/windows-altium-sfp-moddef-gpio8-9-a565f5dc.png)

![gpio28-strap-restored](images/2026-10-04/windows-altium-gpio28-strap-restored-adc38c4a.png)
*Original user-pasted Windows/Altium screenshots; reviewed draft routing.*


### 2026-10-04 — RGMII inline probing proposal
Windows/Altium screenshot review. Selected surface test-point description shows 1.0mm diameter; symbol currently floats above nets. Recommend one small exposed SMT signal pad directly on each continuous RX_D0–3/RX_CLK/RX_CTL route, no branch stub, no added through-hole or dedicated via. Proposed 0.4–0.6mm signal-pad starting size subject to actual probe tip; not a manufacturer requirement. No paste opening, soldermask access, adjacent GND pad/via placed for short probe return. Prefer probe-manufacturer footprint if using solder-in active probe. Place near receiving IC for receiver waveform measurements and retain trace impedance/length rules. One-pin TP symbol attaches at its electrical pin endpoint to the continuous schematic net; schematic placement alone cannot enforce PCB inline routing. Do not add series 0R solely for probing or direct 50ohm scope termination. Low-capacitance active probe preferred for edge/timing assessment; long ground clips can create apparent ringing. No native edit or PCB verification.

Reference: Tektronix application note 51W-60161-2, Probing Techniques for Accurate Voltage Measurements on Power Supplies with Oscilloscopes, p.3 'Probe Loading' and p.4 'Keep Ground Leads as Short as Possible', Figs.3–4: https://download.tek.com/document/51W_60161_2_Probing-Techniques_AN.pdf . These establish generic capacitance/lead-inductance effects; pad size/layout recommendation is project engineering judgment, not an RGMII-specific manufacturer mandate.

![RGMII probe point selection](images/2026-10-04/windows-altium-rgmii-probe-pad-9ef1ced5.png)
*User-pasted Windows/Altium screenshot showing selected 1mm surface test point; inline PCB implementation is proposed.*


### 2026-10-04 — Refreshed paste-ready GPIO tables
Windows/Altium screenshot review. New screenshots show UART on GPIO0/1, module detect SFP0/1 on GPIO8/9, SFP I2C GPIO30–32, GPIO33 SYS_RDYn branch, LOS GPIO42/43, optional TX_DISABLE on EGPIO44/45, and spare EGPIO46/GPIO29 exposure. Prepared separate BOOT000 and BOOT100 runtime-target tables. ALT5 UART requires running internal firmware; no-boot000 does not provide a console merely by pin muxing. GPIO33 proposed ALT2 IRQ0 in000, ROM SYS_RDYn in100; retain EEPROM_SIZE strap and do not treat its initial low as ready. Ordinary GPIO direction is not a numbered alternate function. DNP branches do not establish connectivity until populated at both ends. BOOT100 custom MOD_DET/TX_DISABLE remapping, LED runtime behavior and host SPI coexistence remain unverified, despite available hardware mux choices. No EEPROM image or native edits.

References: LAN9645xF DS00006065D Table3-5 pp.30–31; Table3-9 pp.35–36; Table3-11 p.41; section3.4.1/Table3-12 pp.42–43; section4.1.2.15/Table4-2 pp.51–52. Local source ../eth_SW/docs/LAN9645xF-Data-Sheet-DS00006065.pdf. EDS2 DS50003971A Fig.A-5 p.34 used only as table-format example.

![Latest GPIO optional connections](images/2026-10-04/windows-altium-gpio-final-options-626378b0.png)
*User-pasted Windows/Altium draft: optional SFP, UART and status connections.*

![RGMII and GPIO29 breakout](images/2026-10-04/windows-altium-gpio-rgmii-options-4f2754f9.png)
*User-pasted Windows/Altium draft: straps, RGMII and spare GPIO29.*


### 2026-10-04 — Spare GPIO connections marked bidirectional
Windows/Altium user report: the two spare GPIO connections (GPIO29 and EGPIO46 in the preceding screenshots) changed to bidirectional. Supersedes input-only wording for their schematic interface role in previous tables. Use GPIO I/O / Spare bidirectional breakout in both target tables; actual input/output direction remains firmware-controlled, with input/high impedance proposed until assigned. BOOT100 ROM configuration still requires verification. No new screenshot supplied or native edit verified.


### 2026-10-04 — Pasted GPIO tables visually checked
Windows/Altium screenshot review. Visible BOOT000/100 tables match latest target assignments: GPIO8/9 module detect; GPIO29 and EGPIO46 spare GPIO I/O; GPIO30–32 optional SFP I2C; GPIO33 IRQ0 versus ROM SYS_RDYn; GPIO42/43 SD0/1; EGPIO44/45 optional TX_DISABLE; EGPIO47–50 ALT2 copper LEDs. BOOT100 table explicitly retains unverified custom-target status. No new mapping discrepancy identified in visible text. Suggest spelling managed E44/E45 as GPIO OUT(3) for clarity (note3 already states output only when LAN owns signal); add spare-direction/high-impedance-until-assigned note. Visible references are abbreviated: retain full citations alongside table, especially boot precedence and EEPROM override sections. Review is documentation consistency only, not netlist/EEPROM/hardware validation. No native edit.

Basis: previously verified local LAN9645xF DS00006065D Table3-5 pp.30–31 hardware mux; Table3-9 pp.35–36 Mode D defaults; Table3-11 p.41 SYS_RDYn; section3.4.1/Table3-12 pp.42–43 boot precedence; section4.1.2.15/Table4-2 pp.51–52 EEPROM support. Source ../eth_SW/docs/LAN9645xF-Data-Sheet-DS00006065.pdf.

![Pasted GPIO tables reviewed](images/2026-10-04/windows-altium-gpio-tables-reviewed-be962ef1.png)
*User-pasted Windows/Altium screenshot; table text reviewed, including both spare connections marked bidirectional. BOOT100 remains a proposed configuration.*


### 2026-10-04 — Six-sheet fabrication-risk screenshot review
Windows/Altium visual review, not fabrication approval. EEPROM screenshot shows +3V3 power port and +3V3_ETH_SW_LS on same VCC wire: likely upstream/switched rail merger; remove stale +3V3 assignment and verify compiled net separation. WP jumper source also remains +3V3; use switched EEPROM supply or isolate to avoid powered-off injection. DNP address high pulls use upstream +3V3 too; revise optional population source. Direct STM SDA branch and SCL jumper do not provide two-line powered-off isolation.

RGMII manufacturer Fig8-5 visually verified by rendering local checklist DS00006273C p.21: STM TX data/control/clock -> LAN RX, LAN TX -> STM RX. Switch-side labels/pins consistent, but STM endpoint absent. LAN TX series resistors belong near LAN; STM TX/LAN RX source damping near STM, not automatically near switch just because shown there. RX-side0R placeholders are not verified termination. Both electrical bank supply compatibility and clock-skew configuration remain release gates.

SerDes switch-side mapping passes screenshot check: S0 RX62/60 -> SFP0 RXD P/N; TX58/56 -> SFP0 TXD P/N; S1 RX72/70 -> SFP1 RXD P/N; TX68/66 -> SFP1 TXD P/N. At SFP end TD18/19 are inputs from LAN TX, RD13/12 outputs to LAN RX; connector endpoint not shown. CuPHY0 A122/124 B126/128 C130/132 D134/135; CuPHY1 A136/138 B140/142 C143/144 D146/148 match datasheet. Connector mapping not established. SPI client DI112 must receive STM MOSI, DO111 feed STM MISO, nCS110 and CLK108 from STM. UART RX121 receives adapterTX, TX120 feeds adapterRX. Names alone not end-to-end proof.

Reset remains MIC2774N29 with20k/10k core divider, nominal0.9V core trip and2.93V rail trip; these are below LAN operating minima1.0925V/3.135V and do not qualify full operating envelope. Resolve threshold strategy before release. Power screenshot topology broadly matches1.15V low/core and3.3V high/I/O, but small image not complete pin/cap audit. Verify VDD_PLL155,RTN_PLL154 and grounded ePAD, NC127/131/137/147 open, every supply pad and footprint orientation in native netlist. Core rails post-ferrite/PLL10ohm must remain within limits under load. Crystal10pF each yields5pF plus parasitics, not automatically10pF crystal load; startup/frequency and XI/XO pad parasitics unresolved. ISET6.04k0.5% worst high6.0702k marginally exceeds literal6k1% range6.06k: prefer6.00k1% or6.04k0.1% to satisfy stated range. RESREF200ohm0.1% meets200ohm1%.

Boot100 GPIO/ROM mappings, LED strap low margins and MAC LED drive remain open as already recorded. R? designators and floating alternative resistor symbols visible: annotate and remove/mark non-electrical variants before BOM/ERC. No native edits, tests or fabrication sign-off.

References: LAN9645xF DS00006065D Table3-2 pp.21–22 physical pins, Table3-3 pp.23–24 calibration/SerDes, Table3-5 pp.30–31 mux, section5.2 p.189 rail limits/unpowered signals, section5.8.1/Table5-28 p.209 crystal. LAN9645xS/F Managed Hardware Checklist DS00006273C section8.4/Fig8-5 p.21 establishes RGMII crossing and source resistor placement. Finisar FTLF8519P3xyL RevC1 sectionI p.2 TD/RD directions. MIC2774 DS20006527A Electrical Characteristics p.3/Table5-1 p.11 thresholds. All printed pages cited; local LAN and Finisar sources previously verified. MIC source https://ww1.microchip.com/downloads/en/DeviceDoc/MIC2774-Dual-Micropower-Low-Voltage-Supervisor-DS20006527A.pdf .
![lan-power](images/2026-10-04/fab-review-lan-power-03505284.png)

![lan-rgmii](images/2026-10-04/fab-review-lan-rgmii-b12b4c1a.png)

![lan-control-reset](images/2026-10-04/fab-review-lan-control-reset-21675d98.png)

![lan-eeprom](images/2026-10-04/fab-review-lan-eeprom-4656fe52.png)

![lan-serdes-clock](images/2026-10-04/fab-review-lan-serdes-clock-0cac4a4e.png)

![lan-copper](images/2026-10-04/fab-review-lan-copper-68cedf41.png)
*Original user-pasted Windows/Altium screenshots; reviewed draft, not native-netlist evidence.*


### 2026-10-04 — EEPROM revision: split supplies and DNP STM SDA
Windows/Altium visual review of updated screenshot. EEPROM VCC remains upstream +3V3; SDA/SCL pullups use +3V3_ETH_SW_LS on a separate wire. No direct upstream/switched rail join is visible in this revision. Prior suspected rail-merge finding does not apply to shown wiring; native netlist still unverified. WP high and optional A1/A2 high pulls are +3V3, matching EEPROM VCC, so earlier recommendation to move these to switched power was conditional on switching EEPROM VCC and is not applicable to this split-supply arrangement. Address high pulls remain DNP and10k pulldowns select base0x50; WP low enables writes.

STM SDA link now DNP0R; SCL uses BPROG jumper. With both open the depicted STM branches are disconnected. For STM programming, populate SDA link and install SCL shunt, power both upstream and switched rails, and hold LAN reset. Existing note mentioning SCL jumper only is incomplete. Upstream EEPROM power alone cannot guarantee programming because bus pullups are on switched rail; external programmer pullups must not drive unpowered directly connected LAN pins. DNP SDA is assembly-selectable, not runtime isolation. No arbitrary power-state approval: ensure no STM/programmer high drive or pullups while LAN unpowered and review leakage/power transitions. DS00006065D section5.2 p.189 forbids driving unpowered LAN inputs; section4.1.2.15 p.52 gives EEPROM0x50/400kHz requirements. No native edit or bench test.

![EEPROM split-supply revision](images/2026-10-04/windows-altium-eeprom-split-supply-b8e1f1e7.png)
*User-pasted Windows/Altium screenshot; upstream EEPROM power, switched I2C pulls, DNP STM SDA link and SCL programming jumper reviewed.*


### 2026-10-05 — SFP connector and optional controller paths reviewed
Windows/Altium screenshot review. Likely disconnected TX_FAULT export: connector pin2 net reads SFPX_TX_FAULT_R; detached export/testpoint reads SFPX_TX_FAULT without _R. Join explicitly or use same net label; verify compiled net pin2 to STM. TX18/19 P/N and RX13/12 P/N, VccT16/VccR15, SDA4/SCL5, MOD_DEF0=6,LOS8,TX_DISABLE3, grounds1/9/10/11/14/17/20 match Finisar RevC1 sectionI p.2; rate-select7 NC acceptable. TX disable pullup fitted, pulldown DNP is consistent default-disable; MCU open-drain low enables. Two filters have resistor in capacitor shunt branch, not DC supply path; exact DCR/bias/transient qualification not completed.

MCU I2C/sideband links fitted and LAN links DNP shown. Before LAN I2C population remove STM paths unless a validated ownership arrangement exists. Shared LAN SDA can join two SFP SDA nets and their pullups even when LAN-end resistor is absent; separately switched SFP rails can cross-feed through these pulls. Validate complete population/power matrix or use isolation; no blanket approval of shared-SDA branch. Distinct SCL clocks and isolated STM module selection remain required. Module-present upstream+3V3 pull is usable for unpowered presence detection (module grounds pin6), but populated LAN branch must not expose unpowered LAN to that pull. MCU LEDs are separate active-low outputs, not directly status nets; firmware required. Generic SFPX/power identifiers require correct instance mapping, not literal shared global power across two cages.

Reference: FTLF8519P3xyL RevC1 Aug14 2014 sectionI p.2 pinout/4.7–10k pulls/AC coupling/default TX disable; sectionIII p.3 requires3.15–3.46V at module. LAN9645xF DS00006065D section5.2 p.189 prohibits driving unpowered inputs. No native edit, compiled netlist or hardware verification. Screenshot archived below.

![SFP connector review](images/2026-10-05/windows-altium-sfp-review-4d86d883.png)
*Original user-pasted Windows/Altium screenshot; draft reviewed, TX_FAULT label discrepancy flagged.*


### 2026-10-05 — Correction: TX_FAULT mismatch finding retracted
Windows/Altium screenshot correction. User correctly identifies that there is no SFPX_TX_FAULT_R label. Connector pin2 net and detached testpoint/export wire both read SFPX_TX_FAULT. Prior review misread the screenshot; its alleged label mismatch/disconnected export finding is retracted, not a user circuit fix. Matching same-sheet net labels provide the intended connection; no rename or extra wire is required for this finding. Native compiled connectivity remains unchecked. Other previously recorded power/ownership concerns are independent. Latest screenshot also shows an SFP I2C debug header; no new header validation claimed.

![TX_FAULT label correction](images/2026-10-05/windows-altium-tx-fault-label-correction-187d307d.png)
*Original user-pasted Windows/Altium screenshot confirming matching SFPX_TX_FAULT labels.*


### 2026-10-05 — SFP parent-sheet naming review: incorrect status routing
Windows/Altium screenshot review. Both instances show three incorrect status mappings: MCU_SFPX_TX_FAULT sheet entry connects to ETH_SW_SFPn_MOD_DEF0; ETH_SW_SFPX_MOD_DEF0 connects to ETH_SW_SFPn_LOS; ETH_SW_SFPX_LOS connects to MCU_SFPn_TX_FAULT. Required corrected external ports, top-to-bottom for these three entries: MCU_SFPn_TX_FAULT, ETH_SW_SFPn_MOD_DEF0, ETH_SW_SFPn_LOS (n=0 or1). MCU module-present and LOS first two entries correctly mapped. Both sheet designators visibly read U_Connectors_SFP0; rename lower to U_Connectors_SFP1 while retaining common Connectors_SFP.SchDoc file if intentionally reused. Visible TX/RX P/N and instance indices correct, SCL0/1 and intentionally shared ETH_SW_SFP_I2C0_SDA correct as names only; shared SDA electrical concerns unchanged. Left-side MCU control/I2C/LED and LAN TX_DISABLE instance names consistent.

Prior child-sheet screenshot used SFPX_TX_FAULT and SFPX_*_LED ports whereas current parent uses MCU_SFPX_TX_FAULT and MCU_SFPX_*_LED sheet entries. Child may since have changed; synchronize/check exact parent-entry versus child-port names before release. Also confirm parent ETH_SW_SFPX_RXD/TXD entry names match actual child ports, which earlier screenshots showed as SFPX_RXD/TXD. Do not infer hierarchy resolves based on similar names. No native netlist/source changes or compilation performed; findings based on visible wires/text.

![Parent SFP naming review](images/2026-10-05/windows-altium-sfp-parent-naming-5945d4ce.png)
*Original user-pasted Windows/Altium screenshot; three status mappings per instance and duplicate sheet designator flagged.*


### 2026-10-05 — Retain X names in reusable SFP child sheet
Windows/Altium user decision: retain SFPX naming in reusable Connectors_SFP.SchDoc; parent maps to SFP0/1. Current parent screenshot fixes lower designator to U_Connectors_SFP1. Child screenshot now uses ETH_SW_SFPX_TXD/RXD and MCU_SFPX_TX_FAULT/LED ports matching shown parent naming; previous prefix mismatch concern resolved for visible entries. Three status mappings remain incorrect on BOTH parent instances: MCU_SFPX_TX_FAULT -> ETH_SW_SFPn_MOD_DEF0; ETH_SW_SFPX_MOD_DEF0 -> ETH_SW_SFPn_LOS; ETH_SW_SFPX_LOS -> MCU_SFPn_TX_FAULT. Correct external names respectively MCU_SFPn_TX_FAULT, ETH_SW_SFPn_MOD_DEF0, ETH_SW_SFPn_LOS.

X is literal text, not automatic parameter substitution. Hierarchical child local nets may be repeated under distinct instances; power-port scope needs compiled verification. In particular +3V3_SFPX_LS must not globally join two intended independent switched outputs. User naming choice accepted; no forced renaming. Load-switch image archived for naming context only; no fresh electrical approval or native changes.
![sfp-child-x](images/2026-10-05/windows-altium-sfp-child-x-9392cfa9.png)

![sfp-parent-instances](images/2026-10-05/windows-altium-sfp-parent-instances-49bca286.png)

![sfp-load-switch-x](images/2026-10-05/windows-altium-sfp-load-switch-x-daf6a5a6.png)
*Original user-pasted Windows/Altium screenshots documenting naming convention and current parent mapping.*


### 2026-10-05 — SFP parent status mappings corrected
Windows/Altium screenshot review. Both instances now map MCU_SFPX_TX_FAULT to MCU_SFP0/1_TX_FAULT, ETH_SW_SFPX_MOD_DEF0 to ETH_SW_SFP0/1_MOD_DEF0, and ETH_SW_SFPX_LOS to ETH_SW_SFP0/1_LOS. Supersedes prior outstanding three-status-mapping finding: visually resolved on both instances. Unique U_Connectors_SFP0/U_Connectors_SFP1 designators retained, both reference Connectors_SFP.SchDoc. Remaining visible instance names, RX/TX P/N, MCU I2C, enable/fault, LEDs, TX_DISABLE, LAN SCL0/1 and intentionally shared ETH_SW_SFP_I2C0_SDA are consistent. X retained in reusable child entries. This confirms parent naming/wire mapping only; compiled parent/child connectivity and independent switched-power-net scope remain unchecked, and prior shared-SDA/power-state concerns remain open. No native edits or electrical release approval.

![Corrected SFP parent mapping](images/2026-10-05/windows-altium-sfp-parent-mapping-corrected-57807c5e.png)
*Original user-pasted Windows/Altium screenshot confirming corrected status mappings on both instances.*


### 2026-10-05 — Ethernet top-sheet mapping review: MCU EEPROM SDA/SCL crossed
Windows/Altium screenshot review. On U_ETH_SW_EEPROM, child entry MCU_EEPROM_I2C1_SDA visibly connects to external STM_EEPROM_I2C1_SCL, and MCU_EEPROM_I2C1_SCL connects to STM_EEPROM_I2C1_SDA. Correct parent external mapping SDA->SDA and SCL->SCL. MCU versus STM prefixes may differ across parent wiring, but parent sheet entries must exactly match child ports; earlier child used STM_EEPROM_I2C1_* and may have since changed, so synchronize current source. Failure would break STM EEPROM programming; LAN EEPROM SDA/SCL mapping shown separately remains consistent.

Visible other parent mappings consistent: SerDes SFP0/1 TX/RX P/N, CuPHY0/1 A/B/C/D P/N, RGMII RX/TX groups within LAN hierarchy, SPI CLK/DI/DO/nCS, SFP LOS/MOD_DEF0/TX_DISABLE and I2C SCL0/1/sharedSDA, PHY LED0/1, IRQ/SYS_RDYn and spare GPIO29/46. This is same-name hierarchical routing, not STM MAC endpoint crossing verification. EEPROM_SIZE label appears on both parent wires; intended strap sharing. Upstream+3V3 and switched+3V3_ETH_SW_LS are visibly separate at EEPROM symbol. Previously named voltage-sense rails match visible power entries; component pin/rail electrical checks not repeated. No native compilation or edits; missing endpoints and previously documented power/boot risks remain pending.

![Ethernet top-sheet mapping review](images/2026-10-05/windows-altium-eth-top-mapping-review-4112093d.png)
*Original user-pasted Windows/Altium screenshot: MCU EEPROM SDA/SCL crossing identified; other visible mappings checked for naming consistency.*


### 2026-10-05 — Ethernet top revision: EEPROM crossing remains
Windows/Altium screenshot review. EEPROM MCU entries moved to left side and external prefix changed STM to MCU. Top external MCU_EEPROM_I2C1_SCL still connects horizontally to sheet entry MCU_EEPROM_I2C1_SDA; bottom external MCU_EEPROM_I2C1_SDA still connects to MCU_EEPROM_I2C1_SCL. Prior SDA/SCL crossing remains unresolved. Correct top external to SDA, bottom to SCL, or rewire to like-named entry. LAN EEPROM SDA/SCL pair below appears matched. Other visible mappings remain consistent with preceding review; no new TX/RX or P/N swap identified. Current child-port spelling still requires native synchronization/compiled check. No native file edit or validation performed.

![EEPROM crossing persists in revised top sheet](images/2026-10-05/windows-altium-eth-top-eeprom-still-crossed-2dd0704c.png)
*Original user-pasted Windows/Altium screenshot; changed port placement/prefix does not resolve SDA/SCL crossing.*


### 2026-10-05 — MCU EEPROM crossing corrected on Ethernet top sheet
Windows/Altium screenshot review. MCU_EEPROM_I2C1_SDA now connects to like-named EEPROM sheet entry, and MCU_EEPROM_I2C1_SCL likewise. Prior swapped-pair finding is visually resolved. LAN EEPROM SDA/SCL also match. No additional naming, TX/RX or polarity mismatch identified in the visible top-level mappings. This is parent-sheet visual confirmation only; exact child-port matching, compiled netlist, STM endpoints, power scope, and prior electrical/configuration findings remain unverified/open. No native file edits or fabrication sign-off.

![Corrected MCU EEPROM pair](images/2026-10-05/windows-altium-eth-top-eeprom-corrected-1956b9a9.png)
*Original user-pasted Windows/Altium screenshot confirming SDA-to-SDA and SCL-to-SCL mapping.*


### 2026-10-05 — RJ45 ground-to-chassis capacitor purpose
Windows/Altium screenshot review and EMC reference research. External coupling capacitor now shown10nF100V0603 (earlier screenshot1uF). Explain GND as PCB circuit return and CHASSIS_GND as connector shield/enclosure reference, not necessarily earth. Capacitor gives local high-frequency common-mode return while blocking steady DC; it does not carry intended differential Ethernet data and is not automatically needed for link operation. Ideal10nF reactance15.9ohm at1MHz,1.59ohm at10MHz; real parasitics limit high-frequency behavior. Coupling is bidirectional and can also inject chassis transients/noise into circuit ground. Internal MagJack1nF2kV Bob Smith capacitor serves cable termination and is a separate function.

Propose retaining an optional/DNP tuning footprint pending chassis strategy and EMC/ESD tests, not freezing10nF100V0603 as approved. TI TIDA-010046 design guide TIDUES1A (Feb2020), section2.5.1.1 p.20 recommends1206 chassis coupling footprint and warns packages smaller than0805 may arc duringESD; section2.5.1.2 pp.22–23 discusses alternative bonds and1–4.7nF plane-coupling examples. These are reference-design guidance, not LAN96455 component mandates. Source https://www.ti.com/lit/ug/tidues1a/tidues1a.pdf . Choose voltage/pulse rating and spacing from actual chassis-to-circuit stress; if a required safety insulation barrier is crossed, use appropriate certified safety component/spacing, not plain100V MLCC. No exact replacement selected or native schematic edit.

![RJ45 chassis coupling capacitor](images/2026-10-05/windows-altium-rj45-chassis-cap-d9daa576.png)
*Original user-pasted Windows/Altium screenshot showing external10nF100V0603 GND-to-CHASSIS_GND coupling capacitor.*


### 2026-10-05 ? Full export review status
[Full audit](audits/2026-10-05/FULL_DESIGN_REVIEW.md) and [BOM reconciliation](audits/2026-10-05/bom-reconciliation.csv) cover current26 schematic sheets and123 BOM item rows. Hold fabrication: LAN core/3.3V port join and multipart physical annotation are critical (F01/F02). EEPROM SDA/SCL correction and SFP status correction remain present; no new TX/RX/P-N crossing error found in SerDes hierarchy. RTN_PLL correctly connects to its decoupling capacitor without directGND, per DS00006065D Table3-3 p.28; do not ground it.
BOOT100 is still a conditional custom EEPROM target and does not bypass MCU-controlled power/TX_DISABLE startup. Default BOM omits STM EEPROM SDA R2104, so the BPROG SCL jumper alone does not enable MCU EEPROM programming. DNP branches require both endpoint populations and one owner. SFP literal-X power scope, RGMII timing, SerDes swing/receiver applicability, supply/reset waveforms and two-board loop-free failover remain open. Exact requirements/references and suggested closure steps are in the audit; no native changes or bench sign-off.


### 2026-10-12 - Datasheet correction: two managed SPI-client GPIO mappings
Windows/Codex read-only verification of local LAN9645xF DS00006065D: section 3.3.1 printed/PDF p.29 says managed alternate functions use internal registers; Table 3-5 p.30 lists SI_SLV_DI/DO/nCS/CLK on GPIO0/1/2/3 ALT2 AND GPIO4/5/6/7 ALT6. This corrects the preceding chat answer's implication that only GPIO[7:4] supports SPI client. Existing project wiring uses GPIO[7:4]; no rerouting decision made. Section 3.4.1.2 p.43 identifies MGMT_MODE=3 as SPI client, and section 4.23.1.1 p.164 says strapping automatically takes over appropriate GPIOs, but these passages do not explicitly resolve which group is active at reset. Do not infer reset accessibility of either group solely from the alternate-function table. Confirm reset mapping/remap procedure with Microchip before changing wiring. Source: [DS00006065D](../eth_SW/docs/LAN9645xF-Data-Sheet-DS00006065.pdf), pages above (PDF and printed numbering match). No Altium changes; OneDrive sync not verified.


### 2026-10-12 - Public LAN9645x register source found; EDS2 revision C not verified
Windows/Codex web research following Prescott's user-pasted statement that support is Linux DSA only. Found Microchip's public bsp-6.18-2026 branch register header (~14,887 lines), autogenerated cml-utils 2025-06-26, generator commit 94897ab295c9b59282f07b148fda05aa3e329a68 (not asserted to be repository HEAD). Header sections DEVCPU_GCB:SI_REGS:IF_CTRL/IF_CFGSTAT around lines5344-5364 contain register encodings and field masks. This is code-based register information, not a complete prose programmer manual, and exact LAN96455F silicon applicability/completeness remains to verify.
- [Register header](https://raw.githubusercontent.com/microchip-ung/linux/bsp-6.18-2026/drivers/net/dsa/microchip/lan9645x/lan9645x_regs.h)
- [SPI transport](https://raw.githubusercontent.com/microchip-ung/linux/bsp-6.18-2026/drivers/mfd/lan9645x-spi.c): lines47-56 register offsets, resource arrays lines107-146 target bases, regmap configuration lines230-240 addressing/format. Offsets alone are not full device addresses.
- [Switch implementation](https://raw.githubusercontent.com/microchip-ung/linux/bsp-6.18-2026/drivers/net/dsa/microchip/lan9645x/lan9645x_main.c)
- [Microchip-authored v14 register patch, October8](https://lists.openwall.net/linux-kernel/2026/10/08/839): smaller1932-line autogenerated header; do not mix definitions across versions. [Series](https://lists.openwall.net/netdev/2026/10/08/252). Submission status is not proof of mainline acceptance.
- [EDS2 official public guide](https://ww1.microchip.com/downloads/aemDocuments/documents/UNG/ProductDocuments/UserGuides/LAN96459F-EDS2-Daughter-Card-User-Guide-DS50003971.pdf): search-indexed pages identify DS50003971A, e.g. Appendix A Figure A-1 p.30; fetched PDF metadata48pages but direct page rendering failed. Searches for DS50003971C and revision C did not establish a public copy. Do not claim C does not exist or confuse PCB/schematic revision with guide revision.
Sources accessed October12. A local source download attempt failed due network DNS restrictions; no code/PDF source archive claimed. Public links preserved here. Next pin a coherent source commit and inspect device/revision support plus reset/SerDes/RGMII initialization before STM32 implementation. No firmware/Altium changes, no bench validation, no sync verification.


### 2026-10-12 - Downloadable LAN96455F STM32 reference-source package
Windows/Codex: collected Microchip public sources at pinned repository commit 6ffda6116249fa9e9e892206d5df337821345c76 (bsp-6.18-2026) into [reference ZIP](../output/lan96455f-stm32-reference-2026-10-12.zip). Includes full LAN9645x switch directory, main/SerDes register headers, SPI transport, GPIO pinctrl, PHY/MDIO, IRQ, tagging and EVB configuration examples, licenses and project datasheet. [STM32 guide](../output/lan96455f-stm32-reference-2026-10-12/README_STM32.md) explains source locations, register-address composition, board-specific porting work and unresolved qualification. All 74 upstream files hash-verified; 8863 macros indexed with paths/lines; ZIP CRC verified. This supersedes the earlier failed-download status: actual source files are now archived. It is not complete STM32 firmware, a buildable Linux tree, a complete prose register manual, or bench-validated initialization. No Altium or firmware implementation changes; OneDrive sync not verified.

### 2026-10-12 - Register-source coverage for STM32 firmware
Windows/Codex inspected the pinned source package (commit 6ffda6116249fa9e9e892206d5df337821345c76). SPI interface definitions are concrete: GCB base 0x4000 plus IF_CTRL offset0x1C / IF_CFGSTAT offset0x20 gives byte addresses0x401C/0x4020 and SPI word addresses0x1007/0x1008 (command write flag separate). Evidence: upstream/drivers/mfd/lan9645x-spi.c, offset definitions lines51-52, GCB resource line119, regmap configuration lines243-254, lan9645x_spi_init_regmap and lan9645x_spi_initialize. IF_CTRL comments explicitly define bit0 byte order, bit1 bit order, bits3:2 zero, and special order-independent initialization write patterns. Pinned lan9645x_regs.h lines5953-5976 supplies IF_CTRL[3:0], IF_CFG[3:0] and IF_STAT[16] masks; IF_STAT semantics were not established by these definitions. DS00006065D section4.23.1.1 Table4-57 p164 and transaction/address description p165 supply SPI context (printed/PDF pages match).
Conclusion: enough public source to begin SPI transport and feature-by-feature STM32 porting; not proof of a complete register manual or all legal/reserved field encodings, reset values, access side effects, or complete board initialization. Driver call sites supply implemented settings, not necessarily every supported permutation. Remaining work: map required VLAN/forwarding, PHY, SerDes and RGMII operations to driver sequences, resolve undocumented semantics and revision dependencies, and validate on hardware. No firmware/Altium edits or hardware verification. Sources: [SPI implementation](../output/lan96455f-stm32-reference-2026-10-12/upstream/drivers/mfd/lan9645x-spi.c), [register definitions](../output/lan96455f-stm32-reference-2026-10-12/upstream/drivers/net/dsa/microchip/lan9645x/lan9645x_regs.h), [datasheet](../eth_SW/docs/LAN9645xF-Data-Sheet-DS00006065.pdf). OneDrive sync not verified.

### 2026-10-12 - Reviewed SPI reroute to GPIO0-3 for managed Boot0
Windows/Codex screenshot review, not native schematic/netlist verification. New shown population: STM_ETH_SW_SI_DI ->0R-> STM_ETH_SW_SI_DI_R0 -> GPIO0 pin121 (MOSI); DO -> DO_R1 -> GPIO1 pin120 (MISO); NCS -> NCS_R2 -> GPIO2 pin119; CLK -> CLK_R3 -> GPIO3 pin118. New resistors/test points remain R?/TP? and require annotation. Previous GPIO4-7 links R2034-R2037 are marked DNP. UART links R2052/R2053 and EEPROM links R2054/R2055 are DNP, isolating the competing GPIO0-3 functions if these population states are implemented. No visible DI/DO or CS/CLK swap. Distinct R0-R3 and R4-R7 suffixes preserve branch separation.
Evidence: LAN9645xF DS00006065D Table3-2 p21 verifies package pins; Table3-5 p30 verifies ALT2 SPI-client DI/DO/nCS/CLK on GPIO0/1/2/3. Table3-4 p28 and section3.3 p29 establish supply changes from VDD_IO_B to VDD_IO_A; verify actual rail/STM bank voltage compatibility and no unpowered drive (section5.2 p189). Sections3.4.1.1-2 pp42-43 establish BOOT000 no internal boot, MGMT11 SPI client. User-supplied correspondence attributed to Vishnu explicitly recommends Boot0 management on GPIO0-3 and avoiding simultaneous EEPROM ROM boot/host management; this supports the reset-mapping choice beyond the mux table alone. Source provenance is supplied correspondence, not independently authenticated manufacturer publication; its EDS2 revision-C claim remains unverified. Source: [archived correspondence](references/2026-10-12/user-supplied-vishnu-review-a23fdd8a.txt); [datasheet](../eth_SW/docs/LAN9645xF-Data-Sheet-DS00006065.pdf).
Remaining checks: strap circuit/default table must actually select BOOT000/MGMT11 (not visible here); ensure Altium variant/BOM Not Fitted matches DNP annotations; annotate new parts/TPs; confirm parent and MCU names use identical STM_ETH_SW_SI_* spelling; verify switched VDD_IO_A and STM power states; consider nCS pullup to VDD_IO_A if absent elsewhere (engineering choice, not a cited mandatory resistor). Place optional branch links near the split to avoid long open SPI stubs. EEPROM/UART branches must remain isolated for this population; existing EEPROM fallback is a separate assembly/reconfiguration option, not concurrent managed startup. Firmware and GPIO0-3 ALT2 mapping remain to implement/verify; no firmware, Altium source, hardware or OneDrive-sync verification performed.

### 2026-10-12 - Replacement GPIO schematic-note text prepared
Windows/Codex prepared copy/paste replacements preserving two GPIO assignment/target tables. Both now describe BOOT000/MGMT11, ALT2 SPI client on GPIO0-3, GPIO4-7 unused inputs with old SPI links DNP, UART/EEPROM isolation, and GPIO0-3 VDD_IO_A. Supersedes prior right-hand BOOT100+SPI target note; no concurrent ROM/host boot configuration retained as current target. Other optional sideband/LED/RGMII allocations retained as firmware targets, not verified configuration. References: DS00006065D Table3-5 pp30-31 (mux), Table3-4 pp28-29 (domains), sections3.4.1.1-2 pp42-43 (boot/management), section5.2 p189 (no unpowered drive); supplied Vishnu correspondence archived in references/2026-10-12/user-supplied-vishnu-review-a23fdd8a.txt for Boot0 GPIO0-3 guidance. Text preparation only; no Altium modification. OneDrive sync not verified.

### 2026-10-12 - Managed note updated; EEPROM backup note still needs correction
Windows/Codex reviewed user screenshot 61c06999. Left BOOT000/MGMT11 note now matches reviewed SPI routing: GPIO0-3 ALT2, individually enumerated GPIO4-7 old paths DNP, UART/EEPROM links DNP, VDD_IO_A and no-power-drive warning. Optional runtime assignments remain targets, not validated firmware. User explicitly retains right-hand BOOT100 as a backup; this supersedes prior proposal to make both notes BOOT000. Right screenshot still says SPI host management retained and shows STM32 SPI GPIO4-7, so it does not yet reflect an isolated EEPROM backup assembly. Recommend label EEPROM backup / STM32 SPI disconnected, list GPIO4-7 as no STM connection with actual ROM mux TBD, state four GPIO0-3 STM SPI links removed, R2034-R2037 remain DNP, EEPROM R2054/R2055 populated and EEPROM fitted/programmed, UART optional only after verification. Backup is rework/variant, not strap-only change; MGMT11 must not imply approved simultaneous ROM/host management. No replacement management value selected; backup strap/ROM behavior still requires validation. Supplied Vishnu correspondence section1 is the evidence against combined EEPROM/host operation; DS00006065D Table3-5 p30 lists legal mux choices but does not prove safe simultaneous ownership, sections3.4.1.1-2 pp42-43 distinguish boot/management selections. References: [datasheet](../eth_SW/docs/LAN9645xF-Data-Sheet-DS00006065.pdf), [supplied review](references/2026-10-12/user-supplied-vishnu-review-a23fdd8a.txt). No native Altium edit or compiled/BOM/bench verification; OneDrive sync not verified.

### 2026-10-12 - Shortened managed and EEPROM-backup notes reviewed
Windows/Codex reviewed screenshot eb9cef21; user intentionally omitted some explanatory notes. Left table retains GPIO0-3 ALT2 SPI and GPIO4-7 unused/DNP. Right table now explicitly says BOOT100 EEPROM backup, STM32 SPI disconnected, and unvalidated straps/EEPROM/ROM; numbered actions remove GPIO0-3 STM SPI links, fit R2054/R2055, retain R2034-R2037 DNP, and select optional sideband ownership. Previous concurrent SPI-management wording is removed. Visible text is consistent with intended separate assembly configurations; no need to restore all omitted prose. Minor documentation issue: right-hand TARGET asterisks remain without a definition. Suggested compact legend: '* Proposed target; ROM/EEPROM support must be verified.' This is note review only, not validation of optional ROM remaps, BOM populations, netlist, firmware or native Altium save. OneDrive sync not verified.

### 2026-10-12 - SPI links and test points annotated in screenshot
Windows/Codex reviewed user Windows/Altium screenshot ae0e79c2. New 0R links now R2064 (DI/MOSI ->GPIO0 pin121, TP2006), R2065 (DO/MISO ->GPIO1 pin120, TP2007), R2066 (nCS ->GPIO2 pin119, TP2008), R2067 (CLK ->GPIO3 pin118, TP2009). All four test points are switch-side of their series links. No visible swaps or duplicate references in this crop; previous R?/TP? annotation issue closed for shown components only. GPIO4-7 R2034-R2037, UART R2052/R2053 and EEPROM R2054/R2055 remain marked DNP. Numbering gaps are not errors; full-project uniqueness, actual Not Fitted variant state and compiled connectivity remain unverified. For BOOT100 backup rework, the four STM SPI links to remove can now be explicitly named R2064-R2067. Sources: screenshot below; previously verified DS00006065D Table3-2 p21 (pins), Table3-5 p30 (ALT2 mapping), Table3-4 p28 (domain). [Datasheet](../eth_SW/docs/LAN9645xF-Data-Sheet-DS00006065.pdf). No native Altium edit, PCB ECO or hardware verification performed; OneDrive sync not verified.

### 2026-10-12 - GPIO variant population screenshot reviewed
Windows/Codex reviewed user-pasted variant view da3110b4. R2064-R2067 new GPIO0-3 SPI links show no not-fitted crosses; old GPIO4-7 links R2034-R2037, UART R2052/R2053 and EEPROM R2054/R2055 show crosses. Thus visible SPI assembly matches intended BOOT000/MGMT11 hardware population. This strengthens prior DNP-text-only evidence to a variant-view observation, but variant name and exported BOM are not visible/verified. SFP MOD_DEF0 GPIO8/9 links R2040/R2041, LAN SFP I2C/LOS/TX_DISABLE option links also crossed; consistent with STM sideband ownership, STM-side population not shown. GPIO33 ETH_SW_IRQ_SYS_RDYN series link is crossed: no STM interrupt through that branch as shown; firmware must use available SPI polling/status checks or the branch must be deliberately populated/configured for IRQ0. This is an observation/implementation consequence, not a requirement to fit IRQ. R2031 reset link and visible LED resistors are uncrossed; reset capacitor C2000 crossed. Strap settings and reset timing are outside scope of crop. No native design edits, exported BOM check, compiled connectivity or bench verification. OneDrive sync not verified.

### 2026-10-12 - GPIO33 IRQ link population recommendation
Windows/Codex: recommend fitting GPIO33-to-STM 0R as an optional IRQ0 capability in managed BOOT000, provided the destination is a compatible STM input and EEPROM_SIZE=0 is a resistive pulldown, not a hard GND/closed low-ohm strap. This is a recommendation, not user acceptance or an implemented variant change; prior screenshot has link not fitted. Reverified local DS00006065D Table3-5 p31: GPIO33 ALT2 IRQ0; section3.4.2.8 p46: EEPROM_SIZE applies only to ROM+I2C table boot, 0 selects16-bit addressing; Table3-12 p43: unmanaged straps ignored in Boot0; Table3-11 p41: SYS_RDYn is unmanaged-only. Therefore logical EE_SIZE0 does not prohibit runtime IRQ0, but its physical strap load persists. Keep STM input/high impedance without competing pulls through reset, verify GPIO33 VDD_IO_D/STM voltage compatibility, and configure LAN IRQ mux/source/polarity/ack plus STM interrupt handling before enabling EXTI. Do not interpret reset-time LOW as ready. Polling-only firmware may retain DNP. No mandatory pullup/output-type assertion made; actual strap schematic and IRQ electrical configuration require checking. Source: [DS00006065D](../eth_SW/docs/LAN9645xF-Data-Sheet-DS00006065.pdf), printed/PDF pages match. No hardware edits or sync verification.

### 2026-10-12 - GPIO33 R2003 selected for population
Windows/Altium user update, recorded by Windows/Codex. User accepts GPIO33 IRQ connection and supplies screenshot with R2003 0 ohm no longer marked DNP. Path: ETH_SW_IRQ_SYS_RDYN -> R2003 -> ETH_SW_EEPROM_SIZE / GPIO33, U1800I pin74. Current managed intent BOOT000/MGMT11, EE_SIZE0; GPIO33 intended ALT2 IRQ0, not ROM SYS_RDYn. Supersedes previous screenshot-backed DNP population for this link. Screenshot shows schematic population intent only; native save, active variant Not Fitted state, BOM and compiled STM endpoint were not checked.
Remaining checks from prior review: actual EEPROM_SIZE pulldown/switch network must allow GPIO33 output operation; STM voltage compatibility with VDD_IO_D and high-impedance reset state; configure IRQ mux/sources/ack/polarity and STM interrupt handling. No boot-ready inference from LOW. Source: [LAN9645xF DS00006065D](../eth_SW/docs/LAN9645xF-Data-Sheet-DS00006065.pdf), Table3-5 p31 (ALT2 IRQ0), Table3-11 p41 (SYS_RDYn unmanaged only), section3.4.2.8 p46 (EEPROM_SIZE only ROM+I2C table boot). No Codex Altium edits or hardware tests; OneDrive sync not verified.

### 2026-10-15 - SW1900 strap pull-up/pull-down placement
Windows/Altium work reported by Ethan; recorded by Windows/Codex. Added/positioned SW1900 configuration-strap pull-up and pull-down resistors as inline with the signal paths as practical. User reports pull-downs on the top layer and pull-ups directly underneath on the bottom layer, intending a through-hole via at each relevant strap node to reach both sides. Records physical placement and planned via connection, not series insertion of the shunt pull resistors. Screenshot shows placement around U1800 with remaining airwires; native layer assignments, individual nets and completed routing were not independently verified. No resistor-value, strap-state or boot-mode change reported.
Next: complete the intended connections and verify strap-node continuity, layer/clearance rules and short branches in native PCB data. No Codex Altium edits, DRC result, hardware test or OneDrive sync verification.
Evidence: [archived placement screenshot](images/2026-10-15/windows-altium-sw1900-strap-pu-pd-placement-90e5c596.png).

### 2026-10-17 - EEPROM placement and STM I2C connection intent
Windows/Altium work reported by Ethan; recorded by Windows/Codex. EEPROM circuit placed below the Ethernet switch in the layout, on the far side of the switch from the STM32. "Below" describes placement relative to the switch, not a confirmed bottom-layer assignment. User rationale: STM32-to-EEPROM I2C is not planned to be electrically connected in the intended assembly, so proximity to STM32 is not a placement priority. This supersedes any interpretation of the historical shown STM EEPROM programming interface as an actively connected requirement. Exact isolation/population implementation and optional programming method remain unverified; no resistor designators or removal of optional schematic paths inferred. BOOT000/SPI management and separate EEPROM backup intent otherwise unchanged. No screenshot supplied for this update; placement is user-reported, not native-file verified. Next reconcile optional STM EEPROM links with variant/BOM and continue routing. No Codex Altium edits, tests or OneDrive sync verification.

### 2026-10-17 - SFP MAC_LED5/6 placement
Windows/Altium work reported by Ethan; recorded by Windows/Codex. Placed the SFP indicator LEDs associated with MAC_LED5 and MAC_LED6. Supplied 3D screenshot shows two SFP cages with LED components adjacent to the cage area. Exact top/bottom cage-to-LED assignment is not inferred from this unlabeled view. Records placement progress only; no net mapping, polarity, resistor value, firmware LED configuration or completed routing change verified. Next verify LED connections/polarity and visibility/access with the mechanical assembly, then complete routing. No native Altium edits or saved-state verification by Codex; no hardware test or OneDrive sync verification.
Evidence: [SFP LED placement screenshot](images/2026-10-17/windows-altium-sfp-led-placement-1c018f84.png).

### 2026-10-17 - EEPROM, reset and service-header placement update
Windows/Altium work reported by Ethan; recorded by Windows/Codex. User reports placement of (1) EEPROM circuit, (2) STM32-to-Ethernet-switch reset circuit, (3) Ethernet-switch EEPROM headers and (4) Ethernet-switch debug UART headers. Full-board 3D screenshot archived as the latest overall placement evidence. This consolidates prior EEPROM/reset placement updates and adds the EEPROM/debug-UART header placement milestone. Header presence does not establish populated alternate links or active UART/EEPROM operation in the managed assembly; previous STM-to-EEPROM disconnected intent remains unchanged. No new electrical topology, boot-mode or population change reported. Routing completion, connector pinouts/orientation, accessibility, DRC and native saved state not independently checked. Next continue routing and verify header connections and mechanical access. No Codex Altium edits or bench test; OneDrive sync not verified. Prior discussion of reverting a file does not establish that a revert occurred.
Evidence: [full-board placement screenshot](images/2026-10-17/windows-altium-eeprom-reset-debug-headers-placement-6863c0b3.png).

### 2026-10-18 - Ethernet-switch headers, switches and LEDs labeled
Windows/Altium work reported by Ethan; recorded by Windows/Codex. Added silkscreen labels for Ethernet-switch headers, configuration switches and LEDs. Supplied 3D view shows EEPROM I2C and UART header signal labels, SW1900/SW2000/SW2001 designators and per-position function labels, SFP0/SFP1 labels, SFP header SCL/SDA/GND markings, DET/FLT indicators, ETH_SW RESET, BPROG and EEPROM_WP markings. Records screenshot-backed annotation progress, not independently verified pin mapping or fabrication output. Prior unresolved DIP actuator ON-direction question remains open; this image does not establish a logic1/0 slide direction. Next check label-to-pin/function agreement, readability after assembly and silkscreen-to-mask clearances before output release. No native Altium edits, DRC or OneDrive sync verification by Codex.
Evidence: [silkscreen screenshot](images/2026-10-18/windows-altium-eth-switch-silkscreen-9f227483.png).


### 2026-10-23 - Microchip corrects SPI pin guidance; documentation and initialization follow-up
**Computer/tool:** Windows/documentation; Microchip support correspondence. I received and archived Vishnu R.'s reply, labeled October 23, 2026, 01:06 (message timezone unspecified), verbatim: [archived Vishnu reply](correspondence/2026-10-23_Microchip_Vishnu_reply.txt).

- **SPI correction:** Vishnu explicitly retracts his earlier restriction of GPIO4-7 to hybrid EEPROM/host operation. He states that GPIO0/1/2/3 ALT2 and GPIO4/5/6/7 ALT6 are both valid SPI client groups, mapping respectively DI/DO/nCS/CLK. He cites *LAN9645xS/F Hardware Design Checklist (Managed Mode)*, DS00006273C, Rev C, section 9.1, Table 9-1, printed p.23, Note 3: only one GPIO option per SPI signal may be enabled. He also cites *LAN9645xF Register Information*, CPU / IF_REGS / GENERAL_CTRL, IF_SI_SLV_A_ENA and IF_SI_SLV_B_ENA; document revision/page not supplied. These are vendor-attributed references, not independently reverified in this logging session.
- **Reconcile with actual design:** His description of my schematic using GPIO4-7 appears to refer to an older snapshot: my October 12 record selected GPIO0-3 with alternate links DNP. This reply removes the claimed necessity of that migration; it does not establish a reversal in my current schematic or population. Pin-group selection and fitted links still need reconciliation with the current native design.
- **Boot:** Vishnu supports BOOT000 with STM32 SPI management. He still cannot validate simultaneous ROM/EEPROM boot and host management, and recommends dropping BOOT100 if unnecessary. I have not recorded a new decision to remove the optional backup circuitry/table; the prior separately reconfigured backup intent is not silently overwritten.
- **SDE / NDA:** He suggests requesting SDE access without a reference number while describing the university project and an NDA in progress, and contacting Microchip sales/distribution to initiate an NDA. No NDA is established as in progress by this reply; any application must describe my actual status. He believes the full register information and Linux sources are already in the case collateral; local possession of the complete register document remains to verify. Access links are preserved in the original reply. No application, NDA or message sent in this logging task.
- **Firmware:** He identifies drivers/net/dsa/microchip/lan9645x/ and net/dsa/tag_lan9645x.c as references. His stated reset sequence is CORE_ENA=0, RAM_INIT=1, poll RAM_INIT until 0 (3 us interval, 100 ms timeout), then CORE_ENA=1. He distinguishes this from unmanaged SYS_RDYn. Treat this as vendor guidance pending comparison with our pinned source and full register definitions, not executed or validated STM firmware. The phrase 'before touching any other switch register' needs qualification: establishing SPI and writing the reset/RAM registers necessarily precede that poll. Confirm transport readiness, any earlier setup, addressing and exact sequencing before implementation.
- **Scope / errata:** He says the Linux driver supports sequencing but covers only registers it uses; full numeric maps/bitfields require Register Information. STM firmware must supply port/RGMII/SerDes configuration normally supplied through device tree. He points to public DS80001187 errata and describes items 5/6 as unmanaged limitations; revision and page not given. Full errata applicability remains to review rather than treating this as blanket clearance.

**Next actions:** reconcile the reviewed schematic revision and choose the intended SPI group; locate/check DS00006273C and complete register collateral; compare reset/RAM-init guidance with pinned Linux source; decide explicitly whether to retain isolated BOOT100 backup; review applicable silicon errata and pursue SDE/NDA only as needed. No native Altium changes, firmware implementation, hardware tests or OneDrive sync verification in this logging task. Submission PDF not regenerated.


### 2026-10-23 - Select SPI GPIO4-7 again; screenshot net-label review
**Computer/tool:** Windows/Altium screenshot review. I selected a return to managed SPI on GPIO4-7 after Vishnu corrected his earlier restriction. My supplied screenshot shows R2034-R2037 0 ohm fitted and R2064-R2067 DNP, superseding the October 12 GPIO0-3 selection. BOOT000/MGMT11 intent remains; no new decision about removing the isolated BOOT100 option is inferred.

| STM-side net | Series link | LAN-side net shown | LAN pin / function |
|---|---|---|---|
| STM_ETH_SW_SI_DI | R2034 | STM_ETH_SW_SI_DI_R4 | GPIO4 /112, SPI DI, STM MOSI |
| STM_ETH_SW_SI_DO | R2035 | STM_ETH_SW_SI_DO_R5 | GPIO5 /111, SPI DO, STM MISO |
| STM_ETH_SW_SI_NCS | R2036 | STM_ETH_SW_SI_NCS_R6 | GPIO6 /110, active-low CS |
| STM_ETH_SW_SI_CLK | R2037 | STM_ETH_SW_SI_CLK_R7 | GPIO7 /108, SCK |

Verified against the actual local *LAN9645XS/F Hardware Design Checklist (Managed Mode)*, DS00006273C Rev C, section 9.1, Table 9-1 and Figure 9-1, printed/PDF p.23: GPIO4-7 use ALT6, GPIO0-3 use ALT2, and only one option per SPI signal may be enabled. Figure 9-1 establishes MCU DO to switch DI and MCU DI to switch DO. [Local reference](../eth_SW/docs/LAN9645XSF-Hardware-Design-Checklist-Managed-Mode-DS00006273.pdf). This independently checks the pin-table citation from the preceding correspondence log.

**Label cleanup recommended, not implemented by this review:** GPIO0-3 still carry the old STM_ETH_SW_SI_DI_R0 / DO_R1 / NCS_R2 / CLK_R3 names although R2052/R2053 now feed UART_RXD/TXD and R2054/R2055 feed EEPROM_I2C1_SDA/SCL. These names are misleading, not by themselves an electrical short: the four alternate STM SPI resistors are DNP. Proposed generic LAN-node names ETH_SW_GPIO0 / ETH_SW_GPIO1 / ETH_SW_GPIO2 / ETH_SW_GPIO3 avoid naming the shared node after an inactive function. Rename every occurrence on each corresponding node, including both the DNP SPI branch and UART/EEPROM link side, while preserving left-side STM, UART and EEPROM interface names. Verify compiled connectivity and net-name/ECO impacts; screenshot alone does not establish cross-sheet mappings or active variant population. UART RXD/TXD direction must also match the external header convention; SPI names DI/DO are from the switch perspective.

Next: apply the label cleanup if adopted, confirm ALT6 firmware and one enabled group, update stale target/strap notes, and check the PCB ECO and variant/BOM for R2034-R2037 fitted / R2064-R2067 not fitted. GPIO4-7 are in VDD_IO_B per the displayed symbol; confirm matching powered-domain interfaces. No native Altium changes, hardware tests, submission-PDF refresh or OneDrive sync verification performed in this review.

![Managed SPI return to GPIO4-7 and remaining GPIO0-3 label cleanup](images/2026-10-23/windows-altium-spi-gpio4-7-return-a68bb817.png)
*My original Windows/Altium screenshot: GPIO4-7 links shown fitted and GPIO0-3 SPI alternatives DNP; UART/EEPROM-side LAN labels still use old SPI names. Screenshot-backed intent, not compiled verification.*


### 2026-10-23 - Retain net names; GPIO4-7 variant stuffing review
**Computer/tool:** Windows/Altium screenshot review. I chose to retain the existing semantic net-name scheme unless a functional connectivity error is identified. This supersedes the preceding optional generic GPIO0-3 renaming recommendation; no renaming is required solely for style.

Shown variant population is consistent with the selected GPIO4-7 SPI path: R2034-R2037 fitted; R2064-R2067 crossed out/not fitted. R2052/R2053 UART links and R2054/R2055 switch-to-EEPROM I2C links are shown fitted. The GPIO0-3-side STM_ETH_SW_SI_*_R0-R3 labels are distinct from the active R4-R7 nets and from the unsuffixed STM SPI nets; with the alternate resistors not fitted, the visible labels do not themselves bypass the intended isolation. No functional label conflict is visible within this crop. Cross-sheet duplicate labels and native compiled connectivity remain unchecked. Fitted switch-to-EEPROM links do not establish an STM-to-EEPROM connection or a BOOT100 strap selection.

Other visible not-fitted components: R2000-R2002 (LAN SFP I2C), R2012/R2015 (LAN SFP LOS), R2018/R2019 (LAN TX_DISABLE), R2040/R2041 (LAN module-detect alternatives), C2000 (reset capacitor). These agree with the shown optional/DNP annotations; this crop cannot prove the separate STM SFP-control/monitoring paths. R2003 IRQ link, R2022 spare GPIO, R2023/R2028-R2030 LED resistors and R2031 reset link appear fitted. Reset/IRQ links retained; no new electrical qualification of LED loading or reset timing inferred.

Reference for supported SPI routing: local *LAN9645XS/F Hardware Design Checklist (Managed Mode)* DS00006273C Rev C, section9.1/Table9-1/Figure9-1, printed/PDFp23 (checked in preceding review): GPIO4-7 ALT6 and one enabled option per SPI signal. [Local checklist](../eth_SW/docs/LAN9645XSF-Hardware-Design-Checklist-Managed-Mode-DS00006273.pdf). Assembly population is a project choice; the screenshot shows one variant view, not every variant or exported assembly BOM. Next confirm the intended variant is selected for BOM/assembly outputs and check compiled STM SPI connectivity. No native Altium edit, test, PDF regeneration or OneDrive sync verification performed.

![GPIO4-7 variant stuffing with retained net names](images/2026-10-23/windows-altium-gpio4-7-variant-stuffing-45484ca7.png)
*Original user-pasted Windows/Altium variant view; red crosses indicate the shown not-fitted parts. Visual population review only.*


### 2026-10-23 - GPIO target tables updated; managed-mode footnotes still stale
**Computer/tool:** Windows/Altium screenshot supplied by Ethan; documentation review. I updated the GPIO assignment/target table text. The latest screenshot shows the BOOT000/MGMT11 table with GPIO0/1 ALT5 UART, GPIO2/3 ALT3 EEPROM I2C1, and GPIO4-7 ALT6 STM32 SPI DI/DO/nCS/CLK. The BOOT100 backup table retains GPIO4-7 STM32 SPI paths disconnected, optional UART, and EEPROM I2C1; its header still explicitly says backup operation is not validated. This is screenshot-backed annotation progress, not verified runtime mux configuration or a native-file edit performed during this logging task.

**Remaining inconsistency:** left-hand notes 1-2 below the updated table still say SPI uses GPIO0-3 ALT2/VDD_IO_A, fit STM32-to-GPIO0-3 links, keep R2034-R2037 DNP, keep UART R2052/R2053 and EEPROM R2054/R2055 DNP, and reserve GPIO0-3 for SPI. These statements contradict the revised table and preceding variant screenshot. Pending note correction: SPI GPIO4-7 ALT6/VDD_IO_B; fit R2034-R2037; DNP R2064-R2067; UART R2052/R2053 and switch-to-EEPROM R2054/R2055 fitted in the shown variant. Update/remove the stale terminal-firmware GPIO0-3 reservation instruction. DI=MOSI and DO=MISO remain from the switch perspective. Keep the BOOT100 backup notes distinct: they describe a separate disconnected-host configuration, not current managed stuffing.

No new routing, net-naming, strap or backup-selection decision inferred. Prior decision to retain existing net names remains. Next finish footnote reconciliation and check schematic/variant/BOM consistency. No hardware test, native connectivity check, submission-PDF regeneration or OneDrive sync verification.

![Updated GPIO assignment tables with stale managed footnotes](images/2026-10-23/windows-altium-gpio-target-tables-updated-2b15018a.png)
*Original user-pasted Windows/Altium screenshot: table rows updated for GPIO4-7 SPI; managed notes 1-2 underneath still contain superseded GPIO0-3 routing/population instructions.*
