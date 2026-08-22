# TASK-0158 REPORT — native pane model and layout foundation

## Executive summary

Delivered the header-only, pure, dependency-free pane model/layout planner
for the future Gear, Character, and Passive tabs: `native/client/pane_model.hpp`.
Given a viewport, content minimums, the active tab, and a row count,
`pane::plan()` returns bounded reserved-top/reserved-bottom/panel/header/
tab-bar/content/footer rectangles plus scroll/clipping metadata. Plans never
overlap the reserved 148px top HUD band or the 96px bottom orbs/quickbar
band (constants mirrored from production `hud_safe_zones` and the top-HUD
row planner), shrink to fit between stated minimums, saturate safely at
64-bit scale for extreme row counts, and degrade with an exact tri-state
status (`InvalidViewport`, `InvalidMinimums`, `ViewportTooSmall`) that always
returns fully zeroed geometry instead of off-screen rectangles. No Win32/GDI
include, no I/O, no clocks; identical requests yield identical plans.
Painting and `main.cpp` integration stay outside this packet per SPEC.

State: REVIEW_REQUESTED.

## Recovery context (single lane allowance)

The first post-claim process exited unexpectedly after the claim commit
`d1fd4e40` was pushed, leaving one preserved untracked file:
`native/client/pane_model.hpp`. This recovery resumed from exactly that
state — nothing discarded, reset, cleaned, or duplicated. Preflight re-proved
branch exactness (`0 0` divergence), origin sync, absence of any competing
STATUS.md/RELEASE.md, and owned-path-only dirtiness. The preserved header
was verified against production constants before continuation:

- bottom reserve 96px matches `hud_safe_zones` (native/client/main.cpp:1412);
- minimap 132x132 matches main.cpp:1409;
- top reserve 148 = `kTopHudRow0Y(12) + kTopHudRowCount(4) * kTopHudRowStep(34)`
  (main.cpp:2827-2829).

