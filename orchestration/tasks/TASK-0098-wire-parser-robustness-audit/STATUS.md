---
task: TASK-0098
state: CLAIMED
coordinator: codex
lane: ox-pc-bc
worker: ox-pc-bc (worktree ox-pc-bc)
machine: DESKTOP-TVU7OR7
root: Z:\Code\.worktrees\verdigris\ox-pc-bc
worker_branch: worker/verdigris/pc/ox-pc-bc
base_commit: d2423873c577d299b3b39c56024d1d840993c72b
spec_base_commit: d2423873c577d299b3b39c56024d1d840993c72b
provider: openrouter
model: openrouter/stealth/ox-alpha
harness: OpenCode CLI
resource_capsule: read-only; no live fuzzing; no ports; port 6500 never touched
started_at: 2026-08-23T16:41:19Z
revision: 1
---

Claimed TASK-0098 (native wire parser robustness and abuse-boundary audit) at
the immutable base d2423873c577d299b3b39c56024d1d840993c72b (verified ancestor
of the local line) on worker branch worker/verdigris/pc/ox-pc-bc. Preflight
proved per AGENTS.md: clean worktree, branch exact, pure fast-forward from
cc85786f to origin/codex/native-reconstitution tip c274dafe before editing
(local had zero unique commits), no competing STATUS.md or RELEASE.md in this
task folder, remote worker branch tip 0c373d2f is an ancestor of the local
line so pushes fast-forward without force.

Work will be confined to owned_paths
orchestration/tasks/TASK-0098-wire-parser-robustness-audit/** (STATUS.md,
FINDINGS.md, captures/parser-cases.json, REPORT.md). All other paths are
forbidden and will not be modified; the audit is read-only against native/,
server/, src/, docs/, and config/. No traffic will be sent, no ports opened,
no exploit payloads published, port 6500 never touched; the resource capsule
is honored. Security policy and protocol compatibility are treated as frozen;
findings cite exact parser/handler/test lines with reachable source-to-sink
paths only, severity reported conservatively with preconditions.

Plan: inventory every wire-boundary surface in native/src/networking.cpp,
native/include/verdigris/networking.hpp, native/tests/networking_tests.cpp,
and native/tests/session_tests.cpp — envelope parsing, type/range/size checks,
authentication/rate gates, unknown-event handling, malformed JSON handling,
disconnect cleanup, and deterministic error behavior; map each boundary to a
current test or a red candidate case; include one negative control (a
malformed case lacking a test, not marked safe); run every acceptance command
literally; write FINDINGS.md + captures/parser-cases.json + REPORT.md with
literal transcripts and exit codes.
