# TASK-0108 report — lane ox-sw-a

## Executive summary

Realized the authored `behaviour_type == "ranged"` archetype in tile-space
instance combat (W1 / GAP-RANGED-BEHAVIOUR). Ranged monsters now press from
beyond melee contact with a deterministic, readable telegraph: they mark the
scion's current tile for the shipped 1000 ms telegraph window (line of sight
checked at cast over the tile grid), then resolve against the scion's CURRENT
tile, so stepping out of the mark is a real dodge. Damage stays on the
untouched `kN3MonsterDamage` table; cadence reuses the pack cooldown; the
window/radius reuse the boss telegraph vocabulary. Buffers and every melee
stream are byte-identical to pre-W1 behavior (locked by tests). Ranged hits
land as attributed shooter→scion `combat:hit`s and their warnings ride the
existing telegraph render vocabulary. All acceptance commands pass, including
the full MSVC gate with all 12 client scenarios.

## Approach

- Studied `WorldSimulation::advance_combat` (pack AI loop, active-target loop,
  boss slam), the shipped `Telegraph`/`Damage`/`TargetFlash`/`Impact`
  vocabulary in `render_list.hpp` + `presentation_state.cpp`, and the N3
  constant family in `core.cpp`.
- Extended only the monster-initiated pack-AI path: a ranged branch keyed
  strictly on `behaviour_type == "ranged"` (cast → window → resolve), plus a
  one-clause exclusion so an actively targeted ranged monster can never use
  the instant active-target melee branch. Melee/buffer/boss paths untouched.
- Reach band [3,4] Chebyshev: strictly beyond the two-tile melee contact ring,
  never past the four-tile disengage horizon the swing loop already tolerates.
- Cover: new pure `WorldSimulation::ranged_line_clear` (integer Bresenham over
  `TileGrid.walkable`; intermediate cells must be walkable).
- Locks: core-level stream locks in `core_tests.cpp`; presentation render-op
  locks (ordering + attribution + SPEC negative) in
  `presentation_events_tests.cpp`, constructed exactly as the session seam
  maps the shipped envelopes.
- Mid-wave the full gate exposed a real contract collision: the TASK-0163
  gate-b driver (read-only for this lane) treats EVERY `monster:telegraph` as
  THE elite ground-slam reveal and beelines to its x/y; ranged warnings on
  that channel hijacked the hunt. Fix: ranged warnings ride a dedicated
  `monster:ranged-telegraph` envelope (identical fields) emitted by
  `networking.cpp`; the shipped `monster:telegraph` slam contract stays
  exclusive and byte-identical. The unknown envelope is silently ignored by
  today's client seam (verified), so no existing consumer changes meaning.

## Changed files

- `native/include/verdigris/core.hpp` — `WorldMonster.shot_x/shot_y`
  (marked-tile memory); public `ranged_line_clear` declaration.
- `native/src/core.cpp` — W1 constants (`kN3RangedReachTiles`,
  `kN3RangedShotRadiusTiles`, `kN3RangedSkillId`); Bresenham cover check;
  ranged branch in the pack AI loop; active-target melee exclusion.
- `native/src/networking.cpp` — `emit_combat_event` routes
  `skill_id == "monster:ranged-shot"` telegraphs to
  `monster:ranged-telegraph`; all other envelopes unchanged.
