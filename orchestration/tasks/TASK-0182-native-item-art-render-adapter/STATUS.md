---
task: TASK-0182
state: CLAIMED
revision: 2
lane: claude-a
worker_branch: codex/TASK-0182-native-item-art-render-adapter-claude-r2
claimed_at: 2026-08-24T14:55:16Z
frozen_base: aad3237429d04de250e9bb7b36225e291d379809
---

r2 revision claimed per REVIEW.md: apply the two numbered corrections
(re-key resolve() on real manifest string ids with an explicit tested
sim-id -> manifest-id mapping; add a manifest drift-guard test with a
negative control). Frozen base is the program-branch head the branch was
cut from (full SHA above, per BUS).

## History

- r1: cursor (composer-2.5), branch
  codex/TASK-0182-native-item-art-render-adapter-cursor, head 0171e0a2,
  REVIEW_REQUESTED 2026-08-24T07:56:00Z -> verdict REVISE (see REVIEW.md).
