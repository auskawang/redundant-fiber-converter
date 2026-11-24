# Preliminary Per-Board Power Tree

```text
USB-C PD adapter: 5 V / 3 A or negotiated higher-voltage input, >=15 W
    |
Input protection / eFuse
    |
VIN_SYS
    |-- 3.3 V buck, >=3 A --> +3V3_MAIN
    |       |-- STM32 VDD/VDDIO and small 3.3 V loads
    |       |-- TPS22950 --> +3V3_SFP0, 144 mA max
    |       |-- TPS22950 --> +3V3_SFP1, 144 mA max
    |       `-- TPS22950 --> +3V3_SW_IN, 1.188 A calculated max
    |               |-- LAN96455F 3.3 V domains, 585 mA max
    |               `-- 1.15 V buck, >=2 A --> +1V15_SW, 1.385 A max
    |
    `-- 1.8 V buck, >=1.5 A --> +1V8_STM, approximately 0.77 A characterized max
            |-- STM32 always-on 1.8 V domain
            `-- controlled path as required by selected STM32N6 power scheme
```

## Maximum-current basis

| Net/load | Maximum used | Source location |
|---|---:|---|
| LAN96455F 1.15 V VDD + VDD_PLL | 935 mA | LAN9645xF datasheet, current and power consumption table |
| LAN96455F 1.15 V VDD_AL | 400 mA | Same table |
| LAN96455F VDD_ALS0 + VDD_ALS1 | 2 x 25 mA | Same table |
| `+1V15_SW` total | 1.385 A | Sum above |
| LAN96455F VDD_AH/AHS at 3.3 V | 505 mA | Same table |
| LAN96455F VDD_IO_A/B/C/E at 3.3 V | 75 mA | Same table |
| LAN96455F VDD_IO_D at 3.3 V | 5 mA | Same table |
| LAN96455F direct 3.3 V total | 585 mA | Sum above |
| Each FTLF8519P3BNL | 144 mA | SFP datasheet, Section III, Electrical Characteristics |
| W25Q256JV | 25 mA | NOR datasheet, DC characteristics, program/erase maximum |
| 24AA014H | 3 mA | EEPROM datasheet, DC characteristics, write maximum |
| STM32N6 VDDSMPS case | 757.5 mA | STM32N6 datasheet, Table 39 |

## Downstream 1.15 V converter input

At 80% assumed efficiency:

```text
Iin = (1.15 V x 1.385 A) / (3.3 V x 0.80) = 0.603 A
Ethernet branch load = 0.585 A + 0.603 A = 1.188 A
```

The 3.3 V buck known load is approximately 1.50 A before STM32 I/O, LEDs, pull-ups, supervisor current, and margin. Retain a 3 A regulator rating.

## Important implementation notes

- Do not use a 3.3-to-1.15 V LDO; worst-case dissipation is approximately 2.98 W.
- The Ethernet load switch voltage drop consumes LAN96455F rail tolerance. Verify worst-case buck tolerance, switch RON, current, and temperature. Consider a slightly elevated 3.3 V set point only if every attached 3.3 V device remains within rating.
- Enable the 1.15 V buck only after the switched 3.3 V Ethernet rail is valid.
- Hold LAN96455F nRESET asserted until required rails and reference clocks are stable.
- Apply the STM32N6 sequence from the exact-device datasheet and AN5967 rather than assuming all 1.8 V pins may be tied together unconditionally.



## 2026-09-16 - Always-on EEPROM supply intent and blocking interface issue

User specifies +3V3_ETH_SW_EEPROM as an alias of +3V3_MAIN upstream of the Ethernet switch load switch. Switch VDD_IO_A remains downstream. The 24CSM01-I/SN can retain supply while the switch is off, but its always-on bus pulls must not drive unpowered switch GPIO2/3: DS00006065D section 5.2 p. 189 prohibits this. Both bus lines require verified isolation or manual disconnection for switch-off programming. Reset alone is insufficient. Alias connectivity is not verified in native Altium. Replace the prior 24AA014H power-budget identity with 24CSM01; 3 mA is an EEPROM allocation, with pull-up and bias currents added separately. Power-tree screenshot also shows newer buck/NOR candidates recorded in COMPONENT_SELECTIONS.md; full power circuitry not approved.


## 2026-09-16 - Alternative EEPROM supply under consideration

User is considering +3V3_ETH_SW_VDD_IO_A for EEPROM VCC and programming with the entire switch powered but held in reset. Recommend moving bus pulls and WP/address high options with EEPROM VCC to that same switched rail. This is a proposal, not an implemented change; prior always-on intent is under reconsideration. Account for EEPROM and bus/bias current downstream of the Ethernet load switch if adopted. Programmer/STM32 must not back-drive the domain while off. See LAN96455_CONTROL.md for references and sequence.


## 2026-09-16 - Revised screenshot adopts switched EEPROM supply

Latest EEPROM screenshot now uses +3V3_ETH_SW_VDD_IO_A for VCC, SDA/SCL 2.5 kohm pulls and WP/address high options. This supersedes the earlier upstream +3V3_MAIN EEPROM intent for the displayed design. Programming requires complete switch power and asserted reset. Move the EEPROM branch downstream in the power-tree drawing if still upstream; that drawing update was not shown. Add EEPROM supply current plus bus/bias current to switched-branch budget: at nominal 3.3 V, two 2.5 kohm pulls can draw up to about 2.64 mA total when both lines approach 0 V, plus 0.33 mA through WP's 10 kohm when its jumper is closed. Actual DNP address high options add no DC pull-divider load. Preserve no-back-drive constraint from STM32/external programmer while off. Native rail connectivity and switch power sequencing remain unverified.

## 2026-09-23 - TPS54J061 1.15V draft screenshot review

Windows/Codex: user pauses MagJack work and supplies TPS54J061RPGR regulator sheet. This conflicts with older COMPONENT_SELECTIONS main-table MIC22405 working choice; record TPS54J061 as latest shown draft, not frozen replacement. VIN and VCC shown on +3V3; EN through10k with100k pulldown, MODE grounded, TRIP18k, SS1nF, BOOT100nF/4.7ohm, L1uH, feedback9.09k/10k, optional330pF feedforward DNP. Calculated nominal output0.6*(1+9.09/10)=1.1454V. TI TPS54J061 SLVSFQ0B revisedJune2024: section5.3 p.4 permits2.7V VIN with external VCC bias; VCC3.0-3.6V, so shown external3.3V permits operation. Never apply16V to common VIN/VCC rail despite heading16Vmax. Section6.3.3 Eq1 p.12 divider; Table6-1 p.13 MODE ground selects1.1MHz FCCM. Section6.3.7 Eq4 pp.14-15 and7.2.2.3 p.20:18k gives nominal valley1.667A; with1uH/1.1MHz/3.3V and1.1454V, nominal ripple~0.68App gives average current-limit onset~2.01A, not guaranteed2A across tolerances and not6A. Recalculate margin before choosing resistor. Source https://www.ti.com/lit/ds/symlink/tps54j061.pdf .

PGOOD appears tied to ground/unused in screenshot; recommend exposing it with pullup for power sequencing/status (TI section7.2.2.12 p.24 recommends1k-100k when used). Indicator gate driven by1.15V, unidentified MOSFET: no guaranteed drive established;49.9ohm LED resistor may draw tens of mA, exact LED/driver review pending. Prefer PGOOD-based indication with suitable driver. Output capacitor effective capacitance/stability/transient, inductor Isat/Irms and thermal validation pending. BOOT resistor4.7ohm needs rationale; TI Fig7-5 p.25 uses0ohm for3.3V input (reference example, not mandate).

