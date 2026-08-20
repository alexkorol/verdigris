---
task: TASK-0063
state: CLAIMED
coordinator: cursor
worker_branch: codex/TASK-0063-server-gate-a-surface-cursor
base_commit: 5c41a04821695e38261fdb52f2e86b2dea67f21d
started_at: 2026-08-20T03:51:00-07:00
architect_review_required: true
expected_verification: powershell -File native/build.ps1 -RunTests; C++ envelope unit coverage; PLAYTEST_WS_URL attach N1-N4; unknown-uuid equip negative
notes: closes 0061 server gaps (ground drops, extract, equip wear totals, snapshot ground). Ports 6580-6599. 0064 owns native/client.
---

Claimed per RUN_STATUS: 0063 READY (Gate A protocol surface). 0066 claimed by mac-claude. 0064 REVIEW_REQUESTED by cursor.
