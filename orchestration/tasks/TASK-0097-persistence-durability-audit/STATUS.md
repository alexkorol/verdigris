---
task: TASK-0097
state: INTEGRATED
reviewed_commit: 0c373d2ff2c921a1bfb02ec85d34ac5a380ea77a
reviewed_at: 2026-08-23T17:10:00Z
coordinator: codex
worker: ox-pc-bc (worktree ox-pc-bc)
root: Z:\Code\.worktrees\verdigris\ox-pc-bc
lane: ox-pc-bc
model: openrouter/stealth/ox-alpha
provider: openrouter
harness: OpenCode CLI
worker_branch: worker/verdigris/pc/ox-pc-bc
base_commit: d2423873c577d299b3b39c56024d1d840993c72b
spec_base_commit: d2423873c577d299b3b39c56024d1d840993c72b
claim_commit: c289156ae46979efe32ce53e4d5c928e69f4ce43
branch_tip_at_request: see git rev-parse HEAD of worker/verdigris/pc/ox-pc-bc after this commit; that tip is the frozen review head
capsule: read-only; no real saves touched; no ports bound; port 6500 never used
claimed_at: 2026-08-23T16:20:00Z
expected_verification: rg -n "persist|save|load|profile|serialize|version|reconnect|relic|House|Scion" native/src native/include native/tests; node -e "JSON.parse(require('fs').readFileSync('orchestration/tasks/TASK-0097-persistence-durability-audit/captures/persistence-contract.json','utf8')); console.log('persistence contract: PASS')"; git diff --check; git diff --name-only
---

Claimed TASK-0097 (native persistence durability and fault-model audit,
BOUNDED-DESIGN, read-only) at routed base
d2423873c577d299b3b39c56024d1d840993c72b on worker branch
worker/verdigris/pc/ox-pc-bc. Preflight proved a clean tree, exact lane
branch, base-commit ancestry, and fast-forwarded the lane onto program tip
0bee7f1e (fleet re-point bc -> TASK-0097) before claiming at c289156a.

REVIEW_REQUESTED: evidence complete. Deliverables, all confined to
orchestration/tasks/TASK-0097-persistence-durability-audit/**:

- FINDINGS.md - layer map (core snapshot format, file adapter, session
  layer), persisted-field inventory, save triggers, three serialization/version
  seams, file locations, atomicity analysis, stale-data compatibility,
  reconnect semantics truth table, ranked red risks R1-R7, named negative
  control, 14-row deterministic disposable-profile fault matrix F-01..F-14,
  smallest locking tests L1-L7, every claim cited file:line.
- captures/persistence-contract.json - machine-readable twin of FINDINGS.md.
- REPORT.md - literal acceptance transcripts + exit codes.

Acceptance commands run literally (transcripts in REPORT.md): mapping grep
exit 0 (627 lines); JSON parse gate prints "persistence contract: PASS"
exit 0; git diff --check exit 0 silent; git diff --name-only exit 0 empty
(evidence files were untracked task-folder additions; git status --short
listed exactly the four owned paths).

Headline finding for the successor: the persistence library exists and is
unit-tested (core snapshot/restore + atomic adapter) but has ZERO production
callers - no save trigger, no load-on-startup, and the real ProtocolSession
profile has no serialization seam at all, so D-109 currently holds only
within a living process. Negative control: schemaVersion rejection is
implemented (native/src/core.cpp:1299-1301) but untested by any current test
(F-03); partial-write companions F-01/F-10 also uncovered.

Capsule honored end-to-end: read-only audit, no real owner profile opened or
mutated, disposable temp paths only, zero port bindings, port 6500 untouched,
no forbidden path modified.
