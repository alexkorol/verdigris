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
The supplied Claude demo plates and their provenance boundary are catalogued
in [`CLAUDE_DEMO_ASSET_INTAKE.md`](CLAUDE_DEMO_ASSET_INTAKE.md); binaries remain
outside production until the owner resolves asset provenance/packaging.

## Orchestration state

- Tasks 0001–0037, 0039–0041, 0043–0044: integrated on the coordinator
  program branch.
- TASK‑0043 playtest harness stabilization: `ACCEPTED` and integrated at
  `1f470e3` (program tip `50ca60ad`); current-tip unit/build/full-playtest/
  browser-smoke evidence is green. Report:
  [`TASK-0043 REPORT`](../../orchestration/tasks/TASK-0043-playtest-harness-stabilization/REPORT.md).
  Fable's architect review `1f833081` verified the default-mode contention
  proof: three coordinator runs and one architect run passed 31/31 with the
  observed-lag guard active. Loopback-bind standing guidance is recorded in
  the review for future test/dev servers.
- TASK‑0044 native protocol N2: `ACCEPTED` and integrated at `5b84f51e`
  (program tip `71b6b207`); committed implementation `d476788` plus the
  loopback-bind correction, native/client gates green, unchanged N1+N2 attach
  regression 4/4.
  Report:
  [`TASK‑0044 REPORT`](../../orchestration/tasks/TASK-0044-native-protocol-n2/REPORT.md).
  The latest coordinator attach transcript is
  [`coordinator-native-attach-2026-08-17.txt`](../../orchestration/tasks/TASK-0044-native-protocol-n2/captures/coordinator-native-attach-2026-08-17.txt).
  The JS/native contract comparison and intentional N2 population stub are
  recorded in [`coordinator-dual-run-matrix-2026-08-17.txt`](../../orchestration/tasks/TASK-0044-native-protocol-n2/captures/coordinator-dual-run-matrix-2026-08-17.txt).
- TASK‑0038 controls/rebinding: `INTEGRATED` at `2c0a00c3` from accepted
  candidate `c73fff1`; Fable verified the rendered pointer/rebind captures,
  WS frame log, unit 788/788, smoke 1/1, and playtest 31/31. Shift+RMB keeps
  the legacy context menu available. TASK‑0042 first-loot remains `BLOCKED`
  on its documented ownership seam.
- TASK‑0045 native protocol N3 combat/skills: architect-accepted and integrated
  at `c49f8c51`/master PR #17 (`86688c1f`), with the review handoff at `6d39565c`.
  Native gates and unchanged 7-scenario attach are green; N4 is now the next
  native parity wave.
- TASK‑0046 playability re-evaluation: `REVIEW_REQUESTED` at worker evidence
  commit `ec42e843` (worker candidate `1de6e45b`) on current tip `45846af7`.
  Coordinator completed both approximately ten-minute arcs with headless
  Playwright and same-owned-server correlation; guest combat is improved,
  while the mortal-oath opener remains mechanically silent. The corrected
  evidence is published. Fable's latest architect state records rev2
  acceptance, but the separate REVIEW.md still contains the older REVISE text;
  protocol reconciliation is pending before the coordinator changes lifecycle.
- TASK‑0047 native protocol N4 items/inventory/Vesselforge: `CLAIMED` by
  Kimi Work on `codex/TASK-0047-native-n4-kimiwork` at `d1e30e2b`, based on
  the N3-integrated program line. The unchanged 13-scenario attach,
  item/inventory unit coverage, and authentic-negative transcript remain the
  required handoff gates; no implementation or acceptance is claimed yet.
- TASK‑0048 Chronicles silent combat: `REVIEW_REQUESTED` at coordinator
  candidate `5dde30f2` (worker `810ddd924`). Real WebSocket and rendered
  browser evidence disproved the accepted silent-combat diagnosis: the
  baseline driver never reached the active opener, while the mortal-oath
  regression scenario and rendered first-kill capture both pass. No
  speculative gameplay patch or mana retune was made; architect disposition
  is pending.

## Current coordinator gates

