---
task: TASK-0042
state: CLAIMED
coordinator: codex
worker: Luna browser loot-moment worker
worker_branch: codex/TASK-0042-first-loot-moment
worktree: .codex/worktrees/TASK-0042-first-loot-moment
base_commit: e462c26d
started_at: 2026-08-17T00:00:00-07:00
dependencies: TASK-0040 accepted and present at current program tip
expected_verification: npm run test:unit; npm run playtest; npm run smoke:browser; first-drop and comparison captures; D-115 play gate
known_risks: preserve existing curated item data and wire protocol; no new item/affix/economy design; keep all changes inside owned_paths
architect_review_required: true
---

Claimed by Codex for isolated implementation. The worker must stop on any
new-item, affix, Vesselforge, or economy decision and report it rather than
inventing product canon.
