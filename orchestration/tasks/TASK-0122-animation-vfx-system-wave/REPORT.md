# TASK-0122 REPORT — Native animation/VFX Phase A (client-only readable event beats)

- **Worker:** ox-pc-x (OpenCode Ox Alpha), provider `openrouter`, model `stealth/ox-alpha`
- **Branch:** `codex/TASK-0122-animation-vfx-phase-a-ox-pc-x`
- **Worktree:** `Z:\Code\.worktrees\verdigris\ox-pc-x`
- **Base:** `3341a81feee84f7178742ac0752e5cf321817c3c` (spec parent `8eb95893` included)
- **Claim:** `7a5ef6dd` (original), corrected claim `fdbbdca6` (coordinator line + ISO-8601 PDT timestamp); remote-verified before implementation.

## 1. Executive summary

The frozen Phase A packet is implemented entirely inside the SPEC owned paths.
Combat and lifecycle events now produce distinct, deterministic, named-constant
presentation beats: critical-hit treatment (consuming the already-shipped
`combat:hit` `critical`/`attackStyle` data), ordinary-hit treatment (unchanged
accepted look, now explicitly distinguished), a deterministic monster
spawn/materialization beat, a `BuffExpired(war-cry)` fade beat, and a
`ScionLost` loss beat. The proved client-only monster-facing inversion is
removed. All new TTL/pulse/color/style values live in one named table
(`verdigris::client::phase_a` in `presentation_events.hpp`). A dedicated
`verdigris_presentation_events_tests` binary (24 checks) plus a new
`animation-vfx-phase-a` client scenario prove event timing and produced the two
task captures.

## 2. Approach

- **Seam-first:** new beats are `PresentationEventType`s (`ScionLost`,
  `BuffExpired`) and `EffectFx` kinds (`Materialize`, `WarCryFade`,
  `ScionLostBeat`) carried through `presentation_state` so both the local
  direct path and the remote session path render identically.
- **Local seam mapping:** `local_session.cpp` now maps core `ScionLost` and
  `BuffExpired` through the seam (previously dropped at the `default: continue`)
  and no longer flattens every `UseAction` to Melee (named actions map to their
  core actions so lifecycle beats are reachable by seam consumers).
- **Critical/style consumption:** `remote_session.cpp` copies `critical` and
  `attackStyle` verbatim from the existing `combat:hit` envelope
  (server emits at `native/src/networking.cpp:2002-2005`) onto the outgoing
  `DamageApplied` presentation event. No envelope or server change.
- **Spawn beat:** `detect_monster_spawns` tracks first-sighting ids in
  `PresentationFx::known_monsters` (round-tripped through `ClientState`) and
  pushes one `Materialize` effect per never-seen foe, in snapshot order. It
  reads the authoritative snapshot only; it never creates, moves, or damages
  an actor.
- **Facing correction:** `sync_world_from_model` no longer derives monster
  facing by inverting player facing; without a wire facing field monsters keep
  the neutral default. The telegraph-facing inversion is removed for the same
  reason (radius/position wire work stays deferred per SPEC).
- **Rendering:** all new drawing lives in the existing `draw_effect` switch in
  `native/client/main.cpp` using existing `render::Op` values with distinct
  labels (`spawn`, `warcry-fade`, `scion-lost`, `critical:<style>`);
  `render_list.hpp` was not touched (outside owned paths).
- **Constants:** every new value is in `phase_a` (tick size, crit number/flash
  TTLs, crit/flash/materialize/fade/loss colors, render label strings, loss
  pulse). No scattered magic timing literals were introduced.

## 3. Changed-path inventory (claim `fdbbdca6` → head)

- `native/client/presentation_events.hpp` (event types, crit/style fields, `phase_a` table)
- `native/client/presentation_state.hpp` (EffectFx kinds/fields, known_monsters, spawn API)
- `native/client/presentation_state.cpp` (facing fix, crit mapping, new beats, spawn detector, record_world_ops labels)
- `native/client/local_session.cpp` (ScionLost/BuffExpired seam mapping, named UseAction)
- `native/client/remote_session.cpp` (combat:hit critical/attackStyle consumption)
- `native/client/main.cpp` (beat drawing, legend, spawn wiring in both ingest paths, scenario `animation-vfx-phase-a`, capture dir helper)
- `native/tests/presentation_events_tests.cpp` (new, 24 checks)
- `native/build.ps1`, `native/CMakeLists.txt` (new test binary)
- `orchestration/tasks/TASK-0122-animation-vfx-system-wave/captures/*` (evidence)

`native/client/presentation_events.cpp` is listed in the SPEC but did not exist
at the frozen base; the events surface is header-only and the seam logic lives
in `presentation_state.cpp`. No forbidden path (`native/src/**`,
`native/include/**`, `native/tests/session_tests.cpp`, TASK-0148, server,
browser, assets) was touched; `git diff --name-only` at head proves the
inventory above.

## 4. Public interfaces added/changed

- `PresentationEventType::ScionLost`, `PresentationEventType::BuffExpired`
- `PresentationEvent::critical`, `PresentationEvent::style`
- `EffectFx::Kind::{Materialize, WarCryFade, ScionLostBeat}`, `EffectFx::critical/style`
- `PresentationFx::known_monsters`; `detect_monster_spawns(fx, world, tick)`
- `verdigris::client::phase_a` constants table
- `LocalCoreSession` `UseAction` now honors named actions ("war-cry", "thrust",
  "sweep", "wait", "dash"; default melee unchanged)
