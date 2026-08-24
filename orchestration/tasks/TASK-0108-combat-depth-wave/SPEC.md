---
task: TASK-0108
title: Readable ranged combat successor
state: READY
revision: 3
revision_provenance: D-129 owner ruling 2026-08-24 (projectile wire convention); supersedes rev 1/2 wire approach and both prior heads (b73386c4 ox-pc-ba, cb5f0bc5 ox-sw-a)
packet: IMPLEMENTATION
topology: INDEPENDENT
job: IMPLEMENTATION
priority: P0
base_commit: 28ef2819bdf74356a72572cfea4cc216aa93cdce
owner_visible_contribution: expedition packs visibly mix contact and pressure roles, and every ranged hit is preceded by a readable warning with a client-visible attributed hit beat
dependencies: [TASK-0101 ACCEPTED, TASK-0161 ACCEPTED]
owner_input_dependency: range band and damage values may reuse authored constants; any retune is owner-only
owned_paths: [native/src/core.cpp, native/include/verdigris/core.hpp, native/src/networking.cpp, native/tests/core_tests.cpp, native/tests/networking_tests.cpp, native/client/presentation_state.cpp, native/client/render_list.hpp, native/client/main.cpp, native/tests/presentation_events_tests.cpp, orchestration/tasks/TASK-0108-combat-depth-wave/**]
forbidden_paths: [native/tests/session_tests.cpp, native/client/remote_session.cpp, native/build.ps1, server/**, src/**, playtest/**, CI, assets, projectile art, cadence invention, balance tables, naming, lore, everything else]
resource_capsule: loopback ports 7280-7299; never touch port 6500
ready_promoted_at: 2026-08-24T08:05:00-07:00
promotion_provenance: architect rev 3 after D-129; rev 1 promoted 2026-08-22 by Cursor after TASK-0101/0161 ACCEPTED; W1/GAP-RANGED-BEHAVIOUR from FINDINGS.md
---

# Outcome

Realize the tile-space `behaviour_type == "ranged"` contract so a ranged
monster can damage the player from beyond 2-tile Chebyshev contact while a
melee twin cannot. Every resolved ranged hit in that seeded run must be
preceded by an emitted warning that becomes a `Telegraph` render op locally,
and the hit must land as a `Damage`/`Impact` op attributed to the ranged
attacker.

**Wire contract (D-129, binding):** ranged windups do NOT ride
`monster:telegraph` and do NOT get a new telegraph-family envelope.
`monster:telegraph` stays slam-only. The ranged warning crosses the wire as
`world:projectile`, matching the JS stack's convention — read
`server/core/entities/monster/combat-controller.js` (ranged branch, ~:212-215)
and mirror its payload keys exactly for stack parity. Internal simulation
vocabulary (`WorldCombatEvent`) may keep a distinct event type for ranged
windup; the emit path in `native/src/networking.cpp` must route it to the
`world:projectile` envelope, never to `monster:telegraph`.

Do not invent projectile art, new render ops, cadence, or damage values. Reuse
authored constants (`kN3MonsterDamage` family) until the owner retunes.

Prior heads b73386c4 (telegraph reuse) and cb5f0bc5 (`monster:ranged-telegraph`
envelope) are superseded on the wire question by D-129; their combat/gameplay
logic may be salvaged where it stands review on its own merits.

# Frozen invariants

Actor symmetry, one damage pipeline, D-114 coherence, and D-115 play gate stay
frozen. Invisible ranged damage must not ship. `native/tests/session_tests.cpp`
stays byte-identical (D-129 explicitly preserves the gate-b journey at
:1392-1402 — with `monster:telegraph` slam-only, its every-telegraph-is-a-slam
assumption remains true). Put the client-visible lock in
`presentation_events_tests.cpp` (and core determinism in `core_tests.cpp`)
using the same transcript idea as `session_tests.cpp:284-370`. Lock the wire
envelope in `networking_tests.cpp` alongside the existing slam lock (:206):
ranged windup emits `world:projectile` with the JS payload shape, and never
`monster:telegraph`.

`native/client/remote_session.cpp` remains forbidden: remote-client rendering
of `world:projectile` is a successor packet, not this one. The local
presentation path (owned) carries owner-visible readability for this packet.

# Acceptance

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File native/build.ps1 -RunTests -RunClientScenarios -CaptureRoot orchestration/tasks/TASK-0108-combat-depth-wave/captures/review
native/build/verdigris_core_tests.exe
native/build/verdigris_session_tests.exe
native/build/verdigris_networking_tests.exe
git diff --check
git diff --name-only
```

Expected: denylist/core/networking/camera2d/session/presentation-events/audio
green; all current client scenarios green; new locking tests prove (1) ranged
damage beyond contact, (2) melee twin does not, (3) replay-identical ranged
event stream, (4) every ranged hit is preceded by a Telegraph op and lands as
attributed Damage/Impact, (5) the wire envelope for ranged windup is
`world:projectile` with JS-parity payload and `monster:telegraph` never fires
for a ranged windup. Changed files are exactly the owned paths plus this task
folder. Use `-CaptureRoot` so historical captures stay untouched.

# Negative controls and STOP conditions

- `behaviour_type == "melee"` event streams remain byte-identical on existing
  N2/N3 suites; `session_tests.cpp` byte-identical, full stop.
- A ranged resolution path that emits `combat:hit` / damage without a preceding
  warning must fail the new lock.
- A ranged windup that reaches the wire as `monster:telegraph` (any payload)
  must fail the networking lock.
- No `dev:*`, no direct state mutation, no projectile sprites, no numeric
  retune.
- STOP and file a QUESTION if JS payload parity cannot be established from
  `combat-controller.js` alone, or if readability cannot be proven inside the
  owned paths.