Altium power connectivity depends on Net Identifier Scope. Power symbols normally global; Strict Hierarchical requires explicit paths. Connecting a power symbol to a sheet Port in Hierarchical scope localizes that power net, requiring matching parent sheet entry/wiring. Source Altium Creating Circuit Connectivity, Power Ports section: https://www.altium.com/documentation/altium-designer/schematic/creating-circuit-connectivity . Prefer named power symbols only for shared rails unless deliberately hierarchical; retain STM_1V15_EN signal port. Actual project scope/netlist not inspected. Also reconcile generic+3V3/+1V15 names against switched Ethernet supply intent before implementation. No native edits, full approval or sync verification.

## 2026-09-24 - External VCC sequencing proposal clarified

Windows/Codex: re-read TI TPS54J061 SLVSFQ0B (June2024), sections6.3.1/6.3.2 printed/PDF p.12, section5.5 pp.5-6: https://www.ti.com/lit/ds/symlink/tps54j061.pdf . External bias must be ready before at least one of VIN or EN enables startup; VIN need not follow VCC if EN remains low. Shutdown requires VIN UVLO falling or EN falling before VCC UVLO falling. Previous common VIN/VCC recommendation was incomplete regarding shutdown: common falling rail crosses VCC UVLO (2.65-2.75V) before VIN UVLO (1.55-2.15V). Use EN to enforce ordering; common supply alone does not guarantee it.

Proposed architecture, not implemented/finalized: VIN3 and VCC13 both on +3V3_ETH_SW_LS with local bypass. EN5 default low through existing100k, MCU request through10k. Add a rail-valid hardware interlock that clamps EN low until switched rail is valid, and promptly clamps low on undervoltage independently of MCU. Logical EN=STM_1V15_EN AND ETH_SW_3V3_OK; implementation may use an open-drain supervisor at EN (MCU series resistor prevents output contention). Select monitor threshold/tolerance above3.0V minimum VCC and below minimum healthy loaded rail, with startup release delay; verify propagation and rail fall time to ensure EN falls before VCC reaches2.75V worst-case falling UVLO. Exact monitor not selected; existing reset supervisor not assumed suitable. Instantaneous collapses require hold-up/other mitigation if timing cannot be guaranteed; RC alone is not a guarantee. Operating external bias target3.3V+/-5% per6.3.2; broader section5.3 VCC3.0-3.6V limits do not replace that target.

Normal sequence: hold LAN reset and buckEN low, enable Ethernet3.3V load switch, qualify switched rail, assert buckEN, wait for1.15V PGOOD and all LAN reset/clock requirements before reset release. Shutdown: assert LAN reset, deassert buckEN while3.3V remains up, allow1.15V to decay to a verified safe level, then disable3.3V load switch. A firmware delay for this decay is TBD based on discharge/load/capacitance and LAN requirements; not inferred from2us EN filter. PGOOD deassertion alone does not mean rail is discharged. No fixed startup/shutdown delay or complete LAN sequencing sign-off. Unexpected loss must force EN low in hardware. Next select rail monitor and inspect load-switch/discharge circuit and LAN ramp constraints.

## 2026-09-24 - Passive EN divider sequencing evaluated

User proposes VIN divider to EN with3.0V startup. Calculations neglect leakage/internal6.5Mohm pull/tolerances initially: k=VEN/VIN=1.22/3=0.4067. Falling threshold1.02/k=2.508V typical, later than VCC UVLO2.70V typical; startup range1.17/k..1.27/k=2.877..3.123V, shutdown0.97/k..1.07/k=2.385..2.631V. Cannot guarantee required EN-before-VCC shutdown. More generally, guaranteeing turnoff before2.75V requires k<0.97/2.75=0.3527, whereas guaranteed startup by3.3V requires k>=1.27/3.3=0.3848; no overlap even before divider errors/leakage. Thus a plain divider cannot guarantee both over published limits on this common3.3V VIN/VCC rail. Source TI SLVSFQ0B section5.5 ENABLE/INTERNAL LDO p.5 and sequencing6.3.2 p.12 https://www.ti.com/lit/ds/symlink/tps54j061.pdf . Recommend retaining proposed supervisor/MCU sequencing, exact hardware pending. No divider selected or native edits.

## 2026-09-24 - Feedback setpoint and DCR clarification

Windows/Codex: screenshot senses after buck inductor, so its DC winding resistance is inside feedback loop; do not raise output merely to compensate that DCR. Downstream ferrites/traces beyond sense point can cause uncompensated I*R drop. TI TPS54J061 SLVSFQ0B section6.3.3 Eq1 printed/PDF p.12 specifies upper resistor to positive load node, lower toAGND: https://www.ti.com/lit/ds/symlink/tps54j061.pdf . With10k lower resistor:9.09k upper gives1.1454V;9.17k gives1.1502V (recommended nominal-centering option);9.31k gives1.1586V (conditional elevated candidate only, pending actual downstream resistance/current, LAN rail limits, reference/resistor errors, ripple and overshoot). Resistors proposed0.1%; no exact MPN selected or native change.

User startup intent clarified in preceding conversation: STM reads upstream3.3V PGOOD, enables Ethernet load switch, waits calculated worst-case ramp/settling delay, then enables1.15V. Accepted as normal-startup intent, not downstream fault detection; upstreamPGOOD cannot prove switchedrailvalid. Delay and unexpected-loss interlock remain unresolved.

## 2026-09-24 - 11kohm bottom feedback alternative

With TPS54J061 nominal0.6V reference and11kohm FB-to-AGND resistor, ideal upper resistor for1.15V is10.0833kohm.10.1kohm gives1.15091V;10.2kohm gives1.15636V. Recommend10.1k/11k, both0.1%, for nominal centering;10.2k is only a conditional raised-setpoint alternative pending downstream drop/tolerance review. Same TI SLVSFQ0B section6.3.3 Eq1 p.12 reference as preceding entry. No native change or MPN selection.

## 2026-09-24 - Revised1V15 schematic reviewed

User screenshot b8a7f987 shows VIN/VCC switched +3V3_ETH_SW_LS; STM0ohm enable/100k pulldown,187k upper bias isolated byDNP link, MODE0ohmAGND/upperDNP, SS47nF,18kTRIP, PGOOD4.7k pullup to+3V3 exported BUCK_1V15_PG, AGND net tie,10.7k+10ohm upper/11.5k lower feedback and populated330pF. Setpoint1.15878V nominal, reference/resistor-only1.14608–1.17151V; no whole-rail acceptance implied. Nominal ripple0.684App, current-limit onset2.008A (not guaranteed2A), SS ramp3.13ms. See1V15_BUCK_REVIEW_NOTES.md for paste-ready calculations/sequencing/layout and exact TI SLVSFQ0B references. Effective output cap/loop stability, load-switch delay, power-loss timing, discharge and LAN rail tolerance remain open. Native source not edited; screenshot is evidence of displayed draft only.

## 2026-09-24 - Actual load basis applied to TRIP review

User confirms use of1.385A expected maximum from power tree. Reverified local LAN9645xF DS00006065D section5.5/Table5-4 printed/PDF p.191:935mA combinedVDD+PLL,400mA VDD_AL,25mA perVDD_ALSx; twoSerDes supplies sum50mA. Total1.385A is power-supply design budget, not measurement. Source ../eth_SW/docs/LAN9645xF-Data-Sheet-DS00006065.pdf . This supersedes treating2A as required operating load;2A was design capacity target. At nominal0.684App ripple, load valley1.043A and peak1.727A.18kTRIP nominal valley1.667A yields2.008A average onset,0.623A/~45% headroom relative1.385A. Retain18k as reasonable provisional choice, not guaranteed worst-case sign-off; published KOCL tolerance rows at4.99k/10k must not be silently assigned to18k. Include inductor/frequency/input/threshold tolerances, load transients and startup charging.80uF local output cap charging over3.13ms adds~29.6mA nominal; downstream capacitance additional. TI SLVSFQ0B section6.3.7 Eq4 p.15 and section5.5 p.6: https://www.ti.com/lit/ds/symlink/tps54j061.pdf . Diagram still labels MIC22405YML/4A; stale regulator block versus TPS54J061 schematic, user drawing not edited.

