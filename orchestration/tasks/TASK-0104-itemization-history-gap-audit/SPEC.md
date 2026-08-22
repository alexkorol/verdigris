---
task: TASK-0104
title: Itemization, extraction, and item-history gap audit
state: READY
packet: BOUNDED-DESIGN
topology: INDEPENDENT
job: BOUNDED-DESIGN
priority: P0
base_commit: d2423873c577d299b3b39c56024d1d840993c72b
owner_visible_contribution: protects items as the primary reason to play and exposes the path to memorable history-bearing loot
dependencies: []
owner_input_dependency: Brands/Bonds formula, economy, naming, and balance remain owner-only
owned_paths: [orchestration/tasks/TASK-0104-itemization-history-gap-audit/**]
forbidden_paths: [everything else]
resource_capsule: read-only; no ports
---

# Outcome

Produce `FINDINGS.md` and `captures/item-lifecycle.json` mapping stable IDs,
generation, equipment behavior changes, drops, pickup, inventory, extraction,
death recovery, relic candidacy, scars/history, crafting/storage, persistence,
wire payloads, presentation, and tests. Rank content-neutral lifecycle gaps.

# Frozen invariants and evidence

D-106 recoverability, House ownership, significant-item history, and denylist
firewall are frozen. Do not define affix math, economy, rates, or item names.
Cite every lifecycle edge and test; missing transitions remain red.

# Acceptance

```powershell
rg -n "item|inventory|equip|drop|pickup|extract|relic|scar|brand|bond|forge|stable.*id" native/include native/src native/client native/tests docs/product
node -e "JSON.parse(require('fs').readFileSync('orchestration/tasks/TASK-0104-itemization-history-gap-audit/captures/item-lifecycle.json','utf8')); console.log('item lifecycle: PASS')"
git diff --check
git diff --name-only
```

Expected: only task evidence changes. Negative control: identify one history or
recovery transition without an automated test. Stop at formula/economy/content
choices; continue with stable-identity and lifecycle seams.
