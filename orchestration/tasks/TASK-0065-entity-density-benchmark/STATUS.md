---
task: TASK-0065
state: CLAIMED
coordinator: cursor
worker_branch: codex/TASK-0065-entity-density-benchmark-cursor
base_commit: 5c41a04821695e38261fdb52f2e86b2dea67f21d
started_at: 2026-08-20T04:06:00-07:00
architect_review_required: true
expected_verification: powershell -File native/build.ps1 -RunTests (unchanged default gates); opt-in density benchmark JSON N=50/200/500/1000 three runs; RESULTS.md
notes: measurement only; no native/src behavior changes. Ports 6580-6599 if JS attach is needed. 0063/0064 REVIEW_REQUESTED by cursor.
---

Claimed per RUN_STATUS: 0065 READY (N7 entity-density benchmark). Independent of Gate A.
