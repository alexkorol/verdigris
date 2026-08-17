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
known_risks: uncommitted worker source; core test captures the expected pre-entry position after instance entry; build script masks the core test exit with the later networking test
architect_review_required: true
implementation_commits: none yet (claim commit 6e6279c)
report: orchestration/tasks/TASK-0044-native-protocol-n2/REPORT.md
verification: actual worker server passes unchanged movement/zones 2/2; direct core test EXIT=1 on a test-fixture assertion and networking test EXIT=0; disposable test correction makes both pass
---

TASK-0044 remains actively claimed and is not accepted or blocked. Kimi's
external worktree contains the implementation WIP; the coordinator has not
edited worker-owned native files. The latest build failure is recorded in
`BASELINE.md` and in the report.
