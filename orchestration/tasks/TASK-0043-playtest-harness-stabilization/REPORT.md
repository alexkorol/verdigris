# TASK-0043 — Playtest harness stabilization

## Outcome

Implementation is ready for architect review. The final integrated tip is `7b81f874` (worker source commits through `78434f60` plus combat target stabilization `51c5253d`). Changes are confined to `playtest/**` and the task evidence; no `server/src`, native, protocol, package, or product files were changed.

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
- Task state: `REVIEW_REQUESTED`; architect acceptance is still required.