## 2026-09-24 - Annotated buck screenshot recorded

Latest user screenshot cbedbabb shows MCU sequencing,1.385A load budget and nominal calculation notes placed on Power_1V15.SchDoc. Output port now matches +1V15_ETH_SW power symbol, resolving prior visible naming mismatch (compiled connectivity still unverified). Additional test points around feedback upper-series10ohm and PGOOD shown. LED MOSFET gate now uses BUCK_1V15_PG instead of1V15 rail;4.7k PG pullup and49.9ohm LED resistor remain. Exact MOSFET MPN/LED operating current still unverified; change resolves prior direct1.15V gate-drive arrangement only. Startup note records upstream3V3PG -> load-switch enable -> switchedrailsettle -> STM1V15EN -> buckPG -> resetrelease subject remaining conditions. No shutdown/fault note visible. Preserve EN-low/reset-asserted defaults, calculated settling/discharge delay and fault-response verification as open items. Values otherwise match prior reviewed draft; nominal45% current-limit headroom is not worst-case qualification. Screenshot evidence only, no native edit or sync verification.

## 2026-09-24 - Ethernet load-switch wiring proposed

User begins TPS22950CDDCR sheet; symbol pinout agrees TI SLVSFJ2B (Feb2023) Table6-1 printed/PDF p.4: ON1,VIN2,GND3,ILIM4,VOUT5,FLT6. Proposed wiring: VIN upstream+3V3; VOUT+3V3_ETH_SW_LS; GND boardGND; ON via optional0ohm from STM_ETH_SW_LS_EN with100k pulldown atON (project choices); FLT exported ETH_SW_LS_nFLT with10k pullup to MCU-compatible upstream3V3 (project choice), not power-good. ILIM610ohm provisional preserves earlier selection; TI section7.5 p.6 gives1.54/2.00/2.46A limits at610ohm. At updated1.1588V,1.385A buck load and assumed80% efficiency/3.3V input, total585mA direct plus608mA buck~1.193A before EEPROM, bias and rail sag. Not1.385A passed directly through load switch. Input/output decoupling and actual load-switch rise/settling/discharge/current-limit behavior next; downstream cap inventory required. Source https://www.ti.com/lit/ds/symlink/tps22950.pdf . Bare-symbol screenshot only; wiring not implemented/verified.

## 2026-09-24 - Local load-switch bypass proposal

TI TPS22950x SLVSFJ2B sections10.3/10.4.1 printed/PDF p.17 says1uF input is sufficient in most situations, extra bulk may be needed for transients, and input/output capacitors should be close to device. Source https://www.ti.com/lit/ds/symlink/tps22950.pdf . Project starting choice:1uF X7R10V fromVIN2 toGND3 and1uF X7R10V fromVOUT5 toGND3, short connections; optional input bulk footprint pending upstream impedance/transient review. Output1uF is project choice, not datasheet-mandated minimum. Preserve buck/PHY local decoupling; all downstream3V3 capacitance counts toward load-switch inrush/settling (buck1V15 output bank is not directly connected3V3 capacitance). No native implementation or timing validation.

## 2026-09-24 - Ethernet load-switch draft review

Screenshot69d84707 shows VIN+3V3, VOUT+3V3_ETH_SW_LS, GND, local1uF25V10% caps both sides, MCU ON through0ohm withDNP100nF, FLT through0ohm toETH_SW_LS_FLT, ILIM604ohm0.1%. Pin connections match TI SLVSFJ2B Table6-1 p.4. No ON external pulldown or FLT pullup visible; propose100k ON-toGND at switch and10k FLT-toMCU-compatible upstream3V3 unless existing pullup elsewhere. ON must not float; internal smart pulldown is not a substitute for deliberate reset-state control. FLT is open-drain active-low fault, not PGOOD; for C part, thermal/reverse faults per pin table, not guaranteed indication of every current-limiting condition. Preserve DNP EN cap with direct0ohm MCU drive.

604ohm differs from earlier610ohm target. TI section10.2.2 Eq4 p.16 (R in kohms) gives~2.024A nominal; reasonable approximately2A selection, not exact cutoff. Published1.54–2.46A limits in section7.5 p.6 are for610ohm, not guaranteed604ohm limits;0.1% resistor does not eliminate IC tolerance. Local cap choice consistent with section10.3/10.4.1 p.17 input recommendation and placement guidance; dielectric not visible, use X7R/X5R. Typical550us turn-on at3.3V in section7.6 p.7 uses1uF/100ohm test load and is not worst-case settling time for whole branch. Source https://www.ti.com/lit/ds/symlink/tps22950.pdf (revB Feb2023). Branch current estimate~1.19A remains assumption-based, exclude aux/inrush. Native changes not performed or verified.

Calculation correction: Eq4 evaluates to2.0258646A for604ohm; round to2.026A (~2.03A), superseding2.024A approximation above.

## 2026-09-24 - Load-switch pull resistors now shown

Latest user screenshot1de925e9 resolves previous missing-visible-pulls findings:100kohm ON-toGND at device side of series0ohm and10kohm FLT-to+3V3 before output series0ohm. VIN2+3V3,VOUT5+3V3_ETH_SW_LS,GND3,ILIM4-to604ohmGND unchanged; local1uF caps on input/output. Added VIN-to-enable0ohm override is DNP; keep DNP for MCU sequencing and to prevent contention when MCU drives LOW. ON100nF remainsDNP. FLT labelETH_SW_LS_FLT retained, active-low semantics must be documented in MCU interface. Visual wiring review passes these corrections; exact MCU domain compatibility, capacitor dielectric, compiled netlist, transient/settling and power-loss response remain unverified. Prior TI SLVSFJ2B Table6-1 p.4 and sections10.3/10.4.1 p.17 references apply. No native edit or sync verification.

## 2026-09-24 - AGND net-tie purpose clarified

Net tie is a deliberate copper connection/layout constraint, not isolation or a noise filter. Benefit comes from avoiding shared impedance between power switching returns and FB/SS/TRIP/MODE ground reference. Retain local AGND_1V15 as routing aid only; keep quiet components nearAGND7, PGND1/14 and power capacitor returns low impedance to main ground, with short local quiet join. Do not split board-wide ground plane or put buck/load return current through0402 tie. TI TPS54J061 SLVSFQ0B section7.4.1 printed/PDF p.30 explicitly avoids AGND joins in high-current paths; section7.4.2 pp.31-32 shows PGND planes. Source https://www.ti.com/lit/ds/symlink/tps54j061.pdf . A well-routed commonGND implementation can meet same objective; net tie itself not mandated. Existing footprint copper/placement uninspected; no quantified noise benefit claimed.

## 2026-09-24 - Two power indicators reviewed

User screenshot shows low-side MOSFET LED circuits: first gate BUCK_1V15_PG (prior4.7k pullup), second gate+3V3_ETH_SW_LS, both LED supplies upstream+3V3 with49.9ohm. Topology plausible for N-channel devices but exact MOSFET MPN/physical pin mapping and guaranteed drive at~3V not shown. Rail-driven LED indicates presence, not precise rail-valid threshold. PG LED can mislead with buck unpowered; prior TI off-state PG clamp specifications do not guarantee logic-low for all gates. Do not use LED as sequencing proof. Kento manufacturer datasheet mirrored at https://www.rcscomponents.kiev.ua/datasheets/KT-0805-G.pdf revA3 2017-05-16 p.3 lists Vf2.5–3.0V at5mA,20mA absolute max; actual project supplied LED revision/bin remains to match. Illustrative nominal3.3V/49.9ohm current6–16mA neglecting MOSFET drop and Vf current dependence, not guaranteed bounds. Propose330ohm as lower-current starting choice (~0.9–2.4mA by same simple estimate), verify brightness and actual LED curves/temperature. No native changes or final device qualification.

