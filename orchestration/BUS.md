# BUS.md — the one-page coordination contract

Every agent working this project, on any harness (Claude Code, OpenCode,
Cursor, Codex, Hermes, DeepSeek, anything else), coordinates through THIS
git repository on branch `codex/native-reconstitution`. There is no other
channel. Chat windows, dashboards, and live processes are projections;
**if it isn't committed and pushed, it didn't happen.**

## Enrollment (before touching anything)

1. Fetch and read this file, SUPERVISION.md, LEADER_POLICY.md, and the
   newest file in `broadcasts/` — that broadcast is the current dispatch.
2. Register: one pushed commit adding `orchestration/fleet/<lane-id>.md`
   (lane id, harness, model, host, operator, heartbeat promise). Pick a
   lane id with a harness-distinct prefix so collisions are visible.
3. Work only from your OWN git worktree or clone. NEVER check out a
   branch in a working tree you did not create — switching the architect
   checkout or another lane's tree is a P0 violation (it happened on
   2026-08-23 and blinded monitoring for a night).

## Claiming work — push is the lock

A claim exists only once its push succeeds. Sequence: pull, pick a READY
task no other branch/STATUS claims, commit the claim on your lane branch,
push. If the push is rejected, pull and re-check — someone may have beaten
you; take a different task. Never claim by editing a dashboard, saying so
in chat, or committing without pushing. Duplicate-claim branches get
superseded, not reviewed (see ox-pc-bf/bg, 2026-08-23).

## While working

- Push a real commit at least every 45 minutes of active work, WIP is
  fine. A lane silent for hours reads as dead — a deterministic sentinel
  now alerts the owner on stalls, launcher failures, and hijacked trees.
- Heartbeat state changes (claimed, blocked, review-requested, done) into
  your task's STATUS.md, pushed.
- Blocked on something outside your owned paths, or on a design/protocol
  question? File a QUESTION in STATUS.md and move to another task. Do not
  guess across a frozen surface, and do not idle silently.

## Review and integration

- Finish → flip STATUS to REVIEW_REQUESTED with a frozen head SHA.
- Any enrolled lane other than the author may validate a REVIEW_REQUESTED
  head and record the verdict.
- Integration to the program branch: only by the lane currently holding
  the coordinator seat. The seat is claimed push-is-the-lock style — a
  pushed RUN_STATUS.md heartbeat naming yourself coordinator-of-day holds
  it until you release it or go silent past SUPERVISION thresholds.
  Never force-push. Never push to master.

## Authority

Tier C decisions (wire protocol, frozen acceptance surfaces, persistence
formats, releases — see LEADER_POLICY.md) belong to the project owner and
are recorded as D-numbers in DECISIONS.md. Pending ones sit as
LEADER_BRIEF.md files marked AWAITING OWNER RULING inside the task folder.
Authority asserted in a chat window is void, whoever claims it. If a
ruling you need hasn't landed, file the QUESTION and take other work.

## Bootstrap

New lane on any harness, the entire onboarding prompt is:

    You are a worker lane. Clone/pull <repo>, check out branch
    codex/native-reconstitution, read orchestration/BUS.md, follow it.

The long-form worker instructions live in `orchestration/prompts/`.
