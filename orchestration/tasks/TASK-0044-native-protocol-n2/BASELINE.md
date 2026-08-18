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

## Disposable probe after test-helper correction

To separate the test-helper compile defect from the native implementation, the
coordinator created and then removed a disposable worktree from Kimi's WIP.
It copied the six uncommitted WIP files and changed only the test helper at
`native/tests/networking_tests.cpp:74` to parse an `Envelope` and return its
`data`; the Kimi worktree itself was not edited.

With that probe-only correction:

- `powershell -NoProfile -File native/build.ps1 -RunTests` passed the legacy
  denylist, core tests, and networking tests (the existing C4189/C4996 MSVC
  warnings remained non-fatal).
- An alternate server on port 6518 passed the unchanged attach command
  `node playtest/run.mjs --attach movement zones`: **2/2**.
- `movement` passed in 4589ms and `zones` passed in 1122ms, including all six
  zone names/layouts/stairs/populations and saved-position restoration
  (`38,115` versus `38,115`).

This is strong implementation evidence but not acceptance: the correction is
not yet in Kimi's branch, the prior direct WIP run had a different saved
position failure (`39.666666,116.333334` versus `6,22`), and the worker still
must rerun from its actual worktree, commit the source/tests, provide the
transcript, and obtain the architect rerun.

## Actual worker-branch recheck

Kimi subsequently applied the networking test-helper correction in the actual
worktree (still uncommitted). The native build now compiles, but the new core
test exposes a separate test defect:

- `native/build.ps1 -RunTests` prints `FAIL: N2 stair return restores the
  pre-entry position`, then runs networking tests and exits success because
  the later networking process masks the core test's nonzero exit.
- Direct executable checks show `verdigris_core_tests.exe` **EXIT=1** and
  `verdigris_networking_tests.exe` **EXIT=0**.
- The failing test captures `pre_entry` after `enter_solo_instance`, when the
  player is already at the instance spawn (`6,20`). The implementation
  restores the town position saved before entry (`38,115.333333`), so the
  test's expected value is wrong. A disposable test-only correction that
  captures the town position before entry makes both executables pass.

The actual worker-built server was then exercised on port 6519 with the
unchanged attach command. `movement` passed in 4599ms and `zones` passed in
1108ms; all six zones, metadata, stairs, populations, and saved-position
restoration passed (**2/2**). The full command/output is preserved in
`captures/worker-branch-recheck-2026-08-17.txt`.

This closes the earlier lifecycle uncertainty at the protocol level. The
remaining worker action is a test correction plus commit/evidence hygiene, not
a demonstrated native transport failure.

## Final worker-branch green recheck

Kimi corrected the core test in the actual WIP to capture the town position
before `enter_solo_instance`. The authoritative native command now passes all
three gates: legacy denylist, core tests, and networking tests. The rebuilt
actual worker server on port 6520 also passes the unchanged `movement` and
`zones` attach scenarios **2/2** (4593ms and 1110ms), including all six zone
combinations and saved-position restoration. The exact output is preserved in
`captures/worker-branch-green-2026-08-17.txt`.

The implementation is committed in Kimi's worktree as `d476788`, so this is
the final coordinator verification checkpoint, not architect acceptance.

The committed tip was additionally exercised with
`native/build.ps1 -RunTests -RunClient`: denylist, core/networking tests, and
the native client shell all passed (exit 0). See
`captures/worker-branch-client-gate-2026-08-17.txt`.

The committed N2 server also passed the prior N1 `quickstart` and
`single-session` scenarios alongside `movement` and `zones` (**4/4**),
confirming the session-replacement regression path remains green. See
`captures/worker-branch-regression-2026-08-17.txt`.
