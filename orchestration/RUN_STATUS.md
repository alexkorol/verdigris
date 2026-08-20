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
| 0056 N5 Chronicles (server axis) | deepseek | STALLED ~29h (last commit 08-18 19:36; 4 dirty preserved). OWNER: relaunch dsh + re-paste /goal — it resumes the claim. Paths narrowed to native/src+tests (client is C3's now) |
| 0060 C3 session seam scaffold | architect (Fable) | ARCHITECTURE packet — next architect work block; unblocks 0061 |

## Recently integrated

- 0055 (cursor) ACCEPTED first-pass 01:10 — identity chip + server zone
  preview, mirror data file deleted. Cursor lane calibrated: high trust.

## READY / QUEUED

| Task | Packet | Notes |
|---|---|---|
| 0059 responsive overlay pass | BOUNDED-DESIGN | mac-claude lane; onboarding as of 00:30 |
| 0061 networked guest slice (Gate A) | BOUNDED-DESIGN | PIPELINED after 0060 integrates — **new critical path** |
| 0058 N6 final SERVER-parity wave | BOUNDED-DESIGN | HOLD until 0056 integrated AND Gate A accepted (D-122) |

## Standing duty: peer verification

Idle coordinators rerun peers' REVIEW_REQUESTED browser gates
(REVIEW-PEER-<name>.md) before backoff — see STANDING-LOOP.md.
Verdicts stay architect-only. qwen3.8 is an endpoint, not a
coordinator: mac-claude dispatches MECHANICAL packets to it.

## Fleet + budget

- deepseek: STALLED mid-0056 (see RUNNING). Ports 6540–6559.
- cursor (Grok 4.6): idle post-0055 — next: peer-verify duty or 0059 if mac-claude stays dark. Ports 6580–6599.
- mac-claude (Sonnet): onboarding; 0059 routed. Ports 7000–7019. NEVER native/**.
- kimis: quota-dead until ~08-23. codex: out of tokens.
- architect: token-rationed (eco 3600s sweeps); 0060 is the one
  architect implementation block, per D-120.

## WATCH

- deepseek stall = critical-path risk for N5; C3 (0060→0061) is now a
  second critical path that does NOT depend on N5.
- kimi-work's stale 0055 clone work: discard on resume (cursor owns 0055).
- Single-writer main.cpp: 0061 claims it session-by-session in STATUS notes.
- loot scenario marginal timeout (1 sighting); suite contention (serialize per machine).

## Review procedures on flip

- 0055 (cursor, CALIBRATION): rerun unit+playtest+smoke myself; verify
  adventure-objective-data.js DELETED; ≤1 eyeballed capture.
- 0056 (deepseek): rebuild + CUMULATIVE attach set myself on architect
  port; D-106/ADR-002/D-109 in diff; TaskStop server after.
- 0059 (mac-claude, CALIBRATION): rerun browser gates on Windows;
  JSON asserts at 1366x768 + 1280x720.
- 0061 (later): full gates + I PLAY the exe vs the server + quality
  rubric (no zeroes, ≥9/12).
