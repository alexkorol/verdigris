# TASK-0108-combat-depth-wave — STATUS

state: QUESTION
lane: ox-pc-ba
model: openrouter/stealth/ox-alpha
base_commit: 763684666b07483caeeebc2055c804f80bb1515e
branch: worker/verdigris/pc/ox-pc-ba
claimed_at: 2026-08-22T22:51:31-07:00
question_filed_at: 2026-08-23T10:05:00-07:00
revision: 2 (implementation complete on owned paths; one acceptance gate blocked by a frozen-test vocabulary collision)

## Question

Ranged warnings that reuse the shipped `monster:telegraph` wire event are
indistinguishable from the boss ground-slam telegraph to clients, and the
frozen TASK-0148 gate-b journey (`native/tests/session_tests.cpp:1392-1402`)
treats EVERY `monster:telegraph` envelope as THE elite: it re-aims its whole
hunt at the shooter's tile (`elite_x/elite_y = x/y`) and re-arms its
slam-dodge window (`slam_clear_at = arrival + durationMs + 120`,
`slam_radius = radius`). With trash shooters announcing from inside the
engagement envelope, the heir hunt locks onto lurker tile (7,15), never
engages the real Warden deterministically, and burns its 420 s budget.

Resolving this requires one of:

(a) `session_tests.cpp` coordination (owned by READY TASK-0162; forbidden to
    this lane): gate-b should identify the elite slam by its authored
    signature (boss/rarity snapshot or ground-slam skillId) instead of
    "any monster:telegraph".