## 2026-09-24 - LAN power distribution/reference review

Created LAN_POWER_DISTRIBUTION.md after reading DS00006065D pp.21–22,27–28,189–191 and visually inspecting EDS2 DS50003971A FigA-2 p.31. Reference analog filters/decoupling useful; adapt2V5 sources to project3V3, do not copy module regulator. RTN_PLL154 goes only to PLL decoupling return, notGND; reference1V15->bead->10ohm->155,100nF155-to154. Check ePAD EP designator matches footprint (reference157). Newly explicit constraints: monotonic0.1–100ms ramps, rails within100ms, preferred3V3first, core-firstlead<=200us, off-to-on>=150ms. I/O-voltage choices remain linked to exact MCU domains; DS5.2 IO '+5%/+5%' printing anomaly recorded. No native edits/complete sign-off.

### 2026-09-24 — EV03E14A decoupling comparison
Reviewed 02-01429 Rev3 sheet8: 128-pin LAN96459 has five VDD and five VDD_AL pins, each group with five 100nF plus 4.7uF, versus our fourteen/twelve supply pins. Counts are package-dependent. EV03 PLL omits EDS2's 10ohm resistor; this resistor is a reference choice, not a universal requirement. See LAN_POWER_DISTRIBUTION.md for exact references and remaining qualification. No implementation change.

### 2026-09-24 — EV03 reference current rating versus protection
02-01429 Rev3 sheet8 labels U10/MOD0044 MIC22405 output "1V15 @ 1.5A" and module symbol "Iout=3A max". Neither annotation establishes a 1.5A current-limit threshold. MIC22405 DS20007042A (2025), section1 Electrical Characteristics p.3, Current Limit row:4A min/7.8A typ/14A max at stated test conditions (VIN3.3V,VOUT1.8V,25C defaults,VFB0.5V). Section5.11.1 p.18 explains factory-set high-side switch-current protection, not adjustable average load current. IC rated4A (p.1) is distinct from module3A label and board1.5A annotation. Module internal schematic and reference operating conditions must be checked before assigning an exact board trip current; no evidence of externally programmed1.5A protection here. Source: https://ww1.microchip.com/downloads/aemDocuments/documents/APID/ProductDocuments/DataSheets/MIC22405-4A-Integrated-Switch-High-Efficiency-Buck-Regulator-DS20007042.pdf . No change to our TPS54J061 setting.

## 2026-09-26 — SFP filter explanation and simulation scope
Windows/Codex: user supplied LTspice AC model and SFP sheet19of20. Two separate4.7uH branches feedVCCT/ VCCR, each with100nF directground and22uF series0.51ohm damping leg. Input4.7uF+100nF bypass. Damping resistor is not inloadDCpath; lowersLCpeaking by dissipatingACenergy. Model usesRSRC50milliohm,LSRC50nH,inductorDCR0.288ohm,CRETain0.92,TX84mA/RX60mA,bulkESR89milliohm/ESL0.25nH andHFESR1.3milliohm/ESL0.3nH. Values are model assumptions/screenshot readouts, not independently verified part specifications.

Calculated simpleLCnatural frequency with22uFnom=15.65kHz; with20.24uFeffective=16.32kHz, distinct from capacitorSRF or buck1.1MHz. Idealinductor reactance at1.1MHz32.5ohm. DCbranchdropsTX24.2mV/RX17.3mV plus sharedsource7.2mV at144mA. SeriesbulkR+ESR~0.599ohm limits thatleg's highfrequencyshunting; direct100nF bypass remainsimportant. Fullresponse includesESL,sourceimpedance andcoupledbranches.

AC1 meansnormalizedsmall-signalinput;V(tx) equals source-to-outputtransfer sinceV(source)=1. Toisolatefilterafterinputnode plotV(tx)/V(input). DCcurrentloadsources withoutACterms areopenincrementally anddonot emulateSFPdynamicinputimpedance/loadsteps. NativeSPICEfile/resultsnotprovided; schematicparasitictext mustbeconfirmedactuallyentered. Schematicshows twoinput4.7uF+100nFpairs oncommon+3V3 whilemodelshowsonepair: reconcileaggregateinputcapacitance andplacementinductance beforequantitativeapproval. Noexactattenuationreadfromthumbnail; noSIMrun/noAltiumedits.

Generalbasis: ADI AN-1368,Ferrite Bead Demystified,sectionsLC Resonance Effect(Figs14-19) andDamping Methods, https://www.analog.com/en/resources/app-notes/an-1368.html (HTML,no revision/page shown). Resonance/damping principles apply; thisSFPcircuitusesexplicit4.7uHinductors,notLAN33ohmferritebeads. This is explanation ofshownproposal,notmanufacturerqualification.

## 2026-10-01 — PGOOD startup evidence and unpowered-state distinction

Reviewed TI TPS54J061 data sheet SLVSFQ0B (revised June 2024), Figure 7-15, “EN Start-Up,” in the supplied screenshot. Source: https://www.ti.com/lit/ds/symlink/tps54j061.pdf . Printed page number is outside the supplied crop. Conditions explicitly show VIN = 12 V and RLOAD = 0.3 ohm. The waveform shows PGOOD low before enable/startup and rising after output startup, with a small transient visible near the enable transition. This is typical startup evidence with input power present, not proof of PGOOD behavior with VIN/VCC absent.

Ethan reported checking that the 1.15 V buck PGOOD is off when the controller is unpowered. Record this as the reported conclusion; the attached figure alone does not substantiate the unpowered condition. No bench measurement was supplied. Earlier unpowered-PGOOD/indicator qualification remains open, particularly with the PGOOD pull-up powered from upstream +3V3. Need the applicable off-state sink/clamp specification or a controlled measurement with the actual pull-up/load and VIN/VCC states before closing that item. No schematic or sequencing change.

### 2026-10-01 — Unpowered PGOOD clamp verified in specifications
TI TPS54J061 SLVSFQ0B (June 2024), section 6.3.9 Power Good, printed/PDF p.15, explicitly describes self-clamping when input fails to power the device. Section 5.5 Electrical Characteristics, POWER GOOD continuation, printed/PDF p.7, specifies VIN=VCC=VEN=0 V and pull-up to 3.3 V: 100 kohm gives 0.75 V typical / 1.10 V maximum; 10 kohm gives 0.95 V typical / 1.25 V maximum. Source: https://www.ti.com/lit/ds/symlink/tps54j061.pdf . This closes uncertainty about existence of an unpowered clamp, but not compatibility with our actual 4.7 kohm pull-up, MCU input-low threshold or LED MOSFET off-state. Do not extrapolate the listed limits to 4.7 kohm. Internal transistor-level implementation is not disclosed; depletion-mode technology cannot be inferred. Conceptually, current supplied through the external pull-up can feed an internal clamp without VIN/VCC power. A normal enhancement-mode open-drain transistor alone would not guarantee this. Earlier Figure 7-15 observation remains powered-startup evidence. No circuit changes or bench tests.

