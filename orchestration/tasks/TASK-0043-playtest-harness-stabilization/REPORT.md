---
task: TASK-0043
state: REVIEW_REQUESTED
branch: codex/TASK-0043-playtest-harness-stabilization
commits:
  - 3b16d01c
  - 18dabc06
  - 48b471b6
  - 31a93698
  - f3dc7ce1
  - e9474a0c
  - 121001b3
  - 1e48d120
  - 78434f60
  - 51c5253d
  - bf598d82
  - 9bbb3497
base_commit: f0df8de6
---

# TASK-0043 — Playtest harness stabilization

## Executive summary

Implementation is ready for architect review. The final integrated tip is `7b81f874` (worker source commits through `78434f60` plus combat target stabilization `51c5253d`). Changes are confined to `playtest/**` and the task evidence; no `server/src`, native, protocol, package, or product files were changed.

## Implementation

Load-sensitive setup, state probes, zone admission, death observation, gear/loot controls, and combat retries now use finite load-mode-aware windows. The combat scenario keeps the healer in the pack while targeting one stable non-support UUID, preserving the actual healer-race assertion instead of retargeting under scheduler pressure.

## Changed files

`playtest/README.md`, `playtest/harness.mjs`, `playtest/run.mjs`, `playtest/timing.mjs`, and the scenario files `build-divergence.mjs`, `combat.mjs`, `gear-outcomes.mjs`, `loot.mjs`, and `session-arc.mjs`. Evidence is under this task’s `captures/` directory.

## Interfaces

No product or server protocol interface changed. The harness adds/uses the `PLAYTEST_LOAD_MODE=1` execution mode and bounded timing helpers; normal authored-mode behavior remains unchanged.

## Acceptance evidence

### Ten-run loaded gate

The required full-worker CPU load was a PowerShell background job continuously computing `[Math]::Sqrt` for 250,000 iterations. With `PLAYTEST_LOAD_MODE=1`, ten full `npm run playtest --silent` runs were executed serially from the integrated tip. Every run exited 0 with 31/31 scenarios passed, for 310/310 scenario executions. Timings and diagnostics are in [`captures/full-load-ten-runs-2026-08-17.txt`](captures/full-load-ten-runs-2026-08-17.txt).

Run elapsed times: 191.7s, 191.9s, 187.4s, 188.5s, 183.9s, 187.0s, 193.1s, 187.0s, 187.3s, 188.8s. p99 event-loop lag stayed within 32.16–32.23ms; maximum observed lag was 95.42–114.88ms.

The final harness fix keeps combat retries on the originally selected UUID, excludes support/buffer casters from the kill target while retaining the healer in the pack, and tolerates only transient `dev:state` probe timeouts inside the existing bounded wait. Three focused loaded combat runs also passed before the full gate.

### Authentic negative regression

In a disposable worktree based on `7b81f874`, the real `instance:enterSolo` emission in `playtest/harness.mjs` was replaced with a no-op. Running `PLAYTEST_LOAD_MODE=1 npm run playtest --silent -- session-arc` then failed as expected:

```
FAIL session-arc: Timed out waiting for zone transition to dungeon (21000ms; authored 12000ms)
0/1 scenarios passed
exit code: 1
```

The scratch worktree and mutation were removed after capture. Evidence is also recorded in [`captures/negative-zone-entry-2026-08-17.txt`](captures/negative-zone-entry-2026-08-17.txt).

## Revision 1 — default-mode scheduler contention

Fable’s review found that the original ten-run proof covered only
`PLAYTEST_LOAD_MODE=1`; an architect-run default-mode suite still reached
30/31 under ambient contention. Commit `bf598d82` closes that exact gap:
`adaptiveTimeoutMs` now uses observed p99/max event-loop delay in default
mode, while retaining authored deadlines as the floor and the existing
1.75× cap. The explicit load-mode path and its evidence remain unchanged.

The final correction in `9bbb3497` makes the cap selection explicit: when
observed p99 or maximum delay reaches three times the 20ms baseline, default
mode uses the same existing 1.75× cap; lighter contention remains proportional.

With `PLAYTEST_LOAD_MODE` unset and one moderate CPU spinner worker:

- focused `session-arc`: PASS, 11.5s (p99 32.13ms, max 35.13ms);
- default full run 1: PASS 31/31 (max lag 113.18ms);
- default full run 2: PASS 31/31 (max lag 110.30ms);
- default full run 3: PASS 31/31 (max lag 112.00ms).

This revision preserves the authentic negative regression and all previous
loaded-mode evidence. The raw command output is committed at
[`playtest/TASK-0043-default-load-transcript.txt`](../../../../playtest/TASK-0043-default-load-transcript.txt).
The revised integration tip is `f9527d9c`.

### Coordinator rerun of the revision proof

After discovering the remote architect review was still `REVISE`, the
coordinator repeated the exact requested default-mode proof from `f9527d9c` in
a disposable worktree. With `PLAYTEST_LOAD_MODE` unset and one hidden Node CPU
spinner, three consecutive full runs passed 31/31 with runner exit 0. The
observed p99/max event-loop lag was 32.145/112.525ms, 32.162/110.166ms, and
32.195/97.583ms respectively. Raw evidence is in
[`captures/coordinator-default-moderate-reruns-2026-08-17.txt`](captures/coordinator-default-moderate-reruns-2026-08-17.txt).

This closes the coordinator-side evidence request; Fable's architect review
remains authoritative and still needs its own rerun/update.

## Manual checks

