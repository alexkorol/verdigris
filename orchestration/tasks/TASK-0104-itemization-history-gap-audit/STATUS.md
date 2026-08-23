---
task: TASK-0104
title: Itemization, extraction, and item-history gap audit
state: CLAIMED
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
capsule: read-only; no ports; port 6500 never used
claimed_at: 2026-08-23T16:32:48Z
expected_verification: rg -n "item|inventory|equip|drop|pickup|extract|relic|scar|brand|bond|forge|stable.*id" native/include native/src native/client native/tests docs/product; node -e "JSON.parse(require('fs').readFileSync('orchestration/tasks/TASK-0104-itemization-history-gap-audit/captures/item-lifecycle.json','utf8')); console.log('item lifecycle: PASS')"; git diff --check; git diff --name-only
---

Claimed TASK-0104 (itemization, extraction, and item-history gap audit,
BOUNDED-DESIGN, read-only) on worker branch worker/verdigris/pc/ox-pc-bb.
Preflight proved a clean tree, exact lane branch, and base-commit ancestry;
fast-forwarded the lane onto program tip fca5b7c5 (fleet re-point bb ->
TASK-0104) before claiming. Audit scope is the item lifecycle end to end:
stable IDs, generation, equipment behavior changes, drops, pickup, inventory,
extraction, death recovery, relic candidacy, scars/history, crafting/storage,
persistence, wire payloads, presentation, and tests, with content-neutral gaps
ranked red-first. No formula, economy, rate, or naming decisions will be made.
