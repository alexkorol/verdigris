---
id: TASK-0027
title: Browser 2.5D Phase 4 — HUD-safe compositing, DoF coupling, polish
state: READY
track: web-demo
priority: high
base_commit: program tip after 0024 integration (coordinator records the SHA)
dependencies: [TASK-0024]
parallel_safe: true
owned_paths:
  - src/core/rendering/**
  - tests/unit/perspective-camera.spec.js
  - tests/unit/rendering*.spec.js
  - orchestration/tasks/TASK-0027-browser-25d-phase4/captures/**
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

Execute Phase 4 of the governing plan (`docs/25d-overhaul-plan.md` §7
"Phase 4 — DoF coupling + polish") PLUS the carried-forward defect from
the 0024 review:

1. **(REQUIRED, from 0024 review)** Lighting, vignette, and atmosphere
   passes must composite BELOW the HUD (HP/MP orbs, skill bar, minimap,
   quest tracker): at night the orbs currently render nearly black.
   Reference pass order is authoritative. Prove with a night capture
   where the orbs read exactly as bright as at midday.
2. The plan's Phase-4 items (DoF coupling to the zoom blend and residual
   polish as forecast).

## Invariants / evidence / review focus

Same regime as Phases 1–3: plan governs, §8 binding, renderer toggle
intact, rendering-only, test-expectation updates in owned paths, captures
in the task folder (lossy ≤250KB) reusing the 0024 `capture.mjs` harness
(day + night + open-field + reference side-by-side), knob list old→new in
REPORT.md. Luminance-script proof welcome where relevant.

## Stop conditions

Phase-5 perf temptation; anything outside rendering paths.
