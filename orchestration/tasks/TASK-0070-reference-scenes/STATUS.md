---
task: TASK-0070
state: CLAIMED
coordinator: cursor
worker_branch: codex/TASK-0070-reference-scenes-cursor
base_commit: 27d2be62038bba29abf68735288fd1d177b4c0aa
started_at: 2026-08-20T05:38:00-07:00
architect_review_required: true
expected_verification: powershell -File native/build.ps1 -RunTests; native/build/verdigris_client.exe --reference-scene all (10 captures + 5 JSON, two-run identical)
notes: Stage 1 visual baseline. Seeded local session. No new art. Ports unused.
---

Claimed per RUN_STATUS: 0070 READY (cursor suggested). 0071/0072 left for other lanes.
