# Current hardware and firmware status

Status captured 2026-11-24 from Mac bring-up work and supplied measurements.

## Custom-board electrical state

The following are user-measured or user-observed bring-up results, not a full
board qualification:

| Item | Observed state |
| --- | --- |
| `3V3` | approximately 3.284 V |
| `1V8_STM` | approximately 1.798 V |
| `VDDCORE` | approximately 0.81 V |
| `PWR_ON` | approximately 3.297 V / high |
| `NRST` | released at approximately 3.3 V; red reset LED turns off |
| analog reference | externally supplied at 1.8 V |
| `BOOT1` | jumper connected to 3.3 V for development boot |
| STM core supply topology | internal SMPS; `VLXSMPS` through the board's 1 uH inductor to `VDDCORE` |

These observations support attempting a conservative SWD connection. They do
not establish that every rail, clock, boot strap, or peripheral is correct.

## Stage-0 blink image developed on the Mac

The separate local CubeMX project was created at:

`ece4015/stm_code/stage0_blink`

Its intended behavior is deliberately narrow:

- FSBL-only SRAM-linked development image.
- Internal HSI clock at conservative settings.
- `HAL_PWREx_ConfigSupply(PWR_SMPS_SUPPLY)` to match the custom board.
- Only PD6 is configured as an output.
- PD6 drives the active-low `STM_LED`; code toggles it every 500 ms.
- All other application GPIOs are intended to remain untouched/reset-default.
- Built artifact: `FSBL/build/stage0_blink_FSBL.elf`.

The local stage-0 project is not automatically identical to the `firmware/`
tree on this branch. Copy or merge it only after reviewing the generated
project differences.

## Critical branch-firmware warning

At the time this context was assembled,
`firmware/FSBL/Src/main.c` in this branch still calls:

```c
HAL_PWREx_ConfigSupply(PWR_EXTERNAL_SOURCE_SUPPLY)
```

That does not match the custom board's internal-SMPS topology described above.
Do not download this branch FSBL to the custom board without reconciling the
power configuration and regenerating/rebuilding deliberately. The known-safe
Mac stage-0 intent uses `PWR_SMPS_SUPPLY`.

The branch FSBL also initializes XSPI/external-memory handling and launches an
application. That is broader than the one-GPIO SRAM blink experiment.

## ST-LINK-V3SET state

- Probe: STLINK-V3SET.
- The updater later reported firmware `V3J18M12B7S1` as the current version.
- Earlier attempts showed serial-number/firmware corruption symptoms, error
  `0x21`, Error 33, USB pipe stalls, and `Target USB comms error`.
- VS Code's ST-LINK discovery/background server sometimes held or respawned on
  the USB interface.
- The firmware version appearing current is encouraging, but normal SWD/GDB
  communication had not yet been proven stable at the time of this snapshot.

Validate the probe standalone before reconnecting the target:

1. Remove the update-mode jumper and disconnect the target ribbon.
2. Close STLinkUpgrade, VS Code, and any ST-LINK GDB/discovery server.
3. Power-cycle the probe and verify it remains enumerated.
4. Confirm STM32CubeProgrammer or the ST-LINK GDB server can identify the probe
   without USB transfer errors.
5. Reconnect the target only after the standalone probe is stable.

## VS Code debug configuration intent

The Mac test configuration was designed to connect under reset and load only
ELF symbols, with no `imageFileName`, `preBuild`, or automatic run entry. This
was intended as a connection-only checkpoint before any SRAM download.

Treat any generated/default launch configuration that includes an image to
load as potentially programming or downloading firmware. Review it before
pressing Run.

## Recommended next milestones

1. Prove stable ST-LINK enumeration and a halt-only SWD connection.
2. Read target identity and core registers without downloading an image.
3. Reconcile the branch FSBL supply configuration with the internal SMPS.
4. Build and inspect the minimal PD6-only SRAM ELF/map file.
5. Download to SRAM only, confirm the active-low PD6 LED blink, and power-cycle
   to prove the test was nonpersistent.
6. After the stage-0 gate passes, add SPI switch proof, then SFP I2C/DOM, then
   external-NOR boot support one subsystem at a time.

