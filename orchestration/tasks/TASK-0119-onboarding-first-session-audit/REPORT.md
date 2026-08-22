---
task: TASK-0119
state: REVIEW_REQUESTED
worker: ox-pc-u
base_commit: 9fe673b66ffc082e865e0f0fb66f454ec1984949
implementation_commit: (this commit — see REPORT.md)
---

# TASK-0119 — REPORT

## Executive summary

Delivered the SPEC-owned first-session audit for the native owner journey:
`FINDINGS.md` (evidence-backed, 13-step journey matrix, two negative-control
findings, prioritized non-lore fix list for the onboarding successor) and
`captures/first-session.json` (machine-readable stepwise matrix, 13 steps, each
with client-shown state, required player decision, legibility flag, file:line
evidence, friction, and a smallest structural fix). No game source, SPEC,
REVIEW, other task, other worktree, or port 6500 was touched. Resource capsule
honored: read-only; no ports opened; no play-server mutation.

Headline result: the existing journey is coherent and mostly legible; the two
invisible player decisions are (G-1) expedition goal choice — hardcoded
`tin:1:0`, core phase events dropped by the presentation — and (G-2)
progression beyond gear — server XP/levels and the `passiveTree` payload have
no native client surface. The single active owner-facing lie is the extraction
instruction: the objective strip says "press F there" while the remote path
extracts only by walking onto the stairs (`remote_session.cpp:437-442`).

## Approach

- Preflight per AGENTS.md; verified no existing STATUS.md/RELEASE.md for
  TASK-0119 anywhere in the repo or remotes (first-STATUS-write-wins claim,
  commit `fad856c3`, pushed to this branch only).
- Static evidence audit on routed base `9fe673b6` (spec base `9bd689b4`):
  read `native/client/*` (main.cpp, remote_session.cpp, presentation_state.cpp,
  local/session seams), `native/src/core.cpp` + `include/verdigris/core.hpp`
  (events, phases), `native/src/networking.cpp` (login/chronicles/XP/tree
  payloads), `native/tests/session_tests.cpp`, `native/tools/play-native.ps1`,
  `native/README.md`, `native/persistence/README.md`, constitution, and
  rebuild docs; leaned on in-tree deterministic scenarios (`first-fight`,
  `loot-to-bank`, `telegraph-dodge`, `combat-juice`, `chronicles-gate-b`) and
  the accepted TASK-0145 review for behavioral claims instead of running
  servers, per the read-only capsule.
- Wrote the JSON matrix first, then distilled FINDINGS.md from it; both
  cross-reference the same file:line evidence.

## Changed files (all inside the owned folder)

- `orchestration/tasks/TASK-0119-onboarding-first-session-audit/STATUS.md`
  (claim, then REVIEW_REQUESTED in this commit)
- `orchestration/tasks/TASK-0119-onboarding-first-session-audit/FINDINGS.md`
- `orchestration/tasks/TASK-0119-onboarding-first-session-audit/captures/first-session.json`
- `orchestration/tasks/TASK-0119-onboarding-first-session-audit/captures/acceptance-rg-scan.txt`
  (raw output of acceptance command 1, retained as evidence)

## Public interfaces added/changed

None. Audit-only task; no code, wire, or asset changes.

## Acceptance commands and outcomes

1. `rg -n "launch|connect|House|Scion|guide|tutorial|quest|goal|route|equip|extract|death|reconnect|error" native/client native/tests docs/rebuild docs/product orchestration/benchmarks`
   → exit 0; 952 matching lines; full output preserved at
   `captures/acceptance-rg-scan.txt`. PASS.
2. `node -e "const x=JSON.parse(require('fs').readFileSync('orchestration/tasks/TASK-0119-onboarding-first-session-audit/captures/first-session.json','utf8')); if(!Array.isArray(x.steps)||!x.steps.length) process.exit(1); console.log('first session: PASS')"`
   → printed `first session: PASS`. PASS.
3. `git diff --check` → exit 0, no whitespace errors. PASS.
4. `git diff --name-only` → only
   `orchestration/tasks/TASK-0119-onboarding-first-session-audit/**` entries
   (FINDINGS.md, captures/first-session.json). Expected "only this folder
   changes" satisfied. PASS.

## Manual verification

- Journey coverage check against SPEC: launch/connect, House/Scion, controls,
  goal choice, combat, loot/equip, extraction, progression, death/recovery,
  quit/relaunch, error/reconnect — all present as steps 1–13 in
  `captures/first-session.json` (`summary_counts.steps: 13`).
- Negative control preserved: step 5 (goal choice) and step 10 (progression)
  carry `decision_legible: false` with evidence; step 5 is the SPEC-named
  negative control.
- Guards honored: no narrative copy invented; every proposed fix is structural
  (surface authoritative state, correct a contract mismatch, add an input
  seam); FINDINGS.md explicitly instructs the successor to extend the existing
  loose objective-strip guidance pattern and forbids checklist tutorial design.
- Claim hygiene: only this branch was pushed (`codex/TASK-0119-...-ox-pc-u`);
  no merges, rebases, force-pushes, or other-actor state touched.

## Commit SHAs

- Claim: `fad856c3` (STATUS.md CLAIMED, pushed)
- Implementation/report: the commit containing this REPORT.md (HEAD of
  `codex/TASK-0119-onboarding-first-session-audit-ox-pc-u`)

## Deviations

- SPEC `base_commit` is `9bd689b4`; work was routed onto `9fe673b6` per the
  task routing. The audit is evidence-anchored to the routed base and notes
  both in all deliverables.
- No runtime captures were produced (read-only resource capsule); behavioral
  claims rest on in-tree deterministic scenarios and the accepted TASK-0145
  review, each cited inline.
- The pre-commit hook initially failed because this worktree had no
  `node_modules` (shared yorkie hook); resolved by running `npm install`
  (untracked, ignored) — no tracked files changed as a result.

## Unresolved questions

- Should the native client expose owner-facing House/Scion naming before final
  narrative wording exists? (FINDINGS fix #10 is blocked on this owner
  decision; everything else is unblocked.)

## Risks

- Low. Audit-only. If any cited line drifts before the successor lands, the
  file:line anchors are pinned to base `9fe673b6` in the deliverables.

## Follow-ups (recommended successor shape)

- One small client-only implementation task for FINDINGS fixes #1–#2
  (extraction instruction mismatch; render `ExpeditionPhaseChanged`), then a
  second for #3–#5 (progression surfacing, controls hint + README drift,
  front-door reconnect action). Items #10–#11 route as questions to the
  owner. The recorded zero-life-heir red stays with TASK-0148.
