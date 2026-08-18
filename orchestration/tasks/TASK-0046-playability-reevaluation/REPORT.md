---
task: TASK-0046
state: REVIEW_REQUESTED
branch: codex/TASK-0046-playability-reevaluation
commits:
  - 8dd9aee8
  - f0b6300f
  - 65b51a9a
  - 1de6e45b
base_commit: 45846af7
architect_review_required: true
---

## Executive summary

The current-tip browser build was rebuilt and launched on free loopback ports;
the owner's port 6500 was never used. The in-app browser rendered the game and
server logs recorded connection/login. Its isolated-world evaluator could not
see page-created `window.ws`, while an independent page-context Playwright
probe captured `window.ws.url = ws://127.0.0.1:6542/` with readyState OPEN.

This is a bounded evidence checkpoint, not a claim that the two required
approximately ten-minute arcs or the full friction disposition are complete.

## Verification and bounded arc

- `npm ci --no-audit --no-fund`: 834 packages installed.
- `npm run build`: Vite 5.4.21, 377 modules transformed, passed.
- Production free-port page rendered Delaford Village, HP 110/110, skill bar,
  and Aldwyn onboarding.
- Guest checkpoint reached The Old Barrow: Adventure panel at ~3 seconds,
  zone transition at ~11 seconds, minimap 101,99, expedition settled around
  ~18 seconds.

The required guest ten-minute arc was not completed and the Chronicles
House/Scion mortal-oath arc was not run. No per-item TASK-0034 disposition or
new-friction ranking is asserted.

## Scope and limitation

Only task-folder evidence and coordinator captures changed; no product,
server, native, playtest, package, or other code was edited. The initial null
socket observation is attributed to the in-app browser's isolated evaluation
world, not to a proven client/server failure; the page-context probe is the
authoritative connection evidence for this handoff.

Architect review remains required. The next valid step is a two-arc run using
the page-context browser harness that can observe `window.ws.url`.
