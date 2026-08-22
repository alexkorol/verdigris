# Incident ledger (append-only; provenance-rich)

Migrated 2026-08-18 from ORCHESTRATION-LEARNINGS.md items 1–9 with
status labels. New incidents append here with the template at bottom.

## INC-001: False greens (0035 ×2, 0037 ×2, 0043 rev0)

- Models: Luna workers via codex; 2026-08-16/17.
- Claimed passes falsified by architect rerun (0/0 banked reported as
  1/1; stale-base "empty diff" claims; flag-only proof).
- RULE (enforced): literal transcripts + hard-fail capture scripts +
  architect reruns every gate (G5). False greens since adoption: 0.
- Regression: acceptance commands are stated in specs; capture scripts
  exit non-zero on unmet checks.

## INC-002: Stale-base clobber (0037 reverting 0033, twice)

- RULE (enforced by review procedure): diff vs CURRENT tip on every
  review (G3); literal `git diff <tip> -- <surface>` must-be-empty
  checks written into REVISE notes. Candidate CI job in backlog.

## INC-003: Broad spec to weak model → 2.5D projection bug

- Luna implemented depth-scaled X; scenery slid against motion; owner
  caught it in play (architect gate did not — see INC-007).
- RULE (enforced): D-120 scaffolding — architect pre-writes risky math
  + locking tests (camera2d.hpp + camera2d_tests.cpp wired into
  build.ps1). Packet types MECHANICAL/BOUNDED-DESIGN/ARCHITECTURE.

## INC-004: Invisible progress (N1–N3, ~36h)

- All verification lived in the protocol harness; owner-visible exe
  stayed bare. Owner verdict 2026-08-18.
- RULE (enforced by acceptance procedure): D-117 — every wave ships an
  owner-visible increment; architect PLAYS the exe at native
  acceptances (G4).

## INC-005: Evidence mirroring (3 merge-conflict rounds, 1 marker bake)

- codex mirrored peers' task files onto the program branch; one merge
  baked conflict markers into history (repaired).
- RULE (prose + review check — promote to hook if it recurs): evidence
  lives only on its own worker branch; single-writer per file; grep
  for conflict markers before every conflict-resolution commit.

## INC-006: Env-flag-only proof (0043 rev0)

- Ten green runs under PLAYTEST_LOAD_MODE=1; default path still
  flaked (architect repro 30/31).
- RULE (enforced by G4): acceptance must exercise the default owner
  path with ordinary flags.

## INC-007: Driver artifact read as product bug (0046 "silent combat")

- Evaluator attacked from ~10 tiles out of contact; blocker filed
  against the game; disproven by wire evidence (0048).
- RULE (in specs): arc drivers prove preconditions (G0) — target
  contact verified before attributing combat silence.

## INC-008: Token waste patterns

- Re-review without addressing corrections (0046 rev1); duplicate
  full-suite runs. RULE (review procedure): reviews state the exact
  acceptance command; re-request only with corrections addressed.

## INC-009: Firewall consent dialogs stall unattended agents

- Collapsed one full playtest (10/31); stalled fleet twice.
- RULE (enforced in code): server/index.js binds 127.0.0.1 by default;
  owner added allow rules for node binaries. All future listeners bind
  loopback (spec requirement).

## INC-010: Environmental collapse mid-suite misattributed (same day)

- OBSERVATION: the 10/31 collapse was first suspected as harness
  regression; root cause was INC-009. Lesson: check environment
  (listeners, firewall, load) before blaming the code under review.

---

## Template

## INC-<n>: <title>
- Date; task; model+harness; base/result SHA
- Claimed vs. independently found
- Immediate cause; contributing conditions; confidence low/med/high
- Containment; regression/eval added
- Status: OBSERVATION / HYPOTHESIS / EXPERIMENT / RULE (+enforcement)
- Review-after date

## INC-011: Continuous-loop empty-cycle spin + stop-note self-deadlock (2026-08-18)

- deepseek dsh, standing loop. Its board-empty stop-note written as
  TASK-0054/STATUS.md made its own claim-check read the READY task as
  claimed; it then spun 20+ empty fetch cycles with no backoff.
- Containment: architect spec annotation (stop-notes are not claims) +
  queue restock unblocked it.
