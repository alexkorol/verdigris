# Verdigris reconstitution status

Coordinator snapshot: 2026-08-17. This page is a navigation index, not a
replacement for the task reports, constitution, or Fable's review decisions.

## Current product authority

- [Verdigris Constitution](../product/VERDIGRIS_CONSTITUTION.md) is the
  governing product source.
- The original Delaford checklist is preserved at
  [`docs/archive/DELAFORD_README_FEATURES_CHECKLIST.md`](../archive/DELAFORD_README_FEATURES_CHECKLIST.md).
- The Verdigris reinterpretation checklist is
  [`docs/product/VERDIGRIS_FEATURE_CHECKLIST.md`](../product/VERDIGRIS_FEATURE_CHECKLIST.md).
- Historical-to-current coverage is mapped in
  [`DELAFORD_COVERAGE_MATRIX.md`](DELAFORD_COVERAGE_MATRIX.md); deferred PvP
  and resource skills are deliberately not inferred from the old checklist.
- The current Verdigris checklist-to-evidence gap audit is
  [`VERDIGRIS_GAP_AUDIT.md`](VERDIGRIS_GAP_AUDIT.md); it separates proven
  browser/native slices from parity waves and owner decisions.

## WIZARD components intentionally meshed into Verdigris

- Orbs: live HUD/presentation integration with authoritative simulation state.
- Brands & Bonds / inventory: live item identity, equipment, and inventory
  path; native state remains authoritative.
- Verdigris Splash: retained as a presentation/menu reference, not simulation.
- Cartographer: intended seeded map-content adapter after native collision and
  route validation.
- Arcane Lattice: reference-only until the owner resolves the magic design
  decision; no generic mana system is being invented in this wave.

See [WIZARD Arcane Lattice reference](../product/WIZARD_ARCANE_LATTICE_REFERENCE.md)
and the [legacy matrix](LEGACY_MATRIX.md) for boundaries.
Focused seam evidence is preserved in
[`WIZARD_INTEGRATION_VERIFICATION.md`](WIZARD_INTEGRATION_VERIFICATION.md):
10 files / 73 tests passed for orbs, inventory, and map generation.

## Orchestration state

- Tasks 0001–0037, 0039–0041: integrated on the coordinator program branch.
- TASK‑0043 playtest harness stabilization: `REVIEW_REQUESTED`; current-tip
  unit/build/full-playtest/browser-smoke evidence is green. Report:
  [`TASK-0043 REPORT`](../../orchestration/tasks/TASK-0043-playtest-harness-stabilization/REPORT.md).
- TASK‑0044 native protocol N2: `REVIEW_REQUESTED`; committed implementation
  `d476788`, native/client gates green, unchanged N1+N2 attach regression 4/4.
  Report:
  [`TASK‑0044 REPORT`](../../orchestration/tasks/TASK-0044-native-protocol-n2/REPORT.md).
  The latest coordinator attach transcript is
  [`coordinator-native-attach-2026-08-17.txt`](../../orchestration/tasks/TASK-0044-native-protocol-n2/captures/coordinator-native-attach-2026-08-17.txt).
  The JS/native contract comparison and intentional N2 population stub are
  recorded in [`coordinator-dual-run-matrix-2026-08-17.txt`](../../orchestration/tasks/TASK-0044-native-protocol-n2/captures/coordinator-dual-run-matrix-2026-08-17.txt).
- TASK‑0038 controls/rebinding and TASK‑0042 first-loot moment: `BLOCKED` on
  documented ownership seams; no source workaround should be invented without
  the architect/owner decision.

## Current coordinator gates

- Browser unit suite: 122 files / 779 tests passed.
- Browser production build: passed.
- Isolated full browser playtest: 31/31 scenarios passed.
- Fresh coordinator full playtest at the current source-equivalent tip also
  passed 31/31; raw checkpoint:
  [`coordinator-fresh-full-playtest-2026-08-17.txt`](../../orchestration/tasks/TASK-0043-playtest-harness-stabilization/captures/coordinator-fresh-full-playtest-2026-08-17.txt).
- Browser-critical Playwright smoke: 1/1 passed on an isolated port.
- Committed N2 native gate: denylist, core/networking tests, and client shell
  passed.
- Committed N2 attach regression: quickstart, single-session, movement, and
  zones passed 4/4.

## Next authorized actions

1. Fable performs the required architect reruns and records ACCEPT/REVISE for
   TASK‑0043 and TASK‑0044.
2. Integrate only accepted commits; preserve coordinator-owned metadata during
   the known TASK‑0044 `STATUS.md` add/add merge conflict.
3. Resolve QUESTION‑0007 and QUESTION‑0008 with the owner before implementing
   the blocked first-loot and control-rebinding seams.
4. Keep complete magic, production asset provenance, seasonal inheritance, and
   economy scope behind their documented owner decisions.

The N3 handoff is now explicit in
[`N3_PARITY_IMPLEMENTATION_BRIEF.md`](N3_PARITY_IMPLEMENTATION_BRIEF.md), with
the authority/metadata gap captured in
[`coordinator-n3-core-gap-audit-2026-08-17.txt`](../../orchestration/tasks/TASK-0044-native-protocol-n2/captures/coordinator-n3-core-gap-audit-2026-08-17.txt).
