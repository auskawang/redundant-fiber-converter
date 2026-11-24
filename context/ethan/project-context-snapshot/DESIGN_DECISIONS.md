# Design Decisions and Open Items

## Current decisions

### 2026-09-15 — Shared project context

- OneDrive directory `project_context` is the durable handoff between Mac research and Windows Altium work.
- Both Codex tasks should read these files before working and update them when decisions change.

### 2026-09-15 — System topology

- Use two identical boards and two laptop user-data endpoints.
- Provide two independent, full-duplex 1000BASE-SX channels: primary and backup.
- STM32 devices exchange management telemetry in-band without intentionally pausing user traffic.
- Board A acts as controller/master and Board B as agent through firmware configuration.

### 2026-09-15 — Power topology

- Generate 1.8 V as a first-stage buck from `VIN_SYS`, not from 3.3 V.
- Generate 3.3 V with a first-stage buck rated at least 3 A.
- Place a load switch after the 3.3 V buck for the Ethernet-switch branch.
- Feed the 1.15 V buck from that switched Ethernet 3.3 V branch.
- The Ethernet load switch therefore carries approximately 1.188 A maximum, including the 1.15 V buck input at 80% assumed efficiency.
- Place independent load switches on the two SFP 3.3 V branches.
- TPS22950CDDCR is the current candidate for all three protected branches.

### 2026-09-11 — SFP host mechanical components

- Freeze Molex `74441-0010` as the SFP host receptacle and Molex `74737-0010` as the separate overlapping cage component.
- Use a common manufacturer datum for the two footprints so the connector and cage align when placed at the same PCB location.

### 2026-09-14 — Regulator and via-in-pad library implementation

- Use Microchip `MIC22405YML-TR` as the working 1.15 V synchronous-buck selection, subject to final electrical calculations and schematic review.
- Use the Texas Instruments `DDC0006A` manufacturer land pattern for `TPS22950CDDCR`: six 1.10 mm x 0.60 mm pads on 0.95 mm pitch with 2.70 mm opposing-row center spacing.
- Document exposed-pad via-in-pad arrays as IPC-4761 Type 7, filled and capped on both sides, with top and bottom solder-mask tenting set manually in Altium.
- The Altium Type-7 property is fabrication documentation only; the JLCPCB order must explicitly request epoxy-filled and copper-capped VIPPO processing.

### 2026-09-16 — RGMII0 interface and strap coexistence

- Use LAN96455F `RGMII0` on GPIO16–27 for the STM32N6 Ethernet-MAC connection.
- Retain required external configuration-strap networks on the shared RGMII0 transmit pins GPIO22–27. The straps are sampled at reset release and remain as shunt loads during runtime; this overlap is intentional and appears in Microchip's EDS2 reference design.
- Place these strap resistors at the LAN96455F routes with no branch stubs. Final fixed resistor populations remain open until the boot/management mode and strap voltage margins are frozen.
- Ensure the STM32 pins receiving the switch's RGMII0 transmit group remain inputs/high impedance during LAN96455F reset and strap sampling.

## Open items

- Freeze exact STM32N6 ordering code and package.
- Confirm final STM32N6 power scheme, startup control, external inductor, and 1.8 V rail partitioning.
- Select USB-C PD sink/input protection implementation and input voltage profile.
- Select 3.3 V, 1.8 V, and 1.15 V buck regulators.
- Verify Ethernet-switch load-switch dropout against LAN96455F 3.3 V tolerance across temperature.
- Determine required bulk and local decoupling from each regulator and device reference design.
- Freeze RJ45 magnetics/connectors and decide whether the debug copper port is populated.
- Freeze managed/unmanaged boot mode and LAN96455F strap resistor values.
- Complete the remaining port assignment table for two SerDes/SFP ports and copper ports; RGMII0 is selected for the STM32 interface.
- Define failover thresholds, hysteresis, heartbeat timeout, and split-brain behavior.

## Change-log rule

Append new dated decisions. If a decision is superseded, retain the old entry and clearly mark it superseded rather than silently deleting design history.

### 2026-09-15 — LAN96455F support schematic started (proposal)

- User deferred power and main data-interface wiring; start with control/configuration, straps, and reset.
- Proposed SPI-managed baseline: BOOT_MODE=000 and MGMT_MODE=11, pending STM32 firmware feasibility. This does not freeze the previously open boot decision.
- Clarified that the prior 24AA014H selection is conditional on ROM/EEPROM boot, not a requirement of no-boot managed operation.
- Detailed proposed connections, shared-pin reservations, and reset timing conflict are in `LAN96455_CONTROL.md`.

