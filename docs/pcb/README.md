# Inside the board

A ten-layer Ethernet-to-fiber converter with dual SFP paths, an Ethernet switch, STM32 management, and onboard power conversion. Explore the board from the assembled view down to the routing constraints.

**Status:** board ordered from JLCPCB; DRC/DFM refinements ongoing. Design snapshot collected November 6, 2026.

## The assembled board

### Top
[![Top-side 3D board view](images/pcb-3d-top.png)](exports/pcb-3d-view-1.pdf)

### Bottom
[![Bottom-side 3D board view](images/pcb-3d-bottom.png)](exports/pcb-3d-view-2.pdf)

*Rendered from the supplied Altium 3D exports. Click either view for its original PDF.*

## Explore the design

| Document | What to look for |
| --- | --- |
| [Schematic — 26 pages](exports/schematic.pdf) | System architecture, power conversion, switch, MCU, SFP and copper Ethernet interfaces |
| [PCB layers — 11 pages](exports/pcb-layer-prints.pdf) | Board overview, signal routing, reference copper and power distribution |

## Layout details

| Area | Top | Bottom |
| --- | --- | --- |
| Ethernet switch | [View](images/eth_sw_layout_top.png) | [View](images/eth_sw_layout_bot.png) |
| SFP interfaces | [View](images/sfp_layout_top.png) | [View](images/sfp_layout_bot.png) |
| STM32 | [View](images/stm_layout_top.png) | [View](images/stm_layout_bot.png) |

[1.15 V buck layout](images/1v15_buck_layout.png)

## Stackup, constraints and routing checks

| Evidence | Captures |
| --- | --- |
| Board and stackup | [Board statistics](images/board_info.png) · [Layer stack](images/stackup_manager.png) · [JLCPCB impedance calculator](images/jlcpcb-impedance-2026-10-27.png) |
| Routing rules | [SE50](images/rules_routing_se50.png) · [DIFF100](images/rules_routing_dp100.png) · [DIFF90](images/rules_routing_dp90.png) |
| Length rules | [Overview](images/rules_length_matching.png) · [QSPI](images/rules_length_matching_qspi.png) · [RGMII RX](images/rules_length_matching_RGMII_RX.png) · [RGMII TX](images/rules_length_matching_RGMII_TX.png) · [RJ45](images/rules_length_matching_rj45.png) · [SFP](images/rules_length_matching_sfp.png) · [USB](images/rules_length_matching_usb.png) |
| xSignal results | [QSPI](images/xSignals_QSPI.png) · [RGMII RX](images/xSignals_RGMII_RX.png) · [RGMII TX](images/xSignals_RGMII_TX.png) · [RJ45 port 0](images/xSignals_RJ450.png) · [RJ45 port 1](images/xSignals_RJ451.png) · [SFP](images/xSignals_SFP.png) · [USB](images/xSignals_USB.png) |
| JLCPCB DFM viewer | [Gerber preview](images/jlcpcb_gerber_t.png) · [Wireframe preview](images/jlcpcb_wireframe_t.png) |

The calculator capture is from October 27: 4-mil widths with 50-ohm single-ended and 96/92-ohm differential targets. Its ±1% calculator setting is not a manufacturing tolerance guarantee. Captures document displayed settings and results; they do not establish complete DRC/DFM closure or measured performance.

## Take the complete tour

**[Download the combined design PDF — 69 pages](PHOTON_PCB_Design_Evidence.pdf)**

Top and bottom 3D views, schematic, PCB layer prints, and all 29 supporting captures in one bookmarked document. The black-and-white fabrication drawings are omitted from this showcase. Original resolution and source PDF page sizes are retained; download the PDF for detailed zooming.

[Source manifest and SHA-256 hashes](manifest.json)
