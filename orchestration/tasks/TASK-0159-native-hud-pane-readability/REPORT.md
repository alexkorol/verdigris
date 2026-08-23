# TASK-0159 REPORT — native HUD and gear-pane readability pass

Worker: ox-pc-z (coordinator codex, OpenRouter `stealth/ox-alpha`, OpenCode CLI
1.18.21 variant max), machine DESKTOP-TVU7OR7, worktree
`Z:\Code\.worktrees\verdigris\ox-pc-z2`, worker branch
`codex/TASK-0159-native-hud-pane-readability-ox-pc-z`.

## Executive summary

The native window now reads as a deliberate surface at 960x600 and 1366x768.
Every owner-visible text collision found at the routed base is removed and
proven gone by a new deterministic scenario that opens the real gear pane
through the production presentation path at both resolutions and hard-fails on
any rectangle intersection between global HUD regions, pane chrome, and the
fixed combat surfaces. Four fresh real-GDI captures were produced under this
task folder and visually inspected before handoff. The first-Escape
pane-dismissal and bare-Escape exit contracts are preserved and re-proven.

## Defects found at base (all confirmed on real GDI captures, then fixed)

1. **Identity line painted across the minimap panel** at every resolution —
   the top-HUD planner pinned identity at x=18,y=12 while the minimap occupies
   [12,12]-[120,120]. Visible in the pre-change reference capture
   (`TASK-0070 .../04-named-drop-gear-1366x768.png` at the claim head).
2. **Doubled prefix "House House Verdigris"** in the identity line — it
   prepended `"House "` to `world.house_name`, which is already prefixed; the
   gear-pane title's own comment documents that the name must not be doubled.
3. **Global controls hint collided with the open gear pane** — the planner had
   no knowledge of the pane rect; at 960x600 the 643 px hint could not fit any
   free row and fell back onto the pane title band / objective chip.
4. **Objective/art/connection chips could be placed into the open pane** for
   the same reason (art chip with an active session at 960x600 landed inside
   the pane title area).
5. **CONNECTION LOST banner painted across the minimap** at fixed (18,76).

## Approach

All within `native/client/main.cpp` (owned):

- One pure geometry source of truth (`HudRect`, `gear_pane_rect`,
  `minimap_rect`, `vital_orb_rect`, `quickbar_strip_rect`) shared by the
  painter, the top-HUD planner, and the scenario harness. Historical numbers
  preserved exactly.
- `plan_top_hud(width, height, gear_open, …)` now treats those fixed regions
  (pane when open, minimap always, quickbar/orbs for completeness) as blocked
  zones and walks deterministic fallback ladders: preferred pins → left lane
  beside the minimap → raw gutter. Identity leads the hierarchy from the lane
  beside the map. No region is ever deleted; degenerate widths keep the
  historical fallback pins.
- When even the ladders cannot fit the one-line controls hint (643 px vs ~520
  px of free left column at 960 with the pane open), the planner stacks the
  hint as two lines split deterministically at the " | " separator nearest the
  middle — same words, same authority, wrapped, never deleted. Unwrapped
  frames still draw the full single line.
- A per-frame rectangle trace (`ClientState::hud_rect_trace`) is recorded next
  to every draw (the same discipline as render_list ops), so the scenario
  asserts on exactly what was painted. It is a presentation diagnostic;
  painting does not branch on it.
- CONNECTION LOST banner keeps the left column but starts below the minimap.

Paint order is intentionally unchanged, so committed golden render-list JSONs
from earlier tasks cannot drift from this work.

## Changed files

- `native/client/main.cpp` (owned)
- `orchestration/tasks/TASK-0159-native-hud-pane-readability/STATUS.md`,
  `REPORT.md`, `captures/hud-pane-readability-{closed,open}-{960x600,1366x768}.png`
  (owned task folder)

`git diff --name-only` at the implementation head shows exactly these paths;
`native/src/**`, `native/include/**`, `server/**`, `src/**`, assets,
`remote_session.cpp`, `presentation_state.cpp`, `client_model.hpp`,
`render_list.hpp` are untouched (negative control: simulation, wire, save,
balance, asset, and font layers have zero diff).

## Public interfaces added/changed

- Client-internal only: `HudRect` + pure region helpers, extended
  `plan_top_hud` signature (internal), `ClientState::hud_rect_trace`. No
  simulation, protocol, or persisted interfaces touched.

## New deterministic scenario