### 2026-09-16 — Clarified EEPROM startup versus no-boot proposal

- The 2026-09-15 BOOT_MODE=000 proposal applies only to externally initialized managed operation; it is not an unmanaged EEPROM boot setting.
- For the EEPROM-autonomous-startup approach now under discussion, use BOOT_MODE=100 as the candidate, with MGMT_MODE=11 for SPI client. EDS2 Table 3-5 (DS50003971A p. 20) explicitly supports this combination.
- Do not freeze overall STM32 compatibility until RGMII0 direct-MAC configuration, five-port topology, and internal-ROM versus STM32 runtime-control ownership are verified. This clarification does not claim an Altium change or finalized boot selection.

### 2026-09-16 — Preserve STM32 default and EEPROM backup intent

- User clarified that the EEPROM schematic is a backup option; it does not replace the original 000/SPI initialization plan. Supersedes any interpretation of the previous entry as a default change.
- Recommend retaining 000/11 as intended default, with selectable 100/11 for ROM+EEPROM recovery/bring-up. Neither path has been validated with project firmware; 100 is not an automatic or proven full-feature fallback.
- Verified the five-port hardware resource combination in DS00006065D Table 4-4, p. 54. Added evidence and conditions to LAN96455_CONTROL.md; STM32 driver feasibility and ROM-mode direct-MAC/control ownership remain open.


### 2026-09-16 - EEPROM working selection and independent-power intent

- User schematic now uses 24CSM01-I/SN, superseding the earlier 24AA014H working selection.
- User intends +3V3_ETH_SW_EEPROM = +3V3_MAIN upstream of the Ethernet load switch for programming while switch supplies are off. Retain this intent pending a valid two-line isolation solution; the screenshot's direct connection is not approved (DS00006065D section 5.2 p. 189).
- Boot default remains 000/SPI with manually selected 100 backup. Address straps and pull-up values require correction before backup validation. See LAN96455_CONTROL.md for evidence; no Altium source edits or final isolation-part selection performed.


### 2026-09-16 - Clarified EEPROM address population; supply alternative proposed

- User confirms A1/A2 upper 0-ohm links are intended DNP. Correct intended base address is 0x50; mark actual schematic/variant/BOM accordingly. Supersedes interpretation that high address straps are intended populated.
- User considers shared switched VDD_IO_A power with full switch powered/in reset during EEPROM programming. This is not yet a confirmed replacement of the earlier upstream-supply intent or an implemented change.
- 8.25 kohm was chosen for Standard mode; valid for assumed 100 pF/100 kHz, but full ROM backup-boot suitability remains unapproved. Recommend candidate 2.2 kohm for 100/400 kHz; no final value change asserted.


### 2026-09-16 - Revised EEPROM draft adopts switched rail and 2.5 kohm pulls

- User's latest screenshot adopts +3V3_ETH_SW_VDD_IO_A for EEPROM supply and all local pull-up sources; supersedes earlier always-on EEPROM branch intent. Program with full switch power and reset asserted.
- User selects 2.5 kohm 0.1% SDA/SCL pull-ups; calculated 212 ns at assumed 100 pF supports 100/400 kHz. Supersedes displayed 8.25 kohm choice; prior 2.2 kohm was only a suggestion.
- A1/A2 upper zero-ohm options now visibly marked DNP; pulldowns set base 0x50. Native variant/BOM, ERC and bench verification remain pending. Screenshot review is not full-board sign-off.

### 2026-09-17 - Single LED per populated copper port

- User chooses one physical combined link/activity status LED for each populated copper port, superseding the two-LED indication proposal. Optional debug copper remains optional.
- Candidate active-low PHY_LED0 routes and resistor example are in LAN96455_CONTROL.md. Firmware/EEPROM combined indication setup remains open; physical single-LED population alone does not trigger automatic single-LED mode on the two-output 156-pin Mode A mapping. No Altium implementation claimed.

### 2026-09-18 - Reconsider GPIO Mode D for backup

