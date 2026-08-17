# TASK-0044 native protocol N2 baseline

Captured by the coordinator on 2026-08-17 09:49 -07:00 against the current
program tip. This is read-only evidence; it does not claim or modify Kimi's
TASK-0044 branch.

## Unchanged attach gate

The current native server starts on an alternate port, but the unchanged
movement and zones scenarios expose the N2 gap:

```powershell
native/build/verdigris_server.exe 6516
$env:PLAYTEST_WS_URL = 'ws://127.0.0.1:6516'
node playtest/run.mjs --attach movement zones
```

Result: **0/2 scenarios passed**.

- `movement`: timed out waiting for a continuous movement sample (8000ms)
- `zones`: timed out waiting for a zone transition to dungeon (8000ms)

The server listened and was stopped cleanly after the run. The prior N1
quickstart/single-session attach gate remains recorded in TASK-0039 as 2/2.

## Wire-contract findings

The unchanged harness sends these authoritative verbs:

- `player:login`
- `player:move` for continuous samples
- `instance:enterSolo` for the Adventure menu's zone entry
- `dev:state` for bounded state reads

The current native `ProtocolSession::handle` recognizes `player:login`,
`world:zone:enter`, `dev:give`, and `dev:state`; it does not recognize
`player:move` or `instance:enterSolo`. The N2 implementation therefore needs
to map the harness verbs without editing `playtest/**`.

The browser login/transition contract also carries more than the native N1
stub currently returns:

- login scene payload: map, NPCs, monsters, dropped items, scene name/type/id,
  seed, and safe metadata;
- transition payload: the full scene payload plus `playerState` (uuid,
  position, and scene id);
- `dev:state`: scene name/metadata, continuous position, and a populated
  monster list used by the zones assertions (including stairs metadata and
  layout).

The N2 spec permits a minimum zone/instance stub, but the stub must satisfy
the unchanged movement and `zones` assertions: fractional continuous motion,
instance scene identity, layout/stairs metadata, at least 15 monsters, and
return to the saved surface position.

No source, harness, product, or worker-owned files were changed for this
baseline.

## WIP implementation probe

Against Kimi's uncommitted four-file WIP (built in the Kimi clone on port
6517), the native build completed with the existing gates green:

- `native legacy denylist: PASS`
- `verdigris core tests: PASS`
- `verdigris networking tests: PASS`
- one non-fatal MSVC C4189 warning for an unused local in `networking.cpp`

The unchanged attach run then reached **1/2**:

- `movement`: PASS (4596ms), including an instance entry and movement inside
  the instance;
- `zones`: all six zone entries, layouts, stairs, names, and 18-monster
  populations passed, but the final stair return failed the saved-position
  assertion: `39.666666,116.333334` vs expected `6,22`.

This is WIP evidence, not acceptance. It identifies a remaining lifecycle
edge around the pre-instance position across sequential scenario connections;
the worker-owned source remains untouched by the coordinator.

## Latest WIP recheck

On the next read-only recheck of Kimi's still-uncommitted worktree, the WIP
also included new networking tests. `powershell -NoProfile -File
native/build.ps1 -RunTests` no longer reached the test gate: MSVC stopped in
`native/tests/networking_tests.cpp(74)` because the new `request_state` helper
passes a `JsonValue` to `parse_envelope`, whose current API requires an
`Envelope&`. The existing networking source still emitted the prior unused
`actor` warning at `networking.cpp(306)`. No coordinator or worker source was
changed; this is a precise WIP compile blocker for Kimi to resolve before the
attach probe can be rerun.
