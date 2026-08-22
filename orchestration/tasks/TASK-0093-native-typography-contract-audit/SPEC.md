---
task: TASK-0093
title: Native typography and text-rendering contract audit
state: READY
packet: BOUNDED-DESIGN
topology: INDEPENDENT
job: BOUNDED-DESIGN
priority: P1
base_commit: d2423873c577d299b3b39c56024d1d840993c72b
owner_visible_contribution: defines the readable text system needed for presentation-complete panels
dependencies: []
owner_input_dependency: final font and license approval only; audit proceeds without it
owned_paths: [orchestration/tasks/TASK-0093-native-typography-contract-audit/**]
forbidden_paths: [everything else]
resource_capsule: read-only; no ports
---

# Outcome

Produce `FINDINGS.md` with the current native text operations, browser text
roles, resolution/DPI requirements, glyph ranges, wrapping, alignment,
clipping, contrast, offscreen-capture determinism, and Windows/macOS backend
needs. Recommend a backend-neutral text contract and locking tests; do not pick
a font or renderer.

# Frozen invariants and evidence

Render-list determinism and D-113 art authority remain frozen. Cite source/CSS
lines and reference captures. Required tables: text roles, metrics, accessibility
risks, backend needs, proposed tests, owner-only choices. Base SHA and literal
transcripts are mandatory.

# Acceptance

```powershell
rg -n "Text|Label|font|DrawText|text" native/client src/components src/assets src --glob "*.vue" --glob "*.css" --glob "*.cpp" --glob "*.hpp"
rg -n "1920x1080|1366x768|typography|panel" orchestration/benchmarks orchestration/tasks/TASK-0079-browser-panel-inventory
git diff --check
git diff --name-only
```

Expected: findings cite real paths and only this folder changes. Negative
control: document one text role for which the native render list has no
equivalent. Stop before downloading, generating, licensing, or selecting a
font; continue with backend-neutral metrics when owner input is pending.
