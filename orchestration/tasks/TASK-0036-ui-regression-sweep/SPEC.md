---
id: TASK-0036
title: UI regression sweep + inventory-pane layout fix
state: READY
track: web-demo
priority: critical
base_commit: current program tip (coordinator records the SHA)
dependencies: []
parallel_safe: true
owned_paths:
  - src/components/**
  - tests/unit/ui*.spec.js
  - orchestration/tasks/TASK-0036-ui-regression-sweep/captures/**
forbidden_paths:
  - server/**
  - native/**
  - prototypes/**
  - src/core/rendering/**
  - package.json
acceptance_commands:
  - npm run test:unit
  - npm run smoke:browser
---

## Goal

Fix the owner-reported inventory regression and systematically hunt the
rest of its class: UI panes that mutated/broke across the merged-line
and overhaul work.

## The confirmed regression (owner screenshot, 2026-08-16 ~22:55)

The inventory pane renders the backpack grid BESIDE the paperdoll
instead of below it; both shrink to tiny and a massive empty region sits
underneath. Restore the intended layout (paperdoll top, backpack grid
below/beside at proper scale, no dead space; use git archaeology on the
Inventory component for the pre-regression layout as reference).

## The sweep

Open and screenshot EVERY pane/overlay at 1920×1080 and one smaller
viewport (~1366×768): inventory, character/stats, passive tree, quests,
settings, minimap states, skill bar, party, Chronicles screens, death/
respawn, dialogs, vendors if present. For each: does it lay out sanely,
scale correctly, overlap nothing, and function? Where git archaeology
shows a better earlier state, note the commit. Deliverables:

1. Fixed inventory pane (this task fixes ONLY the inventory; other
   findings are reported, not fixed — they become follow-up specs).
2. `captures/` gallery of every pane, annotated pass/fail.
3. A ranked defect list in REPORT.md with file pointers per issue.

## Acceptance criteria

Gates green; inventory fix proven by before/after captures at both
viewports; the sweep gallery complete; defect list decision-ready.

## Review focus (D-115 play gate applies)

Architect will open the panes personally on the built client.

## Stop conditions

Fixing beyond the inventory pane; touching rendering internals.
