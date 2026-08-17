---
task: TASK-0044
state: REVIEW_REQUESTED
branch: codex/TASK-0044-native-protocol-n2
worker_worktree: C:\Users\Alex\Documents\KimiWork\verdigris
claim_commit: 6e6279c
implementation_commit: d476788
base_commit: 32d7b6e
---

# TASK-0044 — Native protocol N2

## Executive summary

The task is ready for architect review. Kimi Code committed the implementation
as `d476788`; it ports the unchanged harness's world, continuous movement,
solo-instance entry, scene metadata, monster population, and stair-return
contracts into the native simulation/networking layer. The committed tip
passes the native gates and unchanged movement/zones attach suite.

## Implementation observed

The external WIP adds deterministic world state and adventure-zone metadata in
`native/include/verdigris/core.hpp` and `native/src/core.cpp`, extends the
native protocol session in `native/include/verdigris/networking.hpp` and
`native/src/networking.cpp`, and adds movement/zone tests in
`native/tests/networking_tests.cpp`. It maps `player:move`,
`instance:enterSolo`, `dev:teleport`, and `dev:state` without changing
`playtest/**`.

## Scope audit

The implementation commit `d476788` changes only the six allowed native paths:
`native/include/verdigris/core.hpp`, `native/include/verdigris/networking.hpp`,
`native/src/core.cpp`, `native/src/networking.cpp`,
`native/tests/core_tests.cpp`, and `native/tests/networking_tests.cpp`.
The separate claim commit contains the worker's task `STATUS.md` metadata;
no client, browser, server, harness, package, or product paths are part of the
implementation commit.

## Verification so far

The first WIP build passed the native denylist, core tests, and networking
tests. Against an alternate native server, unchanged `movement` passed in
4.596 seconds; `zones` reached all six zone entries and passed names, layouts,
stairs, and monster populations, but failed the final saved-position check
(`39.666666,116.333334` versus `6,22`). Full details are in `BASELINE.md`.

The earlier recheck stopped at `native/tests/networking_tests.cpp:74` because
the new `request_state` helper passed a `JsonValue` to `parse_envelope`, whose
signature requires an `Envelope&`. That helper is now corrected in the actual
WIP. The next worker-branch build compiles, but direct execution shows a
different failure: `verdigris_core_tests.exe` exits 1 on the new
`N2 stair return restores the pre-entry position` test, while
`verdigris_networking_tests.exe` exits 0. The PowerShell build script masks
the first exit code because it invokes the networking test afterward. The test
captures its expected position after instance entry (the spawn), while the
implementation correctly restores the town position captured before entry.
This is a test-fixture correction, not a transport failure.

## Disposable probe result

The coordinator verified that the compile failure is isolated from the native
WIP by using a disposable worktree. It copied the six current uncommitted WIP
files and made only the test-helper correction (`Envelope response; return
response.data`). The disposable probe then passed the native denylist, core
tests, and networking tests, followed by the unchanged attach suite at an
alternate port: `movement` PASS (4589ms), `zones` PASS (1122ms), **2/2**.
The zones run covered all six authored zone combinations and restored the
pre-entry position (`38,115` versus `38,115`). The probe was removed after the
run, and Kimi's source remains untouched.
The concise command transcript is preserved in
[`captures/disposable-probe-2026-08-17.txt`](captures/disposable-probe-2026-08-17.txt).

## Actual worker-branch attach recheck

Against the actual Kimi-built server on port 6519, the unchanged attach
command passed **2/2**: `movement` in 4599ms and `zones` in 1108ms. All six
zone combinations, names, layouts, stairs, 18-monster populations, and saved
position restoration passed. The transcript is in
[`captures/worker-branch-recheck-2026-08-17.txt`](captures/worker-branch-recheck-2026-08-17.txt).