The final integration worktree was exercised against the unchanged WebSocket server with the full scenario suite under the documented CPU spinner. A disposable final-tip scratch worktree suppressed the real `instance:enterSolo` frame and confirmed the transition failure boundary.

## Specification deviations

None. The load-mode timing floors are test-harness-only and bounded; they do not loosen product assertions or production behavior.

## Risks and limitations

The full load proof is intentionally expensive (roughly three minutes per run). The browser server’s pre-existing generated `docs/loop-journal.md` remains dirty outside this task and was not staged. Architect hands-on play confirmation remains a separate D-115 obligation.

## Questions for Fable or the owner

None for implementation. Architect acceptance of the evidence package is required.

## Integration notes

Source is ready on the dedicated integration worktree at `f9527d9c`; integrate only after Fable writes an `ACCEPTED` review. The coordinator metadata/evidence commits are local (`a12d3895`, `a07e33b9`, `e1e2a758`, `2989c3a9`, `e68f408f`) and were not pushed.

## Scope and review notes

- Load-mode-only deadlines remain bounded and are documented in `playtest/README.md`; authored non-load deadlines and assertions remain intact.
- Zone admission and final-death observation now retry only within finite load-mode windows.
- Gear/loot controls use bounded retries; the combat target fix prevents full-suite scheduler churn from changing the regression target.
- No server-side behavior was altered to make the harness pass.
- Generated `docs/loop-journal.md` remains unrelated dirty state and was not staged.

## Coordinator checks

- `git diff --check` clean for task changes.
- Final full loaded gate: 10/10 runs, 31/31 each.
- Negative regression: failed at the intended suppressed-transition boundary.
- Revision raw transcript: three default-mode full runs under one moderate
  spinner, all 31/31; `loadMode:false` in each run.
- Independent validator verdict: **ACCEPT**. The validator confirmed the
  source diff is confined to `playtest/**`, ESLint passes on changed files,
  timing adaptation remains capped at 1.75×, authored assertions remain,
  and the exact 1.15 gear threshold is preserved.
- Final revision validator verdict: **ACCEPT**. The validator confirmed the
  one-spinner label matches all three raw transcript headers and found no
  remaining source, cap, scope, or evidence defects.
- Coordinator current-tip health check: `npm run test:unit` passed 122/122
  files and 779/779 tests; `npm run build` passed; and a hermetic
  `PLAYTEST_PORT=6514 npm run playtest` passed all 31/31 scenarios, including
  the formerly flaky session-arc and zones scenarios.
- Fresh coordinator rerun at 2026-08-17 09:39 -07:00 independently reproduced
  the same result: `npm run test:unit` 122/122 files and 779/779 tests,
  `npm run build` PASS, native `build.ps1 -RunTests -RunClient` PASS, and
  `PLAYTEST_PORT=6514 npm run playtest` 31/31. The rerun included
  `gear-outcomes` (11.93s -> 9.52s) and `session-arc` (critic 80), with no
  scenario failures.

- Fresh isolated coordinator current-tip rerun later on 2026-08-17 used
  `PLAYTEST_PORT=6523 npm run playtest`: `npm run test:unit` passed 122/122
  files and 779/779 tests, `npm run build` passed, and the full playtest passed
  31/31. The raw summary is preserved in
  [`captures/coordinator-current-tip-2026-08-17.txt`](captures/coordinator-current-tip-2026-08-17.txt).

- The freshly built coordinator tip also passed the browser-critical Playwright
  loop 1/1 against an isolated `NODE_ENV=development PORT=6524` server. The
  server was stopped cleanly; raw command evidence is in
  [`captures/coordinator-browser-smoke-2026-08-17.txt`](captures/coordinator-browser-smoke-2026-08-17.txt).
- Isolated browser-critical smoke at 2026-08-17 09:45 -07:00 also passed
  1/1 against an explicit `NODE_ENV=development PORT=6515` server with
  `PLAYWRIGHT_BASE_URL=http://127.0.0.1:6515`; the server was stopped cleanly
  afterward. The explicit environment is required because `/world/players` is
  intentionally development-only and production fails closed.
- Fresh coordinator rerun at tip `9ee547a2` then completed unchanged
  `npm run playtest` **31/31**, including `build-divergence`,
  `gear-outcomes`, `quest`, `session-arc`, and `zones`; raw output summary is
  preserved in [`captures/coordinator-fresh-full-playtest-2026-08-17.txt`](captures/coordinator-fresh-full-playtest-2026-08-17.txt).

- Current coordinator checkout follow-up at `cccca42c` passed unit (122/779),
  build, and the native full gate. Its full browser run was 30/31 because
  `build-divergence` missed one primary-attack hit at the 8s authored wait;
  the immediate isolated rerun passed 1/1 in 3.27s. This confirms the known
  scheduler-sensitive variance and is recorded in
  [`captures/coordinator-current-tip-playtest-2026-08-17-followup.txt`](captures/coordinator-current-tip-playtest-2026-08-17-followup.txt).
  The checkout does not contain the unaccepted `f9527d9c` harness source.

- A disposable integration dry-run then applied the complete TASK-0043
  correction chain to coordinator tip `ade29e6d` without conflicts. Candidate
  `1a9f6b6b` passed unit (122/779), full playtest (31/31), and diff-check with
  playtest-only scope. The exact cherry-pick list and diagnostics are in
  [`captures/coordinator-current-tip-integration-candidate-2026-08-17.txt`](captures/coordinator-current-tip-integration-candidate-2026-08-17.txt).
  This candidate is ready for Fable's architect rerun but remains unmerged
  until the review verdict changes.
- Task state: `REVIEW_REQUESTED`; architect acceptance is still required.
