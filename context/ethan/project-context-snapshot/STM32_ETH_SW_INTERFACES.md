# STM32 / Ethernet-switch interface register

Updated: 2026-09-22, Windows/Codex. Working cross-sheet register, not a native netlist or frozen MCU pin allocation.

> **Current SPI selection (2026-10-23):** GPIO4-7 ALT6; R2034-R2037 fitted, R2064-R2067 DNP. Current nets use STM_ETH_SW_SI_* and GPIO-specific R4-R7 suffixes. See the dated October 23 table below; the original September register and October 12 GPIO0-3 entries are historical.

## Maintenance and scope

Track every signal crossing between STM32 and Ethernet-switch circuitry, including support ICs. Update this file when a reviewed schematic adds/renames/removes an interface or resolves firmware ownership; append the change and evidence to PERSONAL_ENGINEERING_LOG.md. Preserve superseded decisions in DESIGN_DECISIONS.md. Read this register before STM32 pin assignment or Ethernet-control changes. Use exact port/net names; different names around series resistors are different net segments, not assumed aliases. Mark uncertain names TBD instead of inventing them.

All STM32 package pins and peripheral/AF selections below are **TBD** until exact MCU/package and MCU-side schematic are checked. Directions are from STM32's perspective. 'Shown' means screenshot evidence, not proven compiled connectivity. Logical signal counts exclude GND/power and do not allocate MCU pins automatically.

## SPI management — four shown signals

LAN GPIO4-7 domain VDD_IO_B; current rail +3V3_ETH_SW_VDD_IO_B. Runtime ALT6 per working allocation. Intended BOOT000 / MGMT11 baseline; backup BOOT100 retains SPI but ROM/host ownership remains to verify.

| STM-side port | Switch-side net after 0R | LAN GPIO / pin | STM direction | STM function | Status |
|---|---|---|---|---|---|
| MCU_ETH_SW_SI_DI | MCU_ETH_SW_SI_DI_R | GPIO4 /112 | Output | SPI MOSI | Shown |
| MCU_ETH_SW_SI_DO | MCU_ETH_SW_SI_DO_R | GPIO5 /111 | Input | SPI MISO | Shown |
| MCU_ETH_SW_SI_NCS | MCU_ETH_SW_SI_NCS_R | GPIO6 /110 | Output | SPI chip select, active low | Shown |
| MCU_ETH_SW_SI_CLK | MCU_ETH_SW_SI_CLK_R | GPIO7 /108 | Output | SPI SCK | Shown |

DI/DO are named from switch perspective. Keep switch-side voltages within powered domain; do not drive switched-off LAN pins. Earlier screenshots used ETH_SW_SI_* names; latest MCU_ETH_SW_SI_* names above supersede those for this register.

## RGMII0 — twelve shown signals

All domain VDD_IO_C, current +3V3_ETH_SW_VDD_IO_C; ALT1. Names RX/TX refer to the switch, so switch RX connects to STM TX, and switch TX to STM RX. Exact STM MAC pin/AF and clock-delay configuration remain TBD.

| Switch net | LAN GPIO | STM direction | STM MAC function | Reset strap on LAN pin |
|---|---|---|---|---|
| ETH_SW_RGMII0_RX_D0 |16|Output|TXD0|—|
| ETH_SW_RGMII0_RX_D1 |17|Output|TXD1|—|
| ETH_SW_RGMII0_RX_D2 |18|Output|TXD2|—|
| ETH_SW_RGMII0_RX_D3 |19|Output|TXD3|—|
| ETH_SW_RGMII0_RX_CLK |20|Output|TX clock|—|
| ETH_SW_RGMII0_RX_CTL |21|Output|TX control|—|
| ETH_SW_RGMII0_TX_CLK |22|Input|RX clock|SGPIO_EN|
| ETH_SW_RGMII0_TX_CTL |23|Input|RX control|SD_125M|
| ETH_SW_RGMII0_TX_D0 |24|Input|RXD0|BOOT_MODE0|
| ETH_SW_RGMII0_TX_D1 |25|Input|RXD1|BOOT_MODE1|
| ETH_SW_RGMII0_TX_D2 |26|Input|RXD2|BOOT_MODE2|
| ETH_SW_RGMII0_TX_D3 |27|Input|RXD3|MGMT_MODE0|

Names transcribed from archived RGMII sheet screenshots. Bus harness names ETH_SW_RGMII0_RX/TX group these signals; they are not extra MCU pins. Keep STM pins connected to switch TX/strap nets high impedance through strap sampling. External strap loading persists at runtime. Confirm 3.3V compatibility, fixed-link setup and delay ownership before routing approval.

## Reset request — one shown MCU signal

| STM port | Path | STM direction/type | Status |
|---|---|---|---|
| STM_ETH_SW_NRESET |0R -> STM_ETH_SW_NRESET_R -> MIC2774 nMR pin3|Output, open-drain low/release|Shown|

10k bias to +3V3_ETH_SW_LS now populated. Manual switch bank B position8 grounds STM_ETH_SW_NRESET through100R (intermediate net STM_ETH_SW_NRESET_SW). Default open; closed holds reset. This is the supervisor **request**, not direct LAN nRESET feedback.

Supervisor output ETH_SW_NRESET ->0R->ETH_SW_NRESET_R ->LAN pin123 is a separate signal. No STM feedback connection is shown; do not allocate an input unless deliberately added. Output pullup uses VDD_IO_A. Verify MCU reset/power-off state against shared switched-domain pullup. nMR/nRESET capacitors DNP. LED resistor remains1k in latest evidence despite3.3k recommendation. Core threshold/timing review open; see LAN96455_CONTROL.md.

## EEPROM programming — two shown MCU signals

| STM port | Connection | STM direction/type | Status |
|---|---|---|---|
| STM_EEPROM_I2C1_SDA |0R to ETH_SW_EEPROM_I2C1_SDA, shared EEPROM/LAN GPIO2 pin119|Bidirectional open-drain I2C SDA|Shown|
| STM_EEPROM_I2C1_SCL |BPROG jumper to ETH_SW_EEPROM_I2C1_SCL, shared EEPROM/LAN GPIO3 pin118|Open-drain I2C SCL, input-capable as needed|Shown|

