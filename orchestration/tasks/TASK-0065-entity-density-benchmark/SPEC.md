---
task: TASK-0065
title: N7 entity-density benchmark (native vs JS server headroom proof)
state: READY
packet: BOUNDED-DESIGN
lane: any native lane; independent of Gate A and N5
priority: medium (N7 better-than groundwork, D-116)
owned_paths:
  - native/tests/**
  - native/tools/**
  - orchestration/tasks/TASK-0065-entity-density-benchmark/**
forbidden_paths:
  - native/src/** behavior changes (measurement only; a fix is a note)
  - native/client/**
---

# Outcome

A deterministic, repeatable benchmark proving native headroom:
spawn N monsters (N in {50, 200, 500, 1000}) in one instance via the
spawn_monster seam, run 1000 fixed-step ticks with a scripted player
attack loop, report ticks/sec and per-tick p99. Output JSON to the
task folder. Same scenario shape documented for the JS server (harness
attach script driving dev spawns) so the comparison is apples-to-apples
- implement the JS side ONLY if it needs no server/ changes; else note.

# Acceptance

Benchmark exe/target added to build.ps1 (opt-in flag, not in default
gates), three runs' JSON committed, short RESULTS.md table. Unit gates
stay green. Architect reruns one N=500 pass.
