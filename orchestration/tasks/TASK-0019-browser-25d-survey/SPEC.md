---
id: TASK-0019
title: Phase 0 survey for the browser 2.5D renderer overhaul
state: READY
track: web-demo
priority: critical
base_commit: current program tip (coordinator records the SHA in STATUS.md)
dependencies: []
parallel_safe: true
owned_paths:
  - docs/25d-overhaul-plan.md
forbidden_paths:
  - src/**
  - server/**
  - native/**
  - prototypes/**
  - docs/reference/**
acceptance_commands: []
---

Suggested coordinator: **Kimi Code** (survey depth benefits from the
stronger model; Codex fleet stays on the native tasks). First
STATUS-write wins per PROTOCOL.

## Goal

Execute exactly Phase 0 of the owner-supplied overhaul brief
(`docs/reference/25d-overhaul/HANDOFF.md`): survey the browser game's
render loop, coordinate conventions, entity/draw architecture, camera,
and map format, and write the concept mapping + elevation-source decision
into `docs/25d-overhaul-plan.md`. No code changes.

## Why this task exists

Owner directive 2026-08-16: the vendored demo is the look/feel acceptance
target (D-108) and the game must become shippable soon. The brief's own
ground rule 1 requires this survey before any phase-1 code. The browser
game is the near-term shippable product; this survey is its critical
path.

## Product and architectural invariants

- Read `docs/reference/25d-overhaul/docs/ARCHITECTURE.md` IN FULL first
  (its §8 solved-bugs list is binding — do not re-derive).
- Survey only: zero edits outside the single plan document.
- The invariant that must survive the eventual port: one projection
  formula and one height function shared by terrain and billboards.
- Renderer toggle requirement (legacy path preserved) goes into the plan.

## Scope

`docs/25d-overhaul-plan.md` containing: engine/stack findings (Vue +
canvas specifics, current render loop entry points with file/line refs —
the TASK-0005 audit report is a strong starting index), coordinate/depth/
anchor concept mapping to the demo's model, camera integration plan,
elevation-data decision (authored layer vs tile metadata vs flat h=0
interim — recommend one), phase-by-phase file-touch forecast for phases
1-5, risks, and the feature-flag/toggle design.

## Non-goals

Any rendering code, any gameplay changes, any asset work.

## Acceptance criteria

Plan document exists, covers every item above with file/line evidence,
names the elevation decision, and `git status` shows only the plan file.

## Review focus

Whether phase-1 could start from this plan alone without re-surveying;
fidelity to the reference ARCHITECTURE (no invented alternatives to §8
solutions).

## Stop conditions

Contradictions between the brief and the constitution/track isolation →
file a question; do not resolve unilaterally.
