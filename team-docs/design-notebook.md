# PHOTON Team Design Notebook

**Project:** Ethernet-to-Fiber Media Converter with Proactive Fault Detection and Redundant Failover

**Course:** Georgia Tech ECE 4015 Capstone, Fall 2026

**Faculty Advisor:** Dr. Jane Gu

**Coverage:** September 2–21, 2026

**Last updated:** September 21, 2026

## Team and workstream ownership

*Table 1. Proposal-defined team ownership and primary technical workstreams.*

| Team member | Primary workstream | Proposal-defined responsibilities |
|---|---|---|
| Madhawi Alharbi | Hardware design | Part selection, schematic design, PCB layout, hardware bring-up, and hardware/firmware testing |
| Ethan Su | Ethernet-switch hardware | LAN96455F ownership, sourcing, schematic entry, PCB layout, bring-up, and hardware/firmware co-validation |
| Auska Wang | Firmware architecture | Firmware design/management, Ethernet-switch configuration and bring-up, and external-flash management |
| Yichi Zhang | STM32 and monitoring firmware | STM32 development, SFP monitoring, health/failover logic, firmware/hardware co-design, and system integration |

System integration and bring-up are collaborative team activities.

## Notebook conventions

- Entries are chronological team milestones rather than four isolated personal logs.
- **Completed** means an artifact or recorded review supports the statement. **In progress** and **planned** identify unfinished implementation or validation.
- The proposal and slides define group scope and roles; project_context supplies the technical chronology and current design state.
- Datasheets and native design files remain authoritative. Screenshots document progress but do not prove saved connectivity, ERC success, or released hardware.

## Figure and table register

| ID | Description | Related milestone |
|---|---|---|
| Figure 1 | Two-board system architecture | September 2 and 12 |
| Figure 2 | Single-board functional architecture | September 12 |
| Figure 3 | Preliminary per-board power distribution | September 14 |
| Figures 4–6 | LAN96455F and SFP host-component library evidence | September 10 and 11 |
| Figures 7–8 | RGMII strap and switch EEPROM development | September 16 |
| Figure 9 | Switchable-strap voltage-divider implementation | September 17 |
| Figure 10 | LAN96455F configuration-sheet checkpoint | September 18 |
| Figures 11–12 | Updated GPIO/strap and RGMII configuration work | September 20 |
| Table 1 | Team ownership | Project-wide |
| Table 2 | Current project snapshot | September 21 |
| Table 3 | Reference-document matrix | Project-wide |

---

## Wednesday, September 2, 2026 — Architecture definition

**Objective:** Convert the initial media-converter concept into a demonstrable system architecture.

### Team highlights

| Team member | Highlight |
|---|---|
| Madhawi | Established the hardware workstream around two identical boards, copper Ethernet, redundant optics, and board-level power. |
| Ethan | Developed and iterated the initial block diagrams linking the STM32, LAN96455F, RJ45, SFPs, fiber paths, and USB-C power. |
| Auska | Defined the need for switch configuration and a coordinated control path separate from hardware forwarding. |
| Yichi | Defined the STM32 need for local SFP monitoring, health evaluation, and board-to-board failover coordination. |

### Technical outcomes

- Selected two identical boards, each connected to a laptop through 1000BASE-T.
- Defined independent primary and backup full-duplex 1000BASE-SX paths.
- Kept laptop traffic in the LAN96455F data plane; STM32 devices are management endpoints, not software bridges.
- Defined in-band management as Ethernet frames carried alongside user traffic.
- Assigned Board A as controller/master and Board B as agent through firmware configuration.

**Status/evidence:** Completed architecture milestone. Initial Draw.io artifacts are recorded as sbd/photon.drawio and sbd/photon_copy.drawio.

![Figure 1 — Two-board system architecture](assets/design-notebook/figure-01-two-board-architecture.png)

*Figure 1. Refined system-level architecture showing two identical converter boards, two laptop endpoints, primary and backup full-duplex fiber paths, and in-band management. This later export documents the architecture initiated on September 2.*

**Next actions:** Refine proposal graphics; assign workstreams; freeze interfaces, switch modes, power, and parts.

---

## Wednesday, September 9, 2026 — Scope, schedule, and interface planning

**Objective:** Establish the project target, implementation path, and December 8 expo timeline.

### Team highlights

| Team member | Highlight |
|---|---|
| Madhawi | Scoped hardware work around selection, schematic capture, PCB layout, fabrication, and bring-up. |
| Ethan | Prepared project-scope material and the LAN96455F pin-mapping workbook for the multipart Altium symbol. |
| Auska | Owned firmware architecture and Ethernet-switch configuration planning. |
| Yichi | Owned STM32, SFP telemetry, health/failover, and integration planning. |

### Technical outcomes

- Targeted a 1 Gb/s laptop-to-laptop connection over redundant 1000BASE-SX paths.
- Planned the sequence: freeze interfaces/parts; complete schematic/PCB; bring up power, MCU, switch, copper, and SFP; then implement telemetry and failover.
- Planned September architecture/schematic work, PCB and bring-up through mid-November, and integration through December 8.
- Began formal switch-symbol preparation using a pin-mapping workbook.

