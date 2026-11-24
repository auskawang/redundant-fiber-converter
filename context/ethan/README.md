# Ethan firmware handoff context

Snapshot date: 2026-11-24  
Branch when created: `demo`

This folder is Ethan's self-contained handoff for continuing STM32N6 and
LAN96455F firmware work on another computer. It intentionally does not replace
the shared OneDrive `project_context/` directory. Refresh the shared files when
working from the original ECE4015 OneDrive tree; treat this folder as a dated
branch snapshot.

## Read first

1. [CURRENT_STATUS.md](CURRENT_STATUS.md) — board state, safe bring-up limits,
   ST-LINK state, and the difference between the bench blink project and this
   branch's firmware.
2. [SOURCE_INDEX.md](SOURCE_INDEX.md) — what is included, where it came from,
   and which documents remain in OneDrive.
3. [project-context-snapshot/ARCHITECTURE.md](project-context-snapshot/ARCHITECTURE.md)
   and [project-context-snapshot/STM32_ETH_SW_INTERFACES.md](project-context-snapshot/STM32_ETH_SW_INTERFACES.md).
4. [reference-bundle/README_STM32.md](reference-bundle/README_STM32.md) before
   deriving bare-metal switch code from the Linux sources.

## Included material

- A dated snapshot of the most relevant shared project-context notes.
- The complete LAN96455F-to-STM32 reference bundle, including the Microchip
  LAN9645xF datasheet, Linux DSA driver sources, SPI/MFD glue, SerDes, MDIO,
  device-tree examples, source manifests, and licensing files.
- A concise bench/firmware handoff reflecting the work performed on the Mac.

## Repository code locations

- `firmware/` — embedded STM32 project currently tracked on this branch.
- `firmware/Appli/src/drivers/xswitch/` — project LAN96455 driver skeleton and
  register definitions.
- `firmware/Appli/src/drivers/sfp/` — SFP control/telemetry driver work.
- `firmware/Appli/src/drivers/net/` — RGMII and management-frame modules.
- `tools/demo_console/` — desktop behavioral model; useful for policy and
  state-machine intent, but not register-level firmware authority.

## Ground rules for the next device

- Do not assume the Linux DSA driver can be copied directly into STM32 code.
  Use it as implementation evidence for register sequences, bit fields, and
  ordering, then verify every port/register assumption against the exact
  LAN96455F datasheet revision and board schematic.
- Do not program the custom board until the image's power-supply selection,
  linker target, boot destination, and GPIO ownership have been reviewed.
- Keep unrelated GPIOs in their reset/default state during first bring-up.
- Commit new firmware work to the intended personal branch; do not overwrite
  the shared project-context history from this snapshot.