User proposes Mode D for one RGMII/two SerDes. Supersede unconditional preference for Mode A with conditional Mode D candidate (11); no final schematic migration approved. Detailed LED migration and SPI-client GPIO4-7 versus Mode D SFP-sideband conflict recorded in LAN96455_CONTROL.md. Resolve boot100 GPIO precedence, SFP I2C/TX_DISABLE ownership, JTAG/strap loading, and single-indicator behavior before freezing. Normal 000 remains STM32-managed; five-port topology unchanged.

2026-09-22 manufacturer-based pad override reviewed: user screenshot unchecks calculated footprint values, X0.66mm, Y1.00mm, C2.60mm with pitch0.95mm. Matches DS20006527A p.12 SOT23-5LD-PL-1 pad width0.66+/-0.02, length1.00+/-0.02 and equivalent row pitch2.60+/-0.05. Nominal pad outer span C+Y=3.60mm differs from redundant drawing nominal3.63mm but lies within printed3.63+/-0.05mm; disclose rather than silently assert exact nominal agreement. Keep manual override as manufacturer-based footprint choice, superseding preliminary IPC-calculated pad geometry; prior fillet goals not guaranteed by override. Final pad numbering, orientation, masks/courtyard still to inspect. No native edit claimed. Source: https://ww1.microchip.com/downloads/en/DeviceDoc/MIC2774-Dual-Micropower-Low-Voltage-Supervisor-DS20006527A.pdf .

### 2026-09-22 - Reset input bias and manual switch draft adopted
Latest screenshot populates10k nMR pullup to switched3V3_ETH_SW_LS, superseding earlier DNP population, and labels switch8 default0/open for normal operation. Maintained closure holds reset. Input/output100nF timing capacitors remain DNP. LED remains1k despite3.3k recommendation; no acceptance of current margin or completed threshold validation implied. See LAN96455_CONTROL.md latest review.

### 2026-09-22 — Evaluated unmanaged EVB contingency (not adopted)

- EV03E14A can provide standalone onboard copper-to-SFP switching without PHY daughter cards; user-provided USD400 is a budgeting assumption, not verified price/stock.
- Recommend staged contingency: one known-good optical link, live DOM access, then verified control/failover. SFP I2C is accessible at J35 in GPIO mode A; no turnkey DOM API or custom failover capability confirmed.
- Two replacement endpoints require two baseboards or a compatible alternative far end. Preserve original custom-board/default-boot plan; no purchase or scope change authorized/performed. Details in CONTINGENCY_EVB.md.

### 2026-09-22 — Single-EVB optical bench topology proposed

- One EV03E14A can in principle emulate two media-converter ends using separate VLAN access groups: CuPHY0+SFP0 and CuPHY1+SFP1, bridged only by external untagged duplex fiber. Requires explicit VLAN configuration; flat defaults would allow a copper shortcut and a loop.
- Engineering proposal based on DS00006065D Table4-2 p.52, Figure4-7 p.76 and section4.11.4.2 p.91. Validate by disconnecting fiber and confirming traffic stops. No implementation claimed; both SFP cages are consumed by one link. Original project architecture unchanged.

### 2026-09-23 - Reference-based crystal loading proposed

Propose Ca=Cb=10 pF C0G/NP0 for VXM7-9032, following EDS2 DS50003971A Figure A-4 p.33 and Table B-1 p.39. Provisional engineering starting point, not measured 10 pF total load or verified 5 pF LAN stray capacitance. Confirm with FAE and final-board startup/frequency testing; see COMPONENT_SELECTIONS.md. No capacitor MPN frozen or Altium implementation asserted.

### 2026-09-23 - Unused clock pins

With25MHz crystal and SD_125M=0, leave S0_CLKP54 unconnected per DS00006065D section4.22 p.163. Leave CK25OUT151 unconnected because no project clock consumer is allocated; output identity per Table3-3 p.25. Add intentional-unused markers only; no grounding or interconnection. Supersedes pending S0_CLKP disposition, not yet screenshot-verified implementation.

### 2026-09-23 - CK25OUT monitoring point shown

User screenshot adopts ETH_SW_CK25OUT/TP? on pin151, superseding earlier proposal to leave CK25OUT open. Keep S0_CLKP54 unconnected with intentional-unused marker. Test point is project monitoring choice; short route and light probe loading recommended. Screenshot only, no native-file/measurement verification.

### 2026-09-23 - Retain XO drive-tuning footprint as recommendation