**Status/evidence:** Completed planning milestone. See team-docs/intelligent-ethernet-fiber-project-scope-presentation.pptx and eth_SW/LAN96455F-I-8NW_Altium_Pin_Mapping.xlsx.

**Next actions:** Verify the LAN96455F library; select SFP host parts; translate interfaces into schematic sheets and firmware modules.

---

## Thursday, September 10, 2026 — LAN96455F footprint and SFP selection

**Objective:** Establish reliable library foundations for the Ethernet switch and optical interface.

### Team highlights

| Team member | Highlight |
|---|---|
| Madhawi | Focused the hardware workstream on manufacturable exposed-pad geometry and later layout constraints. |
| Ethan | Built/reviewed the 156-pin LAN96455F footprint and selected the Coherent/Finisar FTLF8519P3BNL. |
| Auska | Identified the dependency on SPI switch control and switch bring-up before failover logic. |
| Yichi | Identified the dependency on SFP I²C diagnostics and status/control signals. |

### Technical outcomes

- Corrected switch pad numbering and pin-1 marking.
- Added segmented exposed-pad paste, a 6 × 6 thermal-via array, courtyard, assembly, and body documentation.
- Selected a 3.3 V, 850 nm, 1000BASE-SX, duplex-LC SFP with digital diagnostics.
- Confirmed the SFP module, PCB receptacle, and metal cage are separate sourced items.

**Status/evidence:** Library work completed and visually reviewed; final stackup/fabrication and mapping checks remain.

![Figure 4 — LAN96455F footprint top view](assets/design-notebook/figure-04-lan96455f-footprint.png)

*Figure 4. Altium top view of the LAN96455F footprint. The image documents perimeter pads, the exposed-pad paste pattern, thermal-via array, pin-1 marking, and component outlines; it is review evidence rather than fabrication release approval.*

**Next actions:** Source cage/receptacle; verify footprint rules; define MCU-to-switch and MCU-to-SFP circuits.

---

## Friday, September 11, 2026 — SFP receptacle and cage libraries

**Objective:** Complete the removable optical-module host interface at the library level.

### Team highlights

| Team member | Highlight |
|---|---|
| Madhawi | Emphasized board-edge placement, cage clearance, keepouts, and enclosure constraints. |
| Ethan | Implemented/reviewed Molex 74441-0010 and 74737-0010 as separate overlapping Altium components. |
| Auska | Preserved requirements for independent path control and status reporting. |
| Yichi | Preserved requirements for reading both modules and reporting channel health. |

### Technical outcomes

- Froze 74441-0010 as the receptacle and 74737-0010 as the separate cage.
- Used a common manufacturer datum so both components align at the same PCB coordinates.
- Included contacts, mechanical holes/posts, 3D models, and clearance documentation.
- Confirmed each duplex LC optical path uses separate transmit and receive fibers.

**Status/evidence:** Library components completed and visually reviewed; front-panel and assembled-PCB clearance remain open.

![Figure 5 — Molex 74441-0010 receptacle footprint and 3D model](assets/design-notebook/figure-05-sfp-receptacle.png)

*Figure 5. Molex 74441-0010 SFP receptacle aligned with its PCB footprint and manufacturer 3D model.*

![Figure 6 — Molex 74737-0010 cage mechanical footprint](assets/design-notebook/figure-06-sfp-cage.png)

*Figure 6. Separate cage footprint and mechanical envelope used at the same component datum as the receptacle.*

**Next actions:** Add both SFP channels to the STM32 schematic; finalize SFP power, high-speed coupling, telemetry, and sidebands.

---

## Saturday, September 12, 2026 — Proposal, QFD, and design communication

**Objective:** Consolidate need, requirements, design, demonstration, and schedule into proposal artifacts.

### Team highlights

| Team member | Highlight |
|---|---|
| Madhawi | Contributed proposal Sections 5–11. |
| Ethan | Contributed the executive summary, nomenclature, Sections 1–3, and system block diagrams. |
| Auska | Contributed Section 4 and aligned the approach with firmware/system behavior. |
| Yichi | Prepared the proposal slides and firmware/monitoring direction. |

### Technical outcomes

- Framed the need around AI/data-center bandwidth, copper loss/heat, fiber adoption, and proactive maintenance.
- Defined the novelty as detecting degradation and coordinating pre-failure transfer—not redundancy alone.
- Set proposal targets: 1 GbE, at least 900 Mb/s TCP, at least one complete local telemetry update per second, and a preliminary sub-100 ms interruption stretch goal.
- Defined an attenuation test with continuous traffic and recorded optical power, path state, packet loss, throughput, interruption, and recovery.
- Updated the QFD and two-board, single-board, and preliminary power diagrams.

**Status/evidence:** Completed proposal-development milestone. See the proposal, slides, QFD, and exported diagrams.

![Figure 2 — Single-board functional architecture](assets/design-notebook/figure-02-single-board-architecture.png)

*Figure 2. Single-board architecture used to partition Ethernet switching, STM32 management, optical telemetry, storage, power, and external interfaces.*

**Next actions:** Convert the architecture into component-level design; verify all performance targets experimentally after bring-up.

---

## Monday, September 14, 2026 — Reference-design review and power architecture

**Objective:** Turn the concept into a defensible switch-support and board-power plan.

### Team highlights

