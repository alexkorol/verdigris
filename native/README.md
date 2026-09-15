# Verdigris native workspace

The native C++20 game uses the Fable perspective renderer and directional
animated Scions. The historical browser reference remains in src/ and server/.

## Private multiplayer service

The native player supports `Verdigris.exe --online "wss://<authorized-host>/game"`.
Online accounts, owned Houses/Scions, peer actors, party invitations/readiness,
shared encounters, private inventory, and House entitlements use an independent
transactional Windows service. Local double-click review remains available.
See [service operations](tools/SERVICE-OPERATIONS.md) for enrollment, the explicit
QA policy, secure gateway, backup/recovery, packaging, and verification commands.
Protocol 1 is a private QA contract; deployment and separate-computer acceptance
require an authorized host and endpoint.

## Player typography

All player text uses **Verdigris Sans**, a bundled CC0 derivative of m5x7.
[Source and license notes](client/assets/fonts/sans/README.md). The owner selected
this proportional sans face from actual native captures. The Nox reference's
exact font remains unidentified. The chosen 32px em produces 14px capitals and
visible 2px authored pixel steps without synthetic bold or blur.
`client/ui_typography.hpp` owns Body, Label, Heading, Compact, Title and CompactValue
roles. Ordinary roles use 32px, Title 64px and tiny inventory/orb values 16px.
Viewport tiers multiply these sizes by integers. Heading emphasis uses color
and space. Line metrics are aligned to the source font's pixel grid.

GDI rasterizes cached private fonts at integer positions with
`NONANTIALIASED_QUALITY`, in screen space after world rendering. Per-monitor
DPI awareness prevents Windows from interpolating a bitmap of the window.
Camera calibration is unchanged. Primary ink is `#cfb468`, secondary `#b5a277`;
meaningful state colors remain. NPC labels have one dark pixel of separation.
Measurement and drawing share UTF-8 decoding with a legacy Windows-1252
fallback. The 326-character map includes Latin-1, selected extended Latin,
punctuation and symbols; Cyrillic is not included. Unsupported characters
visibly become this font's `?`; ellipsis uses three periods. Missing smart
quotes and low quotes alias the family's straight quote glyphs.
The font resolves relative to the executable. Missing resources cause an
explicit error; there is no installed-font substitution.

Name input retains its 40 printable-ASCII limit, with measured click/caret
positioning, Left/Right/Home/End, Shift selection, Ctrl+A, Delete/Backspace,
replacement typing and horizontal scrolling. The existing bounded message log
wraps using actual metrics. No new chat or dialogue system was introduced;
Body is their shared role when implemented. `--scenario typography` checks
raster pixels, cached metrics, glyph policy and input, and captures production
paint at 960x600, 1280x800 and 3440x1440. Captured log text and name/roster
stress data are labeled fixtures, not proof of a new dialogue feature.

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
entry until that exact package has passed the relevant native checks and its
production captures have been inspected. Record owner acceptance separately.

Builds and regression passes do not depend on Computer Use. Run
`native/tools/verify-player-package.ps1 -PackageDirectory <package> -EvidenceDirectory <package>/qa/verification`
for exact-package scenarios and two same-profile settings/launcher lifecycles.
Then run `native/tools/test-normal-launch.ps1 -InstallationDirectory <installation> -EvidenceDirectory <evidence>`
against the installed normal entry. Its app-owned hidden `--verify-launch smoke`
mode renders the title, checks menu Quit and owned-process shutdown, records
actual child images and working directory, and compares existing saves and
normal settings. It never enters gameplay or changes settings. No desktop
input is sent; stopping Computer Use does not stop these independent tests.

## Boundaries

Inventory uses the server's full 12x7 capacity and item footprints. Drag an item
to a matching equipment seat; rings support either ring seat. Drag worn gear
to an exact backpack location (U also requests the first available location).
Drop onto uncovered world to leave an item on the ground. The server acknowledges equipment,
rejects two-handed conflicts, and keeps both items unchanged when a swap or
unequip cannot fit. Attack and defense ratings come from the native server.
`--scenario inventory-equipment` verifies these paths in the packaged client.

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
`verdigris_server` on a free **6520â€“6539** port (never 6500), launches
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
â†’ create Scion
â†’ enter route
â†’ move and melee the enemy
â†’ pick up the generated item and trophy
â†’ equip (gear pane: Enter, or 1-9)
â†’ return to the extraction point
â†’ extract durable House value (local: F at the EXIT; remote: walk onto it)
~~~

## Scenario harness (D-119)

The client has an automated, headless scenario runner that drives the real
inputâ†’simulationâ†’presentation pipeline and asserts on three layers:
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
     scenery), then drives commands with `scenario_step(state, Command::â€¦)`
     (dispatch â†’ ingest events â†’ age effects â†’ follow camera â†’ present);
   - asserts with `scenario_check(condition, "label")` against
     `state.simulation->â€¦` (core), `state.render_list` (`render::any` /
     `render::first` / `render::count`), and `state.render_list` Pane*/Hud
     ops (pane/HUD).
2. Register it in `run_scenarios`'s `entries` table.
3. Add a `render::Op` in `native/client/render_list.hpp` if the wave
   introduces a new draw class, and record it in the matching draw function
   in `main.cpp` (recording must live next to the draw so a suppressed draw
   is caught by the scenario).

The runner exits non-zero on any `scenario_check` failure.