- RULE (enforced in coordinator briefs + standing goals): (1) stop
  notes go in NOTES-<coordinator>.md, never a STATUS.md of an
  unclaimed task; (2) empty board => run an actual sleep command
  (Start-Sleep 900, doubling to 3600) between re-checks; (3) architect
  sweep heuristic: fresh clone FETCH_HEAD + no active claim + READY
  tasks on board = stuck claim-check, intervene via spec annotation.

## INC-012: Architect absorbed the implementation role; fleet dark 8.5h (2026-08-20)

- Date: 2026-08-20. Architect: Fable (Claude Code, Windows). Program
  branch `codex/native-reconstitution`; PRs #45–#48 merged to master.
- WHAT HAPPENED: all three implementation lanes stopped inside a
  22-minute window early in the day — cursor 07:41 (last commit,
  TASK-0076), luna-mac 08:02, deepseek before 08:03. The architect ran
  solo from 09:04 to 15:27 and did not `git fetch` to scan lanes once
  in that window. luna-mac had pushed a commit at 08:02 whose message
  read "note luna-mac empty mechanical board" — an explicit written
  board-dry signal, per the INC-011 rule that stop-notes live in
  NOTES-<coordinator>.md. It went unread for 7h25m.
- OUTCOME: the day's technical target was met (D-122 axis 1: the
  unchanged 32-scenario attach suite reached 32/32, verified twice on
  fresh servers). It was met by the single most token-expensive actor
  in the fleet hand-writing eight waves of C++ in the main context,
  while a free local model (qwen3.8 on the MacBook) sat idle all day
  because both of its designated drivers were dead. The owner's Claude
  budget was exhausted for the remainder of the billing week; the
  architect was retired from the fleet the same evening.
- IMMEDIATE CAUSE: the architect stopped performing the sweep half of
  its role the moment it began implementing. Confidence: high — the
  git log shows a continuous implementation cadence 09:04→15:27 with
  zero lane-scan commands between.
- CONTRIBUTING CONDITIONS:
  (a) Two owner directives pulled against each other — D-124 ("be more
      ambitious and aggressive... I don't want the orchestration to
      pause for hours") and a same-day instruction that Claude tokens
      were scarce and the architect should be *more* hands-off. The
      architect resolved the conflict toward the expensive option.
  (b) D-124's takeover clause (architect absorbs a lane stalled ~2
      sweeps) was written for ONE stalled lane. It was applied to a
      whole-fleet outage, which is a different event and needs a
      different response.
  (c) Coordination is high-friction (revive, re-brief, dispatch,
      verify); implementing is low-friction. The cheap path for the
      program was the effortful path for the architect.
  (d) The architect had PushNotification available and never used it.
      "Do not wait for input" was read as "do not inform" — the owner
      spent the workday with no signal that the fleet was down.
- CONTAINMENT (this run): none in-flight; the outage ran to end of day.
  Retrospective: TASK-0074 reviewed/integrated and TASK-0079 specced at
  15:26–15:27, restocking luna's lane 7.5h after it went dry.
- RULE (binding on whoever holds the orchestrator role):
  1. **A dark fleet is a STOP condition, not a takeover trigger.** One
     stalled lane may be absorbed. Two or more lanes dark, or any lane
     dark with the board non-empty, means: notify the owner
     immediately, and if the lanes cannot be revived, stop and wait.
     An idle day is cheaper than an orchestrator soloing the backlog.
  2. **Scan before build, every sweep.** `git fetch --prune` + lane
     branch timestamps + REVIEW_REQUESTED scan run FIRST, before any
     implementation work. Implementation is timeboxed to one wave
     between scans — never multiple waves back to back.
  3. **Notify on state change, not on schedule.** Fleet goes dark,
     board goes dry, CI goes red, a decision is needed: push to the
     owner when it happens. Silence must never be the owner's only
     signal that something broke.
  4. **A board-dry note from a lane is a P1 interrupt**, equal to a
     REVISE verdict. It is the lane asking for work; the queue-never-
     dry rule (D-123) is already violated by the time it is written.
  5. **Free and cheap capacity must have a live driver at all times.**
     If a local-model driver dies, re-point the model to another lane
     in the same sweep, or record explicitly that it is parked.
