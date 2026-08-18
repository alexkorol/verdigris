---
task: TASK-0047
title: Parity wave N4 — items, inventory, and Vesselforge data over the C++ server
state: READY
priority: critical (mission critical path, D-116)
owned_paths:
  - native/**
  - orchestration/tasks/TASK-0047-native-protocol-n4/**
forbidden_paths:
  - playtest/** (the harness is the measuring stick — never edit it here)
  - server/**, src/** (browser reference is read-only for this task)
base: program tip AFTER TASK-0045 integration (>= c49f8c51; verify the
  N3 combat types exist in native/src/core.cpp before branching)
architect_review_required: true
---

## Goal

The UNCHANGED playtest harness passes its item-family scenarios against
the C++ server via `PLAYTEST_WS_URL` attach:

- `loot` (kill → coins/items enter inventory)
- `equipment-slots` (ring seats, belt/waist, seat survival)
- `depth-loot` (guaranteed treasure per floor; item level from the
  live vessel — note this needs depth >1, closing N3 stub #3)
- `overflow` (inventory bounds)
- `vesselforge` + `vesselforge-brand` (brands as combat state, the
  coin-cost brand service, tooltip refresh) — formula parity where
  already owner-ruled; if a formula is NOT owner-ruled, mirror the JS
  implementation and flag it in the REPORT rather than choosing

plus a no-regression rerun of the N1–N3 set (`quickstart`,
`single-session`, `movement`, `zones`, `combat`, `encounter-variety`,
`boss-mechanic`).

## Scope (RULES in core, TRANSPORT in networking — same split)

1. Item identity from curated data (LEGACY_MATRIX KEEP-as-data): the
   item table rows the scenarios touch, with stats, footprints, slots,
   stack rules. N3's minimum drop payloads graduate to real items.
2. Inventory rules: add/remove, stacking, footprint/bounds (overflow),
   equip/unequip with slot rules (two ring seats, waist), and the
   equip → combat-attribute pipeline (the harness measures attack
   changes from gear).
3. Pickup/drop over the wire exactly as the JS server speaks them
   (read the referenced JS handlers first; cite files like 0005 did).
4. Depth >1 descent (`transitionFloor`, "· Floor N" naming) — closes
   N3 stub #3 and unlocks `depth-loot`.
5. Vesselforge brands: brand state exposed to combat (the N3 combat
   pipeline consumes them), the 100-coin brand service, tooltip data.
6. D-106 constraint is absolute: no item-destroying path may exist in
   any code added here (death/disconnect transfers are N5's flow, but
   nothing in N4 may make an item unrecoverable).

Stubs allowed at the minimum the scenarios exercise, each documented
with its N5+ successor — 0044/0045's stub inventories are the format.

## Acceptance evidence (literal transcripts in REPORT.md)

1. `powershell -File native/build.ps1 -RunTests` — all PASS lines.
2. Attach transcript: full 13-scenario set (6 item-family + 7
   regression) — 13/13, harness unchanged (state the harness commit).
3. C++ unit coverage for item/inventory rules (list assertions).
4. One authentic negative (0043's format): break one item rule, show
   the harness catching it, restore.

The architect will rebuild the branch and rerun the attach set
personally before ACCEPTED. Literal transcripts are mandatory.
