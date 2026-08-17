---
id: TASK-0029
title: Browser 2.5D Phase 5 — performance + hardening
state: READY
track: web-demo
priority: medium
base_commit: program tip after 0027 integration (coordinator records the SHA)
dependencies: [TASK-0027]
parallel_safe: true
owned_paths:
  - src/core/rendering/**
  - tests/unit/perspective-camera.spec.js
  - tests/unit/rendering*.spec.js
  - orchestration/tasks/TASK-0029-browser-25d-phase5/captures/**
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

Execute Phase 5 of the governing plan (`docs/25d-overhaul-plan.md` §7
"Phase 5 — Performance + hardening") — the plan's forecast items, with
measured before/after frame timing.

## Invariants / evidence

Same regime as Phases 1–4. Evidence: a frame-time measurement (mean +
p95 over ≥10s of scripted play, method documented and committed like the
luminance script) before and after, at the ARPG default on this machine;
no visual regression (capture pair). Hardening items from the plan (e.g.
context-loss recovery if forecast) proven by the method the plan
prescribes.

## Acceptance criteria

Gates green; frame-time numbers in REPORT.md with the measuring script
committed; visual parity captures.

## Stop conditions

Optimizations that change visual output beyond parity tolerance; anything
outside rendering paths.
