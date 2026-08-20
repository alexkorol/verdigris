---
task: TASK-0070
state: REVIEW_REQUESTED
coordinator: cursor
worker_branch: codex/TASK-0070-reference-scenes-cursor
base_commit: 27d2be62038bba29abf68735288fd1d177b4c0aa
started_at: 2026-08-20T05:38:00-07:00
finished_at: 2026-08-20T05:50:00-07:00
architect_review_required: true
expected_verification: powershell -File native/build.ps1 -RunTests; native/build/verdigris_client.exe --reference-scene all; eyeball one PNG per resolution
notes: Stage 1 baseline. Seed 0xC011AB1E. No new art. Ports unused.
---

Visual reference scenes REVIEW_REQUESTED. 10 PNG + 5 JSON. Two-run JSON identical. See REPORT.md.