`hud-pane-readability` (registered in `run_scenarios`): local path presents
closed then open pane at 960x600 and 1366x768 through `reference_present`;
asserts pairwise clearance among identity/controls/objective/art/minimap/
quickbar/orbs, pane exclusion for each of them (plus the wrapped second
controls line when present), mutual clearance of pane title/stats/seat/
banked/progression/footer/backpack cells, presence of every authority line
(nothing deleted), the single-House-prefix identity, and both Esc contracts
through `handle_escape_key`; saves four PNGs into this task folder and checks
they are non-trivial. Remote leg binds this lane's capsule 7100-7119, drives
the production session to an instance, opens the pane, and proves connection
and art chips clear the open pane at 960x600.

## Acceptance commands and literal outcomes

```
powershell -NoProfile -ExecutionPolicy Bypass -File native/build.ps1 -RunTests -RunClientScenarios
native legacy denylist: PASS
PASS remote-negative: dead endpoint fails start()
   PASS (0 failures)        # core, networking, camera2d binaries
   PASS (0 failures)        # session suite (green retry, see deviations)
   PASS (0 failures)        # presentation events
   PASS (0 failures) x12    # all client scenarios incl. hud-pane-readability
EXIT=0
```

```
native/build/verdigris_client.exe --scenario hud-pane-readability
== scenario hud-pane-readability ==
   ... 100+ ok lines: every pairwise/pane/Esc/capture check ...
   PASS (0 failures)
EXIT=0
```

```
native/build/verdigris_client.exe --scenario first-session-clarity
   ok: first-session-clarity: first Esc closes the pane, client stays alive
   ok: first-session-clarity: bare Escape requests application exit
   ok: first-session-clarity: NEGATIVE CONTROL - remote HUD never says press F
   PASS (0 failures)
EXIT=0
```

```
git diff --check      -> exit 0 (no whitespace errors)
git diff --name-only  -> native/client/main.cpp + TASK-0159 task folder only
```

## Manual verification

All four fresh real-GDI captures were opened and inspected before handoff:
identity now sits beside (not on) the minimap with a single House prefix; the
controls hint is fully visible (wrapped two-line at 960 open-pane) and never
touches the pane; objective/art/connection chips clear the pane; pane chrome
lines are mutually clear; quickbar and orbs stay clear of the pane. Pre-change
evidence for defects 1-4 is preserved in the committed
`TASK-0070-reference-scenes/captures/04-named-drop-gear-1366x768.png` at the
claim head (regenerating sibling captures during probing was reverted; other
tasks' files were never committed modified).

## Deviations and notes

- **Baseline session-suite flake (pre-existing, outside owned paths):**
  `verdigris_session_tests.exe` gate-b reconnect legs failed intermittently at
  the clean routed base before any edits (run 1: 6 failures; run 2: 6 with a
  different flip set; run 3: 1 failure "slain elite surfaces the circulating
  heirloom"; one invocation exceeded 10 minutes under load). The failing
  assertions compare whole chronicle payloads and roster membership under
  fixed wall-clock waits in `native/tests/session_tests.cpp` — a file this
  task may not modify. After the fleet quieted (no listeners in 6520-7199),
  the full literal acceptance command passed end-to-end with EXIT=0. This is
  the same load-sensitive class RUN_STATUS already records for the browser
  loot scenario ("retained timing seam; gate not weakened"). Flagged for an
  owner/architect successor; no gate was weakened or skipped here.
- Running `--scenario all` regenerates sibling tasks' capture PNG bytes
  (TASK-0070/0122/0145/0156 evidence folders). Those paths are not owned by
  this task; regenerated copies were restored after each probe so the pushed
  diff stays inside owned paths. Integration may want a capture-root isolation
  successor (TASK-0161 already sequenced behind 0159).
- `npm ci` was required once to materialize `node_modules` for the repo's
  yorkie pre-commit hook in this fresh worktree (same environment note as
  INC-013-era lanes); hooks ran enabled on every commit.

## Unresolved questions

- None blocking. Owner-only follow-ups (not implementable here): final
  typography/art direction for the wrapped controls hint, and whether the
  gate-b wall-clock waits should become deadline-polled loops (test-side
  successor).

## Risks and follow-ups

- The wrapped controls split point depends on measured font metrics; on a
  machine with radically different system font metrics the planner may wrap
  earlier/later — behavior remains deterministic per machine and always
  collision-free by construction.
- Sibling capture regeneration during full-scenario runs is a repo-wide
  footgun; TASK-0161 (capture-output isolation) directly addresses it.
- Backpack capacity vs pane footer space was not stress-tested beyond the
  scenario's items; cells-vs-chrome clearance is asserted for what renders.

## Commit SHAs

- Claim: `0e74c1854c2a0531ec6d071cb21c0cbde1811f15`
- Implementation + evidence: see STATUS.md `implementation_commit`
- REVIEW_REQUESTED head: see STATUS.md after the review-request commit
