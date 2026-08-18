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
