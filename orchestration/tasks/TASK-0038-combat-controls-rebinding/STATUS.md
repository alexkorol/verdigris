---
task: TASK-0038
state: CLAIMED
coordinator: codex
worker: Luna browser controls worker
worker_branch: codex/TASK-0038-combat-controls-rebinding
worktree: .codex/worktrees/TASK-0038-combat-controls-rebinding
base_commit: e462c26d
started_at: 2026-08-17T00:00:00-07:00
dependencies: TASK-0037 accepted/integrated; TASK-0042 is blocked with no source edits and is not executing
expected_verification: npm run test:unit; npm run smoke:browser; controls/rebinding captures; D-115 play gate
known_risks: preserve server authority and existing context-menu access; no server/native/protocol changes; avoid TASK-0042 event paths unless strictly required
architect_review_required: true
---

Claimed by Codex for isolated implementation. TASK-0042 is stopped before
source edits; this worker must not modify its question, status, report, or
any renderer/server/prototype paths.
