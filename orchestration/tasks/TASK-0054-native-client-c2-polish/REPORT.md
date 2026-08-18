---
task: TASK-0054
state: REVIEW_REQUESTED
coordinator: deepseek
worker_branch: codex/TASK-0054-native-client-c2-polish-deepseek
base_commit: 60ba1305cf7299759523469d4c879c6c92e48e47
architect_review_required: true
---

# TASK-0054 REPORT — Native client C2 polish

## Executive summary

All four small owner-visible deliverables are implemented on the native
client, plus the two mandatory D-119 scenarios.

1. **Pane title** — `"Gear / House House Verdigris"` → `"Gear / House
   Verdigris"` (house name is already prefixed; no more doubling).
2. **Combat juice** — a brief target-sprite flash on every hit, floating
   damage numbers that rise AND fade over ~600ms (12 ticks), and a 150ms
   screen-edge red pulse when the player takes damage.
3. **Uniform zoom** — mouse-wheel zoom clamped to a 0.5x–2x envelope around
   the played-verified default; Home resets. No position-dependent scale
   reintroduced (camera2d invariants stay locked).
4. **New scenarios** — `combat-juice` and `zoom-invariance`, registered in the
   `--scenario` runner and the `-RunClientScenarios` gate.

## Changed files

- `native/client/render_list.hpp` — `TargetFlash`, `ScreenPulse` ops
- `native/client/main.cpp` — title fix, target flash + damage-number fade +
  screen pulse, zoom envelope, two new scenarios
- `orchestration/tasks/TASK-0054-native-client-c2-polish/` — report/status

No `playtest/**`, `server/**`, or `src/**` changes; no core (`native/src`,
`native/include`) changes.

## Test commands and outcomes

`powershell -File native/build.ps1 -RunTests -RunClient -RunClientScenarios`:

```
native legacy denylist: PASS
verdigris core tests: PASS
verdigris networking tests: PASS
camera2d tests: PASS
Verdigris native client shell
House: House Verdigris | trophies stored: 1 | items stored: 1
== scenario move-and-camera ==   PASS (0 failures)
== scenario first-fight ==       PASS (0 failures)
== scenario loot-to-bank ==      PASS (0 failures)
== scenario telegraph-dodge ==   PASS (0 failures)
== scenario combat-juice ==      PASS (0 failures)   [7 checks]
== scenario zoom-invariance ==   PASS (0 failures)   [9 checks]
```

## Authentic negative

Suppressed the target-flash draw call (commented the `TargetFlash` render-list
recording adjacent to the draw) and reran:

```
== scenario combat-juice ==
    FAIL: combat-juice: target sprite flashes on the hit
    ...
   FAIL (1 failures)          [exit code 1]
```

Restored and re-verified green (transcript above).

## Deviation (flagged for the architect)

**Zoom range units.** The spec said "clamped 24–96 px/unit around
camera2d.zoom". `camera2d.hpp`'s `Camera::zoom` default is a 48.0 placeholder,
but the client's actual, played-verified (D-117, TASK-0050) zoom unit is
`0.85 px/world-unit` (`kCameraDefaultZoom`). Re-scaling the client to a
literal 24–96 px/unit would make the world ~56× larger and break the
accepted look, so I implemented the spec's clear intent — a uniform
0.5x–2x envelope around the default — as `kCameraMinZoom = 0.425`,
`kCameraMaxZoom = 1.70`, Home resets to 0.85. If the architect actually wants
a different absolute range (or a world re-scale), that is a one-line
constant change; please note it in REVIEW so I can adjust before any
re-review.

## Commits

- `424de9f9` — claim (STATUS.md)
- implementation + report commit (SHA recorded at push)
