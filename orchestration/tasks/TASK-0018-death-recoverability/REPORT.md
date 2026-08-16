# TASK-0018 report — D-106 death recoverability

## Executive summary

Scion death no longer destroys carried value. `handle_death`'s Scion branch now
moves EVERY carried item (equipped + pack) into `house_.relic_candidates` and
every carried trophy into a new `house_.lost_trophies` pool. Recovery stays
gated by the existing seeded 1-in-4 re-entry roll (TASK-0006 seam,
`kRelicResurfaceOneIn` unchanged), so recoverable ≠ immediately inherited, and
successors still start empty (D-004 untouched).

## Approach

- Items: the death loop flags each carried item `relic_candidate`, appends a
  history line — the equipped item keeps the existing "registered after Scion
  death"; pack items get "lost at <route>, awaiting recovery" — pushes to
  `relic_candidates` (oldest-first order preserved), and records one
  `relic_candidate` legend per item. Nothing enters `stored_items`.
- Trophies: new `House::lost_trophies` vector (deliberately NOT
  `stored_trophies`, which would bank them and kill extraction risk).
  `drop_reward` draws oldest-first from it on the same seeded roll as relics,
  drops the trophy on the instance ground, and emits the new
  `EventType::TrophyResurfaced` (appended last in the enum so recorded streams
  keep stable values). The trophy must still be picked up and carried out to
  reach `stored_trophies`.
- The resurface roll is guarded by pool-non-empty, so the RNG stream is
  unchanged whenever no lost trophies exist.

## Changed files

- `native/include/verdigris/core.hpp` — `House::lost_trophies`, `EventType::TrophyResurfaced`.
- `native/src/core.cpp` — `handle_death` Scion branch (all-items loop +
  trophy pool append), `drop_reward` trophy resurface block.
- `native/tests/core_tests.cpp` — five named tests + two helpers; two old
  assertions rewritten (below).

## Test commands + outcomes

- `powershell -NoProfile -File native/build.ps1 -RunTests` — PASS
  (denylist gate PASS, `verdigris core tests: PASS`), rerun independently by
  the coordinator after implementation.
- `git diff --check` — PASS.

## New named tests (one per D-106 behavior)

- `test_death_moves_every_carried_item_to_relic_pool` — every carried item
  registered exactly once; correct equipped/pack history lines; nothing in
  `stored_items`; trophies in `lost_trophies`, not `stored_trophies`.
- `test_successor_starts_empty_after_geared_death` — D-004 unchanged.
- `test_pack_items_resurface_oldest_first_with_ordered_history` — ordered
  "picked up" → "lost at …, awaiting recovery" → "resurfaced on route …";
  pool drains fully, no starvation at 1-in-4.
- `test_carried_trophies_are_recoverable_not_banked` — full round trip:
  death → pool → `TrophyResurfaced` → ground → pickup → extract →
  `stored_trophies`.
- `test_death_recovery_replay_is_deterministic` — two same-seed sims, full
  multi-item death + recovery loop, identical event/legend streams.

## Old lost-forever assertions rewritten (spec scope item 3)

1. `test_death_and_successor`: "unextracted trophy is not preserved" asserted
   the lost-forever contract. Now asserts the trophy is still NOT banked
   (`stored_trophies` empty — extraction risk intact) AND sits recoverable in
   `lost_trophies`.
2. `test_relic_loss_again_returns_once`: expected the re-lost relic's history
   line to be "registered after Scion death" while carried UNEQUIPPED. Under
   D-106 unequipped items take the pack line, so it now expects "lost at
   route:tin:1:0, awaiting recovery". Equipped-line coverage moved to the new
   multi-item test.

## Deviations / unresolved questions

None. The trophy stop condition was not hit — the pool + resurface hook
sufficed. Cadence constant unchanged; tests show no starvation.

## Risks / follow-ups

- `TrophyResurfaced` is a new event type; any future client event rendering
  should handle it (currently no native client consumer).
- Brands & Bonds quality/scar transformation of recovered items remains open
  future work, as the spec's non-goals note.

## Commit

`c9e86c8` on `codex/TASK-0018-death-recoverability` (base `237e5dd`).
