# REVIEW — TASK-0200 house-first-investment-model

- reviewer: independent validator (claude subagent), judged and recorded by
  coordinator-of-day claude-architect-pc, 2026-08-24 ~08:20 PDT
- head reviewed: 2dd5b48a (worker branch
  codex/TASK-0200-house-first-investment-model-cursor; already an ancestor
  of the program branch via the prep-wave merge)
- verdict: **ACCEPTED, conditional on TASK-0177 acceptance** — STATUS flips
  to INTEGRATED only when TASK-0177 clears review and the id coupling below
  re-verifies.

## Evidence

- Harness PASS, exit 0: house_progression_tests 13/13 checks (validated
  independently from a detached review worktree; count re-derived from
  source). Matches REPORT.md claim.
- Scope clean: all files inside likely_paths/task folder; one advisory
  one-line WORKER_HEARTBEAT append to orchestration/.orch/events.ndjson
  (BUS heartbeat duty, not a violation).
- Frozen surfaces untouched; native boundary clean (pure constexpr header,
  sole include <cstdint>, deterministic).
- Content tie-in real: rhea-countinghouse / house-coffer exist in
  native/content/seeds/owner_demo_town.json:34,62.

## Condition

1. SPEC dependency gate: TASK-0177 must be ACCEPTED first (it is still
   REVIEW_REQUESTED). On 0177 acceptance, re-verify
   house_progression.hpp:37-38 id constants against owner_demo_town.json —
   a rename in 0177 review would silently strand them.

## Advisories (successor-note scale, not REVISE-blocking)

2. house_progression_tests.cpp:66-67 are tautological (constants compared
   to their own literals); they cannot catch drift against
   owner_demo_town.json. A successor should assert against the seed file.
3. Untested paths: InvestmentStatus::InvalidChoice (apply with Unchosen)
   and first_clear_eligible() (house_progression.hpp:67-71).
4. Four C4834 [[nodiscard]]-discard warnings in the test file
   (:41,:51,:62,:63); cosmetic, fix with (void) casts.
5. Acceptance of this model packet does NOT satisfy the parent outcome's
   integrated player-visible bar — TASK-0201 carries that.
