---
task: TASK-0145
released_claims: [4aa9e0c3]
released_at: 2026-08-22 02:31 -07:00
reason: post-claim worker exit after the lane's single activation recovery
---

# TASK-0145 ox-pc-b release

ox-pc-b crossed the ten-minute claim SLA while its worktree was still clean.
The supervisor interrupted pre-claim drift and applied one exact-session,
claim-first recovery. That recovery pushed valid STATUS-only claim `4aa9e0c3`,
then exited after beginning an uncommitted `native/client/client_model.hpp`
change and before any handoff.

The claim is released as P0 post-claim activation failure. Preserve
`Z:\Code\.worktrees\verdigris\ox-pc-b` exactly; do not clean, reset, resume a
second time, or count it as capacity. A fresh independent replacement may
claim from the current program tip and must implement from the SPEC, not copy
the quarantined edit blindly.
