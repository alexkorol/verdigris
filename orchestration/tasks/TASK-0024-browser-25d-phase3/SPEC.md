---
id: TASK-0024
title: Browser 2.5D Phase 3 — lighting + atmosphere retune
state: READY
track: web-demo
priority: critical
base_commit: program tip after 0023 integration (coordinator records the SHA)
dependencies: [TASK-0023]
parallel_safe: true
owned_paths:
  - src/core/rendering/**
  - tests/unit/perspective-camera.spec.js
  - tests/unit/rendering*.spec.js
  - orchestration/tasks/TASK-0024-browser-25d-phase3/captures/**
forbidden_paths:
  - server/**
  - native/**
  - prototypes/**
  - src/components/**
  - package.json
acceptance_commands:
  - npm run test:unit
  - npm run smoke:browser
---

## Goal

Execute exactly Phase 3 of the governing plan
(`docs/25d-overhaul-plan.md` §7 "Phase 3 — Lighting + atmosphere
retune"), burning down §4 gap item 6: the stacked mist blobs, god rays,
cloud multiply, and vignette are retuned as a set against the D-107 ARPG
preset's `fog 0.4`, side-by-side with the reference demo, now that items
1–5 no longer mask their combined effect.

## Why this task exists

Phases 1–2 removed blur, washing, and the broken haze; what remains of
the "muddy" verdict is the over-tuned atmosphere stack. The reference's
lighting mood (soft lightmap multiply, restrained rays, warm grade) is
the D-108 target.

## Product and architectural invariants

Plan governs; reference ARCHITECTURE §8 binding (grading lives in the
lighting pass — Phase 1 moved it there; tune it there, do not reintroduce
canvas or terrain-fetch grading). Renderer toggle intact; rendering-only.
Unit-test expectation updates for retuned constants are IN SCOPE (paths
owned above) per the QUESTION-0004 precedent — update expectations with
the retune, never delete assertions.

## Scope / Non-goals

Plan §7 Phase 3 exactly. No DoF coupling changes (Phase 4), no perf work
(Phase 5).

## Acceptance criteria

- Both acceptance commands green.
- Evidence in this task's `captures/` (lossy ≤250KB): before/after at
  ARPG default INCLUDING an open-field shot toward the map edge where
  horizon and atmosphere are visible (0023 review problem 1), plus
  side-by-side vs the reference demo's mood.
- REPORT.md lists each atmosphere knob touched with old→new values.

## Review focus

Mood parity with the reference without crushing readability; no
regression of Phase-1 crispness or Phase-2 horizon; knob list coherence.

## Stop conditions

Phase-4+ temptation; any need beyond owned paths.
