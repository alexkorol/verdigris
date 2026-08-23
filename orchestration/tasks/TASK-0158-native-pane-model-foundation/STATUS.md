# TASK-0158 STATUS

state: CLAIMED
lane: ox-pc-bd
coordinator: codex (worker lane)
worker: ox-pc-bd (isolated Windows implementation worker, ports 7240-7259 reserved)
machine: Windows (win32), bash shell
ports: 7240-7259 (reserved; task requires no live server; never touches 6500)
model: openrouter/stealth/ox-alpha
branch: worker/verdigris/pc/ox-pc-bd
worktree: Z:\Code\.worktrees\verdigris\ox-pc-bd
base-sha: ad1a1e178e689df442d4655937f8e8e037cf4cd2
started-at: 2026-08-22T22:54:01-07:00

Preflight verified: worktree clean, HEAD exactly equals the immutable task base
`ad1a1e178e689df442d4655937f8e8e037cf4cd2`, no upstream configured yet (fresh
worker branch; will push to `origin worker/verdigris/pc/ox-pc-bd`). Task folder
did not exist before this claim; no competing STATUS found after fetch --prune.

Plan: implement header-only pure pane model at
`native/client/pane_model.hpp` plus self-contained test source and PowerShell
compile/run harness under this task folder only. No edits outside owned_paths.
