---
id: TASK-0034
title: Play the game — first-session playability evaluation
state: READY
track: research
priority: critical
base_commit: current program tip (coordinator records the SHA)
dependencies: []
parallel_safe: true
owned_paths:
  - orchestration/tasks/TASK-0034-playability-evaluation/REPORT.md
  - orchestration/tasks/TASK-0034-playability-evaluation/captures/**
forbidden_paths:
  - "everything else — read-only; play, don't fix"
acceptance_commands: []
---

Suggested coordinator: **kimi** (evaluation quality; same rigor as your
0019/0031 work but pointed at EXPERIENCE, not code).

## Goal

D-110 needs ground truth: actually PLAY the browser game as a brand-new
player would (built client on :6500, guest and Chronicles paths) for a
full session arc and produce a ranked friction inventory — the document
that sets the next wave's priorities.

## Why this task exists

Owner: "I want to have a playable game." Every gate we run proves
protocol correctness, not fun or clarity. Nobody has documented what a
stranger's first 30–60 minutes actually feels like.

## Method

Drive a real session (Playwright-driven play is fine, but judge like a
human): login → first minute (do you know what to do?) → first fight →
first loot → first zone transition → first death → what happens next →
progression legibility (quests? map? goals?) → 30+ minutes in. Capture
liberally (lossy ≤250KB, task folder). Both guest quickstart and the
Chronicles House/Scion path.

## Deliverable (REPORT.md)

1. Session narrative with timestamps and captures.
2. **Ranked friction list**: every point where a new player would be
   confused, bored, stuck, or quit — severity-ranked (blocker / major /
   minor), each with evidence and a one-line proposed fix direction.
3. What already WORKS (keep-list, so fixes don't break it).
4. The single biggest "this is not yet a game because…" statement,
   honestly.
5. Explicit answers: Is the goal of the game communicated? Is combat
   fun for more than 5 minutes? Is loot exciting? Is death
   comprehensible? Is there a reason to keep playing?

## Acceptance criteria

Evidence-dense (captures per claim), the ranked list is decision-ready
(the architect will spec the next wave directly from it), read-only
scope proof.

## Stop conditions

Fix-it temptation — record, don't touch. No code opinions needed beyond
fix directions.