Names I2C1 label this interface; they do not yet select STM32 hardware peripheral I2C1. Both2.5k bus pullups and EEPROM use switched VDD_IO_A. Program with switch supplies ON and switch held in reset. SCL jumper must be closed for STM programming; SDA remains connected through0R. Avoid back-powering and bus-master contention. Firmware programming/boot ownership sequence still to implement.

## Provisional / unresolved interfaces — do not silently allocate

| Interface | Existing candidate/name | Direction from STM | Open issue |
|---|---|---|---|
| LAN IRQ / ready |GPIO33, ETH_SW_EEPROM_SIZE; candidate IRQ0 ALT2 or ROM SYS_RDYn|Input|No final STM port shown; one runtime function at a time, boot100 behavior pending. Domain D.|
| Switch SFP I2C0 bus |ETH_SW_I2C0_SDA_R / SCL0_R / SCL1_R, GPIO30/31/32|TBD|Orange schematic question only; not approved direct connection to STM DOM bus. Domain D; ownership/mux unresolved.|
| Ethernet load-switch enable |Exact net TBD, TPS22950 enable|Output if STM-controlled|Cross-check power sheet; architecture implies controlled branch but MCU route not confirmed here.|
| Ethernet load-switch fault |Exact net TBD, if exported|Input if implemented|Confirm presence/routing/pullup domain; do not count as connected.|
| MIIM0 |GPIO28/29|Not allocated|Former access header removed; GPIO28 now MGMT_MODE1 strap. No STM MDIO management link selected.|
| SFP TX_DISABLE/LOS/TX_FAULT/module-present/rate select |TBD in SFP interface design|Varies|Track separately when routes are finalized; do not duplicate switch GPIO4-7 reserved for SPI.|

## Current inventory and handoff checklist

19 shown logical MCU signals:12 RGMII +4 SPI +1 reset request +2 EEPROM programming. Does not include provisional IRQ, power controls or SFP sidebands. This is not a guarantee all19 require unique final MCU package pins or that the MCU has been validated.

Next: complete STM pin/AF and voltage-domain columns; compare both ends in native Altium; resolve IRQ/ready and SFP bus ownership; verify power-off states and reset defaults; finalize boot firmware, RGMII clocks/delays, EEPROM programming sequence; compile/ERC and preserve evidence. Do not treat a screenshot or this register as connectivity certification.

## Sources and evidence

