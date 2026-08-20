---
task: TASK-0064
state: REVIEW_REQUESTED
coordinator: cursor
worker_branch: codex/TASK-0064-remote-presentation-unify-cursor
base_commit: 5c41a04821695e38261fdb52f2e86b2dea67f21d
architect_review_required: true
---

# TASK-0064 REPORT — Remote presentation unify (Gate A unblock)

## Executive summary

`--remote` no longer opens the 0061 debug painter (dots/squares/text log).
It creates the same Win32 client as local play (`paint_scene`, billboards,
combat FX, damage numbers, telegraph rings, HUD panes, camera2d zoom 0.85)
fed from `ClientModel` + `PresentationEvent`s. There is no in-process
`Simulation` in remote mode. Local D-119 scenarios stay green. A remote
render-list scenario drives a real `verdigris_server` on 6580–6599, presents
through `paint_scene`, and asserts Monster / Swing / Drop ops.

Architect play: `verdigris_client.exe --remote [host] [port]` (default
`127.0.0.1:6580`). Keys match local: WASD, mouse aim, LMB melee, Space dash,
Q/E/R skills, X take-underfoot, F extract (walk stairs still works), I gear,
N enter tin route, 1–9 equip, Esc quit, F3 debug overlay.

## Approach

- Introduced `WorldView` (`native/client/presentation_state.*`). Local
  `paint_scene` syncs it from `Simulation`; remote syncs it from
  `ClientModel`. The GDI painter only reads the view + client FX.
- Presentation events (swing, hit flash, numbers, death, loot sparkle,
  telegraphs, screen pulse) share `apply_presentation_event` between the
  window and the session-test recorder.
- `RemoteProtocolSession` now upserts inferred `ClientMonster`s from
  `monster:telegraph` / `combat:hit` (placed one tile in front of the scion)
  and records kill drops on `model.ground` so the painter has something to
  draw until 0063 snapshots exist.
- Protocol scene coordinates are tile-sized; presentation multiplies by
  `kTileUnits` so billboard scale matches local play.
- `remote_play.cpp` is a non-Windows stub. The Windows `--remote` entry
  lives in `main.cpp` and uses `window_proc` / `paint_scene`.

## Changed files

- `native/client/presentation_state.hpp/.cpp` — WorldView, FX ingest, record_world_ops
- `native/client/client_model.hpp` — monsters, resource, stored counts
- `native/client/local_session.cpp` — richer model publish
- `native/client/remote_session.cpp` — inferred monsters / ground on combat
- `native/client/main.cpp` — paint_scene from WorldView; remote uses same window
- `native/client/remote_play.cpp` — Windows painter removed
- `native/tests/session_tests.cpp` — `remote_render_list_ops`
- `native/CMakeLists.txt`, `native/build.ps1` — compile/link presentation_state
- `orchestration/tasks/TASK-0064-remote-presentation-unify/{STATUS,REPORT}.md`

## Test commands and outcomes

`powershell -File native/build.ps1 -RunTests -RunClientScenarios` (2026-08-20):

```
native legacy denylist: PASS
verdigris core tests: PASS
verdigris networking tests: PASS
camera2d tests: PASS
… session tests passed (incl. journey + render-list Monster/Swing/Drop)
== scenario move-and-camera == PASS
== scenario first-fight == PASS
== scenario loot-to-bank == PASS
== scenario telegraph-dodge == PASS
== scenario combat-juice == PASS
== scenario remote-render-list == PASS
    ok: Monster / Swing / Drop ops in paint_scene render list
    ok: remote present uses no Simulation
== scenario zoom-invariance == PASS
```

## Server / model gaps (no `native/src/**` edits)

1. **No authoritative monster positions.** Painters place foes one tile in
   front of the scion from telegraph/hit ids. 0063 snapshot work should
   replace this.
2. **No ground-item envelope.** Kill loot sparkles at the inferred corpse;
   pickup names still arrive via `core:refresh:inventory`.
3. **Protocol units vs D-114 world units.** Remote positions are scaled by
   `kTileUnits` in presentation only.
4. **No House-bank / wear-total envelopes** (unchanged from 0061). Gear pane
   shows session inventory; banked footer stays 0 until 0063/Gate B.
5. **Enter-route key.** Local `E` is Sweep, so remote zone enter is **N**
   (`tin:1:0`). Walk onto stairs-up still extracts.

## Deviations

- Session tests record Monster/Swing/Drop via `record_world_ops` (same
  WorldView + FX as the window). The client scenario additionally presents
  through offscreen `paint_scene` so the GDI pipeline is machine-checked.
- Remote does not client-side-block movement on decorative scenery (server
  is authoritative).

## Risks / follow-ups

- Architect rescore of Gate A visual cohesion is the remaining acceptance
  item (target: no zeroes, ≥9/12).
- 0063 landing will make monster/loot placement exact; this task renders
  what the model has.