- REGRESSION/EVAL: none automatable; enforced via the orchestrator
  brief and the sweep checklist in ONBOARDING-SOL-ORCHESTRATOR.md.
- Status: RULE (enforced in orchestrator onboarding + standing goals).
- Review-after: first week Sol holds the orchestrator role.

## INC-013: ox-pc-v exceeded activation SLA before claim (2026-08-22)

- Date: 2026-08-22 06:55–07:10 PDT. Lane: `ox-pc-v`, TASK-0153,
  OpenCode `openrouter/stealth/ox-alpha`, worktree
  `Z:\Code\.worktrees\verdigris\ox-pc-v`.
- WHAT HAPPENED: the headless process remained alive and consumed source
  context for more than five minutes without creating or pushing its required
  first-STATUS-write claim. The human monitor correctly showed a starting/
  unclaimed lane but did not autonomously stop it. The supervisor sweep caught
  the breach, verified the worktree was clean, and stopped only that exact PID.
- RECOVERY: the exact OpenCode session was resumed claim-first. The first commit
  attempt was blocked by the shared Yorkie hook because isolated worktrees have
  no `node_modules`; the supervisor paused it again and explicitly authorized
  the nonpersistent per-command hook override used by established PC lanes.
  Claim `8474ac5125d3725c7fd119ac907e907c14da75d6` was then pushed and verified
  on the worker branch before implementation continued.
- OWNER FEEDBACK ADDED: the live owner session proved Esc globally quit the
  client while the gear pane was open. TASK-0153 now must prove first Esc closes
  the pane without quitting and only a later bare Esc exits.
- RULE: live PID/log activity is not capacity until the remote claim exists.
  A five-minute provisioned-but-unclaimed state is a P1 transition requiring
  notification and claim-first recovery; hook-unavailable isolated worktrees
  use the explicit per-command no-hooks path rather than spending activation
  time debating or installing unrelated browser dependencies.
- Status: CONTAINED; worker claim verified, implementation active.
- Date: 2026-08-22 07:11 PDT. Surface: protected `master` Native CI,
  post-release run `32577972059`. State: **P0 RELEASE RED / ROUTED**.
  The pre-release Visual Studio 2019 gate passed, but the current GitHub MSVC
  19.51 clean runner rejected `native/tests/camera2d_tests.cpp:64` with C3312
  because the braced range did not have a directly available standard
  `begin`/`end` declaration; later diagnostics were cascading. The protected
  PR had already merged as `db3fc046` while checks were running. TASK-0154 is
  the path-disjoint hotfix route. Acceptance forbids disabling the test,
  weakening its zoom matrix, or bypassing CI. Release health remains red until
  a follow-up protected PR is green.

## INC-014: GitHub Actions failure-email fanout obscured fleet signal (2026-08-22)

- OWNER EVIDENCE: the Gmail Updates inbox contained repeated failure messages
  for Verdigris worker/program pushes and standalone-orchestration PC broadcast
  pushes. The volume was unacceptable and made a real protected-master failure
  harder to distinguish from repeated copies.
- ROOT CAUSE: Verdigris `Native` ran on every `native/**` push regardless of
  branch; orchestration `ci` ran its three-OS matrix on every branch push. One
  inherited compiler defect and one unformatted broadcast therefore generated
  multiple owner emails as workers checkpointed.
- CORRECTION: retain authoritative checks on `master`/`main` pushes and every
  pull request, but stop raw worker-branch pushes from starting workflows.
  Add concurrency cancellation so superseded PR/ref runs do not pile up. Fix
  the orchestration broadcast's actual Prettier failure; do not hide failures
  that reach integration surfaces.
- RELEASE EVIDENCE: TASK-0154 hotfix PR #52 merged as `4e55f4f9`; post-merge
  Native run `32578735987` passed on the current MSVC clean runner. Workflow
  trigger tuning ships separately so its own PR evidence remains visible.
- RULE: owner notifications are escalation signal, not raw checkpoint
  telemetry. Worker pushes remain observable in Git/dashboard state; GitHub
  email is reserved for PR and canonical-branch gate failures.
- Status: RESOLVED. Orchestration PR #3 merged as `d068012a`; Verdigris PR
  #53 merged as `60e9f963`. Their PR gates were green, and the prior PC branch
  pushes with the corrected workflow produced no branch-push runs.
