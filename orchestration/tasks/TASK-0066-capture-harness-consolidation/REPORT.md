---
task: TASK-0066
state: REVIEW_REQUESTED
coordinator: mac-claude
worker_branch: codex/TASK-0066-capture-harness-consolidation-mac-claude
base_commit: 5c41a04821695e38261fdb52f2e86b2dea67f21d
architect_review_required: true
---

# TASK-0066 REPORT — Shared hard-fail capture helper

## Executive summary

`tests/e2e/lib/capture-harness.mjs` extracts the parts duplicated between
`capture-0055.mjs` and `capture-0059.mjs`: owned server lifecycle
(build+spawn+ready-wait+teardown, isolated guest-save/Chronicles state per
run), Chronicles/guest login, bounding-box helpers (`boxOf`, `boxesOverlap`,
`overflowsViewport`, `roundBox`), the `<prefix>-<viewport>-<label>.png`
naming convention, and the hard-fail JSON-summary + exit(1) pattern. Port is
a required argument with no default (capsule discipline) and explicitly
rejects 6500 (owner's live server).

A demo script (`captures/capture-0066-demo.mjs`) reproduces TASK-0059's
full compact-viewport (1366x768) assertion set — 10 pair-overlap checks + 4
viewport-overflow checks + 2 layout checks, 16 total — through the shared
helper, against a fresh build, without touching TASK-0059's own evidence.

## Changed files

- `tests/e2e/lib/capture-harness.mjs` — new shared module (owned_paths).
- `orchestration/tasks/TASK-0066-capture-harness-consolidation/{STATUS,REPORT,captures/*}`

No `src/**`, `server/**`, or `playtest/**` changes. No existing task capture
folder touched (0055/0059's evidence is untouched — verified via git status
showing only new files under my own task folder and tests/e2e/lib/).

## Test commands and outcomes

`npm run test:unit` (unaffected by e2e-only change):

```
 Test Files  134 passed (134)
      Tests  841 passed (841)
```

`npx eslint tests/e2e/lib/capture-harness.mjs orchestration/tasks/TASK-0066-capture-harness-consolidation/captures/capture-0066-demo.mjs`:
clean, no output.

Demo script, port 7002 (7000 avoided — macOS afs3-fileserver system service
squats it on this Mac; unrelated to the repo):

```
CAPTURE_PORT=7002 node orchestration/tasks/TASK-0066-capture-harness-consolidation/captures/capture-0066-demo.mjs
[capture-harness] production build (PORT 7002)
CAPTURES OK {"1366x768.guide-vs-party":true,"1366x768.guide-vs-minimap":true,
"1366x768.zoneMenu-vs-quickbar":true,"1366x768.zoneMenu-vs-mpOrb":true,
"1366x768.identity-vs-chatPeek":true,"1366x768.identity-vs-hpOrb":true,
"1366x768.chatPeek-vs-hpOrb":true,"1366x768.inventory-vs-hpOrb":true,
"1366x768.inventory-vs-mpOrb":true,"1366x768.inventory-vs-quickbar":true,
"1366x768.zoneMenu-in-viewport":true,"1366x768.party-in-viewport":true,
"1366x768.guide-in-viewport":true,"1366x768.inventory-in-viewport":true,
"1366x768.zoneMenu-fits-party-column":true,"1366x768.inventory-leaves-canvas":true}
exit: 0
```

Reran with `SKIP_BUILD=1` (documented reuse path) — identical result, exit 0.

Negative control (ad hoc, not committed — `runCapture(() => ({a:true,
b:false}), ...)`): prints `Error: CAPTURE FAILED: b`, exit 1, and still
writes the evidence JSON with the false value intact. Confirms the hard-fail
path actually fails, not just that the happy path prints OK.

## Deviations

- Did not extract `closeOpenPanes`/`ensureGuideAndAdventure`/`collectChrome`
  into the shared module — these reference task-specific ARIA labels and
  overlay names (inventory, skill tree, loot, death, settings) that differ
  per SPEC. Kept them local to the demo script, same as any real capture
  script would define its own. The SPEC's explicit extraction list (server
  lifecycle, login, box math, PNG naming, hard-fail summary) is what's
  actually duplicated verbatim between 0055 and 0059; these interaction
  sequences are not.
- Demo covers 1366x768 only (SPEC says "at 1366x768"), not 0059's full
  three-viewport matrix — the 1280x720/1920x1080 passes exist to prove
  0059's own regression coverage, not the harness itself.
