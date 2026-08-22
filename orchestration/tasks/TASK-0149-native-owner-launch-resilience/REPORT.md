# REPORT — TASK-0149 native owner-launch resilience

Revision 2 (REVISE fix): adds the post-spawn readiness-failure orphan-leak
fix and the deterministic live negative control requested in
`REVIEW.md` (verdict REVISE against head
`96f4ccbd1572add96e34ccb230b9935b743d7ff3`, program review commit
`4400ea66`). Revision scope stayed inside the review's numbered item; all
accepted happy paths and prior controls are preserved.

## Revision 2 — leak fix and control

- `Start-OwnerServer` now owns cleanup for every exception after
  `Start-Process`: the whole post-spawn readiness sequence is wrapped so any
  failure path stops the spawned process before rethrowing, with a distinct
  message when the spawn already exited by itself.
- The spawned server PID is published to `$script:lastSpawnedServerPid`
  immediately after `Start-Process` and before every throwable readiness
  check, so callers and fault scenarios can always name the exact PID even
  though PowerShell never completes `$server = Start-OwnerServer ...` on a
  throw.
- New optional switch `-ReadinessFaultControl` deterministically replays both
  live post-spawn readiness failures without touching forbidden files:
  - `readiness-timeout`: impostor process (`powershell.exe -Command
    "Start-Sleep -Seconds 120"`) stays alive silently through the full 12s
    deadline;
  - `port-mismatch`: live impostor prints
    `verdigris_server listening on ws://127.0.0.1:6599` through the redirected
    stdout, tripping the port-match assertion while still alive.
  Each scenario requires the launcher to fail, requires a published PID > 0,
  and then proves that exact PID is gone.
- New guard combos fail fast: `-ReadinessFaultControl` refuses `-Local`,
  `-Port`, and pairing with `-LifecycleSelfTest`.

### Revision gate transcripts

New control (`powershell -NoProfile -ExecutionPolicy Bypass -File
native/tools/play-native.ps1 -ReadinessFaultControl`):

```text
play-native: fault-control readiness-timeout starting on port 6520 (12s readiness deadline against a live silent process)
play-native: starting verdigris_server on ws://127.0.0.1:6520 (capsule 6520-6539)
play-native: startup readiness failed; stopped spawned server pid 24496 to prevent an orphan
play-native: fault-control readiness-timeout observed the expected failure - play-native: verdigris_server (pid 24496) printed no listening line within 12s; see ...\faultctl-readiness-timeout-20260822-035656-759.log
play-native: fault-control readiness-timeout PASS (published pid 24496 is gone; no orphan)
play-native: fault-control port-mismatch starting on port 6520 (port-mismatch assertion against a live impostor process)
play-native: starting verdigris_server on ws://127.0.0.1:6520 (capsule 6520-6539)
play-native: startup readiness failed; stopped spawned server pid 13024 to prevent an orphan
play-native: fault-control port-mismatch observed the expected failure - play-native: verdigris_server reported port 6599 but the launcher chose 6520
play-native: fault-control port-mismatch PASS (published pid 13024 is gone; no orphan)
play-native: readiness fault control PASS (live post-spawn failures left no orphan server)
FAULT-CONTROL exit=0
```

Preserved negative controls (all exit 1):

```text
exit=1 for [-Port 6500]
exit=1 for [-Port 7000]
exit=1 for [-LifecycleSelfTest -Local]
exit=1 for [-LifecycleSelfTest -Port 6521]
```

New guard combos (all exit 1, messages verified):

```text
-LifecycleSelfTest -ReadinessFaultControl -> "...separate controls; run one at a time."
-ReadinessFaultControl -Local             -> "...drop -Local."
-ReadinessFaultControl -Port 6522         -> "...drop -Port."
```

Full native tests (`native/build.ps1 -RunTests`, final run):

```text
native legacy denylist: PASS
[core/networking/camera2d suites: PASS lines as in revision 1]
session tests passed
BUILD+TESTS exit=0
```

Reviewer's exact combo (`play-native.ps1 -Rebuild -LifecycleSelfTest`):