### 2026-10-01 — Explicit reset hold added to proposed startup sequence
Proposed firmware sequence: initialize ETH_SW_LS_EN and STM_1V15_EN low; assert STM_ETH_SW_NRESET low using open-drain before enabling switch power; wait upstream3V3_PG; enable switched3V3; verify settling; enable1V15 buck; wait qualified buckPG and required rail/clock stability; keep LAN nRESET low for at least2ms after supplies AND clock stable; release STM reset request (open-drain high impedance); allow MIC2774 timeout before actual LAN reset deassertion and subsequent initialization. Assertion is electrical LOW/logical0 at the GPIO, although a software reset_asserted Boolean may be true. Do not push-pull HIGH against manual reset switch. Existing supervisor provides hardware undervoltage/reset timing; firmware cannot replace startup protection before STM executes. Core0.9V supervisor threshold does not itself prove1.15V nominal rail is valid. PG unpowered clamp qualification and shutdown/fault handling remain open.

References: LAN DS00006065D section5.7.3 Figure5-2/Table5-17 printed/PDF p.200 (../eth_SW/docs/LAN9645xF-Data-Sheet-DS00006065.pdf); MIC2774 DS20006527A section3.4 p.6 and Electrical Characteristics p.3 establish /MR low assertion and140–280ms timeout after valid release: https://ww1.microchip.com/downloads/en/DeviceDoc/MIC2774-Dual-Micropower-Low-Voltage-Supervisor-DS20006527A.pdf . Screenshot shows previous abbreviated note; updated sequence is proposed, not implemented firmware or native schematic edit.

![Existing MCU sequencing note](images/2026-10-01/buck-sequencing-note-before-reset-step-04bd1dc9.png)
*Original user schematic-note screenshot; lacks explicit initial reset assertion. Proposed revision documented above.*

### 2026-10-01 — Concise startup pseudocode and crystal-startup uncertainty
Reviewed LAN DS00006065D section4.22 p.163, section5.7.3 Figure5-2/Table5-17 p.200, section5.8.1/Table5-28 p.209 and supply descriptions. No guaranteed crystal startup duration or explicit XI/XO oscillator-amplifier supply-domain assignment found in these sections. Do not assign oscillator power to VDD_PLL or VDD_IO_A merely by proximity/name. Reference clock must be stable before actual nRESET release (2ms minimum thereafter); this supports startup while reset held as intended operation, but exact internal enable/rail dependence remains manufacturer-confirmation item. Section4.22 approximately14ms AFTER reset release is boot/operational timing, not crystal startup; earlier reset-recovery table conflict remains unresolved. Hardware checklist DS00006273B section5.2.1 Figure5-1 p.10 confirms passive crystal between XI/XO; section5.3 p.11 identifies CK25OUT asPLL-derived, so it is not automatically a valid pre-reset crystal-ready indication. https://ww1.microchip.com/downloads/aemDocuments/documents/UNG/ProductDocuments/DesignChecklist/LAN96459F-Hardware-Design-Checklist-Managed-Mode-DS00006273.pdf .

Crystal and on-chip sustaining amplifier form one oscillator; amplitude builds and frequency settles after bias becomes available. Startup depends on crystal ESR/load, amplifier margin, supply ramp and temperature; no project-specific time invented. Proposed firmware uses a validated clock-startup bound after all LAN rails are valid, then2ms guard and releases open-drain MR; supervisor adds140–280ms before actual reset release (MIC2774 DS20006527A pp.3,6). Those supervisor milliseconds are a potential timing budget, not proof of crystal startup. No clock-ready input or fixed delay implemented. Ask Microchip which rail powers XI/XO, whether oscillator runs throughout nRESET assertion, worst-case start bound/recommended delay, and CK25OUT validity during reset. Bench qualification remains pending.

### 2026-10-01 — Sequence note aligned to schematic net names
Paste-ready proposed note uses STM_ETH_SW_LS_EN, STM_1V15_EN, STM_ETH_SW_NRESET and BUCK_1V15_PG. Upstream3V3_PG retains the existing note spelling; exact upstream schematic label remains unverified. STM_ETH_SW_NRESET is the open-drain MIC2774 nMR request; ETH_SW_NRESET_R is actual LAN reset, not a currently shown STM feedback input. Timing symbols are firmware constants, not schematic nets. No native schematic edit.

```text
STM_ETH_SW_LS_EN = LOW
STM_1V15_EN      = LOW
STM_ETH_SW_NRESET = LOW       // open-drain reset hold

wait_until(3V3_PG == HIGH)
STM_ETH_SW_LS_EN = HIGH
wait(T_3V3_ETH_SW_LS_SETTLE)

STM_1V15_EN = HIGH
wait_until(BUCK_1V15_PG == HIGH) // qualify with powered rails
wait(T_CLK_START_MAX + 2 ms)

STM_ETH_SW_NRESET = Hi-Z      // release supervisor request
wait(T_SUPERVISOR_RELEASE_MAX) // 280 ms max; valid rails, no other reset request
LAN_init_and_check_ready()
```
T_3V3_ETH_SW_LS_SETTLE and T_CLK_START_MAX remain TBD/validated bounds. No feedback read of ETH_SW_NRESET_R implied. Timeouts/faults retain reset assertion. References for2ms and supervisor timeout remain preceding entries.

### 2026-10-01 — Supply-rise timing annotation
Added proposed sequencing note: all LAN supply ramps must be monotonic, with rise time0.1–100ms; all LAN supplies must power up within100ms of each other. Include switched3.3V settling and1.15V startup in that timing budget; do not interpret100ms as an allowable extra delay before starting the second rail. Observe supply waveforms at LAN pins, not just enable commands. Supervisor reset hold after supplies stabilize does not consume an additional supply-ramp interval and does not relax the power-sequencing requirements. Reference: LAN9645xF DS00006065D section5.3, Figure5-1 and Table5-1, printed/PDF p.190, ../eth_SW/docs/LAN9645xF-Data-Sheet-DS00006065.pdf . No native schematic change.

### 2026-10-01 — Updated Ethernet power sheet archived
Windows/Altium screenshot supplied by Ethan: full sheet shows TPS22950 load switch, TPS54J061 1.15V buck, supply-sequencing datasheet excerpt and revised MCU pseudocode note. Note visibly includes initial low enables/open-drain reset hold, switched3.3V settling, buckPG wait, monotonic/all-rails-within100ms comment, clock-start bound plus2ms reset hold, high-impedance request release and140–280ms supervisor delay (280ms wait). Settling/clock-start constants remain explicitly TBD. This confirms the annotation is present in the shown schematic, not firmware implementation or native-file verification.

Sheet additionally shows an output series current-sense shunt/amplifier path labeled ISNS_1V15, a voltage-sense divider labeled VSNS_1V15, a two-pin header, a DNP load-switch bypass link and power indicators. These are archived as shown only; exact component values, gains, protection, ADC compatibility, shunt voltage drop and global connectivity were not reviewed in this logging task. Existing timing/PG qualification items remain open. No Altium source modified and no bench test or OneDrive sync verification claimed.

![Updated Ethernet power sheet with sequencing and sensing](images/2026-10-01/eth-power-sheet-sequencing-sensing-ec5e6324.png)
*Original user Windows/Altium screenshot, archived unchanged. Revised sequencing note is visible; sensing circuits and full sheet are not newly qualified.*


### 2026-10-02 — Buck UVP/OVP and supplemental clamp review
Windows/datasheet review. UserTPS54J061 screenshot retains10.7k+10ohm upper/11.5k lowerFBdivider: nominalVout0.6*(1+10710/11500)=1.158783V. TI SLVSFQ0B section5.5 p6 thresholds: UVP77/80/83%, OVP113/116/119%, OOB102.5/105/107.5%. At nominalsetpoint UVP0.8923/0.9270/0.9618V;OVP1.3094/1.3442/1.3790V;OOB1.1878/1.2167/1.2457V. These scalednumbers exclude reference/divider error and dynamicovershoot. Section6.3.10 p15: UVP enabledaftersoftstart,64us qualifyingdelay thenFETsofflatched. OVP usesnegativecurrentlimiteddischarge cycling until UVPqualification thenofflatched; ENtoggle/inputresetclearsfault. Not simply permanentlow-sideon. Section6.3.11 p16: OOBcorrects earlyovershoot usingFCCM/negativecurrentlimit withoutlatchingOVfault.

