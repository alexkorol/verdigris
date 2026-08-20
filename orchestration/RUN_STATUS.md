# Run status (snapshot — rewritten each architect sweep)

- Current integration tip: origin/codex/native-reconstitution @ e91b4cec
- Master: 79723db3 (PRs #20–#26 shipped 2026-08-18: 10 tasks)
- Overnight run 2026-08-19→20: architect on 3600s eco sweeps (owner
  token conservation); reviews rerun personally on flip, per goal.
- Immutable objective: D-116 parity (N1–N4 done, N5+N6 remain), D-110

## RUNNING

| Task | Owner | Notes |
|---|---|---|
| 0055 browser follow-ups | cursor | claimed 2ec96b77; delete + chip fix in progress (dirty tree seen 23:45) |
| 0056 N5 Chronicles | deepseek | STALLED ~28h — last commit fbe24587 08-18 19:36, 4 dirty preserved; dsh session likely dead, OWNER RESTART NEEDED (`C:\Users\Alex\tools\dsh.cmd web` + /goal from BOOTSTRAP.md; it will resume its claim) |

## READY (unclaimed)

| Task | Packet | Notes |
|---|---|---|
| 0058 N6 FINAL parity wave (CRITICAL) | BOUNDED-DESIGN | PIPELINED after 0056 integrates |
| 0059 responsive overlay pass | BOUNDED-DESIGN | mac-claude lane (browser-only); no claim pushed yet as of 08-19 23:50 |

## Fleet + budget

- deepseek: STALLED mid-0056 (see RUNNING). Ports 6540–6559.
- cursor (Grok 4.6): ACTIVE on 0055 — first task, calibrates scorecard. Ports 6580–6599.
- mac-claude (Sonnet, MacBook): onboarded (REENTRY-CLAUDE-MAC.md), no origin activity yet. Ports 7000–7019 (Mac-local). NEVER native/**.
- kimi K3 + kimi-work K3: quota-dead until ~08-23. kimi-work holds no claims (0055 released to cursor; its 7 dirty in clone are superseded — architect will reconcile if cursor's 0055 ships first).
- codex: out of tokens.
- qwen3.8 local (MacBook): DISPATCHABLE — MECHANICAL/offline eval only, via mac-claude as executor.

## Standing duty (new, 2026-08-20): peer verification

Idle coordinators (empty board) act as peer-verifiers before backoff:
rerun another coordinator's REVIEW_REQUESTED browser gates, commit
REVIEW-PEER-<name>.md transcripts. See STANDING-LOOP.md +
ORCHESTRATION.md G5 peer-rerun. Verdicts remain architect-only.
qwen3.8 note: it is an endpoint, not a coordinator — it works only
when mac-claude dispatches MECHANICAL packets to it (0059 bulk parts
qualify).

## WATCH

- deepseek stall = top fleet risk: N5+N6 are the critical path and only
  deepseek lane runs native waves right now. If still dead at morning,
  owner restarts dsh (or architect re-scaffolds N5 as own tracked task).
- kimi-work's stale 0055 work-in-clone vs cursor's fresh 0055: on cursor
  acceptance, kimi-work must discard its 7 dirty on resume (annotated in
  0055 RELEASE.md).
- loot scenario marginal timeout under load (1 sighting).
- Suite contention: never two full playtest suites at once per machine.

## Review procedures on flip

- 0055 (cursor, CALIBRATION): rerun test:unit + full playtest + smoke
  myself; verify src/core/adventure-objective-data.js DELETED; capture
  asserts green; 1 eyeballed capture max; scrutinize evidence honesty.
- 0056 (deepseek): rebuild + CUMULATIVE attach set MYSELF on architect
  port (chronicles, chronicles-first-combat, mortality, respawn,
  persistence + all N1–N4 scenarios); verify D-106/ADR-002/D-109 in
  diff; TaskStop server after.
- 0059 (mac-claude, CALIBRATION): rerun browser gates on Windows
  myself; capture-script JSON asserts at 1366x768 + 1280x720.
