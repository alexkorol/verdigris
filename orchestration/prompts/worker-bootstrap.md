# Worker-lane bootstrap (long form)

Paste target for any harness. Supersedes all older ad hoc bootstrap
prompts. The authoritative contract is orchestration/BUS.md — if this file
and BUS.md disagree, BUS.md wins.

---

You are bootstrapping ONE worker lane for the Verdigris fleet.

1. Pull branch `codex/native-reconstitution`. Read orchestration/BUS.md
   fully and follow it — enrollment first, then claim.
2. Read the newest file in orchestration/broadcasts/ — that is the current
   dispatch: which tasks are claimable, who coordinates, any standing
   rules for the day.
3. Create your OWN worktree/clone for your lane branch. Never switch a
   checkout you did not create.
4. Verify your toolchain before claiming: `npm ci` (or the repo's install
   step) in your fresh worktree — a missing node_modules silently breaks
   commit hooks; and make one trivial provider/auth call — a dead session
   burned 10 hours on 2026-08-22 because a 401 went unnoticed.
5. Claim per BUS.md (push is the lock). Pushed claim within 10 minutes of
   starting or self-report stuck — 10 min is the P1 threshold, 30 min P0.
6. Implement inside your task's owned_paths only. Commit+push increments
   at least every 45 minutes. STATUS.md heartbeats on every state change.
7. Hit a wall outside your paths or a frozen surface? QUESTION in
   STATUS.md, move on to another READY task. Never guess, never idle
   silently, never use --auto or blanket permission approval.
8. Done → REVIEW_REQUESTED with frozen head SHA. Do not integrate your own
   work; the coordinator-of-day integrates accepted heads.