```text
play-native: building via native/build.ps1
native legacy denylist: PASS
play-native: selftest normal-close ... client pid 25344 (--remote 127.0.0.1 6520)
play-native: selftest normal-close normal close accepted (client pid 25344 exited by itself with code 0)
play-native: normal-close left no orphan verdigris processes (verified by pid)
play-native: selftest normal-close PASS (port 6520, ...)
play-native: selftest forced-exit ... client pid 21164 (--remote 127.0.0.1 6520)
play-native: selftest forced-exit forced client exit done (pid 21164 killed)
play-native: forced-exit left no orphan verdigris processes (verified by pid)
play-native: selftest forced-exit PASS (port 6520, ...)
play-native: lifecycle selftest PASS (normal close and forced client exit both cleaned up)
REBUILD+SELFTEST exit=0
```

`git diff --check` (revision diff): exit 0; changed file is exactly
`native/tools/play-native.ps1` (+88/-24 vs revision 1 head).

Flake note (unchanged sources): during this revision's first `-RunTests` run,
the same four timing-sensitive `reconnect:` checks failed exactly as during
revision 1, and an immediate rerun passed everything again (exit 0). Pattern:
fails only under back-to-back full-suite load, never in direct
`verdigris_session_tests.exe` runs or the reviewer's independent run. The
reconnect suite is in `native/tests/session_tests.cpp` / client session code,
untouched and unfixable from this task's owned paths.

---

# Revision 1 report

Worker: `ox-pc-j` (branch `codex/TASK-0149-native-owner-launch-resilience-ox-pc-j`)
Base: routed head `30e98e024d4a22a744be4bee63dfcf607f63010a`; immutable SPEC
base `060c11517d2ebb0aec0c4d4a38c5e3eb53141cb2` verified ancestor.
Machine: `DESKTOP-TVU7OR7`, Windows; provider `openrouter`, model
`stealth/ox-alpha`, OpenCode CLI 1.18.21.

## Executive summary

The one-command Windows owner launch (`native/tools/play-native.ps1`) now
fails fast before any side effect, explains its chosen port and log paths in
every message, verifies server readiness by port-matched log assertion, tracks
the exact PIDs it started, and proves after the client exits — by normal
close or by forced kill — that neither process survived, exiting non-zero on
any leak. A new `-LifecycleSelfTest` switch deterministically replays both
exit scenarios against the REAL windowed client (`VerdigrisNativeClient`
window class, found by PID+class enumeration, never a headless substitute).
Default remote mode, `-Local`, `-Rebuild`, and the 6520-6539 capsule are
preserved; port 6500 remains rejected everywhere.

## Approach

- Preflight (all before build/launch): explicit `-Port` must lie inside
  6520-6539 (6500 gets its own reserved-port explanation); `-LifecycleSelfTest`
  refuses `-Local` and explicit `-Port`; missing `build.ps1`, missing exes,
  busy explicit ports (with owning process names), and capsule exhaustion all
  produce actionable fail-fast messages with exit 1.
- Self-explanation: chosen port plus selection reason ("requested" vs
  "auto-selected free"), server endpoint, both log paths, server/client PIDs,
  client command line, client exit code, and a final summary line naming the
  chosen port and server log.
- Readiness: poll the server log for `listening on ws://127.0.0.1:<port>` for
  up to 12s; assert the printed port equals the chosen port; if the server
  dies during startup, surface its exit code and stderr tail.
- Lifecycle proof: teardown always force-stops the session's server and then
  asserts, by exact PID (never machine-wide name scans, which would false-
  positive on peer lanes), that no tracked verdigris_server/verdigris_client
  survives; leaks fail the run. Informational note lists other listeners
  inside the capsule without failing (owner may run parallel sessions).