- Test target `verdigris_presentation_events_tests` (build.ps1 + CMake + ctest)

## 5. Acceptance gates (literal, from the unchanged final tree)

| Command | Exit | Evidence |
|---|---|---|
| `powershell -NoProfile -ExecutionPolicy Bypass -File native/build.ps1 -RunTests -RunClientScenarios` | **0** | core/networking/camera2d/session/presentation-events tests all PASS; all 10 client scenarios `PASS (0 failures)` |
| `native/build/verdigris_presentation_events_tests.exe` | **0** | 24/24 `PASS`, `presentation events tests: PASS` |
| `native/build/verdigris_client.exe --scenario animation-vfx-phase-a` | **0** | 31/31 checks `ok`, `PASS (0 failures)` |
| `npm run playtest` | **0** (after one retry) | first run 31/32 (`mortality` FAIL, 10732 ms under post-`npm ci` load); `mortality` passes standalone on the clean base and the immediate full re-run with this tree passed **32/32** — load flake, no browser/server file is in this diff |
| `git diff --check` | **0** | clean |

Note: `npm ci` was required in this fresh worktree (node_modules absent); the
first full playtest run immediately after install hit the timing-sensitive
`mortality` scenario; the literal re-run passed 32/32. The earlier
session-tests journey pickup/extract failures seen mid-session were caused by
capsule contention with TASK-0148 (per supervisor) and did not reproduce in the
exclusive final run.

## 6. Deterministic timing assertions (scenario `animation-vfx-phase-a`)

- Materialization: exactly one beat per first-sighting foe; expires exactly at
  `phase_a::kMaterializeTtlTicks`; never re-triggers while the foe persists.
- Critical vs ordinary: crit number ttl `kCriticalNumberTtlTicks` (16) vs
  ordinary 12; crit flash `kCriticalFlashTtlTicks` (6) vs 4; render-list label
  `critical:<style>`.
- War-cry fade: appears within `[kWarCryDurationTicks-1, +2]` pipeline steps of
  the authoritative window (the core decrements the fresh buff on the cast
  tick's advance pass), ttl exactly `kWarcryFadeTtlTicks`, clears deterministically.
- ScionLost: beat appears with an authoritative death; pulse
  `kScionLostPulseTicks`; ring ttl exactly `kScionLostRingTtlTicks`.
- Negative controls: applying every drained seam event plus spawn detection
  leaves simulation tick, actor set, scion life, and House store byte-identical
  (scenario checks + `seam_events_cannot_mutate_simulation` in the test binary).

## 7. Captures (fresh, this tree)

- `orchestration/tasks/TASK-0122-animation-vfx-system-wave/captures/animation-vfx-phase-a-960x600.png` — 960x600, 635477 bytes
- `orchestration/tasks/TASK-0122-animation-vfx-system-wave/captures/animation-vfx-phase-a-1366x768.png` — 1366x768, 1146405 bytes

Composition (architect-reviewed through four visual-gate iterations): the
route is cleared and the player relocated through the real pipeline so no
stray combat noise is in frame. The five treatments are spatially separated
and carry in-world legend chips drawn by the renderer itself (recorded as
`Hud` ops, `beat:` labels): `CRITICAL 27` (orange numeral + white-hot burst,
NW), `ordinary hit` (struck foe N), `buff end` (real timed `BuffExpired`
fade at the player), `spawn beat` (teal materialization rings, SE), and
`scion lost` (complete rust double rings, lower-left safe area placed by
unprojecting an explicit 960x600 screen point). Player, EXIT pad, objective
strip, identity chip, quickbar, and orbs are unobscured at both resolutions.

Disclosure: the critical and ScionLost treatments are injected into the frame
via the presentation seam (`EffectFx` pushes using the exact kinds/ttls the
seam produces) because their real triggers are remote-only (`combat:hit`
critical data) or session-ending (permanent loss). Their timing/contract
proofs are real: the pipeline assertions above, the unit tests, and the real
war-cry fade in the same frame.

## 8. Deviations / notes

- `presentation_events.cpp` (SPEC-owned) did not exist at base; nothing was
  created under that name because the events surface is header-only. No
  required client field was absent: `critical`/`attackStyle` ship today, so no
  stop condition triggered.
- The `mortality` playtest flake and the TASK-0148 capsule contention are
  environmental; neither is caused by this diff (base-verified).
- Test-side effects of running gates (playtest journal/TASK-0145 capture
  regeneration) were restored; the committed diff contains only owned paths.

## 9. Risks / follow-ups

- Monster facing is now neutral default in remote mode until the wire ships an
  authoritative facing (deferred wire work).
- `main.cpp` grows further; the renderer extraction remains deferred per SPEC.
- The `phase_a` table is the single tuning point for Phase B timing pins.

## 10. Commits

- `7a5ef6dd` claim; `fdbbdca6` claim correction (pre-implementation, remote-verified)
- Implementation + evidence commits on this worker branch (see git log); only
  this branch is pushed. No merge, no rebase, no force-push.
