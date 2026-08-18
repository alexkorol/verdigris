---
task: TASK-0038
state: INTEGRATED
branch: codex/TASK-0038-rebinding-kimiwork
commits:
  - 01a12d72
  - 4a8983cb
  - c73fff1
base_commit: 9d4f666
integration_commit: 2c0a00c3
architect_review: ACCEPTED
---

## Executive summary

TASK-0038 is accepted and integrated. It delivers cursor-aimed LMB/RMB world
attacks, six persisted rebindable actions, live quickbar labels, conflict
refusal, reset-all, and Shift+RMB context-menu access. The server remains
authoritative through the existing skill-trigger protocol.

## Verification

Fable's architect review verified real rendered captures for quickbar labels,
LMB primary attack, RMB weapon skill, the Settings controls panel, and a
rebind surviving a full page reload. The WS frame log showed LMB and RMB
\`player:skill:trigger\` events followed by \`world:skill:effect\`, with
\`rmbContextMenuOpened: false\`.

- \`npm run test:unit\`: 123 files / 788 tests passed.
- \`npm run smoke:browser\`: 1 passed.
- \`npm run playtest\`: 31/31 passed.

## Scope and integration

The architect explicitly ratified the minimal mounted-component ownership
expansion for \`GameCanvas.vue\`, \`Settings.vue\`, and \`Quickbar.vue\`; no
rendering-file overlap or native changes were introduced. Integrated at
\`2c0a00c3\`, with the accepted program line continuing through the current
N3/native parity work.

## Known design note

The legacy context menu remains available through Shift+RMB. ESC during the
capture flow also closes the settings pane; this is documented behavior, not a
failed gate.
