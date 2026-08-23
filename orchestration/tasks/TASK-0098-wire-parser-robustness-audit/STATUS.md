---
task: TASK-0098
state: REVIEW_REQUESTED
coordinator: codex
lane: ox-pc-bc
worker: ox-pc-bc (worktree ox-pc-bc)
machine: DESKTOP-TVU7OR7
root: Z:\Code\.worktrees\verdigris\ox-pc-bc
worker_branch: worker/verdigris/pc/ox-pc-bc
base_commit: d2423873c577d299b3b39c56024d1d840993c72b
spec_base_commit: d2423873c577d299b3b39c56024d1d840993c72b
content_head: f3c90f09e1ffcb2eb15b036b6344524625059e65
frozen_review_head: f3c90f09e1ffcb2eb15b036b6344524625059e65
provider: openrouter
model: openrouter/stealth/ox-alpha
harness: OpenCode CLI
resource_capsule: read-only; no live fuzzing; no ports; port 6500 never touched
started_at: 2026-08-23T16:41:19Z
revision: 2
---

Claimed TASK-0098 (native wire parser robustness and abuse-boundary audit) at
the immutable base d2423873c577d299b3b39c56024d1d840993c72b (verified ancestor
of the local line) on worker branch worker/verdigris/pc/ox-pc-bc. Preflight
proved per AGENTS.md: clean worktree, branch exact, pure fast-forward from
cc85786f to origin/codex/native-reconstitution tip c274dafe before editing
(local had zero unique commits), no competing STATUS.md or RELEASE.md in this
task folder, remote worker branch tip 0c373d2f is an ancestor of the local
line so pushes fast-forward without force. Claim commit: 5a369bc1.

## Completion (revision 2 — REVIEW_REQUESTED)

Deliverables committed at content_head
f3c90f09e1ffcb2eb15b036b6344524625059e65 (only paths under this task folder,
verified by the gate-4 listing): FINDINGS.md, captures/parser-cases.json
(`verdigris.audit.parser-cases` v1, 30 cases), REPORT.md, and
captures/acceptance-rg-transcript.txt (sha256
C1C385D6C62C8EA927588283EF789D84C617E904CBF6DF15F0633FE836A19444).

Acceptance gates were run literally twice (pass 1 recorded verbatim in
REPORT.md; pass 2 over the final tree):

1. `rg -n "parse|payload|event|rate|auth|limit|invalid|unknown|close|error"
   native/src/networking.cpp native/include/verdigris/networking.hpp
   native/tests/networking_tests.cpp native/tests/session_tests.cpp` —
   exit 0 both passes, 457 matching lines each. Pass-1 bytes committed as the
   capture; pass-2 re-run compared equal as an exact 457-line multiset (zero
   content differences). Note honestly recorded: rg's emission order across
   multiple path arguments is not stable between runs, so byte-hash equality
   does not hold between passes; line-content equality was verified instead.
2. node JSON.parse of captures/parser-cases.json — exit 0 both passes,
   `parser cases: PASS`.
3. `git diff --check` — exit 0 both passes, no whitespace errors.
4. `git diff --name-only` — exit 0 both passes; pass 1 listed exactly the
   three evidence files then present; pass 2 listed only this STATUS.md
   change (task evidence) pending the flip commit.

Negative control delivered: PC-014 (`captures/parser-cases.json`) — a deeply
nested JSON frame case that currently lacks any test, marked
`status: negative-control`, `marked_safe: false`. ESCALATION per SPEC ("stop
and privately escalate a credible high-impact reachable flaw"): F-A (PC-014)
and F-B (PC-015) are crash-class red candidates with full static
source-to-sink paths (JSON nesting depth vs reader-thread stack;
road-node tier driving web_tier_width recursion depth) and zero test
coverage. Dynamic confirmation intentionally NOT performed — the capsule
forbids live probing; owner approval is requested for a minimal offline
harness. Severity stated conservatively with preconditions; no exploit
payloads published anywhere in the evidence.

Authority compliance: read-only audit; no source patched; no forbidden path
touched; no ports opened; port 6500 untouched; security policy and protocol
compatibility treated as frozen; pre-commit hooks ran normally (no bypass).

Frozen review request: review is requested at content_head
f3c90f09e1ffcb2eb15b036b6344524625059e65 with this STATUS flip commit pushed
on top; the branch tip of worker/verdigris/pc/ox-pc-bc at push time is the
frozen pushed head to integrate.