- `-LifecycleSelfTest`: two scenarios, each with a fresh real server + real
  windowed remote client:
  - NORMAL-CLOSE: waits for a visible top-level window of class
    `VerdigrisNativeClient` owned by the client PID (`EnumWindows` +
    `GetClassName`; NOT `Process.MainWindowHandle`, which raced against the
    client's console window during development), posts WM_CLOSE (identical to
    an owner clicking X), requires self-exit within 10s AND exit code 0
    (graceful, not abrupt), then runs teardown + orphan assertion.
  - FORCED-EXIT: `Stop-Process -Force` the client PID, requires death within
    5s, then teardown + orphan assertion.
  Exit 0 only when both scenarios pass.

## Changed files

- `native/tools/play-native.ps1` (+282/-49) — the only non-task-folder change;
  inside `owned_paths`. No forbidden path touched: `git diff --stat` names
  exactly this file.

## Public interfaces added/changed

- New optional switch `-LifecycleSelfTest` on `play-native.ps1`.
- Existing parameters (`-Local`, `-Rebuild`, `-Port`) unchanged in meaning;
  `-Port` is now validated against the full 6520-6539 capsule instead of only
  6500. Documented README invocations behave identically apart from richer
  output and stronger guarantees. No game, client, or server code changed.

## Test commands and outcomes (literal transcripts)

All commands from repository root, `powershell -NoProfile -ExecutionPolicy
Bypass -File ...`.

### Gate A — native/build.ps1 -RunTests (final rerun at implementation head)

```text
native legacy denylist: PASS
verdigris core tests: PASS
verdigris networking tests: PASS
camera2d tests: PASS
PASS local: start succeeds ... [19 local/remote-negative/remote checks]
PASS journey: test server bound inside the cursor capsule 6580-6599 ...
[17 journey checks] ... PASS journey: clean dual-side shutdown
PASS reconnect: cursor-capsule server bound ... [11 reconnect checks]
PASS replaced: cursor-capsule server bound ... [10 replaced checks]
PASS render-list: cursor-capsule server bound ... [6 render-list checks]
session tests passed
BUILD+TESTS exit=0
```

Flake note: one earlier full-suite run failed 4 timing-sensitive
`reconnect:` checks while the machine was under load from repeated launch
runs; immediate direct rerun of `verdigris_session_tests.exe` passed
(exit 0) and the quoted full rerun above passed every check (exit 0). The
reconnect suite lives in `native/tests/session_tests.cpp` / client session
code — untouched by this task; the failure did not reproduce across three
other runs today (two pre-edit, one post-edit).

### Gate B — play-native.ps1 -LifecycleSelfTest (both cleanup proofs)

```text
play-native: selftest normal-close starting on port 6520 (log ...\selftest-normal-close-20260822-032859-254.log)
play-native: starting verdigris_server on ws://127.0.0.1:6520 (capsule 6520-6539)
play-native: server ready (pid 5600, stdout log ..., stderr log ...)
play-native: selftest normal-close client pid 21012 (--remote 127.0.0.1 6520)
play-native: selftest normal-close normal close accepted (client pid 21012 exited by itself with code 0)
play-native: normal-close left no orphan verdigris processes (verified by pid)
play-native: selftest normal-close PASS (port 6520, server log ...)
play-native: selftest forced-exit starting on port 6520 (log ...\selftest-forced-exit-20260822-032900-843.log)
play-native: starting verdigris_server on ws://127.0.0.1:6520 (capsule 6520-6539)
play-native: server ready (pid 20976, ...)
play-native: selftest forced-exit client pid 25456 (--remote 127.0.0.1 6520)
play-native: selftest forced-exit forced client exit done (pid 25456 killed)
play-native: forced-exit left no orphan verdigris processes (verified by pid)
play-native: selftest forced-exit PASS (port 6520, server log ...)
play-native: lifecycle selftest PASS (normal close and forced client exit both cleaned up)
SELFTEST exit=0
```

### Gate C — documented owner command with -Rebuild, owner-style normal close

Launched exactly as documented; an external driver posted WM_CLOSE to the
real `VerdigrisNativeClient` window once it appeared (equivalent to the owner
clicking X):