| Team member | Highlight |
|---|---|
| Madhawi | Advanced the regulator, load-switch, thermal-footprint, and board-power workstream. |
| Ethan | Reviewed the EDS2 reference, compared modes, developed the switch current budget, and sourced power parts. |
| Auska | Defined the relationship between boot/management mode and firmware switch control. |
| Yichi | Preserved STM32 boot, RGMII, telemetry, and sequencing dependencies. |

### Technical outcomes

- Adopted first-stage 3.3 V and 1.8 V bucks; the 1.15 V switch-core buck is fed from switched 3.3 V.
- Calculated about 1.188 A for the Ethernet protected branch, including the 1.15 V converter input at assumed 80% efficiency.
- Selected MIC22405YML-TR as the working 1.15 V buck and TPS22950CDDCR as the load-switch candidate.
- Built/reviewed Altium library components for both parts.
- Treated EDS2 as an electrical reference—not proof that its boot or Linux/DSA architecture should be copied.

**Status/evidence:** Power topology and working components defined; calculations, sequencing, and schematic approval remain open.

![Figure 3 — Preliminary per-board power distribution](assets/design-notebook/figure-03-power-distribution.png)

*Figure 3. Preliminary power-distribution view. Current calculations and later decisions in POWER_TREE.md supersede any conflicting value or connection shown in this diagram.*

**Next actions:** Complete switch control, reset, clock, high-speed, and power-support sheets; freeze 3.3 V and 1.8 V bucks.

---

## Tuesday, September 15, 2026 — Cross-device workflow and control planning

**Objective:** Create a durable handoff between research, Windows/Altium implementation, and team documentation.

### Team highlights

| Team member | Highlight |
|---|---|
| Madhawi | Gained a shared hardware record for parts, schematic status, decisions, and evidence. |
| Ethan | Organized project_context and began the LAN96455F control/strap plan. |
| Auska | Gained a firmware handoff covering switch initialization and ownership questions. |
| Yichi | Gained a handoff covering RGMII, SFP telemetry, management, and integration dependencies. |

### Technical outcomes

- Established project_context as the synchronized source for design summaries, decisions, and history.
- Standardized datasheet evidence and proposed-versus-frozen decision labels.
- Organized switch support into straps/GPIO, EEPROM, reset, clock, RGMII/SPI, SerDes/SFP, and power passes.

**Status/evidence:** Documentation workflow active; no electrical design is released solely because it appears in notes.

**Next actions:** Resolve RGMII strap coexistence, SPI mapping, boot strategy, and EEPROM recovery.

---

## Wednesday, September 16, 2026 — Boot, EEPROM, RGMII, and management

**Objective:** Resolve high-risk LAN96455F configuration interfaces before completing capture.

### Team highlights

| Team member | Highlight |
|---|---|
| Madhawi | Incorporated strap loading, reset-time behavior, domains, and EEPROM programming constraints. |
| Ethan | Reviewed shared RGMII straps, corrected SPI mapping, built configuration tables, and iterated the EEPROM circuit. |
| Auska | Defined STM32/SPI control as the intended default, with ROM/EEPROM retained as selectable recovery pending validation. |
| Yichi | Defined the need for STM32 RGMII receive pins to remain high impedance during switch strap sampling. |

### Technical outcomes

- Proposed default BOOT_MODE=000 and MGMT_MODE=11, with STM32 management through SPI; custom initialization remains to be proven.
- Preserved selectable BOOT_MODE=100 EEPROM recovery; it is not automatic or fully validated.
- Confirmed RGMII0 on GPIO16–27; GPIO22–27 retain strap pulls during runtime.
- Corrected SPI client mapping to GPIO4–7.
- Adopted 24CSM01-I/SN, superseding the earlier 24AA014H concept.
- Revised the displayed EEPROM circuit to switched VDD_IO_A, 2.5 kΩ I²C pull-ups, and DNP address options.

**Status/evidence:** Configuration sheet substantially developed through reviewed screenshots; native connectivity, ERC, EEPROM contents, and complete boot behavior remain open.

![Figure 7 — RGMII0 shared-strap draft](assets/design-notebook/figure-07-rgmii-straps.png)

*Figure 7. Draft RGMII0 interface showing reset-sampled strap networks that remain electrically connected to runtime transmit signals.*

![Figure 8 — Revised 24CSM01 switch EEPROM circuit](assets/design-notebook/figure-08-switch-eeprom.png)

*Figure 8. Revised switch-configuration EEPROM draft on the switched VDD_IO_A domain with I²C pull-ups and optional address populations. Native connectivity and programming sequence still require validation.*

**Next actions:** Review remaining straps and margins; prove initialization/ownership; inspect native variants and connectivity.

---

## Thursday, September 17, 2026 — Strap margins and copper indicators

**Objective:** Refine configuration straps and the optional copper-port indicator strategy.

### Team highlights

| Team member | Highlight |
|---|---|
| Madhawi | Focused on electrically valid, assembly-readable straps compatible with runtime functions. |
| Ethan | Checked 1 kΩ/10 kΩ strap-divider margins and mapped copper LED candidates and port roles. |
| Auska | Preserved runtime pin control while avoiding SPI and SFP-sideband conflicts. |
| Yichi | Preserved status reporting without consuming pins required for SFP monitoring. |

