---
id: TASK-0023
title: Browser 2.5D Phase 2 — terrain + horizon conformance
state: READY
track: web-demo
priority: critical
base_commit: program tip after 0022 integration (coordinator records the SHA)
dependencies: [TASK-0022]
parallel_safe: true
owned_paths:
  - src/core/rendering/**
  - orchestration/tasks/TASK-0023-browser-25d-phase2/captures/**
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

Execute exactly Phase 2 of the governing plan
(`docs/25d-overhaul-plan.md` §7 "Phase 2 — Terrain + horizon
conformance"), burning down §4 gap item 4: adopt the reference haze curve
(≈0 across the playfield, reaching 1.0 within ~5–6% of the top) tuned to
the D-107 ARPG horizon, and any Phase-2 terrain-pass conformance items
the plan forecasts.

## Why this task exists

Phase 1 killed the blur; the frame still lacks the reference's grounded
horizon and cohesive terrain read (visible in the accepted 0022
after-vs-reference comparison). D-108 makes the vendored demo the
acceptance look; reference ARCHITECTURE §3 and §8 are binding.

## Product and architectural invariants

Same as TASK-0022: plan governs, §8 solved-bugs binding, renderer toggle
intact, no gameplay/server changes, shared projection/height invariant
preserved.

## Scope / Non-goals

Plan §7 Phase 2 exactly; no lighting/atmosphere retune (Phase 3), no DoF
coupling (Phase 4). Captures go IN THE TASK FOLDER (see 0022 correction),
lossy ≤250KB.

## Acceptance criteria

- Both acceptance commands green (playtest additionally if any doubt
  arises about scope creep — the diff must remain rendering-only).
- Before/after at ARPG default + side-by-side vs the reference demo
  horizon, committed under this task's `captures/`.
- Haze parameter dump or code cite proving the reference curve shape.

## Review focus

Horizon believability vs the reference, playfield stays clear of haze,
no regression of Phase-1 crispness.

## Stop conditions

Phase-3+ temptation; any needed touch outside rendering paths.
