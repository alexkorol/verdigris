---
task: TASK-0047
state: CLAIMED
coordinator: kimi-work
worker: Kimi Work K3 implementation worker
worker_branch: codex/TASK-0047-native-n4-kimiwork
worktree: C:\Users\Alex\Documents\KimiWork\verdigris (own clone, task branch)
base_commit: 05c3f46 (program tip; contains 0045 N3 INTEGRATED, ancestor c49f8c51 verified)
started_at: 2026-08-17T11:30:00-07:00
expected_verification: powershell -File native/build.ps1 -RunTests; PLAYTEST_WS_URL attach 13/13 (6 item-family + 7 N1-N3 regression, harness unchanged); C++ unit coverage for item/inventory rules; one authentic negative
known_risks: large surface (items, inventory, equip pipeline, depth>1, Vesselforge brands); D-106 absolute - no item-destroying path; harness is read-only
dependencies: TASK-0045 N3 INTEGRATED (verified on program branch)
architect_review_required: true
---