Recommend retaining XO series resistor at0ohm initially for potential measured-overdrive correction; XI link optional isolation only. Refines prior conversational suggestion to remove both. Basis AN849 DS00849A Fig.1 p.1/Purpose of Resistors p.5, generic principle not LAN-specific mandate. Nonzero population unselected, requires FAE/bench qualification; user decision and native changes not asserted.

### 2026-09-24 - Proposed TPS54J061 external-bias interlock

Retain common switched3.3V VIN/VCC as proposal, explicitly conditional on EN ordering. Supplement prior advice with EN-last startup and EN-first shutdown, plus hardware rail-valid clamp for brownouts/MCU reset. This is proposed architecture, not adopted native circuitry; supervisor/threshold/delays remain to select. TI SLVSFQ0B section6.3.2 p.12 and threshold table pp.5-6 establish sequencing rationale; see POWER_TREE.md. MagJack work remains paused.

### 2026-09-24 - Revised buck draft population recorded

Latest screenshot adopts10.7k+10ohm/11.5k divider (1.1588V nominal),47nF soft start, MCU enable with automatic-enable linkDNP, and exported PGOOD. Supersedes earlier screenshot9.09k/10k and1nF values, without claiming full electrical approval.18kTRIP still needs guaranteed-current margin review. Full findings and citations in1V15_BUCK_REVIEW_NOTES.md.

### 2026-09-24 - PGOOD indication and startup notes shown

User annotated buck screenshot now drives indicator MOSFET gate from BUCK_1V15_PG, superseding earlier direct1V15 gate drive; exact device/current validation remains open. MCU sequencing and1.385A load/calculation notes visibly added; output port renamed+1V15_ETH_SW. Screenshot-backed status, not native netlist or firmware verification. See POWER_TREE.md.

### 2026-09-24 - Load-switch current-limit draft value

User requests nominal2A; latest screenshot populates604ohm0.1% ILIM versus earlier proposed610ohm. Record604ohm as shown draft (~2.024A via TI SLVSFJ2B Eq4 p.16), not exact2A or guaranteed limit range. ON pulldown/FLT pullup still not shown; review in POWER_TREE.md.

### 2026-09-24 — PLL reference choice clarified
EV03E14A 02-01429 Rev3 sheet8 uses a PLL bead and 100nF to RTN_PLL without the EDS2 DS50003971A FigA-2 p.31 10ohm resistor. Prior recommendation to reproduce EDS2 is a provisional reference-based choice, not a datasheet mandate for 10ohm. Final filter choice remains open; dedicated RTN_PLL return requirement remains unchanged. No Altium changes.


### 2026-10-01 — RJ45 individual link/activity LED plan
Windows/design review. Ethan clarified that each populated RJ45 should use both its green and yellow LEDs, superseding the September 17 single combined copper LED plan. Proposed color assignment: green steady for link; yellow blinking for RX/TX activity. Both off with no link; with an idle link green on/yellow off. Colors are a board-level choice, not mandated by the LAN.

Normal BOOT_MODE=000: STM32 must configure both PHY LED outputs and individual link/activity behavior over SPI; unmanaged LED_MODE strap is ignored. Backup BOOT_MODE=100: retain proposed LED_MODE=0 for individual mode with two configured outputs. Existing EGPIO49/50 (PHY0) and EGPIO47/48 (PHY1) routes differ from default GPIO Mode D GPIO42/43 and GPIO40/41 routes, respectively; custom EEPROM mapping/ownership remains unresolved. This decision does not change MAC/SerDes LED plans.

Source: [LAN9645xF datasheet DS00006065D](../eth_SW/docs/LAN9645xF-Data-Sheet-DS00006065.pdf), section 3.4.2.5 and Table 3-14, printed/PDF p.45, establish individual link/activity behavior. Table headings LED1/LED2 conflict with notes LED1=link and LED0=activity: do not freeze physical output-to-color assignment until clarified or tested. Table 3-5 pp.30–31 establishes managed alternate functions; Table 3-9 p.36 establishes default Mode D mapping; Table 3-12 p.43 establishes boot precedence. No native Altium edit or bench validation performed; OneDrive sync not checked.

- [ ] Ethan: resolve PHY_LED0/1 behavior and RJ45 LED pinout/polarity before schematic release; verify both boot paths during bring-up. Deadline: before schematic release / bring-up respectively.

