# Verdigris native workspace

The native C++20 game uses the Fable perspective renderer and directional
animated Scions. The historical browser reference remains in src/ and server/.

## Consolidated application

The packaged `Verdigris.exe` opens one native title screen. Continue admits the
saved living Scion; House & Scion opens clickable character cards, appearance
selection, editable names, and a House selector when multiple Houses exist.
Escape opens the session menu; Return to title and Settings stay in the native
application. Settings survive a fresh process.

`verdigris_client.exe --build-info` prints the embedded source commit and clean
state. `--scenario consolidated-flow` verifies the real Win32 menu handlers
against a native socket server, including returning-player Continue. Package
creation requires a clean commit and rebuilds it; package validation checks
the embedded identity against the manifest. Never replace a normal launch
entry until that exact package has passed native and live visual acceptance.

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

For native-only acceptance, use the single gate below. It builds the Windows
client/server, runs the native unit/session/presentation/audio/settings tests,
checks the legacy denylist, and runs the real client `--scenario all` suite.
The scenario capture root is contained under `native/build/` by default.

~~~powershell
powershell -NoProfile -ExecutionPolicy Bypass -File native/tools/verify-native.ps1
# equivalent npm entry point:
npm run verify:native
~~~

The root `npm run verify` points at this native-only gate. The former
browser/reference chain is available only as the explicit `npm run
verify:legacy` workflow.

`npm run playtest` is intentionally not part of this gate. That command starts
the historical JavaScript/browser protocol harness; use it only for browser or
explicit parity work.

The client window is interactive: WASD moves, the mouse aims, left mouse
attacks, right mouse/Space dashes (the answer to an enemy telegraph), Q/E/R
cast Thrust/Sweep/WarCry, X takes the nearest/underfoot drop, Z toggles loot
name labels, I opens the gear pane (Enter or click equips; 1-9 equips from
the backpack), N enters the charted road from the remote flow, the wheel
zooms and Home resets zoom, and F3 toggles the debug overlay. The objective
strip is mode-aware about extraction: in local play F extracts at the EXIT;
on the remote owner path you extract by walking onto the EXIT stairs. A
compact controls line is always on the HUD (no F3 needed). Esc closes an
open pane first; otherwise it opens the session menu. Quit requires a second
explicit choice. The title screen includes Play, Settings, and Quit.

On the normal remote path, a starting Scion walks four tiles per second.
Short melee requires close contact in the aimed direction, with a 100 ms
initial wind-up; leaving reach cancels contact. Character level and XP persist
per Scion, and the HUD uses the level supplied by the server.

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

Replace the path with this clone. Start in the repository root. Choose Quit
from the session menu or close the window; the script prints a no-orphan process
check and the server log path.

## Local Windows review package

Create a new, self-contained folder (the destination must not already exist):

~~~powershell
powershell -NoProfile -File native/tools/package-native.ps1 -OutputDirectory 'C:\Reviews\Verdigris'
powershell -NoProfile -File native/tools/test-player-package.ps1 -PackageDirectory 'C:\Reviews\Verdigris'
~~~

Double-click `Verdigris.exe` at the package root. This Windows GUI launcher
opens the normal title/Chronicles path with its own loopback server and no
developer console. It sets the working directory itself, carries all native
art and shared fonts/UI plates, and stops its child processes when the game
exits. A Windows job also cleans up children if the launcher is terminated.
No checkout, Python, Node, or compiler is required to play the package.

Review saves live in `profile/saves` inside the extracted writable folder.
Settings normally use `%LOCALAPPDATA%\Verdigris\settings.ini`. For QA, launch
`Verdigris.exe --profile 'C:\Reviews\isolated-profile'` from PowerShell to
isolate both saves and settings. The optional `--quick` flag enters a review
guest directly. One launcher can own a profile at a time; session logs are
retained in its `logs` directory. No existing owner saves are imported.

The package includes a SHA-256 file manifest, source commit/tree, and fresh
build timestamps. Packaging requires a clean committed worktree and rebuilds
the native binaries; `-SkipBuild` is rejected to prevent stale source labels. The
validation command checks those hashes and exercises the actual launcher
profile lock, including simultaneous trailing-separator aliases and reopening
after close, and requires the raster catalog's complete asset inventory plus
the launcher, UI plates, fonts, and splash. This is an unsigned Windows x64 local review artifact, with Windows
.NET Framework 4.x required for the launcher. It is not an installer or public
release; signing, public distribution, and clean-machine certification remain
separate work.

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
native/build/verdigris_client.exe --scenario raster-world   # runtime sprites + SE walking-frame paint
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
