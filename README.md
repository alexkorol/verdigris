# Verdigris

Verdigris is a WASD-first multiplayer action RPG about persistent **Houses**
(clans and lineages) and mortal **Scions**. **The native C++ game is the active
product.** The old Vue/Node game is retained as historical source, not the
current game to install, develop, or use for native acceptance.

## Start here: the native game

The default branch, `master`, contains the integrated native game.
Download the Windows starter preview from
[GitHub Releases](https://github.com/alexkorol/verdigris/releases), extract the
whole archive, and run `Verdigris.exe`. No development tools are needed for
the packaged game.

The preview covers House/Scion creation, occupation choice, village defense,
the well boss, forgiving retry, the first skill point, and departure to
Crossroads. The starter map uses procedural woodland scenery. This remains
an in-development preview: enemy art is provisional, and the rest of the
campaign is unfinished. Release notes identify the tested source commit.

### Play from source on Windows

Requirements: Windows, PowerShell, Git, Python 3 available as `python`, and
Visual Studio Build Tools with the C++ toolchain and Windows SDK.
**Node, npm, Vite, and a browser server are not required for the native game.**

For a new checkout:

```powershell
git clone --branch master --single-branch https://github.com/alexkorol/verdigris.git verdigris-native
cd verdigris-native
powershell -NoProfile -File .\native\tools\play-native.ps1
```

For an existing checkout, first preserve local work and check its branch and
upstream. Do not reset or switch a dirty/shared worktree just to follow the
example above.

The supported launcher builds missing or stale native executables, starts
`verdigris_server.exe` on a free local port in 6520–6539, opens
`verdigris_client.exe --remote`, and cleans up its server when the client
exits. Logs are under `native/build/logs/`.

Keep the checkout's assets with this source build. Copying only the client
executable is not a player package. `-Local` and the bare client executable
are developer/test paths, not substitutes for the normal server-backed launch.

For detailed controls and platform notes, see the
[native workspace guide](https://github.com/alexkorol/verdigris/blob/master/native/README.md).
The current native prototype covers House/Scion entry, expeditions, movement,
combat, loot/equipment, and return/extraction. For the exact implemented state
and known limitations, use the
[native handoff](https://github.com/alexkorol/verdigris/blob/master/docs/rebuild/HANDOFF.md).
Do not treat the old browser feature checklist as evidence of native completion.

## Build and verify native changes

Run these from the repository root:

```powershell
# Build the native executables.
.\native\build.ps1

# Build and run native tests plus the client scenario suite.
.\native\build.ps1 -RunTests -RunClientScenarios

# Run a focused client scenario after building.
.\native\build\verdigris_client.exe --scenario raster-world
```

Native presentation work also requires launching the normal game, capturing
and inspecting the actual window, and checking that existing raster art and
animation have not regressed. A helper test, file-hash check, or screenshot
collected without review does not establish a working player experience.

```powershell
powershell -NoProfile -File .\native\tools\capture-window.ps1 -OutPath .\native\build\review.png
```

Use an isolated test profile for persistence or destructive gameplay checks.
A distributed package must also be tested outside the source checkout, with
its own assets, fonts, and recorded build provenance.

The root `npm run playtest` and `npm run verify` scripts belong to the legacy
JavaScript game. They are **not native acceptance gates**. Do not run or repair
them as routine native work.

### Linux and macOS development

CMake and a C++20 compiler can build the headless/core targets:

```sh
cmake -S native -B native/build
cmake --build native/build
ctest --test-dir native/build --output-on-failure
```

The non-Windows client is currently a console fallback, not the Windows
rendered game. Passing those tests does not verify the Windows presentation.

## Development map

| Area | Purpose |
| --- | --- |
| `native/` | Active C++ client, authoritative simulation/server, tests, tools, and assets |
| `docs/product/` | Product decisions and the Verdigris constitution |
| `docs/rebuild/` | Native implementation handoffs and evidence |
| `orchestration/` | Task ownership, coordination, and review records |
| `src/`, `server/`, `playtest/`, root JS tooling | Historical browser implementation and its tests |

Before implementing, read the native branch's
[agent guide](https://github.com/alexkorol/verdigris/blob/master/AGENTS.md),
[product constitution](https://github.com/alexkorol/verdigris/blob/master/docs/product/VERDIGRIS_CONSTITUTION.md),
and [coordination protocol](https://github.com/alexkorol/verdigris/blob/master/orchestration/PROTOCOL.md).

Authorized implementation includes committing and pushing verified work to
its working branch unless the owner requests local-only work. Verify the
remote commit. This does not bypass review, ownership, or default-branch and
release controls.

[WIZARD](https://github.com/alexkorol/WIZARD/tree/gh-pages) is a separate
asset-authoring and reference workspace. Its tools and demos are not the
retired browser game and are not proof of native integration. Candidate art
still requires visual review before production use.

## Historical browser reference

The old Vue/Node implementation remains in this repository for provenance and
selective reference. Its [historical setup notes](docs/development-setup.md)
and root JavaScript scripts are not instructions for launching the current
native product. Routine development does not maintain a second game; legacy
work requires an explicit task. Preserve historical source and attribution
rather than mechanically porting its behavior or deleting it to simplify the
README.

## Attribution

Verdigris grew from Delaford, created by Dan Jasnowski, and preserves its
MIT-licensed foundation. The original copyright notice remains in `LICENSE`.
Additional asset-specific credits are kept beside their respective assets.
