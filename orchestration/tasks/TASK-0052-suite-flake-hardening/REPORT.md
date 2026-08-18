---
task: TASK-0052
state: REVIEW_REQUESTED
worker_commits:
  - d0ef8a6
base_commit: cc67a15
---

## Summary

Hardened the two ambient-load-flaky scenarios exactly per the MECHANICAL
packet. No assertion weakened or removed; only fixed-instant reads became
bounded, load-adaptive waits using the exported `playtest/timing.mjs`
helpers. `playtest/harness.mjs` and `playtest/timing.mjs` untouched.

## Changes

- `playtest/scenarios/first-goal.mjs` — the objective-naming wait now
  follows 0043's zone-admission pattern: the (idempotent — see
  `server/core/first-goal.js:44`) Talk choice is resent at >=1s intervals
  inside one bounded `waitFor`, and under `PLAYTEST_LOAD_MODE` the authored
  floor rises from 8s to 12s (21s effective after the existing 1.75x cap).
  Ordinary runs keep the original deadline.
- `playtest/scenarios/house-treasury.mjs` — a bounded wait for the starting
  road purse (>=100 carried gold) now stands in front of the unchanged
  carried-gold assert; same 12s load-gate floor. The assert text and every
  downstream check (deposit math, persistence, relog) are untouched.

## Evidence

- Solo, default mode: `npm run playtest -- first-goal house-treasury` —
  2/2 PASS (1460ms / 636ms). Transcript: `captures/solo.txt`.
- Negative proofs (scratch copies, never committed to `scenarios/`):
  - `_neg-first-goal.mjs` suppresses the objective push (Talk never sent)
    -> FAILS: "Timed out waiting for Aldwyn names the first-Warden
    objective (8346ms)".
  - `_neg-house-treasury.mjs` suppresses the road purse (requires a
    1e9-gold purse that can never land) -> FAILS: "Timed out waiting for
    starting road purse to land (8304ms)".
  Both fail bounded, proving the waits/asserts still require the real
  server events. Transcript: `captures/negatives.txt`; scratch sources
  kept beside it for reruns.
- Loaded full suite (0043's spinner method: one PowerShell worker
  continuously computing 250k-iteration `[Math]::Sqrt` batches,
  `PLAYTEST_LOAD_MODE` unset, default mode): **first-goal PASS (1747ms)
  and house-treasury PASS (628ms)**; suite 31/32, the one failure being
  `loot` ("second coin drop", 30363ms vs 30000ms authored) — the same
  marginal-timeout flake class in a scenario outside this spec's scope
  (see Risks). Transcript: `captures/full-loaded.txt`.
- Default full suite, no load: `npm run playtest` — 32/32 PASS.
  Transcript: `captures/full-default.txt`.
- ESLint on both scenario files — clean.

## Deviations

None. Owned paths only; forbidden files untouched (`git diff --stat`
shows only the two scenario files plus this task folder).

## Risks / follow-ups

- The zones/respawn marginal timeouts observed during TASK-0042
  verification (8.05s vs 8s authored zone transitions) and the loaded-run
  `loot` failure here (30363ms vs 30000ms authored) are the same flake
  CLASS but were not in this spec's scope; if they recur at the WATCH
  threshold (3 sightings), the same pattern applies — `enterZone` already
  carries the 0043 load floor, so the fix would be raising that floor or
  the transition wait inside it (harness.mjs — would need its own spec
  since 0043's guard is settled).
