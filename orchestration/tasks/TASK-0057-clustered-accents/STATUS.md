---
task: TASK-0057
state: CLAIMED
coordinator: deepseek
worker_branch: codex/TASK-0057-clustered-accents-deepseek
base_commit: 79723db35c4d2873d3c5af3fda13a4503337236d
started_at: 2026-08-18T16:15:00-07:00
architect_review_required: true
expected_verification: npm run test:unit (clustering deterministic); npm run smoke:browser; npm run playtest (32/32); 1-2 rendered captures (village + one zone)
---

Claiming TASK-0057 (clustered accents, seeded generation-side clusters).

Owned paths per SPEC: server/core/map.js, server/core/generation/**,
src/core/rendering/** (only if a draw hook is needed), tests/**,
orchestration/tasks/TASK-0057-clustered-accents/**.

Base = current program tip 79723db3 (post 0053/0054 integration, PR #25).
