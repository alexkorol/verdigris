# FINDINGS — TASK-0092 owner launch and packaging readiness audit

Lane `ox-pc-bd` · branch `worker/verdigris/pc/ox-pc-bd` · claim commit
`39fc7be0` on base `d2423873` · 2026-08-23 · machine DESKTOP-TVU7OR7
(Windows 10 Pro 19045, Windows PowerShell 5.1).

Machine-readable companion:
[`captures/package-inventory.json`](captures/package-inventory.json)
(schema `verdigris-package-inventory/1`). This file is the narrative; the JSON
carries every claim with its file:line citation. Acceptance transcripts for the
rg sweeps are retained verbatim in `captures/acceptance-1-launcher-sweep.txt`
and `captures/acceptance-2-cmake-sweep.txt`.

## 0. Authority, constraints, method

- Read-only resource capsule honored: **no launcher was executed, no server was
  started, no port was bound or probed live, and port 6500 was never touched.**
  Every statement below is cited from committed sources; nothing was built or
  run to produce this inventory.
- No builds were changed, no shortcuts created or edited, no signing/account/
  distribution decision made — per SPEC stop points.
- Frozen invariants restated with their enforcement evidence: owner port 6500
  reserved (launcher rejects it by name, `play-native.ps1:40-47`; capsule
  6520–6539 at :17-18), loopback-only servers (`networking.cpp:2811` binds
  127.0.0.1 only), forgiving persistence (in-memory roster, "no database seam
  yet", `networking.cpp:216`; server stopped when client exits,
  `play-native.ps1:410`), one-command owner path unchanged (`README.md:47-68`).
- Method: full reads of `native/tools/play-native.ps1` (422 lines),
  `native/build.ps1`, `native/CMakeLists.txt`, `native/CMakePresets.json`,
  `native/src/server_main.cpp`, targeted reads of `native/client/main.cpp`,
  `remote_session.cpp`, asset/persistence sources, plus the two literal rg
  acceptance sweeps.

## 1. The launcher today is a developer launcher with owner-grade bones

`play-native.ps1` already proves several things most launchers never do:

- **Port discipline**: explicit 6500 refusal by name, first-free scan across
  6520–6539 with a real loopback bind-probe, occupied-port reports naming the
  owning process (`play-native.ps1:40-79`).
- **Readiness honesty**: it does not trust process liveness; it greps the
  server's stdout for the exact `listening on ws://127.0.0.1:<port>` line
  within 12 s *and* asserts the reported port equals the chosen port
  (:138-163). Both failure modes have proven negative controls
  (`-ReadinessFaultControl` drives a silent impostor and a wrong-port impostor
  through the same path, :311-349).
- **Orphan guarantees**: startup failures stop the spawned pid before rethrow
  (:164-172); every session ends with a pid-based no-orphan assertion for both
  processes (:249-260); graceful-close lifecycle selftests assert exit code 0
  through WM_CLOSE (:262-309).
- **Actionable failures**: missing build script, missing exes, stale exes all
  fail with remediation text (:114, :120-121).

What keeps it a *developer* launcher rather than an owner launcher is not the
run loop — it is everything around it, itemized below.

## 2. Executables, dependencies, generated files

**Shipping pair.** `verdigris_server.exe` (server_main + networking + core +
seasonal, ws2_32) and `verdigris_client.exe` (+ user32/gdi32/ws2_32 under
WIN32). Both land in `native/build/` beside six test binaries and all `.obj`
intermediates — nothing separates "what ships" from "what the build left".

- CRT linkage is **unpinned**: `cl` is invoked without `/MD` or `/MT`
  (`build.ps1:116`), so the effective runtime linkage is whatever this
  toolchain's default produces. A packaging packet must pin and record it;
  inheriting compiler defaults silently is how redistributable surprises ship.
- Optimization is likewise unpinned (no `/O2` anywhere); the MSVC CMake preset
  says Release but the actual one-command path goes through `build.ps1`.
- The client keeps the **console subsystem deliberately** so `--headless` works
  (`main.cpp:6415-6417`) — correct for development, but it means a
  double-clicked client opens a terminal window next to the game window.
- Dynamic DLL use degrades gracefully: `gdiplus.dll`/`msimg32!AlphaBlend` load
  via LoadLibraryA and their absence falls back to the embedded procedural kit
  with truthful HUD status (`main.cpp:569-595, 649-655, 631-646`). This is a
  genuinely portable property worth preserving.

