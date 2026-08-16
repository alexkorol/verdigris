---
id: TASK-0003
title: Automated verification harness for the Founding of a House slice
state: READY
track: web-demo
priority: medium
base_commit: f5b4b72
dependencies: []
parallel_safe: true
owned_paths:
  - prototypes/founding-slice/tests/**
  - prototypes/founding-slice/run-checks.mjs
forbidden_paths:
  - prototypes/founding-slice/slice.html
  - prototypes/founding-slice/index.html
  - prototypes/founding-slice/build.mjs
  - prototypes/founding-slice/assets/**
  - native/**
  - src/**
  - server/**
  - package.json
  - playwright.config.js
acceptance_commands:
  - node prototypes/founding-slice/run-checks.mjs
---

## Goal

One command proves the feel-slice's core arc still works after any future
edit: build freshness, load without console errors, and the full loop
(direction choice → crisis → combat → death → relic → successor →
node clear → graph unlock → founding).

## Why this task exists

The slice was verified manually this session via ad-hoc browser scripting.
That evidence evaporates on the next change. The slice will keep evolving
as a camera/combat laboratory (D-102), so it needs its own cheap gate —
without touching the shared repo test config.

## Product and architectural invariants

- The slice stays self-contained; the harness must not modify game files or
  shared config (`package.json`, `playwright.config.js` are forbidden).
- Use the repo's existing installed `playwright` package (already in
  node_modules) — add no dependencies.
- The game exposes `window.__V` (state, house, scion, player, entities,
  enterNode, spawnEnemy, tick, NODES, save) and ticks via a hidden-tab
  `setInterval` fallback, so headless drive is possible; `__V.tick(1/60)`
  fast-forwards deterministically enough for assertions.

## Inputs and references

- `prototypes/founding-slice/` at base_commit (read `slice.html` for the
  `__V` surface and debug-panel button labels).
- The session's driving pattern: title `#btnnew` → `.choice[data-dir=…]` →
  place player at loot (120,280), send `e` key, ~560 ticks burns the crisis
  timer, real 2s waits cover `setTimeout` beats; debug panel opens with
  backquote; buttons "heal" / "clear node"; standard at (0,-720).

## Scope

1. `run-checks.mjs` (node, ESM): serves `prototypes/founding-slice/` on an
   ephemeral port (node http, no deps), rebuilds `index.html` via
   `build.mjs` into a TEMP copy and fails if it differs from the committed
   `index.html` (drift guard — do not overwrite the committed file), then
   runs the Playwright checks below, prints a pass/fail summary, exits
   non-zero on any failure.
2. Playwright checks (headless chromium), in `tests/`:
   a. Load: zero `pageerror` and zero console errors after 2s.
   b. Fresh-house arc: begin → choose each of the three directions in three
      isolated storage contexts, assert scion stats differ per direction.
   c. Full loop (one context): intro pickup equips a weapon; crisis spawns
      enemies; real LMB melee reduces an enemy's life; debug-clear waves;
      standard completion clears node, unlocks `burning-fields`, standing
      increases; death path: spawn raiders, let player die, assert
      `house.relicPool` gained the carried weapon and lineage grew;
      successor is level 1 with empty pack; jump to `wardens-circle`,
      clear, found the House, assert `house.founded` and name persistence
      across reload.
   d. Persistence: reload page, `Continue` button present, house name kept.
3. Console errors during driven play (not just load) fail the run.

## Non-goals

- No gameplay changes, no balancing, no new features in the slice.
- No CI wiring (a follow-up may call `run-checks.mjs` from a workflow).
- No visual-regression screenshots (optional artifacts are welcome but not
  asserted).

## Deliverables

- `prototypes/founding-slice/run-checks.mjs` + `tests/` sources.
- One coherent commit.

## Acceptance criteria

- `node prototypes/founding-slice/run-checks.mjs` exits 0 at base_commit.
- Deliberately corrupting a copy of `index.html` (drift) or breaking a
  `__V` name in a scratch copy makes the harness fail — demonstrate one
  such negative run in REPORT.md, then restore.
- Wall-clock under ~3 minutes.

## Required verification

```bash
node prototypes/founding-slice/run-checks.mjs
```

Include full output in REPORT.md.

## File ownership

Only the two owned paths. The game files themselves are read-only for this
task; if a check cannot be written without changing the game (e.g., a
missing hook), STOP and file a question instead of editing `slice.html`.

## Dependencies

None. Parallel-safe with TASK-0001 (native sources) and TASK-0002
(build/CI files) — disjoint.

## Parallel-safety assessment

Touches only new files inside the prototype folder.

## Review focus

- The drift guard compares built output without clobbering committed files.
- Checks assert through `__V` state, not brittle pixel or timing hacks;
  real-time `setTimeout` beats get generous waits.
- Failure output is diagnosable (which check, what state).

## Stop conditions

- `__V` surface proves insufficient for an assertion → stop, file a
  question listing the exact missing hook.
- Playwright/chromium unavailable in the environment → stop, report.