### 2026-10-01 — MAC LED fixed-low strap population shown
User screenshot shows GPIO13 LED_MODE and GPIO14 CUPHY_MODE fixed-low via new1k pulldowns and470ohm LED paths; prior10k pulldowns and DIP1k links DNP. Supersedes selectable positions4/5 in shown draft only. See LAN96455_CONTROL.md for source references and pending reset-level/output-drive qualification; no native-file change claimed.


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


### 2026-10-04 — Spare GPIO connections marked bidirectional
Windows/Altium user report: the two spare GPIO connections (GPIO29 and EGPIO46 in the preceding screenshots) changed to bidirectional. Supersedes input-only wording for their schematic interface role in previous tables. Use GPIO I/O / Spare bidirectional breakout in both target tables; actual input/output direction remains firmware-controlled, with input/high impedance proposed until assigned. BOOT100 ROM configuration still requires verification. No new screenshot supplied or native edit verified.


### 2026-10-04 — EEPROM revision: split supplies and DNP STM SDA
Windows/Altium visual review of updated screenshot. EEPROM VCC remains upstream +3V3; SDA/SCL pullups use +3V3_ETH_SW_LS on a separate wire. No direct upstream/switched rail join is visible in this revision. Prior suspected rail-merge finding does not apply to shown wiring; native netlist still unverified. WP high and optional A1/A2 high pulls are +3V3, matching EEPROM VCC, so earlier recommendation to move these to switched power was conditional on switching EEPROM VCC and is not applicable to this split-supply arrangement. Address high pulls remain DNP and10k pulldowns select base0x50; WP low enables writes.

STM SDA link now DNP0R; SCL uses BPROG jumper. With both open the depicted STM branches are disconnected. For STM programming, populate SDA link and install SCL shunt, power both upstream and switched rails, and hold LAN reset. Existing note mentioning SCL jumper only is incomplete. Upstream EEPROM power alone cannot guarantee programming because bus pullups are on switched rail; external programmer pullups must not drive unpowered directly connected LAN pins. DNP SDA is assembly-selectable, not runtime isolation. No arbitrary power-state approval: ensure no STM/programmer high drive or pullups while LAN unpowered and review leakage/power transitions. DS00006065D section5.2 p.189 forbids driving unpowered LAN inputs; section4.1.2.15 p.52 gives EEPROM0x50/400kHz requirements. No native edit or bench test.

![EEPROM split-supply revision](images/2026-10-04/windows-altium-eeprom-split-supply-b8e1f1e7.png)
*User-pasted Windows/Altium screenshot; upstream EEPROM power, switched I2C pulls, DNP STM SDA link and SCL programming jumper reviewed.*


### 2026-10-05 — Retain X names in reusable SFP child sheet
Windows/Altium user decision: retain SFPX naming in reusable Connectors_SFP.SchDoc; parent maps to SFP0/1. Current parent screenshot fixes lower designator to U_Connectors_SFP1. Child screenshot now uses ETH_SW_SFPX_TXD/RXD and MCU_SFPX_TX_FAULT/LED ports matching shown parent naming; previous prefix mismatch concern resolved for visible entries. Three status mappings remain incorrect on BOTH parent instances: MCU_SFPX_TX_FAULT -> ETH_SW_SFPn_MOD_DEF0; ETH_SW_SFPX_MOD_DEF0 -> ETH_SW_SFPn_LOS; ETH_SW_SFPX_LOS -> MCU_SFPn_TX_FAULT. Correct external names respectively MCU_SFPn_TX_FAULT, ETH_SW_SFPn_MOD_DEF0, ETH_SW_SFPn_LOS.

X is literal text, not automatic parameter substitution. Hierarchical child local nets may be repeated under distinct instances; power-port scope needs compiled verification. In particular +3V3_SFPX_LS must not globally join two intended independent switched outputs. User naming choice accepted; no forced renaming. Load-switch image archived for naming context only; no fresh electrical approval or native changes.
![sfp-child-x](images/2026-10-05/windows-altium-sfp-child-x-9392cfa9.png)

![sfp-parent-instances](images/2026-10-05/windows-altium-sfp-parent-instances-49bca286.png)

![sfp-load-switch-x](images/2026-10-05/windows-altium-sfp-load-switch-x-daf6a5a6.png)
*Original user-pasted Windows/Altium screenshots documenting naming convention and current parent mapping.*


