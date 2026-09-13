# REVIEW — TASK-0177 owner-demo-town-content

- reviewer: independent validator (claude subagent), judged and recorded by
  coordinator-of-day claude-architect-pc, 2026-08-24 ~08:55 PDT
- head reviewed: 3d1c66cf (single commit, branch
  codex/TASK-0177-owner-demo-town-content-cursor; already ancestor of the
  program branch)
- verdict: **ACCEPTED — INTEGRATED**

## Evidence

- Harness PASS, exit 0, reproduced from a detached review worktree:
  positive validator (town=verdigris-crossroads npcs=4 facilities=6
  exits=2), negative control (elder removed correctly fails), legacy
  denylist PASS. Zone cross-ref additionally exercised against the
  TASK-0178 blob at program tip: all to_zone refs resolve.
- Scope exact: native/content/seeds/owner_demo_town.json + task dir, all
  inside owned_paths. Frozen surfaces untouched; seed is purely
  declarative JSON.
- SPEC outcome fully met: elder, trainer, merchant, steward, all facility
  kinds, coherent crisis direction, two readable gate exits.
- Id coupling verified for TASK-0200: "rhea-countinghouse"
  (owner_demo_town.json:34,:59,:65) and "house-coffer" (:62) exactly match
  house_progression.hpp:37-38. TASK-0200's acceptance condition is
  satisfied.

## Successor concern (routed to TASK-0190, not a 0177 correction)

1. The coupling surface is wider than the two house ids:
   native/client/town_runtime_layout.hpp:146-170 hardcodes all four NPC
   tile positions and both gate tiles; native/content/
   cartographer_adapter.hpp:220-245 mirrors "verdigris-crossroads" and the
   crossroads gates. No build-time cross-check exists against the JSON —
   any future seed edit silently drifts three headers. TASK-0190's
   integration review MUST add or demand a cross-check.

## Minor non-blocking

2. Validator's validate_position returns True after recording coordinate
   errors (errors still surface; no acceptance hole).
3. Allowed-actions includes unused guidance/expedition entries.
4. Task-dir .gitignore lists C++ artifacts for a Python-only harness.
5. Process: STATUS flipped to REVIEW_REQUESTED without the frozen head SHA
   (recurring cursor lane-template gap; recorded in RUN_STATUS).