After the core-test correction, the authoritative worker-branch native gate
was rerun and passed: legacy denylist, core tests, and networking tests. A
fresh worker-built server on port 6520 then passed unchanged `movement` in
4593ms and `zones` in 1110ms (**2/2**), including saved-position restoration.
The final checkpoint is preserved in
[`captures/worker-branch-green-2026-08-17.txt`](captures/worker-branch-green-2026-08-17.txt).

The broader committed-tip command `native/build.ps1 -RunTests -RunClient` also
passes denylist, core tests, networking tests, and the native client shell
(including the stored trophy/item smoke output). Evidence is in
[`captures/worker-branch-client-gate-2026-08-17.txt`](captures/worker-branch-client-gate-2026-08-17.txt).

As a regression check, the committed N2 server passed the prior N1
`quickstart` and `single-session` scenarios together with `movement` and
`zones`: **4/4**. The session-replacement/loot handoff contract remains green;
the transcript is in
[`captures/worker-branch-regression-2026-08-17.txt`](captures/worker-branch-regression-2026-08-17.txt).

## Coordinator recheck — 2026-08-17

From the clean external Kimi worktree at `d476788`, the coordinator reran:

```
powershell -NoProfile -File native/build.ps1 -RunTests -RunClient
```

The committed tip again passed the legacy denylist, core tests, networking
tests, and native client shell. The shell ended with the expected
`House: House Verdigris | trophies stored: 1 | items stored: 1` line. This was
a source-preserving build recheck; no claim is made here beyond the previously
captured unchanged movement/zones and N1 regression attach runs.

## Coordinator attach recheck — 2026-08-17

The coordinator launched the clean native server from `d476788` on port 6526
and ran the unchanged harness against it:

```
$env:PLAYTEST_WS_URL='ws://127.0.0.1:6526'
node playtest/run.mjs --attach quickstart single-session movement zones
```

All four scenarios passed: movement (4588ms), quickstart (162ms),
single-session (315ms), and zones (1108ms). The zones scenario exercised all
six authored zone/layout combinations and restored the pre-entry position
(`38,115` vs `38,115`). The server was stopped cleanly after the run. Raw
output is preserved in
[`captures/coordinator-native-attach-2026-08-17.txt`](captures/coordinator-native-attach-2026-08-17.txt).

## Coordinator dual-run matrix — 2026-08-17

The same unchanged scenario set was run against the JavaScript reference on
port 6527 and the native server at `d476788` on port 6528. Both passed the
contract matrix **4/4** (`quickstart`, `single-session`, `movement`, `zones`).
The full comparison is preserved in
[`captures/coordinator-dual-run-matrix-2026-08-17.txt`](captures/coordinator-dual-run-matrix-2026-08-17.txt).

The run also makes the documented N2 boundary concrete: JS authored zone
populations were 33/56/31/46/57/54, while native returned 18 in each zone.
The N2 specification explicitly permits a minimum scenario stub; this is not
claimed as population parity. Exact authored composition and counts remain a
later N3+ parity obligation.

## Coordinator N3 surface audit — 2026-08-17

The coordinator verified the N3 handoff against the live browser reference and
native headers without changing implementation. The cited inbound/outbound
browser events are present at the named handler/combat seams, including
`player:move`, `player:skill:trigger`, `combat:hit`, `world:skill:effect`,
`world:projectile`, `player:stats:update`, `monster:telegraph`, and
`game:send:message`. The native `ActionType`, `EventType`, and
`Command::{action_use,pick_up,equip}` hooks are also present. The exact
line-anchored audit is preserved in
[`coordinator-n3-surface-audit-2026-08-17.txt`](captures/coordinator-n3-surface-audit-2026-08-17.txt).
This is evidence for the next task, not an N3 implementation claim; native
combat remains gated on a Fable-issued READY N3 spec.

## Coordinator N3 core-gap audit — 2026-08-17