(b) a wire distinguisher in `emit_combat_event`
    (`native/src/networking.cpp:2062-2078`; NOT in this task's owned_paths):
    e.g. ranged warnings carry their own event name or an explicit
    `kind:"ranged-warning"` field clients can filter on.
(c) shipping ranged pressure without wire warnings — rejected: it ships
    invisible ranged damage, which the SPEC freezes out.

The implementation below is complete and green everywhere else. It stops
here instead of improvising because both files that could unblock the last
gate are outside owned_paths, and weakening either the feature (silent
ranged damage) or the lock tests to force a pass is forbidden.

## Implementation delivered (owned paths only)

- `native/include/verdigris/core.hpp`, `native/src/core.cpp`: deterministic
  `WorldSimulation::seed_monster` content seam (mirrors
  `Simulation::spawn_monster`; empty uuid gets a deterministic one; refuses
  out-of-bounds/unwalkable/occupied tiles). `advance_combat` realizes
  `behaviour_type == "ranged"`: announces every shot with the shipped
  telegraph contract (type `"telegraph"`, skill `"monster:attack"`, the
  authored 1000 ms readable window `kN3BossTelegraphWindowMs`, warning
  anchored at the shooter, target named), resolves damage only while the
  player stays inside the authored engagement envelope
  (`kEngagementEnvelopeTiles = 4`, the pre-existing disengage bound — no new
  distance value), deals the authored `kN3MonsterDamage` family (+2
  empowered), and re-arms on the authored non-boss cadence now named
  `kN3MonsterAttackIntervalMs` (=1500, previously a literal in the same
  function). Stepping out dodges: the announced window evaporates silently
  and the cycle re-arms. Melee and buffer branches byte-identical; the
  active-target melee counterattack explicitly excludes ranged so a twin is
  never resolved twice. No new render ops, art, cadence, or numeric tables.
- `native/client/presentation_state.cpp`: `record_world_ops` now records
  `Op::Impact` for `EffectFx::Kind::Impact` — parity with paint_scene
  (main.cpp:1685) so the headless render list carries the same impact beat
  the GDI painter draws. One case label; no behavior change elsewhere.
- `native/tests/core_tests.cpp` — four locking tests:
  1. `test_ranged_pressure_damages_beyond_contact_melee_twin_does_not`:
     behaviour trio seeded at Chebyshev 4 around spawn via `seed_monster`;
     proves ranged pressure lands from beyond contact with readable warning
     fields naming the threatened player, the melee twin at the same
     distance never reaches the player, and the buffer stays inert.
  2. `test_ranged_shot_is_readable_and_dodgeable`: mid-window exit dodges
     silently (no hit, no new announcement while outside); returning into
     the envelope re-arms and resolves an authored 5-damage beat.
  3. `test_ranged_stream_replay_is_deterministic`: field-for-field replay
     identity of the whole combat-event stream plus final life.
  4. `test_melee_twin_stream_stays_classic_and_untelegraphed` (negative
     control): adjacent melee twin strikes silently, attributed plain hit,
     authored `4 + level*2` damage, unchanged 1200 ms cadence, zero
     telegraph rows ever.
- `native/tests/presentation_events_tests.cpp` — client-visible locks over
  the shipped seam mapping (frame-by-frame transcript, 50 ms ticks, events
  exactly as remote_session.cpp derives them from
  `monster:telegraph` / `combat:hit`(incoming)):
  1. `ranged_hit_beat_is_telegraphed_and_attributed`: Telegraph op renders
     on the announcement frame labeled "thrust", stays up across its
     window, anchors ON THE SHOOTER (not the player); the hit lands only
     after it as Damage("player", amount) + TargetFlash + Impact ops
     anchored at the player; the readable log names both beats.
  2. `untelegraphed_hit_fails_the_readability_lock` (negative control): the
     identical run minus the announcement still resolves player damage but
     FAILS the lock predicate — proving the lock bites.

## Acceptance gates (literal commands, real outcomes)

1. `powershell -NoProfile -ExecutionPolicy Bypass -File native/build.ps1 -RunTests -RunClientScenarios -CaptureRoot orchestration/tasks/TASK-0108-combat-depth-wave/captures/review`
   → EXIT 1 (twice, reproducibly). Build OK; denylist PASS; core PASS;
   networking PASS; camera2d PASS; **session FAILS exactly one check**:
   `FAIL gate-b: slain elite surfaces the circulating heirloom`
   (build.ps1 throws here, so presentation/audio/scenarios did not run in
   this invocation — all three proven green directly below). All other
   session checks green (journeys, render-list, reconnect legs).
2. `./native/build/verdigris_core_tests.exe` → EXIT 0,
   "verdigris core tests: PASS" (incl. all four new locks).
3. `./native/build/verdigris_presentation_events_tests.exe` → EXIT 0,
   "presentation events tests: PASS" (35 PASS lines incl. both new locks
   and the bite-check negative control).
   `./native/build/verdigris_audio_mixer_tests.exe` → EXIT 0,
   "all audio mixer checks passed".
4. Client scenarios (same invocation build.ps1 performs):
   `verdigris_client.exe --scenario all` with VERDIGRIS_CAPTURE_ROOT set to
   the task captures/review dir → EXIT 0, 12 scenarios "PASS (0 failures)",
   captures isolated under
   `orchestration/tasks/TASK-0108-combat-depth-wave/captures/review/`.
5. `git diff --check` → clean (exit 0).
6. `git diff --name-only` → exactly the five owned code paths above plus
   this task folder. No forbidden path touched.

## Negative controls required by the SPEC

- Melee streams unchanged: core suite green includes every N2/N3 world test
  and the new silent-melee control; the only melee-branch edits are literal
  → named-constant substitutions of identical values (1500 ms cadence, 4-tile
  disengage bound).
- Untelegraphed ranged resolution fails the new lock: proven by the
  presentation negative-control test above.

## Root cause evidence (A/B)

- BASE (`git stash` of all five files, rebuild, `-RunTests`): EXIT 0, all
  suites green; gate-b hunt found and slew the Warden in ~45 s
  (heartbeats show kills climbing to #11 "Warden of the Deep",
  `PASS gate-b: slain elite surfaces the circulating heirloom`).
- MINE (stash popped, rebuild, full gate twice): both runs fail the same
  single check. Hunt log: first `monster:telegraph` from lurker
  monster-1-9 (ranged, tile (7,15)) sets `elite=(7,15)` while `hp=-1`
  (no further hp snapshots processed), heartbeats freeze at (6,16)/(10,13)
  chasing that tile for minutes, kills stall at #2/#7, budget expires.
- In-process `ProtocolSession` probe (guest `ox-pc-r-gateb`, dungeon/warren,
  walk north, park, swing): TELEGRAPH from monster-1-9 at (7,15) emitted on
  the wire, swings land 7 dmg, shooter dies in ~0.8 s, no crash, no
  silence → the server pipeline and my events are healthy; the failure is
  exclusively the frozen driver's slam-conflating interpretation.

## Requested decision

Pick (a) or (b) above and grant the corresponding path/coordination; this
lane resumes and finishes REVIEW_REQUESTED immediately after. The work as
committed passes every gate that does not require touching those two files.

## Resource capsule

Loopback 7280-7299 respected; nothing binds ports in this task's additions
(tests drive the seam in-process). Port 6500 untouched throughout.
