---
task: TASK-0044
state: REVIEW_REQUESTED
coordinator: codex
worker: Kimi Code / external native implementation
worker_branch: codex/TASK-0044-native-protocol-n2
worktree: C:\Users\Alex\Documents\KimiWork\verdigris
base_commit: 32d7b6e
dependencies: TASK-0039 integrated
expected_verification: powershell -File native/build.ps1 -RunTests; unchanged movement and zones attach
known_risks: architect rerun and acceptance decision still outstanding
architect_review_required: true
implementation_commits: d476788 (claim commit 6e6279c)
report: orchestration/tasks/TASK-0044-native-protocol-n2/REPORT.md
verification: committed tip d476788 passes native denylist/core/networking/client, unchanged movement/zones 2/2, and N1 quickstart/session regression 4/4; architect rerun outstanding
coordinator_recheck: 2026-08-17 — clean Kimi worktree d476788 independently reran `powershell -NoProfile -File native/build.ps1 -RunTests -RunClient`: denylist, core, networking, and client shell all PASS; no source changes.
coordinator_attach_recheck: 2026-08-17 — native server d476788 on port 6526 passed unchanged `quickstart single-session movement zones` 4/4, including six zone combinations and saved-position restoration; raw transcript in captures/coordinator-native-attach-2026-08-17.txt.
coordinator_dual_run: 2026-08-17 — JS reference and native d476788 both pass the unchanged four-scenario contract matrix 4/4; native intentionally reports 18 monsters per zone versus JS authored counts 33/56/31/46/57/54, documented as the N2 minimum stub and N3+ follow-up.
---

TASK-0044 is ready for architect review. Kimi's external worktree is clean at
`d476788`; the coordinator has not edited worker-owned native files. Fable's
required rerun and acceptance decision remain outstanding.
