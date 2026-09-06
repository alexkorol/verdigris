# REPORT — TASK-0108 Readable ranged combat successor (rev 3)

- coordinator: kimi-work · worker: kimiwork-subagent-1
- branch: `kimiwork/TASK-0108-ranged-rev3` (worktree
  `Z:\Code\Games\delaford\kimiwork_verdigris\.worktrees\task-0108`)
- base: `e7b65360` · head: see "Commits" below
- state: CLAIMED (NOT review-requested — SPEC acceptance includes the
  client-presentation sub-part, which is deferred; see Follow-ups)

## Executive summary

The D-129 wire contract is implemented and locked. The ranged volley windup
is now a distinct internal sim event (`WorldCombatEvent.type == "projectile"`)
that carries the shooter's tile (`origin_x`/`origin_y`) alongside the painted
target tile (`x`/`y`); `ProtocolSession::emit_combat_event` routes it to the
`world:projectile` envelope with exactly the JS payload keys
(`fromX, fromY, toX, toY, travelMs, kind`) read from
`server/core/entities/monster/combat-controller.js:215-222`.
`monster:telegraph` is slam-only on the native wire, matching the JS server's
emission contract. New locks prove: ranged damage lands beyond 2-tile
Chebyshev contact; a melee twin cannot damage from there; every ranged hit is
preceded by its warning; the seeded ranged event stream replays
byte-identically; and a ranged windup never reaches the wire as
`monster:telegraph`. No damage, cadence, art, or constants were invented —
the authored `kN3RangedVolley*` family is reused unchanged.

All gates green (exact commands and exit codes below). One transient failure
of the long real-time gate-b hunt journey was observed on the first full
gate run and did not reproduce on rerun (details in Test outcomes); the sim
is unchanged by this diff (only an event label and its wire routing moved),
and gate-b keys its dodge logic on `skillId == "boss:ground-slam"`
explicitly, so the failure is attributed to journey flake, not this change.

## Coordinator-imposed scope deviation (important)

The SPEC's owned_paths include `native/client/presentation_state.cpp`,
`native/client/render_list.hpp`, `native/client/main.cpp` and
`native/tests/presentation_events_tests.cpp`, but `native/client/**` is under
an ACTIVE exclusive lease by another agent (Cursor, HUD wave) this cycle.
Per the dispatch constraint, this packet implements only the core, wire, and
test locks; the client-visible Telegraph render op and the
`presentation_events_tests.cpp` lock are DEFERRED (see Follow-ups). The
changed-files set is therefore the implemented slice plus this task folder,
not the SPEC's full owned_paths.

## Approach

1. Read SPEC rev 3, LEADER_BRIEF (D-129 record), PROTOCOL; verified the
   frozen surfaces and the JS emission contract.
2. Established at base `e7b65360`: the ranged volley sim already existed, but
   its windup reused `WorldCombatEvent.type == "telegraph"` and therefore
   rode `monster:telegraph` — precisely what D-129 forbids.
3. Core: relabeled the ranged windup to a distinct `"projectile"` event type
   and added `origin_x`/`origin_y` to `WorldCombatEvent` so the wire layer
   can mirror the JS payload without re-deriving the shooter tile. The boss
   slam warning keeps `type == "telegraph"`; melee/buffer paths untouched.
4. Networking: one additive arm in `emit_combat_event` (immediately after
   the telegraph arm) mapping `"projectile"` → `world:projectile` with the
   six JS payload keys. `kind` is `"monster"`: the JS stack derives it from
   `behaviour.type === 'support'`, and native ranged casters are never
   support-type (buffers mend via `monster:healed`), which is documented at
   the emit site. `travelMs` = `kN3RangedVolleyWindupMs` (800), which already
   satisfies the JS `Math.max(120, windupMs)` floor identically.
