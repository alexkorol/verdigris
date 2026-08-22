# TASK-0150 STATUS

state: CLAIMED
coordinator: codex
worker: ox-pc-k (isolated Windows implementation worker, ports 6820-6839)
machine: Windows (win32), pwsh 7.x
ports: 6820-6839 (never touches 6500)
provider: openrouter
model: stealth/ox-alpha
opencode-version: 1.18.21 (supervisor-verified launching CLI; no out-of-worktree probes)
branch: codex/TASK-0150-native-clean-build-convergence-ox-pc-k
routed-head: 30e98e024d4a22a744be4bee63dfcf607f63010a
spec-base: 060c11517d2ebb0aec0c4d4a38c5e3eb53141cb2 (immutable SPEC base)
worktree: Z:\Code\.worktrees\verdigris\ox-pc-k
started-at: 2026-08-22T09:53:21Z
claim-committed-at: 2026-08-22T09:56Z

Preflight evidence at claim time:
- `git status --short` clean.
- Branch is exactly `codex/TASK-0150-native-clean-build-convergence-ox-pc-k`.
- HEAD verified exactly `30e98e024d4a22a744be4bee63dfcf607f63010a`.
- `git fetch --prune origin` completed without error.
- No pre-existing STATUS.md or RELEASE.md in this task folder; first-write-wins claim.

Plan per SPEC.md: prove a clean configure/build/test + all client scenarios from a
disposable build directory with installed toolchains only; remove only evidenced
build-helper friction inside owned paths (native/build.ps1, native/CMakeLists.txt,
native/CMakePresets.json); run every literal acceptance gate; then set
REVIEW_REQUESTED with REPORT.md and push only this worker branch.