```text
DRIVER: launcher pid 21740 / real client pid 7908 appeared / VerdigrisNativeClient window handle 25168430 / WM_CLOSE posted
play-native: building via native/build.ps1
native legacy denylist: PASS
play-native: auto-selected free port 6520 (first free in 6520-6539)
play-native: starting verdigris_server on ws://127.0.0.1:6520 (capsule 6520-6539)
play-native: server ready (pid 25496, stdout log ...\server-6520-20260822-033552-896.log, stderr log ...)
play-native: starting real windowed client: verdigris_client --remote 127.0.0.1 6520
play-native: close the window or press Esc to quit
play-native: client pid 7908
play-native: client exit code 0
play-native: owner session left no orphan verdigris processes (verified by pid)
play-native: chosen port 6520; server log ...\server-6520-20260822-033552-896.log
DRIVER: leftover verdigris processes after run: 0
```

A second fresh-exe owner-path run bound the launcher's literal exit status:

```text
DRIVER3: WM_CLOSE posted to game window 5966692
DRIVER3: launcher exit code file: 0
play-native: client exit code 0
play-native: owner session left no orphan verdigris processes (verified by pid)
play-native: chosen port 6520; server log ...\server-6520-20260822-033701-145.log
DRIVER3: leftover verdigris processes after run: 0
```

### Negative controls (fail fast, before any build)

```text
exit=1 for [-Port 6500]        -> "port 6500 is reserved for the historical browser server..."
exit=1 for [-Port 7000]        -> "port 7000 is outside the owner-play capsule 6520-6539."
exit=1 for [-LifecycleSelfTest -Local]      -> "...drop -Local."
exit=1 for [-LifecycleSelfTest -Port 6521]  -> "...drop -Port."
```

### git diff --check

```text
$ git diff --check ; echo exit=$LASTEXITCODE
warning: LF will be replaced by CRLF in native/tools/play-native.ps1.
DIFF-CHECK exit=0
```

(Benign autocrlf warning only; no whitespace errors.)

## Manual verification

Gate C is the manual-equivalent owner path: real rebuild, real hidden server
on loopback 6520, real visible Win32 client window (class
`VerdigrisNativeClient`) connected via `--remote 127.0.0.1 6520`, closed the
way an owner closes it. The window was launched and observed on this desktop
session; no headless demo or scenario runner substitutes for it anywhere in
the acceptance evidence. Server logs under `native/build/logs/` retain each
run's `listening on ws://127.0.0.1:<port>` line.

## Commit SHAs

- Claim: `2d200041d1b583486a00a421bfd70c9a9d52a186` (STATUS-only, pushed)
- Implementation + report: see push of this branch (single commit on top of
  the claim).

## Deviations

- None from the SPEC. Interpretation note: "verifies cleanup after normal
  close and forced client exit" is implemented BOTH as always-on post-exit
  cleanup assertions in the default flow AND as the deterministic
  `-LifecycleSelfTest` replay of both scenarios; the reviewer can reproduce
  either path without manually closing windows.
- Environment repair recorded per protocol: first commit attempt hit the
  repo's yorkie pre-commit hook with no local `node_modules` (same activation
  fault RUN_STATUS documents for ox-pc-e). Repaired with `npm ci` (14s,
  ignored tree only); hooks stayed enabled; no config skipped.

## Unresolved questions

- None blocking. FYI for the architect: `Process.MainWindowHandle` is unsafe
  as a game-window locator for this client because the client also owns a
  `ConsoleWindowClass` window; class-targeted `EnumWindows` is the reliable
  seam. If a future wave renames the window class away from
  `VerdigrisNativeClient`, `-LifecycleSelfTest` will fail loudly rather than
  silently pass.

## Risks

- The reconnect session-test flake observed once under heavy load predates
  and is independent of this task, but reviewers running Gate A on a loaded
  machine may see it; rerun resolves.
- `-LifecycleSelfTest` briefly opens real client windows twice (~4s total);
  harmless but visible if the owner is actively using the desktop.

## Follow-ups

- Optional: fold `-LifecycleSelfTest` into a scheduled owner-health check or
  CI-visible smoke on a dedicated lane capsule.