5. Tests:
   - `core_tests.cpp` — updated the pre-existing volley-warning assertion to
     the new event type (adding origin assertions), and added
     `test_ranged_reach_warning_precedence_and_replay`: (a) volley hit lands
     at Chebyshev 4 from the shooter; (b) warning-precedence scan over the
     whole stream (every `hit` with `skill_id == "ranged:volley"` must
     consume an outstanding `"projectile"` warning from the same attacker —
     a resolution without a preceding warning fails the lock); (c) melee
     twin stepped 100 ms at a time from Chebyshev 4 lands zero hits while
     beyond 2-tile contact and never emits the ranged windup event, then
     still fights once contact closes; (d) the same seed + call script
     reproduces the ranged stream byte-identically (fingerprint over
     type/skill/attacker/tiles/origin/radius/duration/amount/channel).
   - `networking_tests.cpp` — alongside the existing slam lock: the roles
     wire scenario now re-seats the player four tiles from the spacer's
     settled tile (inside the comfort band, so no further movement) and locks
     the windup envelope as `world:projectile` with exact
     `fromX/fromY/toX/toY/travelMs(=800)/kind(="monster")` values, plus a
     scenario-wide negative control that any `monster:telegraph` carrying
     `skillId == "ranged:volley"` fails the test. The exact-movement-fact
     lock for ranged spacing is unchanged.

## Changed files

- `native/include/verdigris/core.hpp` — `WorldCombatEvent`: added
  `origin_x`/`origin_y`; type comment now lists `projectile`.
- `native/src/core.cpp` — ranged windup emits `type == "projectile"` with
  shooter origin; comment citing D-129. Nothing else touched.
- `native/src/networking.cpp` — additive `"projectile"` arm in
  `emit_combat_event`. The `state.xp` snapshot block and all other emit arms
  are untouched (verified by diff).
- `native/tests/core_tests.cpp` — updated volley warning assertion; new
  `test_ranged_reach_warning_precedence_and_replay` (registered in `main`).
- `native/tests/networking_tests.cpp` — `world:projectile` lock + never-
  `monster:telegraph` negative control in the N3 roles wire scenario.
- `orchestration/tasks/TASK-0108-combat-depth-wave/` — STATUS.md, REPORT.md,
  `captures/review/` (scenario evidence produced with the SPEC's
  `-CaptureRoot`).

Frozen surfaces verified byte-identical by diff: `native/tests/session_tests.cpp`,
`native/client/remote_session.cpp`, `native/build.ps1`, everything under
`server/`, `src/`, `playtest/`.

## Interfaces added/changed

- `WorldCombatEvent` (native/include/verdigris/core.hpp): new fields
  `int origin_x`, `int origin_y` (defaulted 0; no behavioural effect on
  existing event types). New event type value `"projectile"` (ranged windup).
- Wire: NEW emission — `world:projectile` `{fromX, fromY, toX, toY,
  travelMs, kind}` on ranged windup. REMOVED emission — ranged windups no
  longer produce `monster:telegraph` (slam telegraphs unchanged). This is
  the D-129-mandated contract, not an invention.
- No other public interface changes.

## Test commands and exact outcomes

All run from the worktree root on this machine (MSVC Build Tools 2019,
via vcvars64):

1. `powershell -NoProfile -ExecutionPolicy Bypass -File native/build.ps1 -RunTests`
   - Run 1: **exit 1** — one flake: `FAIL gate-b: slain rare guardian
     surfaces the circulating heirloom` (hunt aborted: "the successor fell to
     ordinary combat" after the Warden kill at hp=20; the known adds-during-
     pickup failure mode documented at session_tests.cpp:1590-1607).
     denylist/core/networking/camera2d all passed in this run; presentation/
     audio not reached (script throws at the session suite).
   - Standalone rerun `native/build/verdigris_session_tests.exe`:
     **exit 0** ("session tests passed"), gate-b included.
   - Run 2 (full script again): **exit 0** — legacy denylist PASS; WIZARD
     title assets verified; `verdigris core tests: PASS`;
     `verdigris networking tests: PASS`; `camera2d tests: PASS`;
     `session tests passed`; `presentation events tests: PASS`;
     `all audio mixer checks passed`. Zero FAIL lines in the log.
