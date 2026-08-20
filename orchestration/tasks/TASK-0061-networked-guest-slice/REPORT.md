---
task: TASK-0061
state: REVIEW_REQUESTED
coordinator: cursor
worker_branch: codex/TASK-0061-networked-guest-slice-cursor
base_commit: 872bb94a4334d93ba597ca46c9ce9144cdd8e3f3
architect_review_required: true
---

# TASK-0061 REPORT — Networked native guest expedition (Gate A)

## Executive summary

Remote mode now plays the Gate A journey against `verdigris_server` with
**no in-process simulation**. `RemoteProtocolSession` consumes movement,
combat, inventory, and scene envelopes; `verdigris_client --remote` is a
session-only window; session tests drive the full slice on cursor ports
6580–6599 plus the authentic mid-session disconnect.

Owner path (after `powershell -File native/build.ps1`):

```
powershell -File orchestration/tasks/TASK-0061-networked-guest-slice/run-gate-a.ps1
```

Keys: E enter route · WASD move · mouse aim · click/space fight · X take ·
1–9 equip · walk the gold stairs pad to extract · Esc quit.

## Approach

- Extended `ClientModel` / `PresentationEvent` in place (no forked seam):
  ground/equipped/stairs/hit/kills/extracted, plus `Telegraph`.
- Remote submit maps Aim → local facing, UseAction → `player:skill:trigger`
  with that facing, Equip → `item:equip {item:{uuid}}` (the server reads
  nested `item.uuid`, not a top-level uuid), Extract → hint only (see gaps).
- `apply_envelope` handles `player:movement`, scene transitions,
  `monster:telegraph`, `combat:hit`, `core:refresh:inventory`,
  `player:session-replaced`, and the surface message as extraction.
- Reader-thread drop sets `peer_dropped_`; `poll()` emits `ConnectionLost`
  and stays disconnected — no local fallback.
- `native/client/remote_play.cpp` is the owner-visible remote window.
  Existing `--headless` / `--scenario` still use in-process Simulation.

## Changed files

- `native/client/remote_session.{hpp,cpp}` — journey envelopes, aim, disconnect.
- `native/client/client_model.hpp`, `presentation_events.hpp` — model/events.
- `native/client/remote_play.{hpp,cpp}` — remote Win32 client (no Simulation).
- `native/client/main.cpp` — `--remote [host] [port]` (default 127.0.0.1:6580).
- `native/tests/session_tests.cpp` — journey, mid-session kill, session-replaced.
- `native/build.ps1`, `native/CMakeLists.txt` — link remote play + ws2_32.
- `docs/rebuild/NATIVE_CLIENT_PROTOCOL_MATRIX.md` — 0061 rows filled.
- `orchestration/tasks/TASK-0061-networked-guest-slice/run-gate-a.ps1`

## Server gaps (no `server/**` / `native/src/networking.cpp` edits)

Per SPEC: file a note rather than change server rules.

1. **No `player:extract` handler.** Extract is walking onto stairs-up
   (`player:move`); the server already emits "The party returns to the surface."
2. **No ground-item envelope.** Kill loot is in-world only (`dev:state`
   snapshot). Client shows a sparkle on death; pickup names the item via
   `core:refresh:inventory`. Floor treasure at map centre also feeds pickup.
3. **Equip does not emit wear totals.** Authoritative change is the item
   leaving the backpack plus the identity stats already on that item.
4. **No House-bank envelope.** Returning to town keeps session inventory;
   HUD labels that "SURFACE/BANKED". Durable House banking is Gate B / N5.

## Test commands and outcomes

`powershell -File native/build.ps1 -RunTests` (2026-08-20 ~02:24 PDT):

```
native legacy denylist: PASS
verdigris core tests: PASS
verdigris networking tests: PASS
camera2d tests: PASS
… journey handshake / zone / move / aim / outgoing hit / kill / named pickup /
  equip / incoming hit / telegraph / extract / shutdown PASS
… mid-session server kill → Disconnected + ConnectionLost PASS
… commands after the drop do not revive a local sim PASS
… player:session-replaced ConnectionLost PASS
session tests passed
```

`powershell -File native/build.ps1 -RunClient`:

```
Verdigris native client shell
House: House Verdigris | trophies stored: 1 | items stored: 1
```

Authentic negative (session tests, cursor 6580–6599): server stopped while
Ready → `ConnectionLost`, `last_error` set, position does not keep moving.

## Deviations / risks

- Remote window is a dedicated presentation path, not a dual-write of the
  local D-119 painter. Architect play is `--remote` / `run-gate-a.ps1`.
- Quality rubric is for the architect play pass (no zeroes, ≥9/12).
- Item drop visibility depends on the sparkle+pickup path until the server
  emits ground items.