- `native/tests/core_tests.cpp` — four W1 lock tests + geometry helpers.
- `native/tests/presentation_events_tests.cpp` — two W1 render-contract locks.
- Task folder: STATUS.md, captures/review/* (10 scenario evidence PNGs).

## Public interfaces added/changed

- Added: `WorldSimulation::ranged_line_clear(int,int,int,int) const`.
- Added: `WorldMonster::shot_x`, `WorldMonster::shot_y` (simulation-internal
  state; not serialized anywhere — WorldSimulation monsters are not persisted).
- Wire: NEW envelope `monster:ranged-telegraph` (fields identical to
  `monster:telegraph`). No existing envelope, field, or ordering changed.
- No render ops added; no damage/cadence values changed.

## Test commands + outcomes (exact exit codes)

| Command | Exit | Outcome |
| --- | --- | --- |
| `powershell -NoProfile -ExecutionPolicy Bypass -File native/build.ps1 -RunTests -RunClientScenarios -CaptureRoot orchestration/tasks/TASK-0108-combat-depth-wave/captures/review` | 0 | denylist PASS; core/networking/camera2d/session/presentation-events/audio suites PASS; all 12 client scenarios PASS with evidence isolated under the review capture root |
| `native/build/verdigris_core_tests.exe` | 0 | `verdigris core tests: PASS` |
| `native/build/verdigris_session_tests.exe` | 0 | all session suites PASS (incl. gate-b elite-heirloom leg) |
| `git diff --check` | 0 | clean |
| `git diff --name-only` (working tree vs HEAD after commits) | 0 | clean tree except committed task-folder evidence |

New locks introduced (all passing):
- core: ranged opens with exactly one attributed telegraph marking the scion's
  tile; no in-window resolution; trash-table hit (5) at resolution; reload on
  the pack cooldown; dodge-by-stepping-out control; engaged melee twin cannot
  answer from Chebyshev 3-4 and emits zero telegraphs; adjacent melee AND
  buffer streams unchanged (instant hit, exact `4 + level*2`, cooldown-gated,
  telegraph-free); timed replay transcript is byte-identical across runs with
  warnings > impacts (scripted dodge whiff proven).
- presentation: mapped Telegraph→DamageApplied("incoming") beats yield the
  shipped `Telegraph` op anchored on the shooter through the shared camera,
  then attributed `Damage`(label "player", value)/`TargetFlash`+Impact fx, in
  that order, with readable HUD lines; negative controls prove an
  un-telegraphed ranged hit, a foreign shooter's warning, and a reused warning
  each FAIL the lock.

## Manual verification

- Reproduced then eliminated the gate-b hijack: first full-gate run failed
  `gate-b: slain elite surfaces the circulating heirloom` with the hunt pinned
  at a stale marked tile (`elite=(6,18)` = a ranged mark inside the spawn
  clearing, impossible for a boss tile); after the envelope split the same leg
  passes and the whole suite exits 0 twice consecutively.
- Verified the shipped client seam ignores unknown envelopes
  (`remote_session.cpp` `apply_envelope` has no default handler), so the new
  envelope cannot perturb live clients until its owner wave maps it.
- Confirmed `player_level` C4100 in `advance_combat` and the inet_addr/getenv
  C4996s are pre-existing warnings present at base commit 7636846 (no new
  warnings introduced).

## Commit SHAs

- `9cbe65ac` chore(TASK-0108): claim lane ox-sw-a
- `1f5877a4` feat(TASK-0108): realize authored ranged behaviour with telegraphed shots in tile-space combat
- `ff8dbba3` test(TASK-0108): lock telegraphed attributed ranged hit beats at the presentation seam
- `dcadc2bc` fix(TASK-0108): ride ranged warnings on monster:ranged-telegraph to keep the slam reveal contract exclusive
- `bc2f602f` chore(TASK-0108): capture review evidence from the full gate run

Final head: `bc2f602f4a2223ebed8b48ec9ffd6b8976785b02`

## Deviations

1. **`native/src/networking.cpp` modified** (outside the coordinator's named
   pointer set, but not on the forbidden list; SPEC `owned_paths` were never
   frozen after TASK-0101). Required: the read-only TASK-0163 gate-b driver
   defines every `monster:telegraph` as the elite slam reveal, so ranged
   warnings needed their own envelope to keep both contracts true without
   touching any forbidden file. Emission-side only; existing wire bytes for
   existing content unchanged.
2. **Remote client does not yet map `monster:ranged-telegraph`.** Mapping it
   requires `remote_session.cpp` (forbidden this wave; TASK-0161 hold also
   owns client surfaces). The telegraph remains deterministic and readable at
   the simulation/wire layer, and the presentation lock proves the render
   contract for the mapped beat shape; wiring the envelope into the remote
   seam is filed as follow-up 1. The attributed hit beat IS fully
   client-visible today via the unchanged `combat:hit` path.

## Unresolved questions

- Should ranged monsters also shoot at Chebyshev ≤ 2 (currently they only
  press from 3-4; adjacent ranged mobs are harmless)? Left as authored-later;
  the current band makes the melee-twin contrast exact.

## Risks

- Ranged pressure slightly raises instance lethality during walks (5 dmg per
  ~2.2 s while marked and standing in the open). Session suites, including
  the mortality/succession flows, pass repeatedly; watch owner playtests.
- The `monster:ranged-shot` skill-id literal exists in both `core.cpp`
  (authoritative) and `networking.cpp` (discriminator); a rename must touch
  both until a shared header constant is introduced.

## Follow-ups

1. Client-owner wave: map `monster:ranged-telegraph` in `remote_session.cpp`
   to the same `PresentationEventType::Telegraph` shape (fields already
   identical) so ranged warnings render remotely; add a focused scenario.
2. Introduce a shared header constant for the ranged skill id to de-duplicate
   the core/networking literal.
3. Consider line-of-sight re-check or short flight time if owner feedback
   wants cast-time cover to matter less.
4. After TASK-0161 integrates, freeze TASK-0108 owned_paths retroactively to
   the five native files touched here.
