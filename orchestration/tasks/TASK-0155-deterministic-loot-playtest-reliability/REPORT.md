# TASK-0155 REPORT

## Executive summary

The `loot` goal-harness scenario's second coin drop was load-sensitive: its
wait re-selected "nearest non-elite monster" on every poll, so a wandering
pack could make each teleport+swing pair land on a different member —
wounding many, killing none — until the deadline expired. Protected PR #56
CI run `32594398265` attempt 1 passed 31/32 scenarios but timed out after
52.657 s waiting for `a second coin drop` (authored 30 s × the existing 1.75×
load-mode cap); the same frozen tree passed its one bounded retry. That
evidence is preserved here as a load-sensitive scenario defect, not a
product-rule failure and not permission to hide or rerun failures.

The scenario now locks ONE explicit second victim by uuid before polling and
chases only that identity's current coordinates. The wait also refuses to
accept the first stack again (`uuid !== drop.uuid`), and an explicit
assertion requires the second drop's uuid to differ from the first. Authored
30 s waits, the 400 ms poll interval, and the global load-mode cap are
unchanged. Five focused real-server journeys and the complete 32-scenario
goal harness all exit 0.

## Approach

- Read the incident into the mechanism: `waitFor` deadlines are already
  load-adaptive (harness.mjs `adaptiveTimeoutMs`, max 1.75×), so the failure
  was not a too-short timeout; it was wasted polls attacking changing targets.
- Before the second-drop wait, take one fresh snapshot and retain one live
  non-elite monster (nearest at selection time) as `secondTarget` with an
  existence/live-hp precondition assert.
- Inside the poll: locate that same uuid in each fresh snapshot, reposition
  next to its CURRENT tile only when not adjacent, keep the real attack
  paired with every reposition (a dropped dev frame costs one poll, not the
  deadline), and never switch victims.
- Guard the drop match with `item.uuid !== drop.uuid` and assert
  `drop2.uuid !== drop.uuid` after resolution, so the scenario can never pass
  by accepting the first coin stack twice.
- No server, product, gameplay, or timing changes; no retries added; no
  assertion removed.

## Changed files

- `playtest/scenarios/loot.mjs` — deterministic second-target selection and
  first-stack exclusion (commit `3cae8a2ddba0b61ad7f42431d906f5b08e3e2e38`).
- `orchestration/tasks/TASK-0155-deterministic-loot-playtest-reliability/STATUS.md`
  and this REPORT (bookkeeping commit, SHA below).

## Public interfaces added/changed

None. Scenario-internal logic only; harness API and protocol untouched.

## Test commands + outcomes

All runs from `Z:\Code\.worktrees\verdigris\ox-pc-z` with
`$env:PLAYTEST_PORT='7100'` (reserved capsule 7100–7119, loopback only;
port 6500 never used). Runner boots its own hermetic dev server per run.

1. `npm run lint` → exit 0.
2. Five focused journeys:
   `1..5 | ForEach-Object { npm run playtest -- loot; if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE } }`
   → five exits 0. Scenario durations: 2996 ms, 8410 ms, 2576 ms, 8302 ms,
   2595 ms. Wall durations incl. server boot: 10.5 s, 10.9 s, 5.0 s, 10.7 s,
   5.2 s.
3. Complete goal harness `npm run playtest` → exit 0, **32/32 scenarios
   passed**, wall duration 190.0 s; `loot` passed in 1063 ms; timing
   diagnostics `{"loadMode":false,"p99EventLoopLagMs":32.194559,
   "maxEventLoopLagMs":103.350271}`.
4. `git diff --check` → exit 0 (no whitespace errors).
5. `git diff --name-only` → `docs/loop-journal.md`,
   `playtest/scenarios/loot.mjs` (see Deviations for the journal).

## Manual verification

Not applicable beyond the harness itself: the scenario exercises the real
server, a real living non-elite monster, real attack dispatch, the
guaranteed server coin drop, the real server-built context menu + Take for
the first stack, and the real underfoot pickup command for the second stack,
all unchanged.

## Commits

- Claim: `386aa2ee1005a0415355f7da4b91fdb6d14a2433`
- Implementation: `3cae8a2ddba0b61ad7f42431d906f5b08e3e2e38`
- Bookkeeping (REPORT + REVIEW_REQUESTED): see branch tip of
  `codex/TASK-0155-deterministic-loot-playtest-reliability-ox-pc-z`.

## Deviations

- None from the SPEC. Note: the mandated full-harness run caused the
  `session-arc` critic to append one telemetry row to
  `docs/loop-journal.md` (2026-08-22T20:25:02.166Z row). That file is
  outside owned paths; following TASK-0040/0042/0043 precedent it is left
  uncommitted as generated-by-the-gate dirty state and was not staged.

## Unresolved questions

None.

## Risks

- Residual load sensitivity lives in the shared harness/dev-frame bucket, not
  this scenario; if a future run still times out, the retained-identity
  design makes the surviving failure mode legible (one victim chased to its
  tile every poll).
- The first kill phase still selects nearest-at-poll-time; it starts from an
  untouched pack where any converging kill yields the guaranteed drop, and
  the incident evidence points at the second phase only. Left minimal-diff.

## Follow-ups

- Optional hardening task (outside this scope): give the first-phase chase
  the same identity-lock treatment, or expose a harness-level
  attack-by-uuid verb, if future CI evidence ever implicates phase one.
