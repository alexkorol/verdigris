---
task: TASK-0046
state: REVIEW_REQUESTED
branch: codex/TASK-0046-playability-reevaluation
commits:
  - 8dd9aee8
  - f0b6300f
  - 65b51a9a
base_commit: c3988b29
architect_review_required: true
---

# TASK-0046 — playability re-evaluation checkpoint

## Executive summary

The current-tip browser build was rebuilt successfully and the real client
was launched against worker-owned loopback servers on ports 6542–6545; the
owner's port 6500 was never used. The in-app browser rendered the game and
its server logs recorded client connection and login. Its evaluator returned
`window.ws === null`, however, because it evaluates in an extension/isolated
world that cannot observe page-created expando globals. The coordinator's
independent page-context probe on the same current tip captured
`window.ws.url = ws://127.0.0.1:6542/` with `readyState = OPEN`; that is the
authoritative socket proof for this task.

This worker did not complete the two required approximately ten-minute arcs.
The explicit arc method was stopped after a bounded guest checkpoint because
the available browser surface could not satisfy the required socket proof.
Consequently this report is a REVIEW_REQUESTED evidence checkpoint, not a
claim that the post-friction first ten minutes are fun-adjacent or that the
friction list has been re-evaluated.

## Build and server verification

From the isolated worktree at current tip `c3988b29` (a descendant of
`45846af7`):

```text
npm ci --no-audit --no-fund
PASS — 834 packages installed

npm run build
PASS — Vite 5.4.21; 377 modules transformed; dist emitted
```

Production servers were started with `NODE_ENV=production` on free ports
6542 and 6543; a development server was started on 6545, with a Vite client
using `VITE_WS_URL=ws://localhost:6545`. No server used port 6500. Server
output showed successful client connections and logins, including the guest
page's `Wanderer` session.

## Browser evidence and limitation

The production page at `http://localhost:6543/?play` rendered the live game
screen: Delaford Village, HP 110/110, skill bar, and Aldwyn's onboarding
messages. A bounded guest interaction reached The Old Barrow:

```text
00:00:00  Delaford Village; Aldwyn teaches WASD/arrow movement
00:00:03  Adventure panel; Old Barrow is described as a forgiving first delve
00:00:11  The Old Barrow selected; “The road is taking shape”; minimap 101,99
00:00:18  expedition settled; canvas actors remain outside DOM visibility
```

The in-app Browser Use page evaluator returned `window.ws?.url === null` in
both production and Vite-dev pages, even while the server logged successful
connections. This is attributable to its isolated extension world, not a
client/server failure. The coordinator's independent page-context probe on
the same current tip captured the required free-port proof:

```text
window.ws.url       = ws://127.0.0.1:6542/
window.ws.readyState = OPEN
```

The two socket-mode captures preserve the distinction:

- `captures/connection-proof-blocker-2026-08-18.txt` — initial bounded
  stop-condition observation
- `captures/socket-mode-comparison-2026-08-18.txt` — production/dev source
  comparison and isolated-world explanation

## Arc status

| Required arc | Status | Evidence |
|---|---|---|
| Guest quickstart, ~10 minutes | Not completed | `captures/guest-arc-checkpoint-2026-08-18.txt` records only the bounded opening and Old Barrow transition |
| Chronicles House/Scion, mortal oath, ~10 minutes | Not run | Stopped before the required client-proof method could be satisfied in this browser surface |
| `window.ws.url` proof for each arc | Not captured by this worker | Coordinator's independent page-context proof exists for the free-port client, but is not duplicated here |

## Friction dispositions and ranked new friction

The required per-item 0034 disposition table and new blocker/major/minor
ranking are intentionally **not asserted**. A full two-arc playability pass
is required before classifying fixes as FIXED, SURVIVES, or NOT-REPRODUCED;
the bounded opening is insufficient evidence for that judgment.

## Scope proof

The only files changed on this worker branch are task-folder evidence and
this report/status metadata. No product, server, native, playtest, package,
test, or other orchestration files were edited. The temporary production and
Vite servers were stopped after the checkpoint.

## Commits and captures

- `8dd9aee8` — connection-proof stop-condition capture
- `f0b6300f` — bounded guest arc checkpoint
- `65b51a9a` — production/dev socket-mode comparison

No unresolved product question is created by this read-only checkpoint. The
remaining work is the actual two-arc evaluation using the coordinator's
page-context browser harness that can observe `window.ws.url`.
