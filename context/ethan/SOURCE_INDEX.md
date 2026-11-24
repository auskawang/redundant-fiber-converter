# Source and reference index

## Material committed in this folder

### Project-context snapshot

`project-context-snapshot/` contains dated copies of:

- `PROJECT_OVERVIEW.md`
- `ARCHITECTURE.md`
- `COMPONENT_SELECTIONS.md`
- `POWER_TREE.md`
- `DESIGN_DECISIONS.md`
- `LAN96455_CONTROL.md`
- `STM32_ETH_SW_INTERFACES.md`

The originals remain under the shared OneDrive `ece4015/project_context/`
directory and may be newer. This snapshot must not be used to rewrite or
truncate the originals.

### LAN96455F STM32 reference bundle

`reference-bundle/` is the recovered contents of the existing archive:

`ece4015/output/lan96455f-stm32-reference-2026-10-12.zip`

It includes:

- `README_STM32.md`, `SOURCE_MANIFEST.json`, `LOCAL_DOCUMENTS.json`, and
  `DEFINITION_INDEX.json`.
- Microchip `LAN9645xF Data Sheet`, document DS00006065.
- Linux LAN9645x DSA driver source and register definitions.
- LAN9645x SPI/MFD, interrupt, SerDes, MDIO, pin-control, tag, device-tree, and
  binding sources needed to trace initialization and interface behavior.
- The upstream license files from the original source bundle.

Use `SOURCE_MANIFEST.json` for the exact upstream provenance captured when the
bundle was assembled. Preserve the included licensing when redistributing or
deriving work from those sources.

## Additional authoritative files in the shared OneDrive tree

These were intentionally not duplicated because the OneDrive placeholder
layer returned read timeouts during this transfer. They remain available in
the synchronized ECE4015 folder on both computers:

| Relative path under `ece4015/` | Purpose |
| --- | --- |
| `eth_SW/docs/LAN9645XSF-Hardware-Design-Checklist-Managed-Mode-DS00006273.pdf` | managed-mode hardware checklist and host/SPI interface guidance |
| `eth_SW/docs/LAN96459F_EDS2.pdf` | EDS2 reference/evaluation-board guide |
| `eth_SW/docs/lan96459f_unmanaged_board.pdf` | unmanaged evaluation-board reference |
| `sfp/FTLF8519P3xyL_RevC1_8-14-14.pdf` | selected 1000BASE-SX SFP module data |
| `eth_SW/docs/LAN96455_Project_Reference_Tables.xlsx` | project reference tables |
| `eth_SW/docs/02-01429-R3-SCH.pdf` | switch-board schematic reference |
| `output/pdf/PHOTON_Bringup_Validation_Plan.pdf` | bench bring-up plan export |

The LAN9645xF datasheet is already present inside `reference-bundle/documents/`.

## Working-code relationship

The Linux DSA sources are a reference implementation, not a drop-in embedded
library. The project-owned STM32 implementation lives in:

- `firmware/Appli/src/drivers/xswitch/lan96455_driver.c`
- `firmware/Appli/src/drivers/xswitch/lan96455_regs.h`
- `firmware/Appli/src/platform/bxp_spi.c`

When porting a sequence, record:

1. Linux function and source-file name.
2. Register/mask definition used by Linux.
3. Exact LAN96455F datasheet section/table supporting it.
4. Board port mapping and management mode assumed.
5. Reset/clock/polling prerequisites and timeout behavior.
6. What was tested on hardware versus inferred from source.

