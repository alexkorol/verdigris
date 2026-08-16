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
