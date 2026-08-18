---
task: TASK-0051
state: REVIEW_REQUESTED
coordinator: deepseek
worker_branch: codex/TASK-0051-native-client-harness-deepseek
base_commit: 8ea0887cbfca8f024ca8c8ddb6b19746e5fab4f5
architect_review_required: true
---

# TASK-0051 REPORT — Native client test harness (D-119)

## Executive summary

The native client now has what the server playtest has: an automated,
headless scenario runner that drives the REAL
input→simulation→presentation pipeline and asserts outcomes, so client
regressions are caught the way playtest catches server regressions. Four
starter scenarios pass at the branch tip, and one authentic negative proves
the harness actually catches a suppressed presentation draw.

## Shape delivered

1. **Scenario runner** — `verdigris_client.exe --scenario <name|all>` runs a
   deterministic input script through the real pipeline (dispatch → ingest
   events → age effects → follow camera → paint), headless, via an offscreen
   memory DC. Exits non-zero on any failing check.
2. **Three assertion layers per scenario** — authoritative core state
   (`state.simulation->…`), the recorded render list (`render_list.hpp`:
   `render::any/first/count` over `render::Op`), and pane/HUD ops
   (`PaneStat`/`PaneWeapon`/`PaneItem`/`PaneBanked`/`Hud`).
3. **Starter scenario set** — `move-and-camera` (written FIRST; asserts every
   scenery entity shifts by one uniform delta equal to camera-shift × zoom —
   the exact sliding-billboard regression), `first-fight`, `loot-to-bank`,
   `telegraph-dodge`.
4. **Gate wiring** — `native/build.ps1 -RunClientScenarios` runs the full set.

## Changed files

- `native/client/render_list.hpp` — semantic draw-op recording layer (new)
- `native/client/main.cpp` — render-list recording threaded through the paint
  pipeline; `--scenario` runner + 4 scenarios; `--scenario` flag in `main()`
- `native/build.ps1` — `-RunClientScenarios` flag
- `native/README.md` — "Scenario harness (D-119)" authoring note
- `orchestration/tasks/TASK-0051-native-client-harness/` — this report/status

No `playtest/**`, `server/**`, or `src/**` changes. No core
(`native/src`, `native/include`) changes.

## Test commands and outcomes

`powershell -File native/build.ps1 -RunTests -RunClient -RunClientScenarios`:

```
native legacy denylist: PASS
verdigris core tests: PASS
verdigris networking tests: PASS
camera2d tests: PASS
Verdigris native client shell
House: House Verdigris | trophies stored: 1 | items stored: 1
== scenario move-and-camera ==
    ... 9 ok ...                PASS (0 failures)
== scenario first-fight ==
    ... 5 ok ...                PASS (0 failures)
== scenario loot-to-bank ==
    ... 9 ok ...                PASS (0 failures)
== scenario telegraph-dodge ==
    ... 2 ok ...                PASS (0 failures)
```

## Authentic negative

Suppressed the swing draw call (commented out the `Swing` render-list
recording, which lives adjacent to the swing draw) and reran:

```
== scenario first-fight ==
    FAIL: first-fight: a swing is drawn
    ok: first-fight: a floating damage number is spawned
    ...
   FAIL (1 failures)          [exit code 1]
```

Restored and re-verified green (transcript above). The recording sits in the
same branch as the draw so a suppressed draw is caught, not a parallel list
that could drift.

## Deviations / notes

1. **telegraph-dodge setup.** A within-windup dodge of the elite's half-plane
   Thrust is unreachable with the current constants (thrust band starts at
   melee range 143u, windup is 3 ticks ≈ 33u of movement). The scenario
   therefore exercises the melee-range **Sweep** telegraph and dodges it with
   one Dash (~110u), which is the reachable deterministic dodge. It still
   proves "telegraph drawn" + "moving out avoids damage" end-to-end.
2. Scenarios present into an offscreen memory DC rather than the window, so
   the window paint path and the scenario path share the same
   `paint_scene` — no second renderer.

## Commits

- `6bd12f47` — claim (STATUS.md)
- `ea5d9343` — scenario harness + scenarios + gate + README + REPORT/STATUS
