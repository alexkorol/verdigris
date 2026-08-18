---
task: TASK-0056
title: Parity wave N5 — Chronicles, death, and persistence over the C++ server
state: READY (PIPELINED — claimable only AFTER TASK-0047 is INTEGRATED;
  verify 0047's item types exist in native/src/core.cpp before branching)
priority: critical (mission critical path, D-116)
owned_paths:
  - native/**
  - orchestration/tasks/TASK-0056-native-protocol-n5/**
forbidden_paths:
  - playtest/** (harness is the measuring stick)
  - server/**, src/** (read-only reference)
base: program tip after 0047 integration
architect_review_required: true
---

## Goal

The UNCHANGED harness passes the Chronicles/death family against the
C++ server via attach:

- `chronicles` (final death → crypt → ancestral ring recovery → depth
  record), `chronicles-first-combat`, `mortality`, `respawn`,
  `persistence` (relogin state), `single-session` re-verified

plus the N1–N4 regression set. Full list = wave sets of 0044/0045/0047
+ this family.

## Scope (RULES in core, TRANSPORT in networking — same split)

1. House/Scion lifecycle: named scions, mortal oath flag, final death
   (D-106: items NEVER destroyed — circulation/crypt pools), successor
   flow, depth records.
2. Persistence seam per ADR-002 (snapshot/restore with RNG state):
   relogin rebuilds position/inventory/equipped state; D-109 forgiving
   disconnects (return to town with everything).
3. Death transfer rules mirror the JS implementations the scenarios
   exercise (cite files read, 0005-style).
4. Stubs at scenario-minimum, documented with successor (N6 world-web/
   quests is the wave after).

## Acceptance evidence (the standard)

Literal transcripts: build gates all PASS; attach run of the full
cumulative scenario set green with UNCHANGED harness (state commit);
one authentic negative (break a death-transfer rule → harness catches
→ restore); C++ unit coverage list.

Architect rebuilds and reruns the attach set personally (G5).
