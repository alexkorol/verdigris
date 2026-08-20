# Verdigris native workspace

This is a small dependency-free C++20 proof of the native architecture. The
browser game remains separate in src/ and server/.

## Boundaries

- include/verdigris/core.hpp and src/core.cpp are the fixed-step, deterministic,
  headless simulation.
- include/verdigris/seasonal.hpp and src/seasonal.cpp are the external
  seasonal-mechanic extension point.
- client/main.cpp is a thin presentation shell. On Windows it opens a Win32
  window with placeholder shapes and the requested controls. On other hosts it
  runs a deterministic console demonstration until a focused SDL3/winit-style
  platform decision is made.
- tests/core_tests.cpp proves determinism, actor symmetry, extraction/death risk,
  House persistence, Scion reset, item identity, campaign ownership, optional
  branches, seasonal extension, and shared elite math.
- tools/check_legacy_denylist.py rejects denied legacy identifiers in new native
  production sources.
- platform/, renderer/, networking/, persistence/, and content/ document the
  explicit seams reserved for later native subsystems. They are intentionally
  not coupled into the first core slice.

## Build on Windows (the current checkout)

The repository includes a direct MSVC helper that finds the installed Visual
Studio Build Tools:

~~~powershell
./native/build.ps1
./native/build.ps1 -RunTests
./native/build.ps1 -RunClient
~~~

The client window is interactive: WASD moves, mouse aiming is represented by
the cursor, left mouse attacks, right mouse/Space dashes, P picks up, E equips,
and X extracts.

## Owner play (one command)

`native/tools/play-native.ps1` builds if the exes are missing or stale, starts
`verdigris_server` on a free **6520–6539** port (never 6500), launches
`verdigris_client --remote` against it, tees server output under
`native/build/logs/`, and stops the server when the client exits.

~~~powershell
powershell -NoProfile -File native/tools/play-native.ps1
powershell -NoProfile -File native/tools/play-native.ps1 -Local
powershell -NoProfile -File native/tools/play-native.ps1 -Port 6521 -Rebuild
~~~

Windows desktop shortcut Target (one line):

~~~text
powershell.exe -NoProfile -ExecutionPolicy Bypass -File <clone>\native\tools\play-native.ps1
~~~

Replace the path with this clone. Start in the repository root. Esc or closing
the window quits; the script prints a no-orphan process check and the server log
path.

## Build on macOS/Linux

With CMake and a C++20 compiler:

~~~bash
cmake -S native -B native/build
cmake --build native/build
ctest --test-dir native/build --output-on-failure
./native/build/verdigris_client
~~~

The console fallback is deliberately small; the renderer/platform seam is the
next experiment, not a general-purpose engine.

## First playable proof

~~~text
create House
→ create Scion
→ enter route
→ move and melee the enemy
→ pick up the generated item and trophy
→ equip (client: E)
→ return to the extraction point
→ extract durable House value
~~~

## Scenario harness (D-119)

The client has an automated, headless scenario runner that drives the real
input→simulation→presentation pipeline and asserts on three layers:
authoritative core state, the recorded render list (`render_list.hpp`), and
pane/HUD state.

~~~powershell
./native/build.ps1 -RunClientScenarios   # build + run all scenarios
native/build/verdigris_client.exe --scenario first-fight   # one scenario
~~~

Every future client wave must add its own scenario. To add one:

1. In `native/client/main.cpp`, write `int scenario_<name>()` that:
   - calls `scenario_begin(state)` (enters the seeded route and builds
     scenery), then drives commands with `scenario_step(state, Command::…)`
     (dispatch → ingest events → age effects → follow camera → present);
   - asserts with `scenario_check(condition, "label")` against
     `state.simulation->…` (core), `state.render_list` (`render::any` /
     `render::first` / `render::count`), and `state.render_list` Pane*/Hud
     ops (pane/HUD).
2. Register it in `run_scenarios`'s `entries` table.
3. Add a `render::Op` in `native/client/render_list.hpp` if the wave
   introduces a new draw class, and record it in the matching draw function
   in `main.cpp` (recording must live next to the draw so a suppressed draw
   is caught by the scenario).

The runner exits non-zero on any `scenario_check` failure.

