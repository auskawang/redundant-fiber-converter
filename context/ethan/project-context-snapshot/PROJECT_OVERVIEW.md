# Project Overview

## Goal

Build two identical intelligent Ethernet-to-fiber converter boards. The system carries ordinary Gigabit Ethernet between two laptops over either of two full-duplex 1000BASE-SX fiber paths. Each board monitors its local SFP diagnostic data. The STM32 management endpoints exchange telemetry, heartbeat, and arbitration messages in-band over the active Ethernet path. Firmware can transfer traffic from the primary optical path to the backup path before complete link failure.

## Demonstration topology

```text
Laptop A --1000BASE-T-- Board A ==primary/backup 1000BASE-SX== Board B --1000BASE-T-- Laptop B
                              STM32 A <--in-band management--> STM32 B
```

## Identical-board roles

- Board A firmware role: controller/master for demonstration arbitration.
- Board B firmware role: remote agent reporting local link health and applying coordinated changes.
- Hardware remains identical; role is selected through firmware/configuration.
- Both laptops remain user-data endpoints. The STM32 devices are management endpoints, not inline software bridges.
- Ethernet user traffic and management frames coexist on the active link; user traffic does not need to pause for telemetry.

## Team split

- Yichi and Auska Wang: firmware leadership.
- Ethan and Madhawi Alhabri: PCB and schematic design leadership.

## Current design phase

As of October 6, 2026, Ethan reports the schematic generally complete and PCB layout started. Placement/routing and review closeout are the current phase; final verification and fabrication release remain separate milestones.


## 2026-10-03 — First-look schematic review milestone

Ethan reports an external schematic review held October 2, 2026. Current work remains schematic design and firmware implementation planning. Next: process feedback, repeat review, finalize BOM, and start placement/layout before fall break ends (calendar deadline not supplied). Review is not design sign-off. Recording and named engineering resources are preserved in PERSONAL_ENGINEERING_LOG.md; recording contents not reviewed.


### 2026-10-06 - Schematic generally complete; PCB layout started

Windows/Altium milestone reported by Ethan, recorded by Windows/Codex: the schematic is generally complete and PCB layout has started. This updates the project phase from schematic development to layout. Initial stackup, SE50/DIFF100/DIFF90 impedance profiles and routing-rule work are documented with paired Altium/Sierra screenshots in [PCB_IMPEDANCE.md](PCB_IMPEDANCE.md). General schematic completion is a user-reported milestone, not independent verification or fabrication sign-off. Outstanding review/verification items remain tracked separately. Next: continue placement and routing, incorporate review feedback, and reconcile final schematic/BOM/connectivity and manufacturing checks before release. No native Altium edits or bench tests performed by Codex in this logging task; OneDrive sync not verified.

### 2026-10-15 - Ports room RJ45 test-point placement complete
Windows/Altium work reported by Ethan; recorded by Windows/Codex. User reports the Ports room placement done, explicitly limited to the RJ45 test points. Screenshot shows the test-point group beside U1800 and surrounding placement with remaining airwires. Record as user-reported completion of this placement subset, not completion of all RJ45 circuitry, routing or DRC. No component selection or electrical topology change reported. Next continue board routing and verify test-point net assignments, accessibility and clearances in the native PCB. No Codex Altium edit, native saved-state verification, hardware test or OneDrive sync verification.
Screenshot preserved in the corresponding PERSONAL_ENGINEERING_LOG.md entry.

### 2026-10-16 - Condensed LAN-area placement for routing optimization
Windows/Altium work reported by Ethan; recorded by Windows/Codex. Condensed the component placement around U1800 to improve routing access and organization. User-provided screenshot records the latest placement of surrounding power, strap/control, test-point and clock/support groups. Visible routed segments and numerous remaining airwires indicate routing remains in progress. This snapshot supersedes earlier LAN-area images as the latest visual placement state while preserving their history. No electrical-value, topology or boot-configuration change reported. Placement intent is routing optimization; completed routing, quantified improvement and DRC closure are not established by the screenshot. Next continue routing and check clearances, return paths and connectivity. No native Altium edits or saved-state verification by Codex; OneDrive sync not checked.
Evidence archived in the corresponding PERSONAL_ENGINEERING_LOG.md entry.

### 2026-10-16 - Further LAN-area placement update
Windows/Altium screenshot update supplied by Ethan; recorded by Windows/Codex. New snapshot continues the condensed-placement/routing-optimization work around U1800 and supersedes the earlier 6ef2b32d image as the latest visual state; earlier evidence retained. Surrounding component arrangements have been refined, with numerous airwires still visible. User supplied no additional electrical or topology changes, so none are inferred. Placement/routing remains in progress; native saved state, connectivity and DRC not verified. Next continue routing and clearance checks. No Altium source edits or OneDrive sync verification by Codex.
Latest screenshot embedded in PERSONAL_ENGINEERING_LOG.md.

### 2026-10-18 - Full-board placement and silkscreen snapshot, both sides
Windows/Altium screenshots supplied by Ethan; recorded by Windows/Codex. Archived full-board 2D layout plus top- and bottom-side 3D views as the latest overall placement/annotation evidence. Views document component distribution on both sides, RJ45/SFP connector locations, power/support groups, service headers and configuration-switch/LED silkscreen. Current images visibly label the three switch banks SW1/SW2/SW3; this records displayed labels only, not verified schematic/PCB reannotation or replacement of historical SW1900/SW2000/SW2001 references. Numerous airwires remain in the 2D view, so routing completion is not claimed. No additional electrical changes reported; exact net mappings, model accuracy, mechanical clearances, native saved state and DRC not verified. Next continue routing and check labels/connectivity and manufacturing clearances. Prior snapshots retained. No native Altium edits or OneDrive sync verification by Codex.
All three screenshots are embedded in PERSONAL_ENGINEERING_LOG.md.

