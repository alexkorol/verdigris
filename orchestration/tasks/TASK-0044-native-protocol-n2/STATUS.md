---
task: TASK-0044
state: REVIEW_REQUESTED
coordinator: kimi-work
worker: Kimi Work K3 implementation worker
worker_branch: codex/TASK-0044-native-protocol-n2
worktree: C:\Users\Alex\Documents\KimiWork\verdigris (own clone, task branch)
base_commit: 32d7b6e
head_commit: 293419b (implementation d476788 + loopback-bind fix) + report/STATUS commits on the same branch
spec_base_commit: program tip after 0039 integration — satisfied: 0039 INTEGRATED (STATUS state INTEGRATED, REVIEW ACCEPTED); branched from current tip 32d7b6e which contains it
started_at: 2026-08-17T07:45:00-07:00
expected_verification: powershell -File native/build.ps1 -RunTests; PLAYTEST_WS_URL attach run of zones + movement scenarios against the native server (unchanged harness)
known_risks: wire-contract drift between the JS server and the N2 port (mitigate: read the 0005-cited JS implementations first, attach-run early); movement feel constants must mirror post-0037 browser values
dependencies: TASK-0039 integrated (120abd19, 413b3aff)
architect_review_required: true
---
