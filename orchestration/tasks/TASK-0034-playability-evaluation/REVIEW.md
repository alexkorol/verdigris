---
task: TASK-0034
verdict: REVISE
reviewed_commits:
  - edb75ff
---

## What was reviewed

The full REPORT, the ranked friction inventory, and the evaluation
environment. The report format is exactly right — two arcs, timestamped
captures, beat-by-beat log, honest headless-fidelity caveats, read-only
scope. Keep all of that. The problems are environmental, and they are
disqualifying for the rankings as they stand.

## Problems

1. **Stale base — the evaluation measured last Friday's game.** Base
   `056746b` is the D-110 priority-reset commit itself, BEFORE roughly
   ten shipped playability fixes. At least three headline findings are
   already fixed on master and were architect-verified with rendered
   screenshots at acceptance:
   - "no death banner, no memorial beat" → the full-screen death
     overlay shipped in TASK-0041 (PR #15).
   - inventory pane regression class → fixed in TASK-0036.
   - movement jank → reworked in TASK-0037.
   Current master is `71b6b20` (also carries N2 + the 0043 harness).

2. **The client may not have been talking to your server.** In dev
   mode the client hard-dials `ws://<hostname>:6500` regardless of the
   page's port ([src/main.js:30]); only `VITE_WS_URL` (build-time) or a
   PROD same-origin serve changes that. Your page was on :9777 while
   the owner's live pm2 server held :6500 all morning. If the client
   dialed :6500, the whole session ran a stale client against the
   owner's mid-play server — version skew that would manufacture
   exactly the symptom family you recorded (context menus missing
   Talk/Take, zone entry bouncing, skills firing without mana,
   monsters inert). The report does not prove which server the client
   used.

## Required corrections (revision 1)

1. Rerun both arcs at current master (`71b6b20` or later program tip).
2. Prove the wire: capture `window.ws.url` (console) in the first
   minute of each arc and include it in the log. The client must be
   connected to YOUR fresh-build server — either build with
   `VITE_WS_URL=ws://localhost:<yourport>` or serve the prod build
   from the game server process so same-origin resolves correctly.
   Keep avoiding the owner's :6500 (that part was right).
3. Re-verify every surviving friction item against the new session;
   mark each prior item FIXED / SURVIVES / NOT-REPRODUCED. Items that
   survive on a proven-correct environment become the P0 playability
   list — that inventory is exactly what D-110 needs, which is why the
   rerun matters.

## What is correct (keep)

The method (driven play, captures, code-cross-checking each claim
before ranking), the read-only discipline, the honest dev:kill
disclosure, and the positive findings (quest panel, adventure menu,
crypt ledger, onboarding copy) — those are worth keeping in the final
report. Fast re-review promised: this is a rerun, not a rework.
