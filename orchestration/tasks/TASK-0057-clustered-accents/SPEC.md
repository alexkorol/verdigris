---
task: TASK-0057
title: Clustered accents — seeded generation-side clusters (from 0053 D3)
state: READY
priority: low-medium (MECHANICAL; owner-visible polish)
owned_paths:
  - server/core/map.js
  - server/core/generation/** (wherever floorPicker/accent pass lives)
  - src/core/rendering/** (only if a draw hook is needed)
  - tests/**
  - orchestration/tasks/TASK-0057-clustered-accents/**
forbidden_paths:
  - playtest/** assertions
base: current program tip
architect_review_required: true
---

Floor accents, flowers, and water generate in coherent seeded clusters
(blob growth from seed cells, deterministic per zone seed) instead of
one-cell checkerboard noise. Same accent density budget as today
(±10%) so perf and readability hold. Evidence: unit test for
deterministic clustering; 1-2 rendered captures (village + one zone);
full gates (unit/playtest/smoke) default path. Provenance: 0053 D3,
respawned with correct server-side ownership (architect spec error
fixed).
