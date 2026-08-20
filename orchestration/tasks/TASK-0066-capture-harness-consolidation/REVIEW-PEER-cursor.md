---
task: TASK-0066
peer: cursor
implementer: mac-claude
worker_branch: codex/TASK-0066-capture-harness-consolidation-mac-claude
base_commit: 5c41a04821695e38261fdb52f2e86b2dea67f21d
rerun_at: 2026-08-20T04:17:00-07:00
verdict: facts only — architect decides ACCEPTED/REVISE
---

# REVIEW-PEER-cursor — TASK-0066

Peer rerun in the cursor clone after `git fetch` + fast-forward to
`origin/codex/native-reconstitution` (`5c41a048`). Implementer files and
STATUS were not edited. Demo screenshots produced by this rerun were
discarded (`git checkout --` on the two PNGs).

## Commands (cursor capsule, not 7000–7019)

`npm run test:unit`:

```
 Test Files  134 passed (134)
      Tests  841 passed (841)
 Duration  31.25s
 exit: 0
```

`npx eslint tests/e2e/lib/capture-harness.mjs orchestration/tasks/TASK-0066-capture-harness-consolidation/captures/capture-0066-demo.mjs`:

```
(no output)
ESLINT_OK
exit: 0
```

Demo (port **6588**, cursor 6580–6599; never 6500):

```
$env:CAPTURE_PORT='6588'
node orchestration/tasks/TASK-0066-capture-harness-consolidation/captures/capture-0066-demo.mjs
[capture-harness] production build (PORT 6588)
CAPTURES OK {"1366x768.guide-vs-party":true,"1366x768.guide-vs-minimap":true,"1366x768.zoneMenu-vs-quickbar":true,"1366x768.zoneMenu-vs-mpOrb":true,"1366x768.identity-vs-chatPeek":true,"1366x768.identity-vs-hpOrb":true,"1366x768.chatPeek-vs-hpOrb":true,"1366x768.inventory-vs-hpOrb":true,"1366x768.inventory-vs-mpOrb":true,"1366x768.inventory-vs-quickbar":true,"1366x768.zoneMenu-in-viewport":true,"1366x768.party-in-viewport":true,"1366x768.guide-in-viewport":true,"1366x768.inventory-in-viewport":true,"1366x768.zoneMenu-fits-party-column":true,"1366x768.inventory-leaves-canvas":true}
exit: 0
```

Counted 16/16 true checks. Owned server was torn down by the demo `finally`.

## Negative control (temp script, not committed)

`runCapture(() => ({ a: true, b: false }))` against the shared helper:

```
Error: CAPTURE FAILED: b
exit: 1
evidence JSON: {"a":true,"b":false}
```

Hard-fail names the falsy check and does not exit 0.

## Notes (facts, not a verdict)

- Harness rejects missing `CAPTURE_PORT` and the demo required it.
- Rerun used 6588 because cursor's capsule is 6580–6599; implementer's 7002 is their Mac capsule.
- No READY work claimed during this peer pass.
