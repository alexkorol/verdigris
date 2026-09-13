# REVIEW — TASK-0173 native-actor-animation-model

- reviewer: independent validator (claude subagent), judged and recorded by
  coordinator-of-day claude-architect-pc, 2026-08-24 ~09:35 PDT
- head reviewed: f6a40028 (STATUS-declared implementation_commit, tip of
  codex/TASK-0173-native-actor-animation-model-cursor; already ancestor of
  the program branch via prep-wave merge 5962acde; program-tip file
  byte-identical)
- verdict: **ACCEPTED — INTEGRATED**

## Evidence

- Harness reproduced from detached review worktree: 80 checks PASS, MSVC
  /W4 clean, denylist PASS, git diff --check clean.
- Ownership resolved: actor_animation.hpp is THIS task's deliverable;
  TASK-0186 (AUTO_RELEASE, depends on 0173 ACCEPTED, owns main.cpp) is the
  consumer.
- Determinism verified: header-only pure constexpr over integer
  State+Timing, <cstdint> only; tick() pure in (State, Timing); fixed-step
  friendly. SPEC states delivered verbatim; hit interrupts windup; death
  terminal; attacks visibly non-zero even under degenerate timings.
- Scope exact: 7 files, all owned; frozen surfaces untouched by
  construction (empty diff over native/src|include, server, src).
- Duplicate-claim branch ox/TASK-0173 (c093d278, unrelated 599-line
  implementation) SUPERSEDED per BUS.md — never reviewed, prune with the
  next worktree cleanup.

## Handoff guidance for TASK-0186 (record in its review checklist)

1. Vocabulary mapping is 0186's burden: sim ActionType {Melee, Dash, Wait,
   Thrust, Sweep, WarCry} (core.hpp:27) vs model AttackKind {Swing,
   Thrust, Slam}. Melee->Swing / Sweep->Slam is implied, stated nowhere;
   Dash/WarCry have no phase; sim has no hit-react state so
   DamageApplied/ActorDied events must drive TakeHit/Die.
2. Silent-Ok wart: Intent::Move during Windup/Hit/Recovery returns Ok as a
   pure no-op without updating facing (actor_animation.hpp:192-200) —
   0186 cannot distinguish "moved" from "ignored"; document or split the
   status.

## Non-blocking notes

3. Test-matrix gaps: hit-during-active-swing/thrust/slam, Die-during-Hit,
   attack-from-Locomotion (can_start_attack :148), begin_attack(None)
   InvalidIntent (:159) — all untested; successor material.
4. REPORT.md thin (no commands/exit codes); independent rerun supplied the
   evidence.
5. Cosmetic: dead code at :181/:219 behind dead() guards; test failure
   message swaps got/expected (actor_animation_tests.cpp:24-25).
