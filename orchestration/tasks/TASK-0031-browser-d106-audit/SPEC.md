---
id: TASK-0031
title: Browser death/relic behavior vs owner rulings — read-only audit
state: READY
track: research
priority: high
base_commit: current program tip (coordinator records the SHA)
dependencies: []
parallel_safe: true
owned_paths:
  - orchestration/tasks/TASK-0031-browser-d106-audit/REPORT.md
  - orchestration/tasks/TASK-0031-browser-d106-audit/appendix/**
forbidden_paths:
  - "everything else — read-only outside this task folder"
acceptance_commands: []
---

## Goal

An evidence audit of what the BROWSER game currently does on Scion death
and item loss, compared against the owner rulings D-106 (death never
destroys items; everything recoverable) and the native reference
implementation (relic pool, lost trophies, deterministic re-entry,
TASK-0018/0025 semantics) — so a follow-up implementation task can align
the shippable game without guesswork.

## Why this task exists

D-106 is live in the NATIVE core but was ruled for the product as a
whole; the browser game (the near-term shippable) predates the ruling.
Server gameplay changes need an audit-first approach: the TASK-0005
archaeology proved that evidence-first beats assumption here.

## Scope (REPORT.md)

1. Current browser death path: file/line walkthrough (what happens to
   equipped items, inventory, Chronicles relics, trophies on death;
   guest vs account vs Chronicles-Scion differences).
2. Delta table vs D-106/D-109 and vs the native semantics
   (relic_candidates / lost_trophies / resurface cadence / boundary
   retirement).
3. The minimal change-set forecast (files, systems, tests) for an
   implementation task, including which existing playtest scenarios
   (respawn, session-arc, zones) constrain it.
4. Risks: save-format migrations, live-player compatibility.

## Acceptance criteria

Every claim carries path/line evidence; delta table complete; change-set
forecast concrete enough that the implementation spec can be written
from it alone; `git status` proof of read-only scope.

## Review focus

Whether the follow-up implementation task could start from this report
without re-auditing.

## Stop conditions

Fix-it temptation — record, don't touch.