### Technical outcomes

- Verified usable logic margin for the reviewed switchable strap network.
- Proposed one physical status LED per populated copper port.
- Clarified the planned mix: laptop CuPHY0/DEV0, optional debug CuPHY1/DEV1, optical DEV5/DEV6, and STM32 RGMII0 DEV7.
- Kept LED mapping and optional debug-RJ45 population open.

**Status/evidence:** Electrical review and proposed mapping complete; final population/routing are not frozen.

![Figure 9 — Switchable strap divider](assets/design-notebook/figure-09-strap-divider.png)

*Figure 9. Reviewed 1 kΩ/10 kΩ switchable strap network. The image records the working implementation and voltage-margin review, not a final population release.*

**Next actions:** Compare GPIO modes; select 1000BASE-T jack/magnetics; freeze a per-pin boot/runtime table.

---

## Friday, September 18, 2026 — GPIO modes and control-sheet review

**Objective:** Reconcile pin multiplexing with copper, optical, SPI, and EEPROM interfaces.

### Team highlights

| Team member | Highlight |
|---|---|
| Madhawi | Screened RJ45/magnetics implications and the optional debug-copper interface. |
| Ethan | Reviewed RJ45 candidates, straps, GPIO Modes A/D, EEPROM addressing, and the ETH SW Config Sheet. |
| Auska | Evaluated boot0/boot4 ownership and Mode D conflicts with SPI client pins. |
| Yichi | Evaluated consequences for STM32-controlled telemetry, LOS, TX disable, and failover. |

### Technical outcomes

- Rejected visible RJ45 candidates as unsuitable or unverified for required 1000BASE-T; selection remains open.
- Verified proposed strap directions while retaining final populations as provisional.
- Found Mode D fits the interface count, but its default GPIO4–7 SFP sidebands conflict with SPI.
- Clarified boot0 STM32 initialization versus boot4 ROM/EEPROM behavior and link-event ownership.
- Archived the in-progress configuration sheet with straps, RGMII, SPI, EEPROM/programming, and GPIO groups.

**Status/evidence:** Major control architecture reviewed but not released; native connectivity, designators, variants, and ERC remain.

![Figure 10 — LAN96455F configuration-sheet checkpoint](assets/design-notebook/figure-10-switch-config-sheet.png)

*Figure 10. Full-sheet checkpoint containing boot and management tables, selectable straps, RGMII/SPI labels, EEPROM/programming circuitry, and GPIO symbol groups. Off-sheet alternatives and unresolved assignments remained in progress.*

**Next actions:** Reconstruct GPIO allocation; resolve custom SFP sidebands; select gigabit RJ45/magnetics.

---

## Saturday, September 19, 2026 — Manufacturer support outreach

**Objective:** Reduce integration risk through Microchip applications-engineering review.

### Team highlights

| Team member | Highlight |
|---|---|
| Madhawi | Prepared to supply power, clock, reset, and schematic questions. |
| Ethan | Contacted a Microchip field applications engineer for LAN9645xF support. |
| Auska | Identified questions about non-Linux initialization, internal-controller behavior, and control. |
| Yichi | Identified RGMII, SFP bus/sideband, and coordinated-failover questions. |

### Technical outcomes

- Established a route for manufacturer feedback on the STM32-hosted architecture.
- Prioritized boot, management ownership, RGMII MAC-to-MAC, SFP access, and path-control questions.

**Status/evidence:** Outreach is user-reported; a formal response had not been received.

**Next actions:** Consolidate questions and GPIO mapping, send the review package, and incorporate authoritative feedback.

---

## Sunday, September 20, 2026 — GPIO allocation and review package

**Objective:** Turn pin-mux questions into a reviewable schematic/control package.

### Team highlights

| Team member | Highlight |
|---|---|
| Madhawi | Reviewed strap domains, inter-sheet interfaces, programming circuitry, and unresolved reset/RJ45 items. |
| Ethan | Reconstructed the GPIO table, reviewed configuration screenshots, improved labels/ports, and drafted the FAE email. |
| Auska | Defined questions for SPI initialization, ROM/table ownership, link events, and runtime GPIO configuration. |
| Yichi | Defined RGMII, SFP I²C, LOS, telemetry ownership, and failover-control questions. |

### Technical outcomes

- Produced a GPIO0–50 table with ALT functions, proposed connections, and unresolved entries.
- Proposed SD_MODE=11 for two SFPs and XMII_MODE0=0 for STM32 RGMII0, subject to full initialization validation.
- Distinguished runtime SFP signal detect/LOS from reset-sampled SerDes-mode straps.
- Added horizontal switch tables, corrected labels, and RGMII/SPI/EEPROM/LED inter-sheet ports.
- Drafted a manufacturer review email covering topology, modes, RGMII, SFPs, GPIO mapping, and failover control.

**Status/evidence:** Configuration GPIO circuitry is substantially captured and visually reviewed. The email remained a draft; native netlist/ERC and final ownership remain open.

![Figure 11 — Updated GPIO straps and inter-sheet ports](assets/design-notebook/figure-11-gpio-straps.png)

*Figure 11. Updated GPIO and strap groups with horizontal state tables and inter-sheet ports. Orange notes identify unresolved runtime assignments.*

