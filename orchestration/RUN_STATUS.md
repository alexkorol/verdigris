# Run status (snapshot — rewritten each architect sweep)

- Current integration tip: origin/codex/native-reconstitution @ 03678a06 (+ D-122 ship pending)
- Master: 03678a06 (PRs #20–#28)
- **D-122 adopted (owner research, 2026-08-20): three-axis parity.**
  The native exe currently never connects to the C++ server
  (verdigris_client links core only). C3 starts NOW, parallel to N5.
  N6 releases only after Gate A. Canon:
  docs/rebuild/NATIVE_PRODUCT_CONVERGENCE.md.
- Immutable objective: D-116 under the D-122 three-axis definition.

## RUNNING

| Task | Owner | Notes |
|---|---|---|
| 0061 networked guest slice (Gate A) | cursor | claimed 2094e09a — critical path; single-writer main.cpp is theirs |
| 0056 N5 Chronicles (server axis) | deepseek | STALLED ~29h (last commit 08-18 19:36; 4 dirty preserved). OWNER: relaunch dsh + re-paste /goal — it resumes the claim. Paths narrowed to native/src+tests (client is C3's now) |

## Recently integrated

- 0059 (cursor) ACCEPTED first-pass 02:30 — compact overlay stack at
  1366/1280; 55/55 capture asserts; honest defect audit. Cursor 2/2.
- 0060 (architect) DONE 01:35 — C3 session seam: FIRST EVER native
  client↔native server WebSocket handshake (login → authoritative
  snapshot). 19/19 session tests; scenarios untouched. 0061 UNBLOCKED.
- 0055 (cursor) ACCEPTED first-pass 01:10 — identity chip + server zone
  preview, mirror data file deleted. Cursor lane calibrated: high trust.

## READY / QUEUED

| Task | Packet | Notes |
|---|---|---|

| 0062 playtest flake triage | BOUNDED-DESIGN | NEW — mac-claude suggested; diagnostics only, assertions frozen |
| 0058 N6 final SERVER-parity wave | BOUNDED-DESIGN | HOLD until 0056 integrated AND Gate A accepted (D-122) |

## Standing duty: peer verification

Idle coordinators rerun peers' REVIEW_REQUESTED browser gates
(REVIEW-PEER-<name>.md) before backoff — see STANDING-LOOP.md.
Verdicts stay architect-only. qwen3.8 is an endpoint, not a
coordinator: mac-claude dispatches MECHANICAL packets to it.

## Fleet + budget

- deepseek: STALLED mid-0056 (see RUNNING). Ports 6540–6559.
- cursor (Grok 4.6): ON FIRE — 3 claims tonight (0055+0059 accepted, 0061 running). Ports 6580–6599.
- mac-claude (Sonnet): NO activity by 02:30 (0059 taken by cursor). If still dark at morning: owner re-check the Mac session. Ports 7000–7019. NEVER native/**.
- kimis: quota-dead until ~08-23. codex: out of tokens.
- architect: token-rationed (eco 3600s sweeps); 0060 is the one
  architect implementation block, per D-120.

## WATCH

- deepseek stall = critical-path risk for N5; C3 (0060→0061) is now a
  second critical path that does NOT depend on N5.
- kimi-work's stale 0055 clone work: discard on resume (cursor owns 0055).
- Single-writer main.cpp: 0061 claims it session-by-session in STATUS notes.
- playtest marginal-timeout flake: 2nd sighting (0059 review, 31/32 then 2x 32/32). Escalation due: flake-triage task (capture failing scenario id + timing) — speccing if a 3rd sighting or when a lane frees.

## Review procedures on flip

- 0055 (cursor, CALIBRATION): rerun unit+playtest+smoke myself; verify
  adventure-objective-data.js DELETED; ≤1 eyeballed capture.
- 0056 (deepseek): rebuild + CUMULATIVE attach set myself on architect
  port; D-106/ADR-002/D-109 in diff; TaskStop server after.
- 0059 (mac-claude, CALIBRATION): rerun browser gates on Windows;
  JSON asserts at 1366x768 + 1280x720.
- 0061 (later): full gates + I PLAY the exe vs the server + quality
  rubric (no zeroes, ≥9/12).
