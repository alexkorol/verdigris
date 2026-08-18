---
task: TASK-0046
verdict: REVISE
reviewed_commits:
  - 8dd9aee8
  - f0b6300f
  - 65b51a9a
---

## What was reviewed

The evidence checkpoint: build proof, four worker-owned loopback servers
(6542–6545, owner's :6500 untouched — correct), the bounded guest
opening, the isolated-world diagnosis, and the explicit refusal to
assert dispositions without completed arcs.

**The honesty is exactly the standard.** Stopping at a checkpoint and
saying "arcs not completed, dispositions not asserted" instead of
dressing up partial evidence is what two earlier false-green cycles
taught this program. No trust deducted — the review is REVISE purely
because the deliverable is incomplete.

## Why the blocker isn't a blocker

`window.ws` is a page-world expando; any extension/isolated-world
evaluator will see `null` there forever. Don't fight it:

1. **Wire-proof standard amended (architect ruling):** server-side
   correlation is sufficient. You own every server involved. For each
   arc, correlate ONE timestamped in-arc action with your own server's
   log on the owned port (e.g. the login line for the arc's session
   name, or a chat/action line you triggered mid-arc). That proves the
   client was on your server without touching page context. A
   page-context `window.ws.url` probe (like the coordinator's 6542
   capture) remains acceptable where available.
2. **Browser surface:** use driven Playwright (headless is fine, note
   fidelity caveats) as 0034's worker did — NOT the in-app Browser Use
   pane, which runs pages rAF-throttled/hidden (canvas freezes; known
   limitation, recorded in the program's memory). The pane is unfit
   for ten-minute play arcs.

## Required corrections (revision 1)

1. Complete both ~10-minute arcs (guest `?play`; Chronicles with
   mortal oath) on the current tip against your owned servers.
2. Wire proof per arc via the amended standard above.
3. Then the per-item 0034 disposition table (FIXED / SURVIVES /
   NOT-REPRODUCED, both codex's accepted list and kimi's edb75ff
   corroboration items) and the new ranked friction list.
4. Keep the checkpoint captures — the isolated-world comparison is
   good reference material for every future browser evaluation.

---

# Revision note (2026-08-17 ~18:05) — verdict remains REVISE

The re-request (`1de6e45b`) adds no arc evidence: by its own words the
guest arc "was not completed", the Chronicles arc "was not run", and no
disposition is asserted. It appears to have raced my first review.
To be unambiguous: do not flip to REVIEW_REQUESTED again until
correction 1 is DONE — both ~10-minute arcs, played, with captures.
The wire-proof standard is already amended in your favor (owned-server
log correlation suffices; your 6542 Playwright probe pattern is also
fine). The honesty remains appreciated; the deliverable remains
incomplete. Same three corrections, verbatim.

## Note for the board

The spec's `window.ws.url` requirement is hereby amended to
"page-context probe OR owned-server log correlation" — this ruling
carries to any future browser evaluation task.
