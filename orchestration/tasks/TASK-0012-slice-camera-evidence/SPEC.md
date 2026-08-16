---
id: TASK-0012
title: Camera-preset evidence pack from the founding slice
state: READY
track: web-demo
priority: medium
base_commit: TASK-0010 integration tip (Codex records the actual SHA in STATUS.md on claim)
dependencies: [TASK-0003]
parallel_safe: true
owned_paths:
  - orchestration/tasks/TASK-0012-slice-camera-evidence/REPORT.md
  - orchestration/tasks/TASK-0012-slice-camera-evidence/captures/**
forbidden_paths:
  - prototypes/**
  - native/**
  - src/**
  - server/**
acceptance_commands: []
---

## Goal

A side-by-side evidence pack (captures + parameter table) comparing the
slice's three camera presets across representative scenes, so the owner
can judge the projection direction (D-102) from evidence instead of
memory.

## Why this task exists

The camera envelope is an open experimental area by design. The slice has
a live camera lab; nobody has produced systematic comparative evidence
from it. This is exactly the kind of question the feel prototype exists to
answer.

## Product and architectural invariants

- Read-only for all game/prototype files: the harness surface
  (`window.__V`, debug panel with camera sliders/presets) is sufficient.
- Findings are evidence and observation, not decisions — D-102 stays
  provisional; presentation-taste calls belong to the owner.

## Inputs and references

`prototypes/founding-slice/` (serve statically; drive via Playwright like
`run-checks.mjs` / `tests/slice-checks.mjs` do), camera presets
Miniature / ARPG / High Table in the debug panel, D-102 in DECISIONS.md.

## Scope

1. Scripted Playwright run (script may live in the task folder) that, for
   each of the three presets, captures: the village scene, a mid-combat
   moment with effects visible, and a loot-on-ground moment. Nine PNGs
   into `captures/`, downscaled to ≤1280px wide.
2. REPORT.md: the exact parameter values of each preset (read from the
   page), a table of the nine captures, and neutral observations per
   preset (readability of telegraphs, billboard grounding, depth illusion,
   edge distortion) — describe, do not rank.
3. Note any camera-lab defects found while driving it (bugs are findings,
   not fixes).

## Non-goals

No changes to the slice, no ranking or recommendation, no native camera
work.

## Deliverables

REPORT.md + captures/ in this task folder, one commit.

## Acceptance criteria

Nine captures exist and are referenced from the report; parameter table
present; `git status` proves no edits outside the task folder.

## Required verification

Include the `git status --short` proof in REPORT.md.

## File ownership

This task folder only.

## Dependencies

TASK-0003's harness patterns (integrated).

## Parallel-safety assessment

Read-only beside TASK-0011 (core files) — disjoint.

## Review focus

Whether captures genuinely differ only by preset (same scene state), and
observation neutrality.

## Stop conditions

If the debug panel cannot reach a required state headlessly, capture what
is reachable and record the gap — do not modify the slice.
