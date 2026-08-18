---
task: TASK-0042
state: REVIEW_REQUESTED
coordinator: kimi
worker: kimi-code-cli
worker_branch: codex/TASK-0042-first-loot-moment
worktree: C:\Users\Alex\Documents\Kimi\verdigris
base_commit: ca0dd2d
spec_base_commit: 0b12a0a
started_at: 2026-08-17T18:00:00-07:00
reasserted_at: 2026-08-18T12:20:00-07:00
expected_verification: npm run test:unit; npm run playtest; npm run smoke:browser
known_risks: deterministic first-drop rule must not distort loot tables; D-115 play gate evidence via captures; ground beam/label required one additive method in src/core/rendering/perspective-renderer.js (outside owned_paths - see QUESTION-0007 note in REPORT.md)
dependencies: TASK-0040 integrated
architect_review_required: true
---

Re-assertion per RELEASE.md (2026-08-17 ~20:30): the claim stalled on a
quota reset, not abandonment. Implementation is now complete on this
branch: curated first-find drop rule (server/core/combat/loot.js), ground
beam + name label + Take prompt, and the LootMoment comparison toast -
all verified with unit tests, 31/31 playtest, alternate-port browser
gate, and real captures in the task folder.

Note on the codex STATUS this replaces: that claim (BLOCKED,
QUESTION-0007, zero source changes) predates the RELEASE; the board at
resume lists 0042 as released/claimable, and REENTRY-KIMI-CODE.md
assigns it to kimi. QUESTION-0007's seam concern is answered in
REPORT.md - the LootMoment toast mounts through the existing
open:screen seam with no GameContainer wiring; only the ground
beam/label needed the renderer.