### 2026-10-06 - Retain 6.04 kohm ISET; close F11 by accepted decision
Windows/Codex documentation update. Ethan directs retaining R2210 ERJ-H3ED6041V (6.04 kohm +/-0.5%) because it is the practical available library/sourcing choice and matches the reference schematic as reported by the user. Current distributor availability/reference value not independently rechecked here. F11 is removed from active corrections and recorded as an accepted deviation, not a datasheet error or a measured pass. Historical calculation remains:6.0098-6.0702 kohm versus nominal6.0 kohm +/-1% interval5.940-6.060 kohm; nominal6.04 is inside, upper corner exceeds by10.2 ohm. Source: LAN9645xF DS00006065D section3.2/Table3-3 pp.23-24, [local datasheet](../eth_SW/docs/LAN9645xF-Data-Sheet-DS00006065.pdf). This decision supersedes the earlier replacement recommendation. No Altium/BOM edit or bench measurement performed.
[Updated audit](audits/2026-10-05/FULL_DESIGN_REVIEW.md): eight corrections remain F03,F04,F07,F08,F09,F10,F12,F13; F01/F02/F06 user-reported fixed, F05 retracted, F11 closed by decision. Functional validation gates remain. OneDrive sync not verified.


### 2026-10-06 - F13 generic capacitor package preference: 0805
Windows/Codex. Ethan selects0805 (2012 metric) for the generic capacitor entries covered by F13: BOM rows97-101, values1nF,8.2pF,10nF,10uF and100nF. Treat as requested package target; retain existing electrical specifications pending exact MPN selection. This does not change already specified capacitors elsewhere or the generic resistor in row111. Exact parts, availability, footprint implementation and effective capacitance at working bias remain unverified; particularly the10uF/50V/X7R row previously used1206 and the8.2pF row specifies+/-0.05pF. Do not silently relax these specifications to obtain0805. F13 remains open. No Altium or original BOM edit; no sync verification.


### 2026-10-06 - F13 capacitor sourcing and package clarification
Windows/Codex. Ethan clarifies 10 uF parts should move from1206 to0805; other F13 capacitors may use0402/0603. This supersedes the earlier all-0805 interpretation. Five DigiKey sourcing candidates and order codes recorded in [candidate table](audits/2026-10-06/F13_CAPACITOR_CANDIDATES.md). Three catalog-spec matches; conditional8.2pF +/-0.1pF instead of +/-0.05pF, and10uF25V X7R0805 instead of50V X7R1206. Neither electrical relaxation is accepted or implemented yet. Candidate10uF requires exact-part bias/temperature/ripple and regulator bank qualification, especially main input caps. Stocked catalog candidates found; availability is not reserved. F13 remains open including generic resistor row111. No Altium/BOM edits, procurement, measurements or OneDrive sync verification.

### 2026-10-12 - Screenshot-backed managed SPI routing revision
Windows/Codex: latest user screenshot selects GPIO0-3 via fitted 0R paths and DNPs GPIO4-7, UART and EEPROM alternate links. This supersedes previous GPIO4-7 working allocation for the shown managed assembly. Target BOOT000/MGMT11; strap implementation remains unshown. User-supplied Vishnu correspondence recommends this arrangement and avoiding EEPROM ROM boot with concurrent host management. Preserve old history; do not treat BOOT100 as a verified simultaneous backup configuration. Exact signals, sources and remaining checks are recorded in LAN96455_CONTROL.md and STM32_ETH_SW_INTERFACES.md. No native Altium changes claimed.

### 2026-10-12 - Retain BOOT100 backup documentation
User requests keeping the right-hand note as BOOT100 EEPROM backup, superseding the proposal to convert both notes to BOOT000. Current managed assembly remains BOOT000/MGMT11 with SPI GPIO0-3. Backup requires separate validated population/configuration; concurrent EEPROM ROM boot plus STM SPI management is not approved. See LAN96455_CONTROL.md for screenshot findings and pending correction. No native design modification claimed.

### 2026-10-12 - Populate R2003 for managed IRQ0 capability
User accepts fitting R2003 (0 ohm), connecting ETH_SW_IRQ_SYS_RDYN to GPIO33/pin74, shared with EEPROM_SIZE. Intended BOOT000/MGMT11 and IRQ0 ALT2. Supersedes previous DNP choice; user screenshot shows removed DNP label, not verified variant/BOM state. Physical strap loading, STM input compatibility and IRQ firmware remain to validate. See LAN96455_CONTROL.md and STM32_ETH_SW_INTERFACES.md for references and evidence.

