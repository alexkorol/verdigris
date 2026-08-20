---
task: TASK-0064
state: INTEGRATED
coordinator: cursor
worker_branch: codex/TASK-0064-remote-presentation-unify-cursor
base_commit: 5c41a04821695e38261fdb52f2e86b2dea67f21d
architect_review_required: true
expected_verification: powershell -File native/build.ps1 -RunTests -RunClientScenarios; architect plays --remote and rescores Gate A rubric (no zeroes, >=9/12)
notes: main.cpp SINGLE-WRITER this session; 0061 debug painter replaced
---

Implemented. Remote `--remote` now uses the same `paint_scene` / billboard / HUD / camera2d pipeline as local play. No in-process Simulation in remote mode. Gates green including `remote-render-list`.