![Figure 12 — Updated main straps and RGMII ports](assets/design-notebook/figure-12-main-straps-rgmii.png)

*Figure 12. Updated main boot/management straps and RGMII interface ports after label and switch-table cleanup.*

### Next actions

- [ ] Complete the reset-supervisor circuit.
- [ ] Source and design the 25 MHz reference clock.
- [ ] Complete SerDes/HSIO and switch bulk/decoupling/filter networks.
- [ ] Obtain manufacturer feedback before freezing high-risk choices.

---

## Monday, September 21, 2026 — Architecture review and status checkpoint

**Objective:** Review firmware architecture, record hardware completion, and align the next sprint.

### Team highlights

| Team member | Highlight |
|---|---|
| Madhawi | Reviewed hardware status and retained focus on STM32/SFP completion, PCB development, and bring-up. |
| Ethan | Reviewed switch status; configuration GPIOs are ready to proceed to reset, clock, HSIO, and power support. |
| Auska | Co-reviewed firmware architecture and defined the switch configuration/management skeleton work. |
| Yichi | Co-reviewed firmware architecture and defined the STM32, SFP-monitoring, health, and failover skeleton work. |

### Technical outcomes

- Reviewed the firmware architecture system block diagram and accepted it as a good implementation basis.
- Set the next firmware step: create a skeleton framework for all STM32 and Ethernet-switch code files.
- STM32 schematic is mostly complete; both SFP interfaces remain to be completed.
- Ethernet-switch schematic is approximately half complete. Configuration GPIOs are done; major remaining blocks are reset, 25 MHz clock, HSIO/SerDes, and power bulk/decoupling/filtering.
- Identified MIC2774N-29YM5-TR as a reference-equivalent reset candidate; divider, timing, loading, and final schematic remain open.

**Status/evidence:** Architecture review completed. Code skeletons and remaining schematic blocks are next actions, not completed implementation.

### Next actions

#### Firmware — Auska and Yichi

- [ ] Create the common build/source-tree skeleton for STM32 application and LAN96455F management code.
- [ ] Add placeholder modules for board support, SPI, networking/RGMII, SFP I²C/DOM, GPIO/status, telemetry, event logging, health assessment, arbitration, and coordinated failover.
- [ ] Define interfaces, ownership, configuration data, and test seams before hardware-specific implementation.

#### Hardware — Ethan and Madhawi

- [ ] Complete both STM32-to-SFP channels: power/control, I²C, status, and high-speed connections.
- [ ] Complete and review the LAN96455F reset circuit.
- [ ] Source and validate the 25 MHz crystal or oscillator and support network.
- [ ] Complete LAN96455F HSIO/SerDes connections to both SFP receptacles.
- [ ] Complete switch bulk capacitance, decoupling, ferrite/filtering, sequencing, and test points.
- [ ] Select the required 1000BASE-T RJ45/magnetics and resolve optional debug copper.

#### Team integration

- [ ] Freeze the MCU/switch/SFP interface contract so schematic net names and firmware modules match.
- [ ] Review the completed code skeleton and remaining schematic sheets together before detailed implementation or layout.
- [ ] Incorporate Microchip feedback when available.

---

## Friday, September 25, 2026 — Hardware progress and firmware skeleton checkpoint

**Objective:** Debrief hardware schematic progress, review the completed firmware skeleton, and align on blocking issues regarding budget and Microchip support.

### Team highlights

| Team member | Highlight |
| --- | --- |
| Madhawi | Completed the SFP schematic sheet, simulations, and Ethernet switch power design. |
| Ethan | Completed the Ethernet crystal implementation and STM reset configuration. |
| Auska | Finalized the firmware file layout, uploaded it to GitHub, and continued investigating Linux SPI register access. |
| Yichi | Finished the review of the firmware file layout and skeleton code. |

### Technical outcomes

* Finished SFP schematic sheet, simulations, Ethernet switch power, and Ethernet crystal designs.
* Completed STM Reset configuration.
* Finalized and reviewed the firmware file layout; uploaded the skeleton code to GitHub.
* Continued investigating SPI register access from the Linux implementation.
* Identified current project friction points: delayed communication from Microchip support, budget limitations requiring a structured backup plan, and general Ethernet switch hardware configuration complexity.

**Status/evidence:** Hardware schematics are advancing through power and SFP stages; the firmware skeleton is complete and pushed to GitHub, though further switch implementation is pending Microchip support.

### Next actions

#### Firmware — Auska and Yichi

* [ ] Determine pin-wise behavior after the switch nRESET releases, specifically focusing on unmanaged SPI.
* [ ] Define the interface for flashing the EEPROM with the config tool (evaluate utilizing USB-C like the eval board).
* [ ] Confirm the power sequencing requirements of the Ethernet switch.
* [ ] Define the exact procedure for flashing the STM32 external flash.
* [ ] Finalize the platform and complete CLI commands.
* [ ] Develop optical health monitoring and the basic failover logic.
* [ ] Review SFP datasheets and relevant materials for optical health status definitions.
* [ ] Define peer communication and implement the basic protocol.

#### Hardware — Ethan and Madhawi

* [ ] Finish RJ45 implementation.
* [ ] Work on power sequencing and the system shutdown sequence.
* [ ] Review Ethernet switch GPIO configuration.
* [ ] Conduct schematic reviews and begin building the Bill of Materials (BOM).
* [ ] Finish hierarchical design implementation and align on schematic infrastructure.

