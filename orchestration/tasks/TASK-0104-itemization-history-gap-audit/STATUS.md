---
task: TASK-0104
title: Itemization, extraction, and item-history gap audit
state: INTEGRATED
reviewed_commit: 40c505ac9b05ecc6d14a17391f252d3ddd2e3246
reviewed_at: 2026-08-23T17:50:00Z
packet: BOUNDED-DESIGN
topology: INDEPENDENT
job: BOUNDED-DESIGN
priority: P0
lane: ox-pc-bb
model: openrouter/stealth/ox-alpha
provider: openrouter
harness: OpenCode CLI
root: Z:\Code\.worktrees\verdigris\ox-pc-bb
worker_branch: worker/verdigris/pc/ox-pc-bb
base_commit: d2423873c577d299b3b39c56024d1d840993c72b
spec_base_commit: d2423873c577d299b3b39c56024d1d840993c72b
claim_commit: 2cfe40cf
frozen_review_head: see git rev-parse HEAD of worker/verdigris/pc/ox-pc-bb after this commit; that tip is the frozen review head
capsule: read-only; no ports; port 6500 never used
claimed_at: 2026-08-23T16:32:48Z
review_requested_at: 2026-08-23T17:20:00Z
expected_verification: rg -n "item|inventory|equip|drop|pickup|extract|relic|scar|brand|bond|forge|stable.*id" native/include native/src native/client native/tests docs/product; node -e "JSON.parse(require('fs').readFileSync('orchestration/tasks/TASK-0104-itemization-history-gap-audit/captures/item-lifecycle.json','utf8')); console.log('item lifecycle: PASS')"; git diff --check; git diff --name-only
---

Claimed TASK-0104 (itemization, extraction, and item-history gap audit,
BOUNDED-DESIGN, read-only) on worker branch worker/verdigris/pc/ox-pc-bb.
Preflight proved a clean tree, exact lane branch, and base-commit ancestry;
fast-forwarded the lane onto program tip fca5b7c5 (fleet re-point bb ->
TASK-0104) before claiming at 2cfe40cf.

REVIEW_REQUESTED: evidence complete. Deliverables, all confined to
orchestration/tasks/TASK-0104-itemization-history-gap-audit/**:

- FINDINGS.md - two-universe lifecycle map (D-114 core simulation Item system
  + N4/N5 tile-space parity GameItem/VesselForge world) across stable IDs,
  generation, equipment behavior change, drops, pickup, inventory, extraction,
  death recovery (D-106), relic candidacy/circulation, scars/history,
  crafting/storage, persistence, wire payloads, presentation, and tests;
  22-edge coverage matrix with file:line citations; ranked content-neutral
  gaps R1-R8 (G-01 tile-space durability seam, G-02 scar production edge,
  G-03 cross-session identity, G-04 universe convergence, G-05 untested
  history events, G-06 no history/scars projection, G-07 schemaVersion
  rejection untested, G-08 re-equip history smear).
- captures/item-lifecycle.json - machine-readable twin (JSON parse gate PASS).
- REPORT.md - literal acceptance transcripts + exit codes (rg exit 0; node
  prints "item lifecycle: PASS" exit 0; git diff --check exit 0 silent;
  git diff --name-only exit 0 empty; git status --short lists exactly the
  owned untracked paths).

Negative controls (spec-required): NC-1 - the equipped-item use / equip /
unequip -> EventType::ItemHistoryUpdated emission transition
(native/src/core.cpp:436,448,523,536) has zero automated test assertions
(rg "ItemHistoryUpdated" native/tests exits 1, no matches) while the
underlying state is tested (core_tests.cpp:1557-1563). NC-2 - restore()'s
schemaVersion rejection (native/src/core.cpp:1299-1301) untested; positive
header check only (core_tests.cpp:763).

Capsule honored end-to-end: read-only audit, zero writes outside the owned
folder, no port bound, port 6500 never used, no formula/economy/rate/name
decisions made (Brands/Bonds formula and relic probability tuning remain
owner-only).