### 2026-10-18 - Slashed zeros for silkscreen readability
Windows/Altium update reported by Ethan; recorded by Windows/Codex. Added slashes to silkscreen zero characters so numeric 0 is distinguishable from letter O. Visual labeling/readability change only; no net-name or electrical change reported. No new screenshot supplied and native saved state not independently verified. Next check final silkscreen output for legibility and minimum stroke/clearance compliance. No Codex Altium edits or OneDrive sync verification.

### 2026-10-29 - Board statistics show zero unrouted connections
Windows/Altium screenshot supplied by Ethan, recorded by Codex. Board Information now shows Unrouted/Hidden 0/0, superseding the earlier same-day 1/0 snapshot. Displayed DRC Violations remains 45. Board dimensions 119.8 mm horizontal by 119.8 mm vertical; displayed area 14331.537 mm²; 10 layers, 837 components (502 top, 335 bottom), 432 nets, 15094 tracks, 1881 vias and 127 polygons. This establishes the displayed connectivity-counter milestone only, not a fresh full DRC pass, electrical correctness, fabrication readiness or hardware validation. Next: repour and run/review full DRC, resolve or explicitly justify remaining violations. No Codex native PCB edits or OneDrive sync verification.
Evidence archived in the corresponding PERSONAL_ENGINEERING_LOG.md entry.

### 2026-10-30 - Layout-review follow-up largely addressed
Windows/Altium work reported by Ethan; recorded by Windows/Codex. Ethan reports handling nearly every layout-review comment. Individual comment IDs, resolutions and remaining exceptions were not supplied; record progress rather than complete review closure. Next: identify remaining comments and verify their dispositions alongside refreshed DRC/connectivity checks. No fabrication sign-off, native PCB changes by Codex or OneDrive sync verification.

### 2026-11-05 — Board ordered from JLCPCB; DRC/DFM iterations ongoing
Ethan reports the board ordered from JLCPCB, with continuing DRC/DFM iterations. This advances project status beyond layout-only work; it does not establish complete DRC/DFM closure or independently verified production acceptance. Order identifier/date, quantities, assembly scope, submitted revision and shipping status were not provided. Next preserve submitted/revised manufacturing files, track manufacturer feedback to closure, and prepare arrival/bring-up. Virtual GUI/CLI demo and its source archive are documented in DEMO_CONSOLE.md and the November5 personal log. No native PCB edits or bench verification performed during logging.

### 2026-11-15 - Preliminary enclosure sketch for 3D CAD

Ethan supplied a rough sketch of a nominal 120 mm square outline with proposed power, RJ45 and SFP openings/notches. This starts enclosure CAD planning; dimensions and connector access remain preliminary, with no completed CAD model or fit verification reported. The original sketch is archived in the November 15 PERSONAL_ENGINEERING_LOG.md entry. Next reconcile the sketch with the PCB/model, mounting height and connector envelopes before finalizing the enclosure. No electrical design change or native CAD edit by Codex.

### 2026-11-17 - Chassis base developed in Onshape

Ethan expanded the raven_chassis base sketch on the Top plane, showing outer 140 by 140 and inner 122 by 122 boundaries with four 11 by 11 corner features (displayed values; units not visible). This advances the November 15 rough sketch into dimensioned CAD planning. Parts (0) is shown; extrusion, connector openings and mechanical fit remain unverified. These preliminary enclosure dimensions do not supersede PCB dimensions. Evidence is archived in the November 17 PERSONAL_ENGINEERING_LOG.md entry. Next reconcile mounting and connector geometry and define base/wall thickness. No native CAD edits by Codex.


### 2026-11-19 - Chassis solid model and one prototype print queued

Ethan developed the raven_chassis base into solid geometry with corner supports, an underside support rim, segmented walls/access gaps, a lid-frame ledge and interior/exterior fillets. One prototype print is queued: the supplied Hive Makerspace Pluraprint capture shows part_chassis Revision 2 approved and Queued for a Bambu Lab P1S in black PLA (0.4 mm nozzle, 0.2 mm layers, 20% infill, supports enabled; estimated print duration 4 h 33 m). This supersedes the November 17 sketch-only progress status, while physical fit and print completion remain pending. Twenty original screenshots with individual captions are archived in the November 19 PERSONAL_ENGINEERING_LOG.md entry. Next collect and fit-check the prototype against the board and connectors; no native CAD edits by Codex.


### 2026-11-19 - Chassis gallery available in repository

Seven selected enclosure captures and captions are published on demo at docs/chassis/README.md (commit 1b34858), linked as text from the root README. Source images and hashes accompany the gallery. This is documentation publication; queued-print and pending-fit status are unchanged.


### 2026-11-21 - First chassis printed; second queued

Ethan reports completion of the first chassis print and a second chassis queued. Three photos archived with captions in the November 21 PERSONAL_ENGINEERING_LOG.md entry show fabrication, the first printed interior and its underside. This supersedes the first-print queued status recorded November 19. PCB/connector fit and dimensional checks remain pending; second-print queue details were not supplied. Next fit-check the first chassis and record the second print outcome.
