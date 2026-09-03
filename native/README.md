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

The client window is interactive: WASD moves, the mouse aims, left mouse
attacks, right mouse/Space dashes (the answer to an enemy telegraph), Q/E/R
cast Thrust/Sweep/WarCry, X takes the nearest/underfoot drop, Z toggles loot
name labels, I opens the gear pane (Enter or click equips; 1-9 equips from
the backpack; selected Vesselforge gear shows its active properties and worn
totals across two fitted Framekit rows), J opens the authoritative Chronicle campaign (including its current
act and twenty-three-point total), N enters the charted road from the remote flow, the wheel
zooms and Home resets zoom, and F3 toggles the debug overlay. The objective
strip is mode-aware about extraction: in local play F extracts at the EXIT;
on the remote owner path you extract by walking onto the EXIT stairs. A
compact controls line is always on the HUD (no F3 needed). Esc closes an
open gear pane first; a bare Esc requests quit.

In the Crossroads, `T` hails the nearest townsfolk. Tamar the Vesselwright
opens a dedicated Framekit forge: Up/Down selects carried vessel gear and
Enter or a click sears an eligible Brand for 100 gold. Capacity, patience,
existing lines, purse, and disabled reasons come from the server; the service
cannot be invoked away from Tamar. Vessel gear worn through a cleared floor
earns 16–30 Attunement and remembers that road's themes. At 80 Attunement,
then 55 more per evolution, it forms and deepens Bonds toward tier III and can
eventually awaken into its Scion's name. Tamar's detail pane shows the exact
progress and evolution count. Conditional Bond and awakened powers are marked
Dormant until their combat triggers are implemented.

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
→ equip (gear pane: Enter, or 1-9)
→ return to the extraction point
→ extract durable House value (local: F at the EXIT; remote: walk onto it)
~~~

## Scenario harness (D-119)

The client has an automated, headless scenario runner that drives the real
input→simulation→presentation pipeline and asserts on three layers:
authoritative core state, the recorded render list (`render_list.hpp`), and
pane/HUD state.

~~~powershell
./native/build.ps1 -RunClientScenarios   # build + run all scenarios
native/build/verdigris_client.exe --scenario first-fight   # one scenario
native/build/verdigris_client.exe --scenario first-session-clarity   # TASK-0153 contracts
native/build/verdigris_client.exe --scenario deep-roads-campaign     # Act III Framekit evidence
native/build/verdigris_client.exe --scenario crownless-campaign      # Act IV / 23-point Framekit evidence
native/build/verdigris_client.exe --scenario verdigris-crown-campaign # Act VI finale at 960x600 + 1366x768
native/build/verdigris_client.exe --scenario vesselforge-final-implicits # range/speed/pierce evidence
native/build/verdigris_client.exe --scenario town-vesselforge          # 960x600 + 1366x768 service evidence
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
