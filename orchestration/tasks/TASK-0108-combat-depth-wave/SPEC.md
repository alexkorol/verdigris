---
task: TASK-0108
title: Readable ranged combat successor
state: READY
packet: IMPLEMENTATION
topology: INDEPENDENT
job: IMPLEMENTATION
priority: P0
base_commit: 763684666b07483caeeebc2055c804f80bb1515e
owner_visible_contribution: expedition packs visibly mix contact and pressure roles, and every ranged hit is preceded by a readable warning with a client-visible attributed hit beat
dependencies: [TASK-0101 ACCEPTED, TASK-0161 ACCEPTED]
owner_input_dependency: range band and damage values may reuse authored constants; any retune is owner-only
owned_paths: [native/src/core.cpp, native/include/verdigris/core.hpp, native/tests/core_tests.cpp, native/client/presentation_state.cpp, native/client/render_list.hpp, native/client/main.cpp, native/tests/presentation_events_tests.cpp, orchestration/tasks/TASK-0108-combat-depth-wave/**]
forbidden_paths: [native/tests/session_tests.cpp, native/client/remote_session.cpp, native/build.ps1, server/**, src/**, playtest/**, CI, assets, projectile art, cadence invention, balance tables, naming, lore, everything else]
resource_capsule: loopback ports 7280-7299; never touch port 6500
ready_promoted_at: 2026-08-22T17:22:00-07:00
promotion_provenance: Cursor successor after TASK-0101 ACCEPTED/INTEGRATED and TASK-0161 ACCEPTED/INTEGRATED; W1/GAP-RANGED-BEHAVIOUR selected from FINDINGS.md; session_tests.cpp left to TASK-0162
---

# Outcome

Realize the already-broadcast tile-space `behaviour_type == "ranged"` contract
so a ranged monster can damage the player from beyond 2-tile Chebyshev contact
while a melee twin cannot. Every resolved ranged hit in that seeded run must be
preceded by an emitted telegraph that becomes a `Telegraph` render op, and the
hit must land as a `Damage`/`Impact` op attributed to the ranged attacker.
Reuse shipped telegraph/damage vocabulary only. Keep `buffer` inert unless a
later packet owns it.

Do not invent projectile art, new render ops, cadence, or damage values. Reuse
authored constants (`kN3MonsterDamage` family) until the owner retunes.

# Frozen invariants

Actor symmetry, one damage pipeline, D-114 coherence, and D-115 play gate stay
frozen. Invisible ranged damage must not ship. `native/tests/session_tests.cpp`
is occupied by READY TASK-0162; put the client-visible lock in
`presentation_events_tests.cpp` (and core determinism in `core_tests.cpp`)
using the same transcript idea as `session_tests.cpp:284-370`.

# Acceptance

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File native/build.ps1 -RunTests -RunClientScenarios -CaptureRoot orchestration/tasks/TASK-0108-combat-depth-wave/captures/review
native/build/verdigris_core_tests.exe
native/build/verdigris_session_tests.exe
git diff --check
git diff --name-only
```

Expected: denylist/core/networking/camera2d/session/presentation-events/audio
green; all current client scenarios green; new locking tests prove (1) ranged
damage beyond contact, (2) melee twin does not, (3) replay-identical ranged
event stream, (4) every ranged hit is preceded by a Telegraph op and lands as
attributed Damage/Impact. Changed files are exactly the owned paths plus this
task folder. Use `-CaptureRoot` so historical captures stay untouched.

# Negative controls and STOP conditions

- `behaviour_type == "melee"` event streams remain byte-identical on existing
  N2/N3 suites.
- A ranged resolution path that emits `combat:hit` / damage without a preceding
  telegraph must fail the new lock.
- No `dev:*`, no direct state mutation, no projectile sprites, no numeric
  retune, no edits to `session_tests.cpp`.
- STOP and file a question if readability cannot be proven without owning
  `session_tests.cpp` or inventing new art.
