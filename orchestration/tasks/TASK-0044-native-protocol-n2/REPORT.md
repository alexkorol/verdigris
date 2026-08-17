---
task: TASK-0044
state: CLAIMED
branch: codex/TASK-0044-native-protocol-n2
worker_worktree: C:\Users\Alex\Documents\KimiWork\verdigris
claim_commit: 6e6279c
base_commit: 32d7b6e
---

# TASK-0044 — Native protocol N2

## Executive summary

The task is actively in implementation by Kimi Code. The WIP ports the
unchanged harness's world, continuous movement, solo-instance entry, scene
metadata, monster population, and stair-return contracts into the native
simulation/networking layer. It is not ready for architect review: the source
is uncommitted and a newly added core test currently fails, although the
actual worker-built server passes the unchanged movement/zones attach suite.

## Implementation observed

The external WIP adds deterministic world state and adventure-zone metadata in
`native/include/verdigris/core.hpp` and `native/src/core.cpp`, extends the
native protocol session in `native/include/verdigris/networking.hpp` and
`native/src/networking.cpp`, and adds movement/zone tests in
`native/tests/networking_tests.cpp`. It maps `player:move`,
`instance:enterSolo`, `dev:teleport`, and `dev:state` without changing
`playtest/**`.

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

## Acceptance status

Not ready. The actual worker-built server and unchanged attach gate are green,
but the core test executable still exits 1 on an incorrectly authored
pre-entry-position assertion, and the WIP source/tests remain uncommitted.
Required evidence is still missing: a corrected worker-branch native gate with
exit status preserved, committed implementation, driven transcript, and
architect rerun. The coordinator has made no native source changes and has not
loosened the harness assertions.

## Next action

Kimi should capture the town position before `enter_solo_instance` in the core
test, rerun both native executables while preserving each exit status, commit
all WIP files, and attach the already-green worker-branch transcript. Only
then should this report move to `REVIEW_REQUESTED`.