**Generated files.** Build outputs (`*.obj`, six test exes, the shipping pair)
under `native/build/`; per-session server logs `server-<port>-<stamp>.log[.err]`
under `native/build/logs/` with **no rotation or cap**; scenario/reference PNG+
JSON captures confined to repo-contained roots when `VERDIGRIS_CAPTURE_ROOT` is
set (`main.cpp:2790-2814` rejects outside-repo roots) or to committed task
capture folders by default.

## 3. Assets and save locations

**Assets.** Two-tier: a compiled-in procedural visual kit
(`assets/generated/visual_kit.h`, kit version `task0147-gen-2`,
`visual_kit.h:8`) always present, plus an optional PNG billboard kit discovered
from `<exe-dir>\assets`, up to six directory levels of `\assets` /
`prototypes\founding-slice\assets` walk-ups, then cwd-relative walk-ups
(`main.cpp:599-626`). The installed-style root is the packaging seam: TASK-0142
already supports shipping PNGs beside the exe with zero code change. Without
PNGs the game still runs and the HUD honestly says "embedded vector kit"
instead of claiming art it lacks.

**Saves.** There are none on disk in normal play. House/session state lives in
the server process (`networking.cpp:216`); the persistence adapter exists but
only tests exercise it. The launcher stops the server when the client exits,
so **every owner session ends with progression evaporating**. That matches the
frozen forgiving-persistence invariant today, but no owner-facing text mentions
it (`README.md:47-68` is silent on persistence loss) — a double-clickable build
must disclose it or a future packet must wire disk saves (out of scope here).
The only durable artifacts an owner generates are logs and optional captures.

## 4. Negative control (SPEC requirement): the unproven clean-machine assumption

**`python` must resolve on PATH, and the current launcher never checks it.**

`build.ps1:159` runs `python tools/check_legacy_denylist.py` unconditionally on
every build — including the build triggered by the owner one-command path. The
script contains zero Python availability probing; the gate fires *after* roughly
thirty compile/link steps. On a clean Windows machine without Python, the owner
waits minutes for a successful-looking build and then gets:

```
python : The term 'python' is not recognized as the name of a cmdlet, ...
```

—or worse, the Microsoft Store alias stub intercepts and exits silently. The
missing check is one guard beside the existing vcvars probe (which models the
correct pattern at `build.ps1:43-86`):

```powershell
if (-not (Get-Command python -ErrorAction SilentlyContinue)) {
  throw "Python 3 is required for the legacy denylist gate " +
        "(native/tools/check_legacy_denylist.py). Install Python 3.x and re-run."
}
```

Secondary unproven assumptions inventoried in the JSON (NC-2..NC-5): execution
policy friction (the documented shortcut must carry
`-ExecutionPolicy Bypass`, `README.md:63`); `powershell.exe` 5.1 presence;
Add-Type inline C# compilation availability (selftest paths only); and the
staleness watcher missing `native/audio/` entirely (`Test-ExeStale` watches
src/client/include + build.ps1 only, `play-native.ps1:83-92`, while audio
sources are compiled into the client at `build.ps1:136-139`) — owners can play
stale binaries after audio edits.

## 5. Failure-message inventory (condensed; full lists in JSON)

Launcher Fail/throw messages cover port reservation/capsule/occupancy, missing
scripts/exes, server startup death (with stderr tail), readiness timeout,
port mismatch, close-timeout, nonzero close exit codes, and orphan leaks —
each names the remediation or the offending pid/process. Build throws cover
capture-root containment, vcvars absence with the probed list, the
native-Windows define guard, per-test-suite failures, and scenario/bench/soak
failures. Client-side remote failures surface honest transport messages
("connection refused at <host>:<port>", "reconnect failed after 3 attempts",
etc., `remote_session.cpp:251-541`) via `verdigris_client --remote: <error>` on
stderr (`main.cpp:6361`). Gaps: server bind-failure text ("bind/listen failed")
never reaches an owner who launches the exe bare, since stderr closes instantly
on a double-click.

## 6. Version metadata

