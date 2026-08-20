---
task: TASK-0061
state: CLAIMED
coordinator: cursor
worker_branch: codex/TASK-0061-networked-guest-slice-cursor
base_commit: 872bb94a4334d93ba597ca46c9ce9144cdd8e3f3
started_at: 2026-08-20T02:10:00-07:00
architect_review_required: true
expected_verification: native/build.ps1 -RunTests; remote guest journey scenario (handshake, move, fight, loot, equip, extract, shutdown) on 127.0.0.1:6580-6599; authentic negative (server killed mid-session → visible disconnect, no local fallback)
notes: main.cpp SINGLE-WRITER this session (0061 migrates the exe onto IClientSession remote mode)
---

Claimed per RUN_STATUS: 0061 READY top-priority Gate A; native lane (deepseek stalled on 0056, kimi quota-dead). Cursor ports 6580-6599. 0059 remains REVIEW_REQUESTED on its worker branch.
