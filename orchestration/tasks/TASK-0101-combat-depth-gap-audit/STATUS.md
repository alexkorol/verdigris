# TASK-0101 STATUS

- state: CLAIMED
- coordinator: codex
- worker: ox-pc-ai (OpenRouter `stealth/ox-alpha`, variant `max`, opencode harness)
- machine: DESKTOP-TVU7OR7
- worktree: `Z:\Code\.worktrees\verdigris\ox-pc-ai`
- branch: `codex/TASK-0101-combat-depth-gap-audit-ox-pc-ai`
- routed base: `610a240e1e4bdfacfd77bec49e36be945a1ced13` (verified: HEAD equals base; base is ancestor of pushed `origin/codex/native-reconstitution`)
- started-at (UTC): 2026-08-22T23:26:46Z
- resource capsule: read-only analysis; no play server, never port 6500
- owned paths: `orchestration/tasks/TASK-0101-combat-depth-gap-audit/**` only
- expected verification: literal SPEC acceptance commands (`rg` combat-vocabulary sweep, JSON parse of `captures/combat-matrix.json`, `git diff --check`, `git diff --name-only`) plus negative control (one constitution action family marked absent rather than generic-attack parity)