Manufacturer requirements/function availability:
- [LAN9645xF Data Sheet DS00006065D](../eth_SW/docs/LAN9645xF-Data-Sheet-DS00006065.pdf): Table3-2 pp.21-22 physical156-pin mapping; Table3-4 pp.28-29 IO domains; Table3-5 pp.30-31 per-pin ALT availability; Table3-12 p.43 boot precedence; Table4-4 p.54 port mix; section5.2 p.189 unpowered input constraints; section5.7.3/Fig5-2/Table5-17 p.200 reset timing. Printed/PDF pages agree. Runtime ALT assignments remain our working proposal, not proof of initialization code.
- [MIC2774 DS20006527A](https://ww1.microchip.com/downloads/en/DeviceDoc/MIC2774-Dual-Micropower-Low-Voltage-Supervisor-DS20006527A.pdf): Table2-1 p.5 and section3.4 p.6 establish nMR request and open-drain nRST output behavior.
- [Working GPIO allocation](LAN96455_GPIO_ALTIUM.txt): retain full LAN GPIO coverage; this register adds STM-facing connectivity/status.

Schematic evidence (user screenshots, not native netlist):
- [SPI/reset/control latest](images/2026-09-22/windows-altium-reset-final-review-d5b1d259.png).
- [RGMII and straps](images/2026-09-20/windows-progress-main-straps-rgmii-5681207f.png).
- [EEPROM programming](images/2026-09-20/windows-progress-eeprom-annotations-07165149.png).

### 2026-09-22 - MIIM clarification
MIIM0 external-PHY controller interface is not an added STM management link: STM uses SPI and RGMII0 MAC-to-MAC. STM has no PHY address0 assigned by this excerpt. Boot100 fixed-link/no-external-PHY behavior needs confirmation. MIIM1 stays internal (CuPHY0 address1, CuPHY1 address2); no external MCU signals. ROM10ms polling in DS00006065D section4.1.2.12 p.51 belongs to unmanaged operation, not a proven boot000 service. Detailed explanation in LAN96455_CONTROL.md; signal count unchanged.

### 2026-09-22 - SerDes monitoring constraint
DS00006065D section4.4.4 pp.56-57 recommends external SFP LOS for optical links; routing/polarity and switch-vs-MCU ownership remain to resolve. Section4.4.5 p.57 eye monitor interrupts normal reception, so do not allocate it as live health telemetry. Two1000BASE-X paths use1.25Gbps electrical line rates; no new STM nets allocated.

### 2026-09-22 - Boot100 hybrid ownership check
ROM+I2C table plus SPI client documented by EDS2 DS50003971A Table3-5 p.20. Custom GPIO mapping possible (DS00006065D Table4-2 p.52), but startup transient mux, ROM link-event writes and ownership of SPI/SFP pins, GPIO33 and host forwarding policy need validation. No new nets allocated; boot000 remains current baseline. See LAN96455_CONTROL.md feasibility review.

### 2026-09-22 - FAE brief sideband proposal
User requests STM-first SFP sideband routing with optional0R links to LAN GPIOs where feasible. Input fanout may be possible after domain checks; shared driven signals need exclusive population/arbitration. No specific additional nets/MCU pins frozen. Boot comparison/GPIO proposal recorded in correspondence/2026-09-22_LAN96455F_FAE_review.md and ../output/pdf/ECE4015_LAN96455F_FAE_Design_Review.pdf.

### 2026-10-01 — STM32N6 RGMII voltage and SFP SerDes swing review
ST AN5967 Rev6 section13.4.2 p.32 and Figure18 p.33 explicitly illustrate 3.3V STM32N6 RGMII: https://www.st.com/resource/en/application_note/an5967-getting-started-with-hardware-development-for-stm32n6-mcus-stmicroelectronics.pdf . ST DS14791 Rev10 section5.3.17 Table65 pp.170–171 gives 3.3V-range output VOL<=0.4V and VOH>=VDDxx-0.4V under specified drive/current conditions; input threshold formulas at3.3V evaluate to1.05V low and1.606V high. https://www.st.com/resource/en/datasheet/stm32n657l0.pdf . LAN DS00006065D Table5-7 p.194 RGMII input low<=0.4*VARIO=1.32V, high>=0.6*VARIO=1.98V; output low<=0.4V/high>=VARIO-0.4=2.9V at5mA. Nominal3.3V DC levels compatible, conditional on actual STM package/pin power domains,3.3V configuration and sequencing. Final pin allocation, loading, drive strength and RGMII timing/delay budget still pending. RGMII is parallel single-ended CMOS, not the SFP differential SerDes interface.

LAN DS00006065D Table5-9 p.195 specifies configurable SFP TX differential swing300–800mVppd. Selected Finisar FTLF8519P3ByL RevC1 (Aug14,2014), sectionIII p.3, local ../sfp/FTLF8519P3xyL_RevC1_8-14-14.pdf specifies TX input250–1200mVpp SINGLE ENDED and receiver output250/450/550mVpp min/typ/max SINGLE ENDED into100ohm differential. For balanced complementary signals, differential pp is twice per-wire pp: SFP input500–2400mVppd and output500/900/1100mVppd. LAN->SFP overlap500–800mVppd before interconnect losses; cannot assume lowest LAN swing setting is sufficient or select an exact register code without its specification. LAN Table5-8 p.194 receiver120–1200mVppd row notes list SGMII/SGMII2G5/QSGMII despite SFP in table title: SFP-mode applicability should be confirmed before complete receive-path sign-off. AC coupling/common mode, exact setting and loss/eye budget remain unqualified. No schematic or firmware change, no bench results. Web revisions differ from older search-index excerpts; cited actual fetched ST Rev10/Rev6.

### 2026-10-01 — Minor reset schematic update archived
Windows/Altium screenshot supplied by Ethan. Visible MCU hierarchical port remains STM_ETH_SW_NRESET, while its wire label is now STM_ETH_SW_RESET_N; 0ohm series link leads to STM_ETH_SW_RESET_N_R at MIC2774 nMR pin3 with a test point. Supervisor nRST output is labeled ETH_SW_RESET_N; 0ohm link leads to ETH_SW_RESET_N_R at LAN nRESET pin123, with a test point. These supersede earlier wire-label spellings STM_ETH_SW_NRESET_R / ETH_SW_NRESET / ETH_SW_NRESET_R in the visible crop; cross-sheet/global rename not verified. The MCU port name itself is unchanged, so prior pseudocode using STM_ETH_SW_NRESET still matches the shown port.

Visible retained circuitry: 10kohm nMR pull-up to +3V3_ETH_SW_LS; supervisor VDD on that switched rail with100nF bypass; core sense uses two series10kohm upper resistors and10kohm lower toGND (20k/10k total, previously recorded nominal0.9V trip). Output10kohm pull-up and LED anode feed from +3V3_ETH_SW_VDD_IO_A; LED series resistor remains1kohm. LED annotation now shows30mA/2.4V, which does not establish actual circuit current or an exact part specification. Both reset-related100nF capacitors remainDNP. Supervisor symbol caption MIC2774N-29YM is abbreviated; exact ordering-code/library association not reverified.

Status: minor schematic changes recorded from image, not a new complete electrical review or native-file edit. Existing supervisor-threshold, reset LED loading and MCU power-off/timing checks remain open. Manual reset switch path is outside this crop and is not reverified.

![Updated LAN reset supervisor circuit](images/2026-10-01/lan-reset-circuit-update-fbbbefc6.png)
*Original user Windows/Altium screenshot. Records revised reset wire labels, test points and retained values; saved unchanged.*


### 2026-10-05 ? Full export audit: boot, RGMII names and debug reset
Windows/Codex review of archived October5 PDF+BOM. [Full audit](audits/2026-10-05/FULL_DESIGN_REVIEW.md) F02/F03/F06/F07: STM GPIO and power parts exported as two devices U1402/U1500; flash on XSPIM_P1 cannot satisfy the documented ROM NOR boot path on XSPIM_P2 (PN6 clock, PN1CS, PN2IO0, PN3IO1). Those PN assignments currently overlap SFP0 controls/status and require a new allocation. Source: ST UM3234 Rev5 ?3.12.3 Table27 p.28, linked in audit.
MCU TXD2/TXD3 pin labels omit the underscore present in resistor-side TX_D2/TX_D3 labels, disconnecting the named paths on p.14. Top-level STM TX->LAN RX and STM RX<-LAN TX crossing is otherwise consistent. DBG_RST_N reaches PB4/NJTRST instead of F2/NRST: verify probe wiring and correct system-reset routing (ST DS14791 Rev11 Table18 p.89, Table20 p.115). USB CC1/CC2 do connect to MCU UCPD1 pins in this export; current detection/PD firmware is not proven. No pin reassignment, native edit or firmware change performed.


### 2026-10-06 - Three audit corrections reported complete
Windows/Codex status update from Ethan: RGMII TXD naming (F06), LAN1.15V/3.3V rail-port connection (F01), and multipart physical reference grouping (F02) fixed in Altium. Record as USER-REPORTED FIXED, not independently verified: no revised screenshot/PDF/BOM/netlist provided in this turn. F05 remains retracted; it was an audit misread, not a hardware defect. Nine correction items remain open: F03,F04,F07,F08,F09,F10,F11,F12,F13; ten functional/verification gates G01-G10 remain open. [Updated audit](audits/2026-10-05/FULL_DESIGN_REVIEW.md) contains exact evidence, references and closeout steps. Next verify revised compiled rail separation, TXD2/3 continuity, and one physical LAN/STM package per board in regenerated BOM/ECO. No native edits, measurements or OneDrive sync verification by Codex. No new design choice adopted.

### 2026-10-12 - Reviewed SPI reroute to GPIO0-3 for managed Boot0
Windows/Codex screenshot review, not native schematic/netlist verification. New shown population: STM_ETH_SW_SI_DI ->0R-> STM_ETH_SW_SI_DI_R0 -> GPIO0 pin121 (MOSI); DO -> DO_R1 -> GPIO1 pin120 (MISO); NCS -> NCS_R2 -> GPIO2 pin119; CLK -> CLK_R3 -> GPIO3 pin118. New resistors/test points remain R?/TP? and require annotation. Previous GPIO4-7 links R2034-R2037 are marked DNP. UART links R2052/R2053 and EEPROM links R2054/R2055 are DNP, isolating the competing GPIO0-3 functions if these population states are implemented. No visible DI/DO or CS/CLK swap. Distinct R0-R3 and R4-R7 suffixes preserve branch separation.
Evidence: LAN9645xF DS00006065D Table3-2 p21 verifies package pins; Table3-5 p30 verifies ALT2 SPI-client DI/DO/nCS/CLK on GPIO0/1/2/3. Table3-4 p28 and section3.3 p29 establish supply changes from VDD_IO_B to VDD_IO_A; verify actual rail/STM bank voltage compatibility and no unpowered drive (section5.2 p189). Sections3.4.1.1-2 pp42-43 establish BOOT000 no internal boot, MGMT11 SPI client. User-supplied correspondence attributed to Vishnu explicitly recommends Boot0 management on GPIO0-3 and avoiding simultaneous EEPROM ROM boot/host management; this supports the reset-mapping choice beyond the mux table alone. Source provenance is supplied correspondence, not independently authenticated manufacturer publication; its EDS2 revision-C claim remains unverified. Source: [archived correspondence](references/2026-10-12/user-supplied-vishnu-review-a23fdd8a.txt); [datasheet](../eth_SW/docs/LAN9645xF-Data-Sheet-DS00006065.pdf).
Remaining checks: strap circuit/default table must actually select BOOT000/MGMT11 (not visible here); ensure Altium variant/BOM Not Fitted matches DNP annotations; annotate new parts/TPs; confirm parent and MCU names use identical STM_ETH_SW_SI_* spelling; verify switched VDD_IO_A and STM power states; consider nCS pullup to VDD_IO_A if absent elsewhere (engineering choice, not a cited mandatory resistor). Place optional branch links near the split to avoid long open SPI stubs. EEPROM/UART branches must remain isolated for this population; existing EEPROM fallback is a separate assembly/reconfiguration option, not concurrent managed startup. Firmware and GPIO0-3 ALT2 mapping remain to implement/verify; no firmware, Altium source, hardware or OneDrive-sync verification performed.

### 2026-10-12 - SPI links and test points annotated in screenshot
Windows/Codex reviewed user Windows/Altium screenshot ae0e79c2. New 0R links now R2064 (DI/MOSI ->GPIO0 pin121, TP2006), R2065 (DO/MISO ->GPIO1 pin120, TP2007), R2066 (nCS ->GPIO2 pin119, TP2008), R2067 (CLK ->GPIO3 pin118, TP2009). All four test points are switch-side of their series links. No visible swaps or duplicate references in this crop; previous R?/TP? annotation issue closed for shown components only. GPIO4-7 R2034-R2037, UART R2052/R2053 and EEPROM R2054/R2055 remain marked DNP. Numbering gaps are not errors; full-project uniqueness, actual Not Fitted variant state and compiled connectivity remain unverified. For BOOT100 backup rework, the four STM SPI links to remove can now be explicitly named R2064-R2067. Sources: screenshot below; previously verified DS00006065D Table3-2 p21 (pins), Table3-5 p30 (ALT2 mapping), Table3-4 p28 (domain). [Datasheet](../eth_SW/docs/LAN9645xF-Data-Sheet-DS00006065.pdf). No native Altium edit, PCB ECO or hardware verification performed; OneDrive sync not verified.

### 2026-10-12 - GPIO variant population screenshot reviewed
Windows/Codex reviewed user-pasted variant view da3110b4. R2064-R2067 new GPIO0-3 SPI links show no not-fitted crosses; old GPIO4-7 links R2034-R2037, UART R2052/R2053 and EEPROM R2054/R2055 show crosses. Thus visible SPI assembly matches intended BOOT000/MGMT11 hardware population. This strengthens prior DNP-text-only evidence to a variant-view observation, but variant name and exported BOM are not visible/verified. SFP MOD_DEF0 GPIO8/9 links R2040/R2041, LAN SFP I2C/LOS/TX_DISABLE option links also crossed; consistent with STM sideband ownership, STM-side population not shown. GPIO33 ETH_SW_IRQ_SYS_RDYN series link is crossed: no STM interrupt through that branch as shown; firmware must use available SPI polling/status checks or the branch must be deliberately populated/configured for IRQ0. This is an observation/implementation consequence, not a requirement to fit IRQ. R2031 reset link and visible LED resistors are uncrossed; reset capacitor C2000 crossed. Strap settings and reset timing are outside scope of crop. No native design edits, exported BOM check, compiled connectivity or bench verification. OneDrive sync not verified.

### 2026-10-12 - GPIO33 R2003 selected for population
Windows/Altium user update, recorded by Windows/Codex. User accepts GPIO33 IRQ connection and supplies screenshot with R2003 0 ohm no longer marked DNP. Path: ETH_SW_IRQ_SYS_RDYN -> R2003 -> ETH_SW_EEPROM_SIZE / GPIO33, U1800I pin74. Current managed intent BOOT000/MGMT11, EE_SIZE0; GPIO33 intended ALT2 IRQ0, not ROM SYS_RDYn. Supersedes previous screenshot-backed DNP population for this link. Screenshot shows schematic population intent only; native save, active variant Not Fitted state, BOM and compiled STM endpoint were not checked.
Remaining checks from prior review: actual EEPROM_SIZE pulldown/switch network must allow GPIO33 output operation; STM voltage compatibility with VDD_IO_D and high-impedance reset state; configure IRQ mux/sources/ack/polarity and STM interrupt handling. No boot-ready inference from LOW. Source: [LAN9645xF DS00006065D](../eth_SW/docs/LAN9645xF-Data-Sheet-DS00006065.pdf), Table3-5 p31 (ALT2 IRQ0), Table3-11 p41 (SYS_RDYn unmanaged only), section3.4.2.8 p46 (EEPROM_SIZE only ROM+I2C table boot). No Codex Altium edits or hardware tests; OneDrive sync not verified.

### 2026-10-14 - RGMII escape and termination placement proposal
Windows/Codex discussion from user Windows/Altium screenshot: mixed inner/outer-row RGMII pads and nearby bypass capacitors constrain breakout. User proposes internal-layer escape, return to top beyond capacitors for resistors, then internal routing toward STM. Recommendation is provisional: separate signals by actual driver; source-series termination near LAN only for LAN-driven signals, near STM for STM-driven signals. Do not add two termination banks per signal merely for symmetry. Confirm any existing second resistor's purpose (0-ohm link/other) before changing schematic. An internal escape and top return can be used if needed, but minimize pre-resistor electrical length and unnecessary transitions; consider bottom-side source resistors if physically clear. Match total delay through both net segments/resistor within each direction's data/control group relative to its own clock and configured clock skew; TX and RX groups need not match each other. L3 between L2/L4 GND is a candidate from recorded stackup, not a finalized route. Preserve continuous reference copper and nearby GND stitching at reference changes. No numeric length/skew limit or routing approval established. Screenshot does not verify filled/capped via-in-pad construction; confirm fabrication if drilling solder lands.
Source-placement principle: Microchip VSC8572 Hardware Design Checklist DS00004444B, Table7-4 Note1 printed/PDF p18, https://ww1.microchip.com/downloads/aemDocuments/documents/UNG/ProductDocuments/DesignChecklist/VSC8572%2BHardware%2BDesign%2BChecklist%2B-%2BDS00004444.pdf . This is another device's manufacturer example, not LAN96455F resistor-value requirements. Exact LAN/STM driver mapping, termination populations and clock-delay budget require native schematic/datasheet confirmation. No Altium edit or routing implementation, DRC, or OneDrive sync verification.

### 2026-10-17 - EEPROM placement and STM I2C connection intent
Windows/Altium work reported by Ethan; recorded by Windows/Codex. EEPROM circuit placed below the Ethernet switch in the layout, on the far side of the switch from the STM32. "Below" describes placement relative to the switch, not a confirmed bottom-layer assignment. User rationale: STM32-to-EEPROM I2C is not planned to be electrically connected in the intended assembly, so proximity to STM32 is not a placement priority. This supersedes any interpretation of the historical shown STM EEPROM programming interface as an actively connected requirement. Exact isolation/population implementation and optional programming method remain unverified; no resistor designators or removal of optional schematic paths inferred. BOOT000/SPI management and separate EEPROM backup intent otherwise unchanged. No screenshot supplied for this update; placement is user-reported, not native-file verified. Next reconcile optional STM EEPROM links with variant/BOM and continue routing. No Codex Altium edits, tests or OneDrive sync verification.

### 2026-10-17 - Reset-supervisor placement between STM32 and Ethernet switch
Windows/Altium work reported by Ethan; recorded by Windows/Codex. STM32-to-Ethernet-switch reset-supervisor circuit placed between the STM32 and LAN switch. Supplied Altium 3D screenshot records the placement context. This is a user-reported physical placement update; no reset topology, component values or sequencing changes reported. Screenshot alone does not establish net connectivity, completed routing, native saved state or clearance/DRC results. Next continue reset/sense routing and connectivity/clearance checks. No Codex Altium edits, bench measurements or OneDrive sync verification.
Evidence: [placement screenshot](images/2026-10-17/windows-altium-reset-supervisor-placement-00369204.png).


### 2026-10-23 - Vendor correction to GPIO4-7 restriction; routing decision unchanged
Vishnu R.'s [archived Vishnu reply](correspondence/2026-10-23_Microchip_Vishnu_reply.txt) explicitly retracts the previous claim that GPIO4-7 is restricted to hybrid EEPROM/host management. He cites DS00006273C Rev C section 9.1/Table 9-1 p.23 Note 3: GPIO0-3 ALT2 or GPIO4-7 ALT6 are supported, with only one option enabled per SPI signal. Vendor attribution only; source document not independently checked in this logging session. This supersedes the earlier vendor rationale requiring GPIO0-3, not the October 12 screenshot-backed GPIO0-3 population choice. His GPIO4-7 schematic description appears stale relative to that revision. No user decision to switch back or remove BOOT100 is recorded. Concurrent ROM boot/host management remains unvalidated; his recommendation to remove backup is pending an explicit project decision. Detailed firmware/SDE follow-up is in LAN96455_CONTROL.md and PERSONAL_ENGINEERING_LOG.md. No native changes or tests.


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


### 2026-10-27 - SFP/RJ45 naming and hierarchy review: three visible errors
**Computer/tool:** Windows/Codex screenshot review, manufacturer pin-table verification. Six screenshots traced SFP TX/RX/P/N and0/1 hierarchy, copper port0/1 A-D pairs, and selected control buses. Found:
1. ETH_SW_SerDes: on positive SFP0 TX line after R2201, net label reads ETH_SW_SFP0_TXD_N although port is ETH_SW_SFP0_TXD_P and source is S0_TXP/pin58. Proposed correction: ETH_SW_SFP0_TXD_P.
2. Same error after R2205 on SFP1 TX positive line from S1_TXP/pin68: ETH_SW_SFP1_TXD_N should be ETH_SW_SFP1_TXD_P. These duplicate N labels could merge P/N nets, not merely cosmetic labeling; actual compiled connectivity and net naming must be checked.
3. Top-level U_ETH_SW_Top sheet symbol shows ETH_SW_SFP0_MOD_DEF0 on both module-detect entries; lower row wired to connectors ETH_SW_SFP1_MOD_DEF0 must reference ETH_SW_SFP1_MOD_DEF0. Synchronize the sheet entry with the child-sheet port rather than just changing visible text elsewhere.

Other visible SFP data hierarchy mappings preserve channel0/1, TX/RX and P/N. Switch-side receiver labels/resistor-side suffixes are consistent. Copper P0/P1 A-D labels agree with symbol functions and visible156-pin numbering; RJ45_0 maps generic PX toP0, RJ45_1 toP1; PHY LED0/1 mappings match. SFP0/1 connector-sheet entries map SFPX to matching index for the shown data, LOS, module detect, TX fault and temperature outputs. RGMII shown STM TX->switch RX and switch TX->STM RX, with D0-D3 preserved; SPI MOSI->switch DI and MISO<-switch DO are directionally consistent. No all-sheet correctness or native netlist closure implied. Earlier RJ45 pad6/7 pair-B PCB/schematic polarity discrepancy remains unresolved; these hierarchy screenshots do not settle it.

Verified references: Microchip LAN9645xF DS00006065D Rev D, Table3-2 156-pin VQFN-DR Assignments, printed/PDFpp21-22, local eth_SW/docs/LAN9645xF-Data-Sheet-DS00006065.pdf. SerDes0 TXP/N58/56,RXP/N62/60; SerDes1 TXP/N68/66,RXP/N72/70. Finisar FTLF8519P3xyL Rev C1 Aug14,2014 sectionI Pin Descriptions p2, local sfp/FTLF8519P3xyL_RevC1_8-14-14.pdf: TD+18/TD-19 are module inputs, RD+13/RD-12 module outputs. Expected hostTX->moduleTD, moduleRD->hostRX with preserved polarity. SFP connector primitive pins are not supplied in this batch, so full end-to-end proof still requires that child sheet/native netlist. Next correct three labels/entry, compile, inspect net membership, update PCB ECO and check pair continuity. No native edits or OneDrive sync verification.


### 2026-10-27 - Three SFP naming errors visibly corrected
**Computer/tool:** Windows/Altium screenshots supplied by Ethan; Windows/Codex review. New SerDes screenshot shows R2201 output net ETH_SW_SFP0_TXD_P and R2205 output net ETH_SW_SFP1_TXD_P, matching their positive ports; corresponding negative paths retain TXD_N. New top hierarchy screenshot shows distinct ETH_SW_SFP0_MOD_DEF0 and ETH_SW_SFP1_MOD_DEF0 sheet entries wired to matching connector entries. All three preceding visible naming findings are corrected in these screenshots. Other shown SFP0/1 TX/RX and P/N names remain consistent. This supersedes the prior visually incorrect state only; compiled net separation, sheet synchronization, PCB ECO, actual SFP connector pins and the earlier RJ45 pair-B pad6/7 discrepancy remain unverified. Next compile/validate, confirm P/N nets are separate and apply/check PCB ECO. No native files edited by Codex, hardware test or OneDrive sync verification.

### 2026-10-28 - RGMII RX xSignal margins unavailable
Windows/Altium screenshot xSignals_RGMII_RX has six numeric paths, zero displayed unrouted and all Margin N/A. CLK60.928, CTL62.662, D0 60.489, D1 60.554, D2 61.055, D3 62.593mm; spread2.173mm. Likely missing/inapplicable xSignal-scoped Matched Lengths rule; actual rule scope not yet shown. Earlier priority screenshot showed a schematic RGMII rule scoped InNetClass, which may not target end-to-end xSignals across resistor-split nets. Proposed diagnostic: inspect enabled Group Matched Lengths scope InxSignalClass('xSignals_RGMII_RX'), separately TX; test query/apply/refresh. Do not adopt a new numerical tolerance or automatically equalize clock before clock-delay ownership/timing budget is established. Altium Design Rule Support for xSignals, Matched Lengths Rule, updated2026-10-08 (unpaginated): https://www.altium.com/documentation/altium-designer/pcb/high-speed-design/xsignals/design-rule-support . No native changes or DRC/sync verification.

### 2026-10-28 - RGMII matching scope confirmed
Windows/Altium screenshot confirms Schematic Matched Net Lengths_1 scopes Net Class RGMII, Group Matched Lengths, tolerance0.635mm. It does not explicitly scope xSignals_RGMII_RX/TX. Proposed separate xSignal-class group rules to check complete resistor-spanning paths; retain0.635mm only as existing project constraint, not verified device requirement. Check original net-class membership for combined directions and split-net overconstraint; replace/disable redundant old constraint only after new scopes verified, and reconcile schematic directive to prevent ECO reintroduction. Clock inclusion remains conditional on RGMII delay ownership/timing plan; no new routing/timing approval or native change.

### 2026-10-28 - RGMII quick arithmetic length check
Windows/Altium user screenshots, STM-perspective names. TX CLK53.006 CTL52.843 D0 53.784 D1 52.701 D2 53.138 D3 52.979mm; longest-shortest1.083mm. Signal-minus-clock offsets CTL-0.163,D0+0.778,D1-0.305,D2+0.132,D3-0.027mm. RX CLK60.928 CTL62.662 D0 60.489 D1 60.554 D2 61.055 D3 62.593mm; spread2.173mm; offsets CTL+1.734,D0-0.439,D1-0.374,D2+0.127,D3+1.665mm. Both exceed existing0.635mm group-spread criterion if applied end-to-end; this is arithmetic comparison, not verified DRC or RGMII timing failure. All displayed unrouted lengths0 and marginsN/A; no broken rows. Separate TX/RX; timing-delay ownership, path definitions and layer-dependent propagation remain to be validated. No rule/routing changes or sync verification.

### 2026-10-28 - RGMII RX displayed lengths updated
Windows/Altium user screenshot: RX CLK62.347 CTL62.662 D0 62.344 D1 62.345 D2 62.345 D3 62.593mm. Spread0.318mm, below previously displayed0.635mm project group criterion. All six paths numeric with zero displayed unrouted; CTL marked Target and other margins numeric, supporting applicable target reporting now. Earlier spread2.173mm superseded for displayed RX lengths. Clock offsets CTL+0.315,D0-0.003,D1-0.002,D2-0.002,D3+0.246mm. No new TX evidence. Does not establish full DRC, propagation-delay/timing closure, or configured RGMII clock delay. No Codex native changes or sync verification.

### 2026-10-28 - RGMII TX displayed lengths updated
Windows/Altium screenshot shows STM TX CLK53.443 CTL53.467 D0 53.784 D1 53.575 D2 53.575 D3 53.711mm. Spread0.341mm, below existing0.635mm project group criterion; supersedes prior TX spread1.083mm. All six paths numeric with zero displayed unrouted; D0 marked Target. Last RX spread0.318mm also below criterion. Screenshot-backed arithmetic length compliance only; fresh DRC, layer-dependent propagation/timing budget and clock-delay configuration remain unverified. No Codex native changes or OneDrive sync verification.

### 2026-10-28 - QSPI displayed length matching reviewed
Windows/Altium screenshot xSignals_QSPI shows CLK16.377 IO0 16.372 IO1 16.377 IO2 16.440 IO3 16.377 CS_N16.357mm. Rounded length spread0.083mm; panel margin indicates0.084mm using additional precision. Below previously shown0.127mm project matched-length tolerance. All six paths numeric with zero displayed unrouted and IO2 Target. No further tuning indicated for that length criterion; full clearance/short/unrouted/matched-length DRC and QSPI operating-frequency/device timing qualification not established. No native edits or sync verification.

### 2026-10-28 - SFP SerDes length-check plan
Windows/Codex reference review: inspect four independent differential pairs SFP0 TX/RX and SFP1 TX/RX, matching P/N within each end-to-end path across existing series links. No group matching between TX/RX or ports proposed. Create8 conductor xSignals in a proposed xSignals_SFP class and scope Within Differential Pair Length; verify existing SFP differential-pair membership across split nets or use separate two-xSignal group per physical pair if necessary. No numeric tolerance adopted; inspect existing SFP rule and actual lengths before tuning. Microchip DS00006273C RevC section12 Layout Considerations printed/PDFp28 requires100ohm differential SerDes routing, low intra-pair skew, limited vias, continuous references,45degree/curved bends; does not give a device-specific numerical length limit there. Physical xSignal endpoints are switch pads and SFP connector pads, not internal module traces. Earlier DS00006065D Table3-2 pp21-22 and Finisar RevC1 sectionI p2 mappings retained. No new screenshot or native change; timing/compliance and sync unverified.

### 2026-10-28 - SFP xSignal target mismatch review
Windows/Altium screenshot xSignals_SFP eight paths: SFP0 RX N42.852/P43.160 delta0.308mm; TX N39.174/P39.119 delta0.055; SFP1 RX N32.290/P33.400 delta1.110; TX N33.281/P33.232 delta0.049. All displayed unrouted0. One global Target43.160mm and margins relative to it indicate group-style reporting; actual active rule mode and end-to-end pair recognition not yet shown. Proposed0.127mm intra-pair project target (not published device limit) would pass both TX pairs and flag both RX pairs if path endpoints are valid. Inspect rule scope/mode, overlapping group rule, series-link endpoints and P/N association before tuning; do not add~10mm to shorter independent pairs based on global margins. No native edits or DRC pass claim.

### 2026-10-28 - SFP rule correctly configured visually
Windows/Altium screenshot shows MatchedLengths_SFP scoped xSignal Class xSignals_SFP, Within Differential Pair Length, tolerance0.127mm, dynamic phase matching off. Correct intended mode; prior class-wide margin display does not establish wrong rule configuration. Next apply and run this specific rule DRC; compare results with prior arithmetic RX0 delta0.308mm,RX1 delta1.110mm,TX0 delta0.055mm,TX1 delta0.049mm. If no RX violations or wrong associations, inspect differential-pair definitions across resistor-split paths and overlapping schematic rule scopes/priorities before tuning. No native edits, DRC confirmation or sync verification.

### 2026-10-28 - SFP within-pair DRC association confirmed
Windows/Altium screenshot selects MatchedLengths_SFP priority1, scope InxSignalClass('xSignals_SFP'), tolerance0.127mm, and shows2 violations explicitly pairing RX_N/RX_P xSignals for SFP1 and SFP0. This supports correct end-to-end P/N association; global panel margins are not pair mismatch. Current SFP0 RX N42.852/P43.160 delta0.308mm; SFP1 RX N33.089/P33.400 delta0.311mm (supersedes previous1.110). Both TX differences remain0.055/0.049mm. For unchanged P lengths, shorter N needs >0.181mm/>0.184mm additional length to get inside0.127mm; proposed roughly0.25mm per N gives~0.058/~0.061mm residual, verify live rule-based targets rather than global43.160 target. No native edits or full DRC/sync verification.

### 2026-10-28 - SFP tuning uses individual-net rule instead of xSignal rule
Windows/Altium selected Accordion Properties shows ETH_SW_SFP0_RXD_N individual net Length33.828mm; From Rules lists only Schematic Matched Net Lengths min33.510/max34.145mm. End-to-end xSignal N42.852/P43.160mm differs0.308mm. MatchedLengths_SFP not listed; priority alone cannot select absent rule. Differential Pair Information blank, association at selected net unresolved. Correction to earlier guidance: never enter43.160mm as individual-net target. Conditional manual net target33.828+0.250=34.078mm would add0.250 to full path leaving0.058mm mismatch if other geometry unchanged. Inspect/temporarily disable obsolete DIFF-100 schematic group constraint for this operation (preserve other uses, restore/reconcile afterward); manual targets still subject to active rule ranges. Recheck full xSignal and specific DRC after one local change. No native edits or verified repair/sync.

### 2026-10-28 - All four SFP pairs display matched lengths
Windows/Altium user screenshot: SFP0 RX N43.187/P43.160 delta0.027mm; TX N39.174/P39.119 delta0.055mm; SFP1 RX N33.369/P33.400 delta0.031mm; TX N33.281/P33.232 delta0.049mm. All four below proposed0.127mm project intra-pair criterion. All eight paths numeric, zero displayed unrouted, no BROKEN. Supersedes prior RX violations for displayed length arithmetic only; fresh MatchedLengths_SFP, clearance, short-circuit and unrouted DRC still needed. SFP0 N is now slightly longer than P but within tolerance; no further tuning indicated by length criterion. Global panel margins remain irrelevant to P/N difference. No Codex native edit, SI qualification or sync verification.

### 2026-10-28 - RGMII return-via guidance
Windows/Codex reference review: single-ended RGMII needs a nearby ground return transition when routing changes reference planes. Recorded stackup L1 referencesL2; L3 betweenL2/L4; L8 betweenL7/L9; L10 referencesL9. Proposed practice: GND stitching via beside signal transitions connecting relevant signal-GND planes, especially L3-to-L8 and top-to-L8; top-to-L3 also benefits from connectingL2/L4 as return redistributes. Existing nearby correctly connected GND vias may suffice; not a mandatory unique via per signal. Keep multiple local return vias for a bus rather than one distant shared return; no connection to CHASSIS_GND. TI High-Speed Interface Layout Guidelines SPRAAR7J RevJ Feb2023 section2.4 and Figure2-11 Stitching Vias printed/PDFp8 recommends stitching changed GND references within200mil center-to-center, closer better: https://www.ti.com/lit/an/spraar7j/spraar7j.pdf . Proposed board placement goal about0.5-1mm where clearances permit, not a manufacturer-specific LAN/STM limit. Verify plane contacts/antipads and repour/DRC after any placement. No native edits or transition inspection completed.

### 2026-10-28 - HSIO return-transition vias added where placement permits
**Computer/tool:** Windows/Altium work reported by Ethan; Windows/Codex documentation.
Ethan reports adding ground-return transition/stitching vias near HSIO signal transitions wherever placement permits. Some locations cannot accommodate additional vias because of layout constraints; their exact locations and existing alternative return paths have not been documented or reviewed. This records user-reported implementation, not native-file inspection or confirmation that every HSIO transition has an adequate return path. No screenshot supplied with this update.
Next: identify the constrained transitions and check nearby existing GND connections to the relevant reference planes; verify plane connections after repour and run clearance/connectivity DRC. Remaining exceptions are open review items, not approved SI exceptions. No Codex Altium edits, electrical validation, or OneDrive sync verification.

### 2026-10-29 - Inner-layer SFP chassis ring purpose review
Windows/Altium user screenshot shows CHASSIS_GND copper interconnecting cage mounting pads on a user-identified inner layer. Exact layer and adjacent routing not established. Explained potential role as additional cage-tab/chassis bonding paths, not a standalone shielding enclosure or requirement to repeat rings on all layers. Main review concern: clearance around chassis copper in a signal-GND reference layer can form slots and interrupt return paths; overlapping chassis/signal copper also adds capacitive coupling. No deletion or topology change recommended without inspecting actual reference layers, routing and enclosure bond. Existing separation of CHASSIS_GND and GND remains unchanged. No native edit, EMC validation or sync verification. General return-path reference: TI High-Speed Interface Layout Guidelines SPRAAR7J, Rev J Feb 2023, section 2.3 (reference-plane guidance), https://www.ti.com/lit/an/spraar7j/spraar7j.pdf ; not a cage-specific inner-ring requirement.

Reference correction for the preceding inner-layer SFP ring entry: TI SPRAAR7J reference-plane guidance is section 2.4, Figures 2-6 through 2-8, printed/PDF pages 6-7 (not section 2.3). Figure 2-10, page 8, illustrates unwanted capacitive coupling between overlapping planes.


### 2026-10-30 - HSIO length-matching recheck
**Computer/tool:** Windows/Altium recheck reported by Ethan; Windows/Codex screenshot transcription.
User reports rechecking HSIO length matching following layout edits. Displayed xSignal lengths (mm) and arithmetic from rounded displayed values:
- SFP0 RX N/P 43.187/43.160, mismatch 0.027; TX N/P 39.035/38.938, mismatch 0.097. SFP1 RX N/P 33.343/33.400, mismatch 0.057; TX N/P 33.240/33.212, mismatch 0.028. All below the previously recorded 0.127 mm project intra-pair target; global class Margin is not the intra-pair mismatch.
- RJ45 port 0 A N/P 38.619/38.664, B 38.115/38.164, C 36.758/36.807, D 37.342/37.322: mismatches 0.045, 0.049, 0.049, 0.020 mm. Port 1 A 31.773/31.821, B 33.313/33.361, C 33.890/33.962, D 32.520/32.588: mismatches 0.048, 0.048, 0.072, 0.068 mm. All below prior 0.127 mm project target.
- RGMII RX CLK/CTL/D0/D1/D2/D3: 62.574/62.891/62.574/62.574/62.574/62.518; rounded-value spread 0.373 mm (panel shows worst Margin 0.374 mm). TX CLK/CTL/D0/D1/D2/D3: 53.736/53.518/53.784/53.835/53.575/53.733; spread 0.317 mm. Both below prior 0.635 mm project group target. Clock-delay ownership and timing closure are separate checks.
- QSPI CLK/IO0/IO1/IO2/IO3/CS: 16.377/16.372/16.377/16.440/16.377/16.357; spread 0.083 mm (panel worst Margin 0.084 mm), below prior 0.127 mm project target.
- USB displays four paths with Margin N/A: N PP3 48.784, N PP4 50.435, P PP3 48.358, P PP4 50.054 mm. If matching PP3 and PP4 identify corresponding endpoints, mismatches are 0.426 and 0.381 mm respectively, exceeding the previously recorded 0.152 mm project tolerance. Confirm paired endpoints and active matched-length rule before declaring USB closed; N/A is not a pass.
All displayed rows show zero unrouted length. These screenshots document length checks, not full DRC, SI compliance or verified native PCB state. Project targets are not newly verified manufacturer limits. Minor panel/arithmetic differences reflect display precision. No native edits or OneDrive sync verification. Evidence: seven original screenshots embedded in PERSONAL_ENGINEERING_LOG.md under this date.
