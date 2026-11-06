# PHOTON demo console

Working local CLI + browser GUI for the two-board Ethernet/fiber demo. Python 3.10+; **no third-party dependencies**. All SFP readings, traffic and switch operations are simulated. No boards, actual iperf processes or register writes are involved.

## Start

From the repository root on Windows, macOS or Linux:

```sh
cd tools
python -m demo_console serve
```

Open **http://127.0.0.1:8765**. On macOS use `python3` if needed. Leave the server terminal running. Other terminals and browser windows share the same session. Ctrl+C stops it; restarting resets state. Diagnostic exports are durable; simulation state is not automatically restored.

```sh
python -m demo_console status
python -m demo_console watch
python -m demo_console scenario fade
python -m demo_console scenario healthy
python -m demo_console switch 0
python -m demo_console inject attenuation 10 --board B --link 0
python -m demo_console inject stale true --board B --link 1
python -m demo_console inject drop_messages true
python -m demo_console traffic --mbps 900
python -m demo_console events --count 30
python -m demo_console export session.json
python -m unittest demo_console.test_engine -v
```

Policy accepts JSON: `python -m demo_console policy '{"auto":true,"revertive":false}'`. Shell escaping differs in Windows PowerShell; use GUI policy controls if your shell strips quotes. `--url http://HOST:8765` goes **before** the CLI subcommand.

For a trusted isolated demonstration LAN, explicitly start `serve --host 0.0.0.0`; point both laptop browsers/CLIs to the server laptop's IP. There is no authentication/TLS. Do not expose this control service to an untrusted network. The two laptops are views of one simulated two-board system, not distributed STM32 firmware.

## Suggested demo

1. Healthy: 900 Mbps offered; primary SFP0 selected on A+B. Observe four DOM cards and VLAN tables.
2. Fade primary: gradually attenuates **A Tx → B Rx** only. B's Rx crosses -16 dBm; 1.5-second persistence qualifies the fault.
3. Observe PREPARE / QUIESCE / COMMIT and ACKs (enable Protocol detail). Both boards block old user forwarding before either enables the backup. Traffic interruption appears on the chart and outage counter.
4. Restore healthy: optics recover above -13 dBm for five seconds. Default policy stays on backup. Manually return to primary, or enable revertive operation; automatic transitions respect eight-second hold-down.
5. Demonstrate brief flaps (below hard-fault persistence), failed standby, stale DOM, switch write/readback failure and management partition. Restore healthy clears injections, not counters or policy.
6. Export JSON and CSV. JSON contains last 600 events and 600 history samples, current state, policy, optical values, VLAN membership and counters. CSV is the rolling chart history.

## Model and policy

- Coordinator A owns monotonically increasing epochs; B is an agent. Both participants receive explicit protocol commands; stale epochs are ignored. Commands retry every 0.4 s. A complete transaction times out after 2 s. ACKs represent modeled switch readback, not receipt alone.
- PREPARE verifies availability, QUIESCE removes the old user uplink, COMMIT enables the selected one. An incomplete PREPARE retains old forwarding; timeout after QUIESCE blocks forwarding. A local peer watchdog blocks forwarding after 1.5 s without heartbeat. Recovery reconciles both boards via a fresh transaction.
- This single-process simulation models local transaction watchdog expiry together at timeout; it does not prove a distributed consensus protocol under clock skew/reboots. No coordinator election or failover to B is implemented. A offline causes fail-closed operation.
- Primary and backup both have **separate, CPU-terminated management VLANs**, proposed VIDs 4090/4091; user VLAN100 includes copper DEV0 and only the active optical DEV5/6. CPU DEV7 is not a user bridge. This is a proposed change from management solely on the active fiber and requires firmware/switch validation.
- Rx trip -16 dBm, recovery -13 dBm; soft persistence 1.5 s, hard-fault hold 0.3 s, recovery hold 5 s, hold-down 8 s. DOM older than 2 s is faulty. These are **demo engineering choices**, not Finisar limits. Temperature/voltage/bias are displayed but do not currently trigger policy.
- Optical baseline -7 dBm, synthetic link loss below -19 dBm, synthetic LOS below -23 dBm. Noise is seeded. Quality declines below -14 dBm. None of these curves establish measured transceiver sensitivity or BER.
- Synthetic traffic is unidirectional A→B, capped at 940 Mbps; loss derives from offered/delivered rate and worst-direction optical quality. RTT/jitter are illustrative. It is not TCP congestion control, real packet timing, or an iperf-compatible implementation.
- The model uses 0.1-second simulation ticks and 0.5-second UI/history samples. Under load, simulation time can run slower than wall time. Fault state reflects both boards; local/remote sensor transport and packet serialization are abstracted.

## Files / hardware integration boundary

`engine.py`: deterministic policy, sensor/physical model, abstract switch state, protocol and traffic model. `__main__.py`: shared HTTP service and CLI. `static/`: dependency-free dashboard. `test_engine.py`: behavioral regression tests.

API: GET `/api/state`, `/api/export`, `/api/history.csv`; POST `/api/command` with JSON actions `scenario`, `inject`, `policy`, `switch`, `traffic`. Invalid commands return HTTP400 and an error. GUI and CLI use the same API.

Before hardware use, extract adapters for: (1) local DOM/GPIO and timestamps; (2) LAN96455 VLAN programming + readback; (3) peer message transport, replay protection, per-board durable epochs/watchdogs; (4) real iperf telemetry/port counters. Preserve the policy tests and add hardware-in-loop verification. Do not simply mark synthetic metrics as measured. Boot/management register setup remains outside this demo.

Logical DEV numbering uses Microchip **DS00006065D**, Table4-3 p53 / Table4-4 p54 (CuPHY0/DEV0, SerDes0/DEV5, SerDes1/DEV6, RGMII0/DEV7). The demo VLAN assignments and timing values are proposed software allocations, not manufacturer requirements.

## Relationship to embedded firmware

This desktop tool is independent of the CubeIDE build in [firmware](../../firmware). It does not execute or validate the C failover implementation. It needs only Python, not STM32CubeIDE or boards. Run all `python -m demo_console ...` commands from `tools/`. Windows users can also double-click `start-demo.cmd` in this directory.

At integration baseline `084ae79`, [failover.h](../../firmware/Appli/src/app/failover.h) defines -18 dBm failure, 2 dB recovery margin, 3000 ms failure/recovery confirmation and 5000 ms hold-down. The simulation retains its previously demonstrated -16/-13 dBm, 1.5/5 s confirmation and 8 s hold-down defaults. These differences are intentional and **not firmware parity**. GUI policy controls can approximate the firmware thresholds, but do not make the algorithms equivalent.

Likewise, [peer_comm.h](../../firmware/Appli/src/app/peer_comm.h) declares a heartbeat/cutover interface; the simulator's epoch-based prepare/quiesce/commit protocol and separate management VLANs are proposed behavior, not the current firmware wire format. Firmware integration must reconcile those choices explicitly. No embedded source is changed by adding this demo.

After starting the server, validate its HTTP routes and error handling from a second terminal in `tools/`:

```sh
python -m demo_console.smoke_test
```
