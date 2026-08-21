---
task: TASK-0086
title: Gate C campaign-decision contract audit
state: READY
packet: MECHANICAL
topology: INDEPENDENT
priority: high (D-122 Gate C preparation)
lane: deepseek or any exact-audit lane
base_commit: 1f82623d9a3936513327cc43362703443e14b02a
owned_paths:
  - orchestration/tasks/TASK-0086-gate-c-contract-audit/**
forbidden_paths:
  - native/**
  - server/**
  - src/**
  - playtest/**
  - docs/product/**
---

# Outcome

Produce `FINDINGS.md` and `captures/gate-c-contract.json` mapping the current
N6 server surface to every Gate C information requirement in
`NATIVE_PRODUCT_CONVERGENCE.md`: concrete goal, boss/danger, expected trophy or
material or item family, depth, branch consequence, and extraction/return
condition.

For each field, cite the current event/payload/source/test label, classify it
as AVAILABLE, DERIVABLE-WITHOUT-GAMEPLAY-RULES, or MISSING, and name the
smallest future owner path. A route name by itself is explicitly insufficient.
No reward/economy/balance values may be invented.

# Acceptance commands

Paste literal transcripts and exit codes in `REPORT.md`:

```bash
node -e "JSON.parse(require('fs').readFileSync('orchestration/tasks/TASK-0086-gate-c-contract-audit/captures/gate-c-contract.json','utf8')); console.log('gate-c contract JSON: PASS')"
rg -n 'world:road:chart|world:zone:enter|nodeId|warden|trophy|depth|stairs|extract' native/src/networking.cpp native/tests/networking_tests.cpp playtest/scenarios/world-web.mjs playtest/scenarios/quest.mjs
git diff --check
git diff --name-only
```

# Stop conditions

STOP and mark MISSING when evidence is absent. Do not turn an owner-only
campaign, loot, economy, or risk choice into an implementation assumption.
