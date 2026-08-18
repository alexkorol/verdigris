# Acceptance registry (stable; versioned changes only)

Exact gates, their preconditions, and invalidation rules. Evidence is
bound to task + commit SHA + integration base + command + timestamp.

## Browser game

| Gate | Command | Expected | Notes |
|---|---|---|---|
| Unit | `npm run test:unit` | all files/tests pass, count stated | |
| Browser smoke | `npm run smoke:browser` | e2e critical loop passes | includes build |
| Full playtest | `npm run playtest` | 32/32, default flags | ~3.5 min; serialize per machine; boots :6510 |
| Targeted playtest | `npm run playtest -- <scenarios>` | named scenarios pass | for iteration; full suite still required at acceptance |
| UI evidence | hard-fail Playwright capture script | script exits non-zero unless on-screen assertions hold; real PNGs committed | pattern: TASK-0038 captures/capture-0038.mjs |

## Native

| Gate | Command | Expected | Notes |
|---|---|---|---|
| Build+tests | `powershell -File native/build.ps1 -RunTests` | denylist PASS, core PASS, networking PASS, camera2d PASS | |
| Headless proof | `... -RunClient` | `trophies stored: 1 \| items stored: 1`, exit 0 | non-zero exit on any other count |
| Parity attach | start `verdigris_server.exe` (loopback, own port range) then `PLAYTEST_WS_URL=ws://127.0.0.1:<port> node playtest/run.mjs --attach <set>` | named set green, harness UNCHANGED (state harness commit) | N-wave sets defined in wave specs |
| Owner-visible | architect plays the exe (D-117) | recorded verdict in REVIEW | native acceptances only |

## Universal preconditions and invalidation

- G0: drivers prove they reached the target state (e.g. combat drivers
  verify contact) before their result counts.
- Negative control where practical: break the property, show the gate
  fail, restore (0043's format).
- A green is stale after: base advance touching the surface, evaluator
  change, environment change, or harness change. Re-run affected gates
  post-merge (G6).
- Default path only: no env flags beyond what the owner runs, unless
  the spec says otherwise AND the default path is also proven.
- Architect reruns the stated acceptance command personally before
  ACCEPTED (G5). Implementer testimony is never sufficient.