### 2026-10-17 - STM-to-EEPROM I2C not intended connected
Ethan confirms no planned active STM32-to-EEPROM I2C connection for the intended assembly. EEPROM placed beyond the LAN switch away from STM32 accordingly. Supersedes historical assumptions that the shown optional programming interface must be connected. Exact disconnect/population and programming alternative not verified; optional circuitry is not declared removed. See LAN96455_CONTROL.md and STM32_ETH_SW_INTERFACES.md for the user-reported placement and remaining checks.


### 2026-10-23 - Vendor correction to GPIO4-7 restriction; routing decision unchanged
Vishnu R.'s [archived Vishnu reply](correspondence/2026-10-23_Microchip_Vishnu_reply.txt) explicitly retracts the previous claim that GPIO4-7 is restricted to hybrid EEPROM/host management. He cites DS00006273C Rev C section 9.1/Table 9-1 p.23 Note 3: GPIO0-3 ALT2 or GPIO4-7 ALT6 are supported, with only one option enabled per SPI signal. Vendor attribution only; source document not independently checked in this logging session. This supersedes the earlier vendor rationale requiring GPIO0-3, not the October 12 screenshot-backed GPIO0-3 population choice. His GPIO4-7 schematic description appears stale relative to that revision. No user decision to switch back or remove BOOT100 is recorded. Concurrent ROM boot/host management remains unvalidated; his recommendation to remove backup is pending an explicit project decision. Detailed firmware/SDE follow-up is in LAN96455_CONTROL.md and PERSONAL_ENGINEERING_LOG.md. No native changes or tests.


### 2026-10-23 - Return managed SPI to GPIO4-7
Ethan explicitly selects GPIO4-7 ALT6 again. Supersedes October 12 GPIO0-3 routing selection, following corrected vendor guidance. Current screenshot shows R2034-R2037 fitted and R2064-R2067 DNP. BOOT000/MGMT11 unchanged; no decision to delete BOOT100 backup recorded. Local DS00006273C Rev C section9.1/Table9-1/Figure9-1 printed/PDFp23 independently confirms mapping and one-option-per-signal restriction. Screenshot, exact nets and recommended generic GPIO0-3 label cleanup are in LAN96455_CONTROL.md and STM32_ETH_SW_INTERFACES.md. Native implementation/compiled connectivity remains unverified.


### 2026-10-23 - Retain existing SPI/alternate-function net naming
Ethan elects to keep existing net names unless a functional error is found. Supersedes the optional generic GPIO0-3 renaming suggestion; no visible functional label conflict in the new variant crop. R2034-R2037 fitted / R2064-R2067 not fitted preserve GPIO4-7 selection. UART R2052/R2053 and switch EEPROM R2054/R2055 shown fitted. Screenshot and detailed review archived in PERSONAL_ENGINEERING_LOG.md and interface/control notes. No native connectivity verification or schematic edit.

### 2026-10-27 - Use JLCPCB calculator for impedance geometry
Ethan selects migration from Altium/Sierra-derived geometry to JLCPCB calculator geometry. Current screenshot-backed change is outer SE50 width 0.1029 mm and clearance rule update; calculator output gap 0.1374 mm. Inner copper now displayed 0.0152 mm. Existing routed widths have not automatically updated; bulk-edit steps provided. Altium still predicts 54.19 ohm for the outer profile, so solver inputs and actual ground geometry remain to reconcile. No blanket approval of other profiles or measured impedance. See PCB_IMPEDANCE.md and personal log for evidence and outstanding arc-clearance issue.

### 2026-10-27 - SE50 migration rule-state follow-up
Latest screenshots show consistent track/arc-to-polygon clearances: outer 0.137 mm and inner L3/L8 0.202 mm. Width rule linked to SC50 displays min/preferred/max 0.103 mm on Top/L3/L8/Bottom. This supersedes the earlier displayed arc-clearance mismatch; exact stored precision, inner-profile intent and existing copper updates remain unverified. See PCB_IMPEDANCE.md and personal log for original evidence. No new impedance qualification or DRC closure claimed.

### 2026-10-27 - Impedance target tradeoff for routing spacing
Ethan selects JLCPCB calculator differential targets of 96 ohm and 92 ohm in place of nominal 100/90-ohm targets to accommodate spacing; SE remains 50 ohm. Latest output uses 0.1016 mm widths throughout with layer-specific output gaps tabulated in PCB_IMPEDANCE.md. Nominal offsets are -4% and +2.22%; no hardware compliance or fabrication tolerance approval implied. Latest intended geometry supersedes earlier calculator settings, while Altium implementation and verification remain pending. Original screenshots preserved in personal log.

