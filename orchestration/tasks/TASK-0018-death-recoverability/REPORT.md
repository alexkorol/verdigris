# TASK-0018 report — D-106 death recoverability

## Executive summary

Scion death now preserves every carried item and trophy as recoverable value.
Items enter the existing FIFO `House::relic_candidates` pool; equipped items
retain the established `registered after Scion death` history line, while pack
items receive a route-specific `lost at <route>, awaiting recovery` line.
Trophies use a separate FIFO `House::lost_trophies` pool so they remain
recoverable without becoming durable House storage. Both pools resurface from
the seeded reward stream at the existing 1-in-4 cadence, and successors still
start with empty carried inventory.

## Changed files

- `native/include/verdigris/core.hpp` — added `House::lost_trophies` and an
  appended `EventType::TrophyResurfaced` value (existing event ordinals remain
  stable).
- `native/src/core.cpp` — all-item/all-trophy death transfer, ordered trophy
  resurfacing, dedicated event/legend records, and no change to extraction.
- `native/tests/core_tests.cpp` — updated the old lost-forever assertion and
  added named D-106 coverage for multi-item death, ordered pools, trophy
  resurfacing, pack-item history, successor emptiness, and deterministic replay.

No client, build, persistence, seasonal, or prototype files were changed.

## Verification

- `powershell -NoProfile -File native/build.ps1 -RunTests` — PASS;
  `native legacy denylist: PASS` and `verdigris core tests: PASS`.
- `git diff --check` — PASS.
- `test_d106_all_carried_value_is_recoverable` — verifies two carried items
  and two carried trophies are transferred exactly once, nothing is stored at
  death, equipped/pack history differs as specified, and the successor starts
  empty.
- `test_d106_recovery_is_ordered_and_deterministic` — verifies identical seeds
  produce identical recovery pools, oldest-first item/trophy resurfacing, pack
  history after item re-entry, dedicated trophy recovery events, and eventual
  recovery of both trophies.
- Existing relic round-trip tests remain green; the prior assertion that an
  unequipped relic always receives the equipped wording was deliberately
  updated to the new route-specific pack history required by D-106.

## Ownership and extraction notes

Death clears the Scion’s carried vectors but does not append anything to
`stored_items` or `stored_trophies`. Recovery removes the oldest candidate from
its pool before placing it on the ground, preserving the single-owner
invariant. Picking up a resurfaced item/trophy follows the existing pickup
path; extraction remains the only path into durable House storage.

## Deviations and risks

None. Trophy re-entry uses the unchanged `kRelicResurfaceOneIn` cadence and a
separate roll only when the trophy pool is non-empty; item re-entry behavior is
unchanged when no trophy pool exists. Architect review is required for
acceptance.
