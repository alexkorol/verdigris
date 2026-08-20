---
task: TASK-0059
title: Responsive overlay pass — compact-viewport collisions
state: READY
priority: medium (BOUNDED-DESIGN; mac-claude lane suggested)
owned_paths:
  - src/components/**
  - src/assets/** (styles)
  - tests/**
  - orchestration/tasks/TASK-0059-responsive-overlay-pass/**
forbidden_paths:
  - server/**, native/**, playtest/** assertions
base: current program tip
architect_review_required: true
---

## Why

0034 flagged responsive-overlay collisions (major 6), 0036's sweep
corroborated, and 0046 marked it NOT-REPRODUCED only because no
compact-viewport pass was run. Nobody has actually fixed it.

## Deliverables

1. Audit at 1366x768 AND 1280x720: open every overlay/pane combo a
   first session hits (chat ticker + guide banner + adventure panel +
   inventory + settings + skill tree + loot toast + death overlay) and
   record which collide/overflow (screenshot each defect).
2. Fix the collisions with layout/z-index/anchoring changes — no
   feature removals; panes must remain fully usable at both viewports
   and unchanged at 1920x1080.
3. Hard-fail capture script asserting the fixed layouts at both
   compact viewports (bounding-box non-overlap checks for the pairs
   you fixed) + before/after captures for the worst two defects.

## Acceptance

npm run test:unit + full npm run playtest + npm run smoke:browser
literal transcripts (default flags); the capture script exits non-zero
on regression; per-defect table (found -> fixed/wontfix+reason).
Architect reruns gates on Windows and inspects 1-2 captures.
