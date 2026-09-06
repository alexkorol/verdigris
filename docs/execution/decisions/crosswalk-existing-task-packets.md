# VG-GOV-004 — Crosswalk existing task packets

Draft 2026-09-06 by Cursor Grok. Planning IDs stay DRAFT; this does not
mint TASK numbers.

Canonical tables:

- Do-not-duplicate rules and Cursor notes: `docs/execution/CROSSWALK.md`
- 200-row registry: `docs/execution/CROSSWALK_REGISTRY.md`

## Required dispositions (acceptance)

| Existing | Disposition | Why |
|---|---|---|
| TASK-0108 combat-depth-wave | **extend, never re-spec** | Kimi Work claimed rev 3 core+wire on `kimiwork/TASK-0108-ranged-rev3` (`bebb1aba`, `72b25d85`, `3b929637`). Cursor owns only the local Telegraph presentation ingest on the `native/client/**` lease. VG-ART-003/006 and VG-ACT-004/007 extend; they must not open a second ranged-combat packet. |
| TASK-0145, 0177, 0178, 0197, 0203, 0205–0207 Owner Demo | **extend, never duplicate** | Journey/content/perf gates. VG-UI-006, VG-ART-008, VG-PERF-008, VG-GOV-007 attach to those packets. |
| TASK-0095 | **superseded** | Absorb findings into VG-TOOLS-001 (Kimi). Do not re-audit. |
| TASK-0097 | **superseded** | Absorb findings into VG-SAVE-001 (Kimi). Do not re-audit. |
| TASK-0114 | **verify/extend** | GPU trial direction; VG-GOV-005 / VG-GPU-001. |
| TASK-0166 | **REVIEW_REQUESTED** | Architect/Cursor review queue; not a new VG task. |
| TASK-0205 | **AUTO_RELEASE** | Owner Demo; do not mint a competing packet. |

## Negative control

Two planning goals that map to one implementation without a
`reuse`/`extend`/`verify` flag fail this crosswalk. Minting a new TASK
number for a VG ID also fails.

VG-GOV-002 remains unstamped. This file does not grant claims; exclusive
leases live in `orchestration/CURSOR_KIMI_LANES.md`.
