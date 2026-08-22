# TASK-0154 STATUS

state: CLAIMED
coordinator: codex
worker: ox-pc-w (isolated Windows implementation worker, ports 7060-7079 reserved)
machine: Windows (win32), pwsh 7.x
ports: 7060-7079 (reserved; task requires no live server; never touches 6500)
provider: openrouter
model: stealth/ox-alpha
opencode-version: 1.18.21
branch: codex/TASK-0154-native-clean-runner-camera-portability-ox-pc-w
routed-head: d55e128952ec41107705b743801139c560b70f11
spec-base: 3933c366d8b6205e74a588634698627786e66767
worktree: Z:\Code\.worktrees\verdigris\ox-pc-w
started-at: 2026-08-22T07:16:49-07:00

First committed STATUS write wins. At preflight the task folder contained only
SPEC.md, the worktree was clean, HEAD was exactly the routed head, and the
branch identity matched. Commit and push this claim before implementation.
Then make only the standards-portability correction in the owned camera test,
run every literal gate, and hand off as REVIEW_REQUESTED.