Absent. No `.rc`, `.manifest`, or `.ico` exists under `native/` outside build/;
no VERSIONINFO/FileVersion/ProductVersion string anywhere. Explorer shows
unknown publisher and no version for both exes; there is no git SHA, build
date, or config stamp inside any binary. Today's identity strings are
developer-facing: window title "Verdigris Core Testbed" (`main.cpp:6464`),
window class VerdigrisNativeClient (which the launcher couples to at
`play-native.ps1:196`), and the embedded kit version. Signing workflows will
also want FixedVersionInfo before they can bind anything — sequencing PK-2
before any signing conversation is deliberate.

## 7. Platform gaps

**Windows**: no layout manifest or packaged artifact of any kind; console flash
on double-click; developer window title; no icon; bare-launched
`verdigris_server.exe` binds the frozen browser port 6500 by default
(`server_main.cpp:10`) — safe only because the launcher always passes an
explicit capsule port, which makes a packaged folder that invites direct
double-clicks a latent invariant violation; shortcut creation is a manual
README instruction; installed-style asset loading is supported in code but has
no end-to-end proof from a copied-out folder.

**macOS**: no windowed client at all — the non-Windows build is the console
demo (`main.cpp:6477-6503`; README admits the fallback is deliberately small).
Zero APPLE branches in CMakeLists (only WIN32 at lines 19/56/75); no bundle,
Info.plist, icon, or codesign seam; presets are Debug/NMake-only; and the
acceptance sweep confirms **no `install` or `package` target exists anywhere**
(`captures/acceptance-2-cmake-sweep.txt`: matches are CMAKE_/MSVC/WIN32 lines
only). macOS sockets are assumed by the WIN32 guard without committed proof.

## 8. Separation required by SPEC

| layer | state today | smallest successor |
|---|---|---|
| Build portability | MSVC-only in practice; discovery solid; flags/CRT unpinned | pin /O2 + CRT choice; provenance stamp (PK-0) |
| Packaging | absent — no rule for what ships | written layout contract + validator (PK-1) |
| Signing/notarization | absent; owner-only per SPEC; version resource must precede it | PK-2 creates the prerequisite, stops at the boundary |
| Installer | absent — no install(), MSI/NSIS, or shell integration | only after PK-1/PK-2 |
| Launch UX | strong core loop; four frictions (execution policy, console flash, dev title, undocumented persistence loss) | PK-4, after handoff artifact exists |

## 9. Sequenced packaging packets

Mechanism documentation only — nothing here was executed. Each packet is
independently revertible and lands its own evidence.

1. **PK-0 Build determinism + preflight hardening** — python preflight before
   any compile step (fixes NC-1), pinned Release/CRT flags, build-provenance
   stamp, staleness watch extended to native/audio/ (fixes NC-5). Small,
   localized, no owner-path behavior change.
2. **PK-1 Release layout contract + validator** — define the minimal folder
   (exe pair + optional assets/), validator proves installed-style billboard
   loading via the existing TASK-0142 seam. Documentation-first.
3. **PK-2 Executable identity** — .rc VERSIONINFO + icon + window-title
   successor; keep the launcher's window-class constant in lockstep; decide the
   server default-port successor so a bare-launched server can never touch
   6500.
4. **PK-3 Unsigned Windows handoff artifact** — zip of the PK-1 layout from
   PK-2-versioned binaries, SmartScreen expectations documented, persistence
   loss disclosed. Stops exactly at the signing/distribution boundary.
5. **PK-4 Launch UX polish** — .cmd shim against execution-policy friction;
   console-suppression decision preserving harness output; shortcut generation
   remains out of scope unless the owner requests it.
6. **PK-5 macOS parity spike** — blocked on the renderer backend decision
   (TASK-0073/TASK-0088 territory); until then the README console-fallback
   statement stays the honest macOS story.

## 10. Risks

- The 6500-default latent violation (§7) is the only finding that touches a
  frozen invariant, and only via a path the current launcher never takes; PK-2
  closes it cheaply.
- Unpinned CRT/optimization means today's "it builds" says nothing about what a
  release-grade rebuild would link against; PK-0 removes the ambiguity before
  any packaging claims are made.
- Log growth under `native/build/logs/` is uncapped; harmless on a dev box,
  surprising in an owner folder over months.
- Everything else is absence (packaging/signing/installer/macOS), not defect:
  the audited run path itself showed no broken behavior, and its failure paths
  are unusually well-proven by selftest/fault-control machinery.