Two evident-intent gaps in the mid-edit header were completed minimally:
a jammed double definition line was split (formatting only) and the missing
`Plan::tab` member was declared (`plan()` already assigned `out.tab`;
SPEC's "tab stability" acceptance requires the plan to echo the active tab).

## Approach

1. **Pure planner** (`native/client/pane_model.hpp`): `Request{viewport, tab,
   minimums, rows}` → `Plan{status, tab, reserved_top, reserved_bottom,
   panel, header, tab_bar, content, footer, scroll}`. Validation order:
   viewport → minimums (contradiction/negative/zero-row-height/chrome-overflow)
   → safe-band fit. On success the panel is centered in the safe band
   between the two reserved HUD rects, clamped to preferred size but never
   below minimums; chrome stacks header→tabs→content→footer inside the
   panel. Scroll metadata derives from real content height with all products
   computed in `long long` and saturated to INT_MAX before narrowing.
   Helpers `clamped_scroll_offset` and `first_visible_row` keep callers
   inside valid ranges. Failed plans carry only their status plus zeroed
   rects, so degraded results are unpaintable by construction.
2. **Self-contained tests** (task folder `pane_model_tests.cpp`): includes
   only the production header and the standard library; exits nonzero on
   first failure like `native/tests/core_tests.cpp`. Covers every SPEC
   dimension (below).
3. **MSVC harness** (task folder `run-tests.ps1`): discovers the installed
   Visual Studio Build Tools exactly like `native/build.ps1` (vswhere with
   `-property installationPath`, validated by `Test-Path`, then the same
   deterministic candidate list preferring 2019 BuildTools), compiles the
   production header through the test TU with
   `cl /nologo /std:c++20 /EHsc /W4 /I<native/client>`, links and runs the
   test exe, and writes every artifact under this task folder only
   (`build/`, gitignored following the repo's per-directory convention).

## Changed files (exactly the SPEC-owned paths)

- `native/client/pane_model.hpp` (new, header-only production model)
- `orchestration/tasks/TASK-0158-native-pane-model-foundation/pane_model_tests.cpp`
- `orchestration/tasks/TASK-0158-native-pane-model-foundation/run-tests.ps1`
- `orchestration/tasks/TASK-0158-native-pane-model-foundation/.gitignore`
- `orchestration/tasks/TASK-0158-native-pane-model-foundation/STATUS.md`
- `orchestration/tasks/TASK-0158-native-pane-model-foundation/REPORT.md`

No forbidden path touched. Nothing in production includes the new header
yet — integration is explicitly a later packet. No port used; port 6500
never approached.

## Public interfaces added/changed

- Added: `namespace pane` — `Tab`, `Status`, `Viewport`, `Rect`
  (`right/bottom/empty/overlaps/contains`), `ContentMinimums`, `Request`,
  `Scroll`, `Plan` (`ok/status_text/with_status`), `plan()`,
  `clamped_scroll_offset()`, `first_visible_row()`,
  constants `kTopHudReserveHeight/kBottomHudReserveHeight/kSideMargin`.
- Changed: none. No existing interface, wire event, schema, simulation,
  server, or build file changed; CMake untouched.

## Test commands + outcomes (literal transcripts)

### 1. Acceptance harness (SPEC command)

```
PS Z:\Code\.worktrees\verdigris\ox-pc-aa2> powershell -NoProfile -ExecutionPolicy Bypass -File orchestration/tasks/TASK-0158-native-pane-model-foundation/run-tests.ps1
**********************************************************************
** Visual Studio 2019 Developer Command Prompt v16.11.42
** Copyright (c) 2021 Microsoft Corporation
**********************************************************************
[vcvarsall.bat] Environment initialized for: 'x64'
pane_model_tests.cpp
**********************************************************************
** Visual Studio 2019 Developer Command Prompt v16.11.42
** Copyright (c) 2021 Microsoft Corporation
**********************************************************************
[vcvarsall.bat] Environment initialized for: 'x64'
pane model tests: PASS (413 checks)
TASK-0158 pane model acceptance harness: PASS
cmd1-exit=0
```

Coverage proven by the 413 checks: 960x600, 1366x768, and 1920x1080 plans
(exact deterministic panel positions asserted), shrink-to-fit clamping at
900x540, explicit degradation on narrow/short/impossible viewports and all
invalid-minimum classes, zero row sets across all three resolutions,
large (10^6) and overflow-scale (2x10^9) row sets including INT_MAX
saturation and last-row reachability at max offset, tab stability (geometry
and scroll invariant under Gear/Character/Passive while each plan echoes its
tab), byte-level determinism of healthy and failed plans, reserved-region
non-overlap for every pane rectangle at every resolution, and zeroed
geometry on every failure path.

### 2–3. Diff hygiene (SPEC commands)

```
git diff --check        -> cmd2-exit=0 (no output)
git diff --name-only    -> cmd3-exit=0 (no output; work is new files)
```

### 4. Legacy denylist (extra evidence; new .hpp is in scan scope)

```
python native/tools/check_legacy_denylist.py -> native legacy denylist: PASS
```

## Manual verification

- Compiled clean at `/W4` with MSVC 2019 v16.11.42 (`/std:c++20`); zero
  warnings emitted.
- Header includes verified limited to `<algorithm>` and `<cstdint>` — no
  Win32/GDI/windowing/I/O/clock dependency anywhere (STOP condition not
  approached).
- Expected panel geometry hand-computed for all three resolutions
  (e.g. 1366x768 → panel 420x460 at (473,180)) and asserted literally in
  the tests, cross-checked against the production constants cited above.

## Commit SHAs (this worker branch)

- claim: `d1fd4e40` (pre-recovery, already pushed)
- implementation: `5c65a0f2`
- evidence/review-request: `<this commit>`

Base at provisioning: `1f69e82af310121ec0d20548ff15735984467406`
(SPEC base `ad1a1e178e689df442d4655937f8e8e037cf4cd2` is an ancestor).

## Deviations

1. **Recovery process used (single lane allowance).** Resumed from the
   preserved untracked header left by the crashed first process; completed
   its two evident-intent gaps (line split; `Plan::tab` member declaration).
   No prior work rewritten or discarded.
2. **Harness vswhere call hardened.** The first harness draft copied
   `native/build.ps1`'s bare vswhere invocation; this machine's vswhere
   prints a locator banner to stdout, so `-First 1` captured the banner and
   the fallback landed on a VS2017 toolchain that rejects `/std:c++20`.
   Fixed by requesting `-property installationPath` (single-line output),
   validating candidates with `Test-Path`, and mirroring build.ps1's
   2019-first preference. Production build files were not modified.
3. **Dependencies installed in this fresh worktree** (`npm install`) so the
   yorkie/lint-staged pre-commit hook could run with hooks enabled; it ran
   on both commits (lint-staged selects none of this packet's file types).
   No `--no-verify` was used.

## Unresolved questions

- None blocking.

## Risks / follow-ups

- The planner is intentionally unreferenced by production code; wiring it
  into `main.cpp` painting is the successor packet's job (forbidden here).
- Reserved-band constants are duplicated from the shipped client by design
  (pure seam); if the owner later moves the HUD bands, update both together.

## STOP conditions

- Not triggered. The planner remained independent of Win32/GDI throughout;
  no CMake edit, painting, hotkey, authored copy, typography/backend choice,
  content value, or gameplay/network change exists in this packet.
