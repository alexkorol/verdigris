---
task: TASK-0058
title: Parity wave N6 — world web, quests, economy, party (final SERVER-parity wave, D-122)
state: READY (released 2026-08-20 ~11:45 — N5 integrated at 18/19, Gate A green. ALSO IN SCOPE now: session-arc + town-amenities town/NPC surface, the deliberate N5 leftover)
priority: critical (D-116 — completes full 32-scenario parity)
owned_paths:
  - native/src/**
  - native/tests/**  # NOT native/client/** — C3 lane owns it (D-122)
  - orchestration/tasks/TASK-0058-native-protocol-n6/**
forbidden_paths:
  - playtest/** ; server/**, src/** read-only
base: program tip after 0056 integration
architect_review_required: true
---

## Goal

The UNCHANGED harness passes the ENTIRE 32-scenario suite against the
C++ server via attach — the remaining family: gates, world-web,
first-goal, quest, session-arc, crossroads, economy, town-amenities,
house-treasury, party, party-stories, build-divergence, skilltree,
gear-outcomes, encounter-variety (aura recheck), depth-loot deep
floors — plus every prior wave as regression. Acceptance = full
32/32 attach, architect-rerun.

## Scope

Same RULES-in-core / TRANSPORT-in-networking split. Read the JS
reference implementations the scenarios exercise and cite them. World
web (roads/tiers/warden gating), quest chain (Aldwyn objectives +
quest points + tree capacity), economy (shop/bank verbs), party
(formation, stories), town amenities, house treasury, skill tree
allocation. Stubs only where a scenario truly doesn't reach; document
each with a successor note (N7 better-than wave).

## Evidence

Standard: literal transcripts (build gates + full 32/32 attach with
harness commit stated), one authentic negative, C++ unit list.
Architect rebuilds + reruns the full attach personally.
