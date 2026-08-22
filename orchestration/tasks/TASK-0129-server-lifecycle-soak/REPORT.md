# TASK-0129 report — native WebSocket server lifecycle soak

## Executive summary

Implemented the READY packet exactly within its owned paths: a new
`-RunServerLifecycleSoak` gate on `native/build.ps1` builds and runs
`native/tools/server_lifecycle_soak.cpp`, which drives the REAL
`verdigris::networking::WebSocketServer` through 100 sequential
start/connect/login/close/stop cycles on loopback ports 6680-6699 plus an
eight-client connect/login/close burst before a final stop. Both independent
gate invocations completed 100/100 cycles with 100 upgrades, 100 login
acknowledgments, 100 clean closes, and an 8/8 burst; both JSON captures are
preserved. A negative control (capsule fully occupied) exits 1 with 100
recorded failures. No server or client behavior changed.

## Approach

- The soak uses only public interfaces: `WebSocketServer(port)/start/stop`
  and the existing client seam `RemoteProtocolSession` (the same transport
  the session tests use) for TCP connect, RFC6455 upgrade, `player:login`,
  ready acknowledgment, and close-frame shutdown.
- Port allocation rotates through this lane's capsule 6680-6699 and proves
  each bind against the real listener (parallel-suite collision safe). Port
  6500 is never touched; all binds are 127.0.0.1 (server-side invariant).
- Per-cycle phases are independently counted: `server-start`, `client-upgrade`,
  `client-login` (bounded wait, 5 s), `client-close` (shutdown reaches
  Disconnected), and a timed `server->stop()` per cycle (the PR #46
  reader-thread join path is exercised 101 times per run).
- The process writes timestamped JSON (counts, stop durations, total
  duration, failure details) and exits non-zero on any failed cycle or
  timeout; a 15-minute detached watchdog converts a hang into exit code 3.
- `build.ps1` links the tool against the proven session-seam object set
  (remote_session/local_session/presentation_state + networking/core/seasonal,
  ws2_32) and writes each invocation's JSON to this task's `captures/` folder
  so the two literal gate invocations preserve independent evidence.

## Changed files

- `native/tools/server_lifecycle_soak.cpp` (new; owned)
- `native/build.ps1` (added `-RunServerLifecycleSoak` param + gate block; owned)
- `orchestration/tasks/TASK-0129-server-lifecycle-soak/STATUS.md`, `REPORT.md`,
  `captures/*.json` (owned)

## Public interfaces added/changed

- Build interface: new opt-in switch `native/build.ps1 -RunServerLifecycleSoak`.
- New tool `native/build/server_lifecycle_soak.exe` with `--out <path>`.
- No changes to any header, server, client, test, or wire behavior.
  `git diff 0d40d79d..HEAD` touches only owned paths (verified below).

## Acceptance commands — literal transcripts and exit codes

1. `powershell -File native/build.ps1 -RunTests -RunClientScenarios`
   - Exit code: 0
   - Tail: `native legacy denylist: PASS` / `verdigris core tests: PASS` /
     `verdigris networking tests: PASS` / `camera2d tests: PASS` /
     `session tests passed` / all seven client scenarios `PASS (0 failures)`
     (full transcript captured in session log; every suite green).

2. `powershell -File native/build.ps1 -RunServerLifecycleSoak` (first run)
   - Exit code: 0
   - Tail:
     `[soak] burst port=6680 ok (upgrades 8/8, logins 8/8, clean closes 8/8, stop 149.0 ms)`
     `SOAK RESULT: PASS — cycles 100/100, upgrades 100, logins 100, clean closes 100, burst upgrades 8/8, burst logins 8/8, burst closes 8/8, total 15764 ms`
   - JSON preserved:
     `orchestration/tasks/TASK-0129-server-lifecycle-soak/captures/lifecycle-soak-20260821-224506.json`
     (passed=true, 100/100, upgrades=100, logins=100, closes=100,
     burst 8/8/8, stopDurationMaxMs=155, totalDurationMs=15764)

3. `powershell -File native/build.ps1 -RunServerLifecycleSoak` (second run)
   - Exit code: 0
   - Tail: `SOAK RESULT: PASS — cycles 100/100, upgrades 100, logins 100, clean closes 100, burst upgrades 8/8, burst logins 8/8, burst closes 8/8, total 15809 ms`
   - JSON preserved:
     `orchestration/tasks/TASK-0129-server-lifecycle-soak/captures/lifecycle-soak-20260821-224623.json`
     (passed=true, 100/100, burst 8/8/8, stopDurationMaxMs=143,
     totalDurationMs=15809)

4. `git diff --check`
   - Exit code: 0 (no output; whitespace clean)

## Negative control (G2)

Occupied all 20 capsule ports (127.0.0.1 TcpListener on 6680-6699), ran the
built soak exe directly: every cycle fails at `server-start` with
`no free port in capsule 6680-6699 (bind/listen failed)`, `passed=false`,
`failures=100`, process exit code 1. Listeners released afterwards; ports
verified free again by the passing gates above (which ran before this
control). Evidence: `captures/negative-capsule-occupied.json`.

## Ownership boundary proof

`git diff --name-only 0d40d79db80c53280bb7cfe6f42318b39dab6f4c HEAD` at
implementation head returns only:
`orchestration/tasks/TASK-0129-server-lifecycle-soak/STATUS.md` (claim commit)
plus this commit's owned files (`native/tools/server_lifecycle_soak.cpp`,
`native/build.ps1`, `captures/*`). No forbidden path appears in any commit on
this branch (forbidden: `native/client/**`, `native/src/**`,
`native/include/**`, `native/tests/**`, `playtest/**`, `server/**`).

## Commit SHAs

- Claim (STATUS.md only): `a5bd928e`
- Implementation (tool + build gate + captures): `d5cfec59`
- REVIEW_REQUESTED transition: this commit (STATUS.md + REPORT.md)

## Manual verification

- Confirmed both JSON captures parse and carry `passed=true`, 100/100 cycles,
  100 upgrades/logins/clean closes, and burst 8/8/8 (ConvertFrom-Json probe).
- Confirmed the soak drives the real server: cycle logs show rotating capsule
  ports 6680→6699, per-cycle stop durations ~140 ms (accept/tick/reader join
  path), burst reuses port 6680 after its cycle-1 release.
- Confirmed no process leaks: repeated runs rebind cleanly (SO_REUSEADDR +
  full stop joins), and the negative control's exit code is 1.

## Deviations

- None from the SPEC. The soak reuses the existing `RemoteProtocolSession`
  client seam rather than hand-rolling a socket client — the SPEC requires
  exercising the real `WebSocketServer` (it does, end to end) and authorizes
  no client changes (none made; the seam is consumed, not modified).
- Variant/reasoning for the experimental unit was not observable in-session;
  omitted per the launch packet ("variant only if observed").

## Unresolved questions

- None.

## Risks and follow-ups

- The soak binds only inside 6680-6699; if a peer lane ever squats the whole
  capsule the gate fails closed (by design, proven by the negative control).
- Watchdog hard cap is 15 minutes per invocation; typical run is ~16 s.
- Future work (not this packet): wire `-RunServerLifecycleSoak` into
  `native/tools/ci-native.ps1` if the controller wants it on CI; that file is
  outside this task's ownership.
