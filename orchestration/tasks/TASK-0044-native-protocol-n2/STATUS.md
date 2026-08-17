---
task: TASK-0044
state: CLAIMED
coordinator: codex
worker: Kimi Code / external native implementation
worker_branch: codex/TASK-0044-native-protocol-n2
worktree: C:\Users\Alex\Documents\KimiWork\verdigris
base_commit: 32d7b6e
dependencies: TASK-0039 integrated
expected_verification: powershell -File native/build.ps1 -RunTests; unchanged movement and zones attach
known_risks: uncommitted worker source; saved pre-instance position across sequential sessions; test helper currently mismatches parse_envelope API
architect_review_required: true
implementation_commits: none yet (claim commit 6e6279c)
report: orchestration/tasks/TASK-0044-native-protocol-n2/REPORT.md
verification: WIP previously reached movement PASS and zones 5/6 assertions; latest recheck stops at native test compilation
---

TASK-0044 remains actively claimed and is not accepted or blocked. Kimi's
external worktree contains the implementation WIP; the coordinator has not
edited worker-owned native files. The latest build failure is recorded in
`BASELINE.md` and in the report.