A second read-only inspection of `d476788` confirms the principal seam: the
protocol maps movement only into `WorldSimulation`, while deterministic combat
already lives in a separate `Simulation`; no UUID/position bridge exists. The
N2 roster is 18 shallow `<theme>-lurker` records, the snapshot has empty ground
loot arrays, and `dev:give` does not reach simulation inventory. Core event
metadata also needs source/skill/killer fields for lossless `combat:hit`
projection, and the unchanged combat scenario's `dev:setlevel`/`dev:heal`
commands are not handled. Full line-oriented findings and reusable hooks are
preserved in
[`coordinator-n3-core-gap-audit-2026-08-17.txt`](captures/coordinator-n3-core-gap-audit-2026-08-17.txt).
These findings constrain the future N3 spec; they do not authorize source
changes in TASK-0044.

## N3 negative baseline — 2026-08-17

To make the next wave’s entry criteria executable, the coordinator ran the
unchanged `combat` and `encounter-variety` scenarios against native `d476788`.
Both failed at the expected N2 boundary: `combat` observed 18 monsters where
the reference scenario requires at least 20, and `encounter-variety` lacked the
authored melee/ranged/buffer pack roles. The exact negative transcript is
[`coordinator-n3-baseline-2026-08-17.txt`](captures/coordinator-n3-baseline-2026-08-17.txt).
No N2 source was changed.

## Integration dry-run

A disposable merge of `origin/codex/TASK-0044-native-protocol-n2` into the
current coordinator tip produced one add/add conflict only:
`orchestration/tasks/TASK-0044-native-protocol-n2/STATUS.md`. The six native
implementation files applied cleanly; the conflict is expected coordinator
metadata ownership and was aborted without changing either branch. On
acceptance, preserve the coordinator's `STATUS.md`/`REPORT.md` and merge only
the implementation commit `d476788` plus its accepted native files.

## Acceptance status

The committed worker-branch implementation and unchanged attach gate are
green. Acceptance is still pending only because Fable has not performed the
required architect rerun and written the review decision. The coordinator has
made no native source changes and has not loosened the harness assertions.

## Coordinator current-tip baseline (not an implementation result)

For clarity, the coordinator also ran the unchanged attach matrix against
HEAD `8385ef09`, before integrating the review-requested worker commit
`d476788`. The ancestry check returned false, and the expected pre-N2 result
was **2/4**: `quickstart` and `single-session` passed, while `movement` and
`zones` timed out. The exact transcript is in
[`captures/coordinator-current-tip-attach-baseline-2026-08-17.txt`](captures/coordinator-current-tip-attach-baseline-2026-08-17.txt).
This confirms why the disposable `d476788` candidate must not be described as
already shipped; it remains pending Fable's architect rerun and acceptance.

## Fresh current-tip candidate

A fresh disposable candidate `f602dab4` cherry-picked `d476788` onto current
coordinator tip `27db1611`. The native denylist/core/networking/client gate
passed, including the literal `trophies stored: 1 | items stored: 1`
transcript. The unchanged attach matrix then passed **4/4**:
`quickstart`, `single-session`, `movement`, and `zones`. The disposable
worktree and server were removed after the run. Full output is in
[`captures/coordinator-current-tip-n2-candidate-2026-08-17.txt`](captures/coordinator-current-tip-n2-candidate-2026-08-17.txt).
This is integration-readiness evidence only; Fable's architect rerun and
acceptance remain outstanding.

## Coordinator current-tip integration candidate

The implementation commit `d476788` was cherry-picked cleanly onto the
coordinator tip in disposable candidate `8be3dacd`. The candidate passed the
full native gate and the unchanged `quickstart`, `single-session`, `movement`,
and `zones` attach matrix 4/4. The exact scope and transcript are preserved in
[`captures/coordinator-current-tip-integration-candidate-2026-08-17.txt`](captures/coordinator-current-tip-integration-candidate-2026-08-17.txt).
This is a review handoff, not an acceptance or merge.

## Next action

Fable should rerun the committed tip `d476788` against the unchanged
movement/zones scenarios and record the architect decision. No additional
native behavior change is indicated by the current evidence.