### 2026-10-27 - JLCPCB profile migration milestone
Ethan reports profile migration complete using JLC10161H-1080 calculator geometry: 4mil widths, SE50 and differential 96/92-ohm targets. Latest imperial-equivalent gap table is preserved in PCB_IMPEDANCE.md and personal log. Supersedes profile implementation pending status only; full-board routing, DRC/tuning and impedance validation remain unverified. No new tolerance approval or native edit by Codex.


### 2026-10-27 - ESD204 adopted in displayed copper-Ethernet protection schematic
User-supplied Altium screenshots show U2600/U2601 selected as ESD204DQAR on the PHY side of integrated RJ45 magnetics, covering pairs A/B and C/D with returns to signal GND. Supersedes earlier candidate-only status for the shown circuitry; does not establish compiled PCB/BOM state or electrical qualification. Layout evidence archived in PERSONAL_ENGINEERING_LOG.md. Pair-B polarity labels differ between layout and schematic crops (pad7 N versus P, pad6 P versus N); native reconciliation remains pending. No Codex native edit or sync verification.

### 2026-10-27 - Copper Ethernet inter-pair matching clarification
Windows/Codex reference review: DS00006273C Rev C section12 Layout Considerations printed/PDFp28 emphasizes equal P/N lengths for BASE-T to reduce mode conversion, but does not give a numerical MDI inter-pair constraint there. Microchip AN2054 Gigabit Ethernet Design Guide DS00002054A Rev A (2016), Ethernet Layout Guidelines / Ethernet Differential Pairs, printed/PDFp5 recommends intra-pair skew <50mil (1.27mm) and inter-pair skew <600mil (15.24mm). This is general manufacturer guidance, not a LAN96455-specific guaranteed limit. Source https://ww1.microchip.com/downloads/en/AppNotes/00002054A.pdf . No verified basis for the wizard's tighter 2.54mm group limit; proposed approach is retain existing 0.127mm within-pair project constraint and use a documented loose per-port group check rather than add serpentine solely to satisfy 2.54mm. Do not match separate ports. Current screenshot priorities: schematic rules above MatchedLengths_ETH (priority3), RJ45_0 priority4 and RJ45_1 priority5; overlapping schematic scopes/modes still need verification. No rule changed; recommendation supersedes any implication that tight group matching was required.
Status: proposed constraint clarification, pending actual rule selection/application; not an implemented design change.

### 2026-10-29 - Revised buck layout adopted as current working layout
Ethan reports reworking the three-buck area for improved noise isolation, density and efficiency. Treat the newly supplied overview as the current user-reported layout direction, superseding earlier placement screenshots for progress tracking, without asserting changed electrical topology or measured performance. See POWER_TREE.md and the dated PERSONAL_ENGINEERING_LOG.md entry with archived screenshot. Native connectivity/DRC and hardware validation remain pending; room-copy mechanics not confirmed.

### 2026-11-02 — Proposed simulation failover policy and dual management paths
Implemented PHOTON demo software with A-coordinated, epoch-scoped prepare/quiesce/commit, participant acknowledgements/readback, fail-closed watchdogs, hysteresis/persistence and nonrevertive default. Proposed management-only VLAN4090 on primary and4091 on backup are CPU-terminated independently, while user VLAN100 forwards on one link. This refines active-link-only management for the executable simulation so a fiber cut does not remove the sole handshake path. No hardware/firmware adoption or register validation implied. Thresholds/timing are demo allocations; see DEMO_CONSOLE.md and ../demo_console/README.md. Distributed reset/epoch recovery and real adapters remain open.

### 2026-11-23 - Management path selection clarified for presentation

User requested primary in-band management for gradual optical degradation and backup management for abrupt link loss. Adopted as intended behavior in the [UML-style functional diagram](../output/pdf/RAVEN_Failover_UML.pdf), with backup fallback when primary control stops responding. This refines the November 2 dual-management-path intent; it does not claim that the current aggregate demo transport or embedded firmware implements explicit path selection. Existing demo thresholds/timers remain presentation allocations, not newly approved hardware settings. Per-path timeout/fallback implementation and validation remain open.