- Browser unit suite: 122 files / 779 tests passed.
- A dependency-complete current-tip browser health run at `a62951e4` repeated
  that result from a fresh worktree with normal install scripts; the server
  boot smoke also passed. Evidence:
  [`coordinator-current-tip-browser-unit-health-2026-08-17.txt`](../../orchestration/tasks/TASK-0044-native-protocol-n2/captures/coordinator-current-tip-browser-unit-health-2026-08-17.txt).
- Browser production build: passed.
- Isolated full browser playtest: prior clean checkpoints passed 31/31; the
  latest coordinator-tip run was 30/31 on the known scheduler-sensitive
  `build-divergence` hit timeout, with an immediate isolated 1/1 rerun. See
  the TASK-0043 follow-up capture before claiming a blanket current-tip gate.
- Fresh coordinator full playtest at the current source-equivalent tip also
  passed 31/31; raw checkpoint:
  [`coordinator-fresh-full-playtest-2026-08-17.txt`](../../orchestration/tasks/TASK-0043-playtest-harness-stabilization/captures/coordinator-fresh-full-playtest-2026-08-17.txt).
- Browser-critical Playwright smoke: 1/1 passed on an isolated port.
- TASK‑0043's full correction chain is now on the accepted program tip;
  earlier disposable candidates and their timing variance remain preserved
  in the task captures for provenance.
- The TASK‑0043 revision proof is now architect-accepted: idle timing
  preserves the authored 8000ms floor, induced scheduler lag adapts within
  the bounded 14000ms cap, and default-mode full runs pass 31/31.
- Committed N2 native gate: denylist, core/networking tests, and client shell
  passed.
- A current-tip native health rerun at `a62951e4` independently repeated the
  denylist/core/networking/client gate and the literal House/trophies/items
  transcript. Evidence:
  [`coordinator-current-tip-native-health-2026-08-17.txt`](../../orchestration/tasks/TASK-0044-native-protocol-n2/captures/coordinator-current-tip-native-health-2026-08-17.txt).
- Committed N2 attach regression: quickstart, single-session, movement, and
  zones passed 4/4.
- N2's clean current-tip candidate was architect-rebuilt and accepted; its
  native gate and 4/4 attach matrix are now integrated at `5b84f51e`.
- A combined parity dry-run at `aa45c78f` applies both review-ready chains
  conflict-free; browser unit 122/779, native gate PASS, and native attach
  4/4. Full details are in
  [`PARITY_INTEGRATION_CANDIDATE_2026-08-17.md`](PARITY_INTEGRATION_CANDIDATE_2026-08-17.md).
- A refreshed combined candidate `3636b729` reapplied both chains before N2
  acceptance; its evidence remains useful provenance, while the accepted
  implementation is now integrated at `5b84f51e`.
  browser unit 122/779, unchanged browser playtest 31/31, native gate PASS,
  and native attach 4/4 all pass. It is superseded by the accepted program
  tip and retained only as provenance.

## Next authorized actions

1. Monitor TASK‑0047's N4 implementation and independently validate its
   13-scenario attach and D-106 item recoverability evidence.
2. Obtain Fable's re-review of the newly published TASK‑0046 two-arc evidence.
3. Obtain Fable's disposition of TASK‑0048's disproven Chronicles diagnosis.
4. Integrate only accepted commits; preserve coordinator-owned metadata during
   the known TASK‑0044 `STATUS.md` add/add merge conflict.
5. Resolve QUESTION‑0007 and QUESTION‑0008 with the owner before implementing
   the blocked first-loot and control-rebinding seams.
6. Keep complete magic, production asset provenance, seasonal inheritance, and
   economy scope behind their documented owner decisions.

The N3 handoff is now explicit in
[`N3_PARITY_IMPLEMENTATION_BRIEF.md`](N3_PARITY_IMPLEMENTATION_BRIEF.md), with
the authority/metadata gap captured in
[`coordinator-n3-core-gap-audit-2026-08-17.txt`](../../orchestration/tasks/TASK-0044-native-protocol-n2/captures/coordinator-n3-core-gap-audit-2026-08-17.txt).
