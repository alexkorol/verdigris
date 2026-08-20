---
task: TASK-0063
state: INTEGRATED
coordinator: cursor
worker_branch: codex/TASK-0063-server-gate-a-surface-cursor
base_commit: 5c41a04821695e38261fdb52f2e86b2dea67f21d
started_at: 2026-08-20T03:51:00-07:00
finished_at: 2026-08-20T04:05:00-07:00
architect_review_required: true
expected_verification: powershell -File native/build.ps1 -RunTests; C++ envelope unit coverage; PLAYTEST_WS_URL attach N1-N4 (13/13); unknown-uuid equip negative
notes: closes 0061 server gaps (item:change/world:itemDropped, player:extract + stairs-up bank, player:equippedAnItem wear+combat, login/dev:state ground). Ports 6580-6599. Did not edit native/client (0064) or playtest.
---

Implemented Gate A protocol surface. Attach 13/13 on ws://127.0.0.1:6587.
See REPORT.md. Architect extends the 0061 drive script for live drop labels.
