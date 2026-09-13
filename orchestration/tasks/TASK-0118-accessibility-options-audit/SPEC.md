---
task: TASK-0118
title: Native accessibility, options, and input audit
state: SUPERSEDED
superseded_by: integrated (reviewed head 6b87d49c, 2026-08-23)
packet: MECHANICAL
topology: INDEPENDENT
job: MECHANICAL
priority: P1
base_commit: 9bd689b4cebac0fe1f79ba54edcc9967a1a8f0d4
owner_visible_contribution: exposes the settings required for a readable, controllable, broadly playable owner build
dependencies: []
owner_input_dependency: final defaults may require owner play verdicts; audit is not blocked
owned_paths: [orchestration/tasks/TASK-0118-accessibility-options-audit/**]
forbidden_paths: [everything else]
resource_capsule: read-only; no ports or settings mutation
---

# Outcome and invariants

Produce `FINDINGS.md` and `captures/accessibility-matrix.json` covering key/
mouse rebinding, hold/toggle behavior, sensitivity, focus, text scale, contrast,
color independence, reduced motion/flash, subtitles/captions, audio controls,
minimap/pane options, difficulty-independent readability, persistence, reset,
keyboard-only navigation, and test/capture coverage. Preserve D-007 controls
and do not turn accessibility into balance changes.

# Acceptance and evidence

```powershell
rg -n "rebind|keybind|sensitivity|focus|contrast|color|motion|flash|subtitle|caption|volume|scale|accessibility|setting" native/client native/tests src tests docs/product
node -e "JSON.parse(require('fs').readFileSync('orchestration/tasks/TASK-0118-accessibility-options-audit/captures/accessibility-matrix.json','utf8')); console.log('accessibility matrix: PASS')"
git diff --check
git diff --name-only
```

Expected: only this folder changes and every green cites evidence. Negative
control: identify one action or state conveyed only by color/visual motion or
without a rebinding/focus proof. Stop before choosing balance or owner-only
defaults; continue with standards and content-neutral contracts.
