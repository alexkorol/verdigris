---
task: TASK-0115
title: Browser panel and typography inventory
state: READY
packet: MECHANICAL
topology: INDEPENDENT
job: MECHANICAL
priority: P1
base_commit: d2423873c577d299b3b39c56024d1d840993c72b
supersedes: TASK-0079
owner_visible_contribution: freezes the load-bearing panel and typography contract for native presentation parity
dependencies: []
owner_input_dependency: final font choice remains owner-only; inventory is not blocked
owned_paths: [orchestration/tasks/TASK-0115-browser-panel-typography-inventory/**]
forbidden_paths: [everything else]
resource_capsule: ox-pc-a ports 6620-6639 if later routed; loopback only; never 6500
---

# Outcome and frozen invariants

Produce `FINDINGS.md`, one hard-fail capture per persistent/situational browser
panel, and `captures/panels.json`. For every panel record trigger, anchor,
approximate 1920x1080 and 1366x768 size, mounted Vue path, font family/size/
weight/color, gameplay load, and native phasing rank. Inventory only: no code,
redesign, font selection, or native work. Browser behavior and capture evidence
remain unchanged.

# Evidence and acceptance

Use the shared capture helper on one disposable capsule port. Driver assertions
must prove the named panel is visible before its PNG counts. Report base SHA,
exact commands/exit codes, source/CSS citations, capture summary, and files.

```powershell
rg -n "font-family|font-size|font-weight|color" src --glob "*.vue" --glob "*.css"
rg -n "component|panel|pane|chat|quest|expedition|guide|context" src/components --glob "*.vue"
node -e "const p=JSON.parse(require('fs').readFileSync('orchestration/tasks/TASK-0115-browser-panel-typography-inventory/captures/panels.json','utf8')); if(!Array.isArray(p.panels)||!p.panels.length) process.exit(1); console.log('panel inventory: PASS')"
git diff --check
git diff --name-only
```

Expected: every row cites a mounted path/capture and only this folder changes.
Negative control: run the capture driver with one intentionally false visibility
assertion against a disposable output path, record nonzero, restore/remove it.
Stop if reaching a panel requires source/assertion changes or port 6500.
Fallback: finish static source/style inventory and mark uncaptured panels RED.
