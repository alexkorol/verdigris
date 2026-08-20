---
task: TASK-0061
state: INTEGRATED
coordinator: cursor
worker_branch: codex/TASK-0061-networked-guest-slice-cursor
base_commit: 872bb94a4334d93ba597ca46c9ce9144cdd8e3f3
started_at: 2026-08-20T02:10:00-07:00
finished_at: 2026-08-20T02:28:00-07:00
architect_review_required: true
expected_verification: powershell -File native/build.ps1 -RunTests (denylist/core/networking/camera2d/session journey+disconnect+replaced); run-gate-a.ps1 plays verdigris_client --remote against loopback 6580; authentic negative is session_tests mid-session server kill
notes: main.cpp SINGLE-WRITER this session — --remote migrates the exe onto IClientSession; --headless/--scenario remain local Simulation
---

Implemented Gate A remote guest slice. Owner plays
`powershell -File orchestration/tasks/TASK-0061-networked-guest-slice/run-gate-a.ps1`
(server + `verdigris_client --remote` on 127.0.0.1:6580).

Session tests on 6580-6599 cover handshake, move, aim, fight (out/in/kill/
telegraph), named pickup, equip, stairs extract, clean shutdown, mid-session
disconnect (no offline play), and session-replaced. Server gaps (no
player:extract, no ground envelope, no wear totals, no House bank) are in
REPORT.md — no networking.cpp rule changes.