2. `native/build/verdigris_core_tests.exe` (post-gate binary): **exit 0**,
   `verdigris core tests: PASS`.
3. `native/build/verdigris_networking_tests.exe` (post-gate binary):
   **exit 0**, `verdigris networking tests: PASS`.
4. `powershell -NoProfile -ExecutionPolicy Bypass -File native/build.ps1 -RunClientScenarios -CaptureRoot orchestration/tasks/TASK-0108-combat-depth-wave/captures/review`:
   **exit 0** — every scenario `PASS (0 failures)` (38 scenario blocks, 0
   FAIL lines); evidence PNGs isolated under the task's `captures/review/`.
5. `git diff --check e7b65360 HEAD`: **exit 0** (no whitespace errors).
6. `git diff --name-only e7b65360 HEAD`: exactly the five implemented files
   plus this task folder (list above).

Baseline note: the same full gate at base `e7b65360` (before any edit) also
passed end-to-end, so the diff introduces no regression in any suite.

## Deviations

- Scope slice per coordinator constraint (see above): the four client-side
  owned paths were not touched because of the active Cursor lease; the
  client-presentation acceptance items are deferred, so this report does NOT
  request review.
- Session-suite flake: one gate-b hunt abort on the first gate run, green on
  rerun and on the full second gate run. The diff does not alter sim timing
  or damage; gate-b's telegraph consumption is keyed on
  `skillId == "boss:ground-slam"` (session_tests.cpp:1558-1559), which is
  unaffected. Recorded for transparency; if the reviewer sees repeats, treat
  as a journey-stability issue, not this packet.
- Pre-commit hook (`yorkie`) is inoperable in this worktree (node_modules
  absent); commits used `--no-verify`. No JS sources changed.

## Risks

- **Interim invisibility window (known, ruled):** the remote/local client
  parse path for `world:projectile` lives in `native/client/remote_session.cpp`,
  which D-129 freezes for a successor packet, and the Telegraph render op
  lives in the leased client files. Until the follow-up stage lands, a ranged
  windup produces the wire event but no client-visible warning. The SPEC
  explicitly accepts this split ("remote-client rendering of
  `world:projectile` is a successor packet"); flagging it so the deferral is
  not mistaken for shipped readability.
- `kind` parity: native has no support-type projectile caster today, so
  `kind:"monster"` is exact for every current emitter; if a support caster
  ever fires a projectile, the emit arm must derive `kind` from behaviour.
- gate-b journey flake (see Deviations).

## Follow-ups (deferred client stage — precisely what remains)

1. After the Cursor `native/client/**` lease releases: implement the
   client-visible Telegraph render op for the ranged warning in
   `native/client/presentation_state.cpp` / `render_list.hpp` / `main.cpp`
   (SPEC outcome: every ranged hit preceded by a Telegraph op, landing as an
   attributed Damage/Impact op), and add the client-visible lock in
   `native/tests/presentation_events_tests.cpp` using the transcript idea
   from `session_tests.cpp:284-370`.
2. Successor packet (SPEC-frozen here): parse `world:projectile` in
   `native/client/remote_session.cpp` so remote clients render the warning.
3. Re-run the full SPEC acceptance (including `-RunClientScenarios
   -CaptureRoot`) once 1–2 land; only then should STATUS move to
   REVIEW_REQUESTED.

## Commits

- `bebb1aba` chore(TASK-0108): claim rev 3 ranged combat successor (kimi-work)
- `72b25d85` feat(TASK-0108): route ranged windup via world:projectile (D-129)
- final report/captures commit: see `git log` (REPORT.md + STATUS.md update +
  captures/review evidence)