#### Team integration

* [ ] Follow up on the delayed Microchip support response.
* [ ] Organize a backup plan to address current budget limitations.
* [ ] Collaboratively address and document the Ethernet switch hardware configuration complexity.

---

## Monday, September 28, 2026 — PHOTON Weekly Update

**Objective:** Debrief schematic design progress, verify firmware skeleton integration, track team task completion against prior action items, and establish ownership across hardware and firmware workstreams.

### Current Status

#### Schematic Design
* Finished SFP schematic sheet and verified simulations.
* Completed Ethernet switch power supply topology.
* Finalized Ethernet reference crystal implementation.
* Completed STM32 reset configuration circuit.
* Finished RJ45 connector schematic page.

#### Firmware Design
* Finalized directory structure, architectural file layout, and skeleton code.
* Pushed baseline repository layout to GitHub.
* Actively blocked on register-level driver details pending Microchip support escalation.

#### Friction Points & Risks
* **Microchip Support Lag:** Delayed communication on Ethernet switch register maps and unmanaged SPI pin-state behaviors.
* **Budget Limitations:** Tightly constrained project budget requiring structured sourcing contingencies and an immediate BOM cost rollout.
* **Switch Configuration Complexity:** Managing bootstrap strap pins, strapping resistor tolerances, and GPIO multiplexing on the Ethernet switch IC.

---

### Task Status: Actions on Prior Commitments
* **Debrief hardware schematic progress:** Completed across SFP, power rails, reset, and RJ45 subsections.
* **Review file layout and skeleton code:** Completed; repository merged and verified by Auska and Yichi.
* **Linux SPI register mapping:** Ongoing reverse-engineering of the upstream Linux driver implementation to identify functional switch offsets.

---

### Planned Tasks: Upcoming Week Action Items

#### Auska

* [ ] Define and document the flashing workflow for the STM32 external flash memory.
* [ ] Implement the Ethernet switch initialization sequence and baseline register read/write access functions.
* [ ] Map high-level switch register names to physical hexadecimal addresses using the Linux driver reference.
* [ ] Format and submit targeted technical inquiries to Microchip support regarding unconfirmed register offsets.
* [ ] Implement a software mock/stub layer for switch register access once interface definitions are complete.

#### Madhawi

* [ ] Complete hierarchical design implementation across all schematic sheets.
* [ ] Model power sequencing logic and validate the active system shutdown sequence.
* [ ] Integrate voltage sense circuits across critical switch and MCU power rails.
* [ ] Execute a full schematic peer review.
* [ ] Initiate PCB layer stackup design and generate controlled-impedance routing profiles (diff pairs and single-ended).
* [ ] Finalize the complete Bill of Materials (BOM) and release components for purchasing.

#### Yichi

* [ ] Configure STM32 core peripherals, alternative function registers, and board-level pin assignments.
* [ ] Implement remaining CLI command handlers and validate module boundaries.
* [ ] Define SFP optical diagnostic parameters (RSSI, bias current, temperature) and operational health states.
* [ ] Implement the initial health evaluation engine using mock SFP telemetry buffers.
* [ ] Define exact optical failover trigger criteria, hysteresis limits, and state transition logic.

#### Ethan

* [ ] Audit Ethernet switch GPIO assignments, incorporating strapping modularity and diagnostic test point redundancy.
* [ ] Add Zener transient/clamping diodes on sensitive supply lines for localized rail regulation and protection.
* [ ] Align schematic block structure, global net names, and sheet infrastructure with Madhawi.
* [ ] Complete hierarchical sheet linking and off-page connector rules.
* [ ] Lead internal schematic cross-reviews.
* [ ] Co-develop and audit the project BOM.

---

## Monday, October 5, 2026 — PHOTON Weekly Update

**Objective:** Debrief completion of full schematic peer review, verify transition into PCB layout and stackup definition, track progress on switch register extraction and power sequencing, and align hardware purchasing with firmware bring-up planning.

### Current Status

#### Schematic Design

* Completed schematic design across all functional blocks.
* Finished full internal schematic peer review.

#### PCB Layout

* Defined layer stackup configuration.
* Calculated and finalized controlled-impedance routing profiles (differential pairs and single-ended nets).

#### Firmware Design

* Finalized repository directory structure and architectural file layout.
* Outlined power sequencing and hardware initialization routine for the Ethernet switch IC.
* Actively gathering and cross-referencing required register offsets and addresses for switch initialization and runtime path switching.
* Awaiting official technical clarification from Microchip support.

#### Friction Points & Risks

* **Microchip Support Lag:** Ongoing delays in direct communication regarding unconfirmed register offsets, strapping behaviors, and SPI interface quirks.
* **Budget Limitations & Contingency Planning:** Constrained project capital requiring tight BOM cost tracking and structured sourcing contingencies across vendors.

---

### Task Status: Actions on Prior Commitments

* **Execute full schematic peer review:** Completed; design verified across all functional subsections.
* **Consolidate Bill of Materials (BOM):** Completed initial BOM audit and component pricing rollup.
* **Define PCB stackup & impedance:** Completed; layer count and transmission line profiles finalized.
* **Initiate board layout:** Completed preliminaries; transitioning directly into component placement.