LAN DS00006065D sections5.1/5.2 p189: normalcore1.0925–1.2075V, absolutemax1.265V. MainOVPthreshold evenminimum atnominalsetpoint exceedsLANabsolutemax; doesnotguaranteeloadprotection. OOB uppercorner1.2457V beforeothererrors leaveslittlemargin anddoesnotprotectagainstallFB/FETfaults. RecommendrefrainfromaddinggenericZener: no chosenpartguarantees negligibleleakage atnormalrail andclampbelow1.265V atfaultcurrent; power/energyneeded, regulatorprotectionsdependonFB. If independentfaultprotectionrequired,designprecisionmonitor plusdisconnect/crowbar withthreshold,tolerance,response,overshoot,energy andLANsequencingverified. Microchip p189Note5-1suggestsclampifpowersupplyspikespossible,notarbitraryZenermandate. Proposedadditionalprotectionnotimplemented; no componentselected.

References: [TI TPS54J061 SLVSFQ0B June2024](https://www.ti.com/lit/ds/symlink/tps54j061.pdf),section5.5 p6 andsections6.3.10–11 pp15–16; [LAN9645xF DS00006065D](../eth_SW/docs/LAN9645xF-Data-Sheet-DS00006065.pdf),sections5.1/5.2 p189. Printed/PDFpagesagree. No nativeedit orbenchvalidation.

![Buck output protection review screenshot](images/2026-10-02/1v15-buck-uvp-ovp-review-a9a4f4ef.png)
*Original userWindows/Altiumscreenshot; existingcircuit, notnewclampimplementation.*


### 2026-10-02 — 3.3V buck output protection note
Windows/datasheet review. TPS54J061 screenshotFBdivider43.2k+1.8k over10k gives3.300Vnominal. UVP77/80/83% ->2.541/2.640/2.739V; OVP113/116/119% ->3.729/3.828/3.927V; OOB102.5/105/107.5% ->3.3825/3.4650/3.5475V. Scaledrangesexclude reference/dividererror and transientovershoot. LAN3V3domainsnormal3.135–3.465V, absolutemax3.63V: primaryOVPevenminimum exceedsmaximum, so not independentloadprotection. OOB acts earlier but notguaranteedclamp againstFET/FBfailure. No generic3.3Vzener recommended asguaranteedprotection; precisionindependentOVmonitor/disconnect orqualifiedcrowbar iffaultprotectionrequired. Other3V3loads needtheir own limitschecked; no boardwidequalification.

Correction/qualification to prior1V15note: TI section6.3.7 p14 explicitly specifies approximately14msautomaticretry forovercurrent-inducedUVP, while section6.3.10 p15 describesUVP/OVPlatchedshutdown andEN/inputreset. Do not describe everyUVevent aspermanentlatchoff. Paste note distinguishes OCP/UVhiccup andOVfaultrecovery; exactotherUVcasebehavior mayneedTIclarification. Applies tobothbucks.

References: [TPS54J061 SLVSFQ0B June2024](https://www.ti.com/lit/ds/symlink/tps54j061.pdf),section5.5 p6 thresholdranges,section6.3.7 p14 OCP/hiccup,section6.3.10 p15 UVP/OVP,section6.3.11 p16 OOB. [LAN9645xF DS00006065D](../eth_SW/docs/LAN9645xF-Data-Sheet-DS00006065.pdf),sections5.1–5.2 p189 supplylimits. No nativeedit, componentselection orbenchvalidation; syncnotverified.

![3.3V buck protection review](images/2026-10-02/3v3-buck-uvp-ovp-review-427b0921.png)
*Original userWindows/Altiumscreenshot; existing3V3buck, proposedprotectionannotationonly.*


### 2026-10-02 — 1.8V buck protection annotation
Windows/calculation. User TPS54J061 screenshot shows20k/10k FB divider, nominal1.800V. UVP77/80/83% scales to1.386/1.440/1.494V; OVP113/116/119% to2.034/2.088/2.142V; OOB102.5/105/107.5% to1.845/1.890/1.935V. Values exclude reference/divider tolerance anddynamicovershoot. UVP aftersoftstart64us qualification; overcurrent-inducedUVP approx14msretry; OVPnegativecurrentlimiteddischarge thenlatchedoff,EN/inputresetrecovery. Same TI section6.3.7 versus6.3.10 distinction aspreviousnotes; no claim allUVcases manuallylatched.

FinalSTM32N6supplypins/scheme andall1V8loadlimits not verified inthis task: donot invent1V8absolute maximum orcopy LAN1V15/3V3limits. BuckOVP not established asloadprotection. Generic1.8Vzener not recommended withoutleakage,clampvoltage atfaultcurrent,tolerance/temperature andenergy checks; independentprecisionOVprotection remains optionalengineeringdesign, notimplemented. This task covers annotation notfullschematicreview.

Reference: [TI TPS54J061 SLVSFQ0B June2024](https://www.ti.com/lit/ds/symlink/tps54j061.pdf),section5.5 p6,section6.3.7 p14,sections6.3.10–11 pp15–16. No nativeedit,benchtest orsyncverification.

![1.8V buck output protection review](images/2026-10-02/1v8-buck-uvp-ovp-review-d9c69c45.png)
*Original user Windows/Altium screenshot; protection note prepared fromshownnominaldivider.*


### 2026-10-05 ? Full export audit: power release blockers
Windows/Codex PDF+BOM review; no Altium edit. [Full audit](audits/2026-10-05/FULL_DESIGN_REVIEW.md) F01/F04/F05/F08/F10/F12 records: p.18 LAN +3V3_ETH_SW_LS port joins the +1V15_ETH_SW core bus; p.6 R600?R603 all DNP/absent BOM; p.11 external VCC bias R1111 DNP with3.3 V VIN; p.7 12 V-fed PG indicator can exceed PG limits; three current-shunt footprint fields blank; power-tree diagram/input budget stale. These supersede any interpretation of earlier notes as full power approval. Do not power/fabricate this revision before correction.
References: LAN DS00006065D ??5.1?5.2 p.189; TPS54J061 SLVSFQ0B ?5.3 p.4 and ?6.3.2 p.12; TPS25947 SLVSFC9C ??6.1/6.3 pp.7?8. Exact source links, evidence and calculations are beside each audit finding. Current shunt MPN encodes3.300m?, not displayed3.33m? (Vishay30402 rev19-Aug-2026 p.1). MCU supply sequencing and switched-SFP net isolation remain compiled/bench gates. Enlarged MCU export confirms VDDIO4=3.3 V and PLL tied to AON is an allowed ST main-group option; do not introduce false corrections there. No hardware measurements or sync verification.


### 2026-10-06 - F05 retracted: VCC bias present; R1111 selects MODE
Windows/Codex correction prompted by user's enlarged Altium screenshot. Rechecked original archived PDF p.11: U1100 pin13 VCC is directly wired to +3V3_ETH_SW_LS, bypassed by C1107. R1111 is the MODE-to-VCC option, not the external-bias feed. Original F05 and the follow-up recommendation to populate R1111 were incorrect and are withdrawn. R1111 DNP plus R1115=0 ohm selects1100kHz forced CCM; reversing those populations selects1100kHz skip mode. NEVER fit both, which would short the3.3V rail to AGND. This is a review correction, not a hardware change; remaining sequencing checks are unchanged.
Reference: TI TPS54J061 SLVSFQ0B June2024, section6.3.5/Table6-1 p.13 (MODE choices), section6.3.2 p.12 (external VCC sequencing), section5.3 p.4 (operating ranges). [Verified local datasheet](audits/2026-10-05/references/tps54j061.pdf). [Corrected audit](audits/2026-10-05/FULL_DESIGN_REVIEW.md) retains F05 as retracted and now has12 active corrections, not13. Earlier October5 summaries mentioning missing bias are superseded by this entry; other findings are not closed by this correction. No Altium source edit or bench test; OneDrive sync not verified.


### 2026-10-06 - Three audit corrections reported complete
Windows/Codex status update from Ethan: RGMII TXD naming (F06), LAN1.15V/3.3V rail-port connection (F01), and multipart physical reference grouping (F02) fixed in Altium. Record as USER-REPORTED FIXED, not independently verified: no revised screenshot/PDF/BOM/netlist provided in this turn. F05 remains retracted; it was an audit misread, not a hardware defect. Nine correction items remain open: F03,F04,F07,F08,F09,F10,F11,F12,F13; ten functional/verification gates G01-G10 remain open. [Updated audit](audits/2026-10-05/FULL_DESIGN_REVIEW.md) contains exact evidence, references and closeout steps. Next verify revised compiled rail separation, TXD2/3 continuity, and one physical LAN/STM package per board in regenerated BOM/ECO. No native edits, measurements or OneDrive sync verification by Codex. No new design choice adopted.

### 2026-10-13 - LAN bulk, ferrite, PLL and voltage-sense placement progress
Windows/Altium work supplied by Ethan; recorded by Windows/Codex. User reports placement progress for the larger Ethernet-switch bulk capacitors, ferrite beads (FB), PLL capacitor and voltage-sensing resistor dividers, following placement of the 0201 100 nF bypass capacitors. Close-up shows these support components grouped around the LAN footprint in the U_ETH_SW_Power room. The overview shows this room still outside the board outline, with numerous unrouted connection lines; record as intermediate placement/grouping, not final on-board placement or completed routing. No new capacitance, resistor value, MPN or electrical design change is inferred from these screenshots.
Evidence: original user-pasted screenshots archived unchanged below. Native Altium files, saved state, individual net assignments, footprint clearances and DRC were not checked. Next: integrate the power-component group into the board placement and verify rail/filter connectivity, PLL supply routing, sensing connections and power/ground paths during routing. No hardware measurements or OneDrive sync verification.

### 2026-10-14 - Bulk-capacitor placement revised for inner-row vias
Windows/Altium update from Ethan: revised LAN bulk-capacitor placement to provide via room for the inner capacitor row, following the October 13 placement pass. SerDes room parts also received a placement pass. No electrical value or topology change reported. Screenshot and full progress entry are archived in [PERSONAL_ENGINEERING_LOG.md](PERSONAL_ENGINEERING_LOG.md), October 14. Via clearances, routing and native saved state have not been independently verified; airwires remain.

### 2026-10-14 - Placement recovery follow-up
Ethan reports accidental displacement of the LAN-area placement, now mostly corrected. Latest screenshot is archived with the placement-recovery entry in PERSONAL_ENGINEERING_LOG.md; it supersedes the preceding screenshot as the current visual placement state. Inner-row via-space intent remains, but exact restoration and clearances are not verified. No electrical design change reported.

### 2026-10-25 - Ethernet-switch top-layer power-pour progress
Ethan reports top-layer Ethernet-switch power pours added/refined. Screenshot shows local copper regions around the LAN package perimeter and central EP GND via array. Evidence is archived in the dated [personal engineering log](PERSONAL_ENGINEERING_LOG.md) and [original screenshot](images/2026-10-25/windows-altium-eth-sw-top-power-pours-0cc881f8.png). No rail topology/value change inferred; polygon net assignments, connectivity, neck widths and DRC remain native-layout verification items. This records routing progress, not power-integrity or fabrication approval. No Codex Altium edits or OneDrive sync verification.

### 2026-10-25 - 1.15 V pour overview and detail
Ethan supplied board-level and close-up screenshots documenting further 1.15 V Ethernet-switch pour work, following the top-layer pour entry. Original images and captions are preserved in the dated [personal engineering log](PERSONAL_ENGINEERING_LOG.md): images/2026-10-25/windows-altium-1v15-pours-overview-252abf65.png and windows-altium-1v15-pours-detail-a827c956.png. Exact layer number and surrounding polygon net assignments are not exposed. Verify rail/filtered-domain separation, via connectivity, copper necks and DRC in the native PCB. No rail/topology change, completed verification or measured result claimed.

### 2026-10-25 - Ethernet-switch bottom-layer pour progress
Ethan supplied a bottom-layer screenshot showing local pours around the Ethernet-switch ground-via array and nearby passive pads/tracks. Original evidence is preserved in the [personal engineering log](PERSONAL_ENGINEERING_LOG.md) and [screenshot](images/2026-10-25/windows-altium-eth-sw-bottom-pours-9a6c9828.png). Routing remains in progress with visible airwires. No new rail topology or voltage inferred; exact net assignments, rail isolation, pad/via continuity, neck widths and DRC remain native-layout checks. No Codex Altium edits, hardware measurements or OneDrive sync verification.

### 2026-10-25 - 3.3 V layer pour progress
Ethan supplied a screenshot identified as the 3.3 V layer, showing broad copper coverage and local boundaries around the switch via field. [Archived original](images/2026-10-25/windows-altium-3v3-layer-pours-8ae9da4c.png) and entry in [personal engineering log](PERSONAL_ENGINEERING_LOG.md). Exact layer number and polygon net assignments are not exposed; rail-domain isolation, via connectivity and DRC remain native verification items. No changed rail topology, measured result or completed routing claimed. No Codex Altium edits or OneDrive sync verification.

### 2026-10-25 - Updated top-layer overview
Latest user-supplied top-layer view records local Ethernet-switch pours and surrounding routing after the bottom-layer/3.3 V snapshots. [Original screenshot](images/2026-10-25/windows-altium-eth-sw-top-layer-update-c158fc01.png) is embedded in the [personal engineering log](PERSONAL_ENGINEERING_LOG.md); earlier evidence retained. No new rail topology or net assignment inferred. Airwires remain; native connectivity/DRC and saved-state verification pending. No Codex Altium edits or OneDrive sync verification.

### 2026-10-29 - 3.3 V buck layout reworked; replication planning
Windows/Altium user report: left-hand 3.3 V buck layout redone; wants to propagate placement/routing to other two bucks. Screenshot archived, not independently verified in native PCB or electrically approved. Proposed use Copy Room Formats only with verified one-to-one channel/source component mapping; for separately drawn circuits consider Tools > PCB Layout Replication. Different feedback values and any topology/footprint differences must remain accounted for; image alone cannot establish equivalent blocks. Copy only local copper, exclude board-wide pours, verify target net assignments and existing routing handling, then repour/DRC. Replication not reported completed. References: Altium Working with Rooms on a PCB, Creating a Room / Copying Room Formats (unpaginated), https://www.altium.com/documentation/altium-designer/pcb/rooms ; PCB Layout Replication, steps 1-5 (updated Oct 5 2026, unpaginated), https://www.altium.com/documentation/altium-designer/pcb/layout-replication . No Codex native edits, tests or sync verification.

### 2026-10-29 - 1.8 V buck layout progress screenshot
Windows/Altium user-supplied evidence, recorded by Codex. Latest close-up shows the 1.8 V buck placement, local routing and copper pours: AGND_1V8 copper to the left, input-side copper and GND via groups above/right, feedback/support routing around the regulator, +1V8_OUT pad, and BUCK_1V8_SW copper extending toward the inductor. Supplied after discussion of reusing the revised 3.3 V buck layout; screenshot does not establish whether room-format copying was successful or the earlier linked-room movement issue was resolved. No electrical values or topology changes inferred. No native PCB edit/inspection, DRC, connectivity validation, thermal/SI signoff, or OneDrive sync verification performed. Next: verify destination net mapping and room membership, repour, and check clearance/connectivity before treating the rework as complete.
Evidence: [1.8 V buck layout screenshot](images/2026-10-29/windows-1v8-buck-layout-7442e256.png).

### 2026-10-29 - Three-buck layout rework for noise isolation, density and efficiency
Windows/Altium user report, recorded by Codex: Ethan reports redoing the buck layout to optimize noise isolation, placement density and efficiency. Supplied overview shows all three buck sections with similar placement arrangements, revised local routing/pours and via arrays, plus output copper/capacitor groups below. This updates the earlier left-buck-only and 1.8 V progress entries: rework across the buck area is now user-reported. Exact room-copy method and earlier room membership issue resolution are not established by the screenshot. No component-value, rail assignment or electrical topology changes are inferred. Noise/efficiency improvements are design intent, not measured results; source PCB state, complete DRC, connectivity, thermal performance and EMI remain unverified. Next: repour and run full DRC; verify each rail's distinct feedback values/net mapping and local current/return paths, then validate ripple, efficiency and temperature on hardware. No Codex native Altium edits or OneDrive sync verification.
Evidence: [Three-buck layout overview](images/2026-10-29/windows-three-bucks-layout-rework-089ca5ee.png).

### 2026-10-29 - Load-switch routing rework focused on power stitching
Windows/Altium user report, recorded by Codex: Ethan reports optimizing load-switch routing, particularly power stitching. Screenshot shows three load-switch sections with local power copper and distributed via connections; visible rail labels identify the SFP1, SFP0 and Ethernet-switch switched 3.3 V branches. Record as user-reported layout implementation/progress. No component-value, current-limit or topology change inferred. Exact via dimensions, layer spans, rail isolation and current sharing have not been independently checked; reduced impedance or improved thermal/electrical performance is not measured. Next: restore/repour applicable polygons, verify input/output net separation and intended plane connections, then run clearance/connectivity DRC. No Codex native PCB edit, hardware measurement or OneDrive sync verification.
Evidence: [Load-switch power stitching overview](images/2026-10-29/windows-load-switch-power-stitching-75a211c3.png).

### 2026-10-30 - Board-wide power stitching and pour reorganization
**Computer/tool:** Windows/Altium work reported by Ethan; Windows/Codex documentation.
Ethan reports reviewing the entire board, adding power stitching vias and reorganizing power pours. This extends the earlier buck and load-switch layout work to board-wide power distribution. Four original screenshots document the switch-area local power copper/vias, the explicitly labeled L5 power-layer view, an additional inner-pour view, and the switch/SFP-area overview. Exact layer identity for the unlabeled views and individual pour net assignments are not inferred from color. No rail-voltage, component-value, sequencing or electrical topology change was reported.
Status: user-reported layout work supported by screenshots; native saved PCB, final connectivity/DRC and electrical/thermal performance were not independently checked. The previous October 29 zero-unrouted snapshot does not establish the status after these edits. Next: repour and check intended via-to-plane connections, rail separation and clearances, then refresh connectivity/full DRC. Earlier SFP chassis/reference-plane review remains open; these images alone do not resolve it. No Codex Altium edits or OneDrive sync verification.
Evidence: four original screenshots in the corresponding PERSONAL_ENGINEERING_LOG.md entry and images/2026-10-30/.


### 2026-10-30 - Additional MCU power-pour evidence
Windows/Altium work reported by Ethan; Windows/Codex documentation. Additional image supplements today's board-wide power stitching/pour reorganization: MCU-area local copper, via connections and visible VDDCORE, VLXSMPS and VDD2SMPS labels. Records current user-supplied layout evidence without inferring changed topology or verifying electrical performance. No native edit or final DRC performed by Codex; OneDrive sync not verified.
Evidence: [MCU power pours](images/2026-10-30/windows-mcu-power-pours-20642b8c.png).


### 2026-11-22 - Windows/Codex: latest-export presentation review

Reviewed the repository's 26-sheet Rev 1.0 schematic dated 2026-11-05 (identical schematic blob on fetched demo/main). This is an exported-design review, not a native Altium edit or bench result. Detailed calculations and pin explanations are in [Design defense notes](DESIGN_DEFENSE_NOTES.md), sections 2-10, and the [presentation PDF](../output/pdf/RAVEN_Design_Defense_Notes.pdf).

- Sheet 6 still marks R600-R603 DNP. This supersedes any summary suggesting that the latest export proves those links fitted; reconcile assembly BOM and continuity before claiming an operational common input bus.
- At 1.1 MHz and 2.2 uH, 3.3 V ripple current is 0.464 A at 5 V input and 0.989 A at 12 V. The 1.8 V values are 0.476/0.632 A. Some sheet annotations mix the input cases. The 1.8 V rail's copied UV/OV/OOB numbers should be approximately 1.440/2.089/1.891 V nominal. TI TPS54J061 SLVSFQ0B, §§7.2.2.2-7.2.2.4/Eqs.8-22, pp.20-22 and §5.5 p.6 establish the calculation model and thresholds: [local datasheet](audits/2026-10-05/references/tps54j061.pdf).
- The core buck is set to about 1.1588 V; typical 116% OVP is 1.344 V, above LAN9645xF's 1.265 V absolute maximum. Do not describe this as independently chip-safe overvoltage protection. LAN9645xF DS00006065D §5.1-5.2 p.189 gives absolute/operating limits: [local datasheet](../eth_SW/docs/LAN9645xF-Data-Sheet-DS00006065.pdf). Actual effective capacitance, overshoot and loop margin remain validation items.
- TPS259474 is a circuit-breaker variant; 1.1k ILIM gives 3.031 A nominal and 15 nF ITIMER gives 12.58 ms nominal blanking. TI SLVSFC9C device table p.4 and §7.3.5.2 Eqs.5-6 p.30 establish the behavior/calculation: [local datasheet](audits/2026-10-05/references/tps25947.pdf).
- TPS22950C FLT can remain Hi-Z during an output-short/current-limit state; thermal/reverse-current faults are different. TI SLVSFJ2B Table9-1 p.15: [local datasheet](../power_ic/tps22950.pdf). Firmware must not interpret released FLT alone as proof of no overload. TPS22997 PG indicates gate/full-load readiness, not a precision output-voltage comparator; CT requires at least 30 V rating per TI SLVSGT0 §8.3.2 and §8.3.5 p.14: [local datasheet](audits/2026-10-05/references/tps22997.pdf).
- MCU reset divider is (42.2k+4.7k)/10k, approximately 1.707 V nominal trip; its tolerance can release below the MCU AON 1.71 V operating minimum. MIC2774 DS20006527A electrical limits p.3/Table2-1 p.5: [local datasheet](references/2026-11-22/mic2774.pdf). STM32 AN5967 Rev6 Table3 p.6 and §3.5 pp.8-11 set AON limits and main/auxiliary sequencing: [manufacturer application note](https://www.st.com/resource/en/application_note/an5967-getting-started-with-hardware-development-for-stm32n6-mcus-stmicroelectronics.pdf).

No approved power architecture/component change is made by this review. Next: reconcile fitted links, confirm exact MLCC/inductor corners, then measure startup/shutdown, ripple, load steps and fault behavior. OneDrive sync not verified.
