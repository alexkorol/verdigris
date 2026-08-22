# REVIEW — TASK-0101 combat depth and feel gap audit

- verdict: REVISE
- reviewed frozen worker head: `7794883eb98f69eb1203d22221774b75fbaebb41`
- reviewed base: `610a240e1e4bdfacfd77bec49e36be945a1ced13`
- reviewer: PC architect/orchestrator
- reviewed_at: 2026-08-22T23:45:00Z

The audit is substantive, source-cited, contained to its owned task folder,
and its deterministic matrix parses. It is not yet accepted for routing.

## Required corrections

1. **Run the final diff gate over the committed artifact.** The reported
   `git diff --check` ran while the new evidence files were untracked and did
   not inspect them. Independent frozen-head review finds trailing whitespace
   in `FINDINGS.md` at the W2 and W4 owned-path lines. Remove it and record
   `git diff --check 610a240e..HEAD` exit 0 after the revision is committed.

2. **Do not route an invisible ranged-damage wave.** W1 owns only
   `native/src/core.cpp` plus core tests, while the existing remote client turns
   incoming `combat:hit` into a generic damage pulse and has no ranged attack
   telegraph/projectile vocabulary. That cannot support the claimed
   owner-visible outcome that composed packs “visibly mix” pressure roles.
   Either make the readable telegraph/presentation contract part of W1 with
   exact client/network paths and a deterministic client/session lock, or rank
   the telegraph-catalog/presentation packet ahead of ranged resolution and
   make it an explicit dependency. Do not invent projectile art, cadence,
   damage, or balance values.

3. Update `REPORT.md`, `STATUS.md`, and the JSON matrix so their acceptance
   claims and immediate successor order match the corrected frozen evidence.
   Re-run JSON parse, vocabulary grep, absence negative control, final committed
   diff check, and owned-path scope proof; then push a new
   `REVIEW_REQUESTED` head without amending or force-pushing.