---

### Planned Tasks: Upcoming Week Action Items

#### Auska

* Research and finalize STM32 external NOR flash memory interface architecture and flashing workflow.
* Isolate and verify the exact switch register set required for dynamic optical path switching.

#### Madhawi

* Execute critical component placement on the PCB layout.
* Route critical nets and complete approximately 80% of board layout routing.

#### Yichi

* Configure STM32 core peripherals, alternate function mapping, and pin assignments.
* Finalize remaining CLI command handlers and verify module interface boundaries.
* Define SFP optical diagnostic telemetry parameters (RSSI, bias current, temperature) and operational health thresholds.
* Implement initial optical health evaluation logic using mock SFP telemetry buffers.
* Define precise failover trigger criteria, hysteresis margins, and state transition logic.

#### Ethan

* Finalize and split consolidated BOM into order streams:
* Procure personal/direct-purchase parts.
* Submit requisition for auxiliary components through the ECE Shop.
* Outline hardware bring-up and board validation test plan.
* Initiate Senior Design Expo project documentation and presentation assets.

---

## Monday, October 12, 2026 — PHOTON Weekly Update

**Objective:** Finalize component placement across dense MCU passives, track progression toward complete board routing, consolidate individual firmware modules, and validate Ethernet switch registers and DMA infrastructure against project deadlines.

### Current Status

#### Schematic Design

* Finalized complete schematic design across all subsections.

#### PCB Layout

* Completed local component placement for main functional sheets.
* Actively executing component placement for STM32 GPIO-associated passive components and decoupling networks.

#### Firmware Design

* Finalizing implementation and unit testing across individual firmware modules.
* Pre-integrating peripheral drivers with baseline architectural skeletons.

#### Friction Points & Risks

* **Ethernet Switch Hardware Configuration Complexity:** Managing dense strap pin definitions, impedance tolerances, and peripheral routing overhead under strict space constraints.
* **PCB Layout Schedule Compression:** Tightly compressed timeline to finish routing, verify design rules (DRC), and release files for fabrication.
* **Microchip Support Communication & Documentation Access:** Continued friction obtaining complete register-level documentation and prompt technical feedback on edge-case switch behaviors.

---

### Task Status: Actions on Prior Commitments

* **Execute PCB component placement:** Completed ~70% of total component placement across major functional blocks.
* **Isolate path-switching registers:** Identified and documented the specific Ethernet switch register set required for runtime optical path switching.
* **Define external memory execution workflow:** Generalized and documented the complete toolchain process for flashing and executing code from external NOR flash on the STM32.

---

### Planned Tasks: Upcoming Week Action Items

#### Auska

* Configure and implement DMA-driven Ethernet frame transmission routines.
* Cross-reference and verify switch initialization sequence and register offsets directly against upstream Linux driver source.

#### Madhawi

* Complete remaining PCB component placement and finalize 100% of board routing.
* Execute full design rule check (DRC) and release the finalized BOM for procurement.

#### Yichi

* Finalize precise optical failover threshold values, hysteresis margins, and transition rules.
* Update and refine the core failover decision algorithm.
* Execute automated integration tests of the failover state machine using synthetic SFP telemetry buffers.

#### Ethan

* Audit and synthesize Microchip technical support feedback regarding schematic review points and firmware driver feasibility.
* Consolidate and execute procurement streams across the project BOM:
* Place orders for direct personal/team-purchased components.
* Submit official requisition orders for auxiliary components via the ECE Shop.


* Continue outlining the post-assembly hardware bring-up and verification plan.
* Draft initial Senior Design Expo presentation assets and project documentation.

---

## Monday, October 19, 2026 — PHOTON Weekly Update

**Objective:** Transition board layout from finalized placement to power delivery routing, prepare PCB files for fabrication release by week's end, implement Ethernet transmission and power sequencing firmware, and migrate core platform code into the target IDE environment.

### Current Status

#### PCB Layout

* Finalized component placement across all functional sheets.
* Initiating power delivery network (PDN) routing, rail geometry, and plane pours.

#### Firmware Design

* Finalizing individual firmware module implementations and isolated interface tests.

#### Friction Points & Risks

* **PCB Layout Schedule Compression & Timeline Delays:** Tight delivery window to finish critical routing, complete design reviews, and submit files for board fabrication by week's end.
* **Microchip Support Communication & Documentation Access:** Continued delays in direct technical escalations regarding unconfirmed switch registers and operational edge cases.

---

### Task Status: Actions on Prior Commitments

* **Execute PCB component placement:** Completed ~99% of component placement across the board.
* **Procure Bill of Materials (BOM):** Finalized and submitted all component purchasing orders.
* **Verify Ethernet switch control:** Validated the baseline hardware initialization sequence and path-switching procedures.

---

### Planned Tasks: Upcoming Week Action Items

#### Auska

* Develop and validate firmware routines for raw Ethernet frame transmission.
* Analyze and specify supply rail power sequencing timing constraints and ramp requirements.

#### Madhawi

* Coordinate and execute the final PCB layout design review before the end of the week.
* Complete routing, pass DRC, generate fabrication packages, and submit the PCB order by week's end.

#### Yichi

