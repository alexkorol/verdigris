---
task: TASK-0072
state: CLAIMED
coordinator: cursor
worker_branch: codex/TASK-0072-owner-launcher-cursor
base_commit: 27d2be62038bba29abf68735288fd1d177b4c0aa
started_at: 2026-08-20T05:53:00-07:00
architect_review_required: true
expected_verification: delete native/build; powershell -File native/tools/play-native.ps1; Esc quits; script prints no-orphan check
notes: Owner-play ports 6520-6539. Never 6500. -Local / -Port / -Rebuild.
---

Claimed per RUN_STATUS: 0072 READY (any lane). 0071 left for mac-claude.