* Refine and harden the optical failover state machine algorithm.
* Migrate the firmware repository from VS Code into the native vendor IDE environment.
* Complete hardware pin matching and peripheral alternate function mappings.
* Refactor and clean up underlying platform abstraction code.

#### Ethan

* Track and synthesize incoming escalation responses from Microchip support.
* Assist with active PCB layout routing and power delivery planes.
* Draft the hardware bring-up and post-fabrication validation test plan.
* Initiate Senior Design Expo project documentation and exhibit assets.

## Current project snapshot — October 19, 2026

| Workstream | Status | Immediate gate |
| --- | --- | --- |
| Requirements and architecture | Complete | Maintain interface consistency |
| Component/interface selection | Complete | BOM fully ordered; track shipping/lead times |
| Firmware architecture | Complete | Finalize individual module interfaces and target IDE migration |
| STM32 schematic | Complete | Passives placement locked; align pinout with firmware config |
| LAN96455F schematic | Complete | Verified strapped power sequencing, reset, and crystal nets |
| PCB layout | In progress (placement done) | Route power delivery network, execute review, and release board order by end of week |
| Firmware implementation | Active module development | Implement Ethernet frame transmission, verify switch registers, and harden failover logic |
| Bring-up and validation | Planning phase | Draft bring-up checklist and validation test matrix awaiting fab arrival |

## Near-term review checklist

* Complete power delivery network (PDN) routing, polygon pours, and thermal relief checks.
* Run full PCB Design Rule Check (DRC) and Electrical Rule Check (ERC) with zero unresolved violations.
* Conduct final layout peer review prior to panelization/Gerber export.
* Generate fabrication and assembly packages (Gerbers, drill files, IPC-D-356, pick-and-place) and submit board order.
* Verify supply rail ramp rates and timing constraints against Ethernet switch monotonic power sequencing specs.
* Validate STM32 pin configurations and peripheral mappings match finalized layout netlists inside the vendor IDE.
* Test optical failover state machine transition criteria and hysteresis limits using synthetic SFP data.

## Reference-document matrix

*Table 3. Team artifacts, manufacturer references, and shared technical-context documents used by this notebook.*

| Reference | Use in this project | Status |
|---|---|---|
| [ECE Proposal Report](<ECE Proposal Report.docx>) | Scope, requirements, demonstration, schedule, roles, and proposal contributions | Team source |
| [ECE Proposal Report Slides](<ECE Proposal Report Slides.pptx>) | Group architecture, firmware layers, targets, risks, schedule, and responsibilities | Team source |
| [Initial project-scope presentation](intelligent-ethernet-fiber-project-scope-presentation.pptx) | Early target, architecture, implementation plan, and expo timeline | Team source |
| [LAN9645xF data sheet](../../eth_SW/docs/LAN9645xF-Data-Sheet-DS00006065.pdf) | Switch pins, domains, ports, GPIO muxing, straps, boot modes, electrical requirements, and timing | Primary manufacturer reference |
| [LAN96459F EDS2 user guide](../../eth_SW/docs/LAN96459F_EDS2.pdf) | Reference schematic for reset, clock, EEPROM, straps, RGMII, power, and SFP-related wiring | Reference design; not automatically project-mandatory |
| [LAN96455 reference tables](../../eth_SW/docs/LAN96455_Project_Reference_Tables.xlsx) | Project GPIO and configuration working tables | Project working document |
| [FTLF8519P3xyL SFP data sheet](../../sfp/FTLF8519P3xyL_RevC1_8-14-14.pdf) | Optical/electrical ratings, host interface, diagnostics, and module timing | Primary manufacturer reference |
| [Molex 74441-0010 receptacle drawing](../../sfp/sfp_receptacle/744410010_sd.pdf) | Receptacle footprint and mechanical datum | Primary manufacturer reference |
| [Molex 74737-0010 cage drawing](../../sfp/sfp_cage/747370010_sd.pdf) | Cage footprint, posts, clearances, and mechanical envelope | Primary manufacturer reference |
| [MIC22405 data sheet](../../power_ic/mic22405.pdf) | 1.15 V buck design and layout | Primary manufacturer reference |
| [TPS22950 data sheet](../../power_ic/tps22950.pdf) | Ethernet-switch and SFP load-switch design | Primary manufacturer reference |
| [Project overview](../../project_context/PROJECT_OVERVIEW.md) | Current topology and team split | Shared technical context |
| [System architecture](../../project_context/ARCHITECTURE.md) | Interfaces, data/control paths, and board roles | Shared technical context |
| [Component selections](../../project_context/COMPONENT_SELECTIONS.md) | Current part numbers and selection status | Shared technical context |
| [Power tree](../../project_context/POWER_TREE.md) | Rail topology, maximum-current basis, and sequencing notes | Shared technical context |
| [Design decisions](../../project_context/DESIGN_DECISIONS.md) | Frozen/proposed decisions and open items | Shared technical context |
| [LAN96455 control notes](../../project_context/LAN96455_CONTROL.md) | Boot, management, EEPROM, straps, reset, and GPIO analysis | Shared technical context |
| [Personal engineering log](../../project_context/PERSONAL_ENGINEERING_LOG.md) | Detailed chronological evidence supporting this team summary | Shared technical context |
