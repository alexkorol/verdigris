---
task: TASK-0091
title: Native client protocol coverage sentinel design
verdict: ACCEPTED
reviewer: deepseek-v4-flash (independent validator)
reviewed_commit: 98df3301
reviewed_at: 2026-08-23T19:00:00Z
revision: 1
---

# Review — TASK-0091 (native client protocol coverage sentinel design)

## Verdict: ACCEPTED

Frozen head `98df3301` (worker branch `worker/verdigris/pc/ox-pc-bc`),
deliverables commit `bac82984`, reviewed in detached worktree
`review-task0091-98df3301`.

## Scope

Worker-only delta `64fab114..98df3301` touches only
`orchestration/tasks/TASK-0091-native-protocol-coverage-sentinel/**`
(FINDINGS.md, REPORT.md, STATUS.md, captures/coverage.json,
captures/acceptance-transcripts.txt). Matrix and native sources unedited;
read-only capsule honored (no ports, no servers, port 6500 untouched).

## Acceptance gates

1. `rg -n "Gate A|Gate B|Gate C|chronicles:|world:|player:" .../NATIVE_CLIENT_PROTOCOL_MATRIX.md`
   → 14 lines, exit 0.
2. `rg -n "RemoteProtocolSession|ClientSnapshot|PresentationEvent|render_list" native/client native/tests`
   → 305 lines, exit 0.
3. `node -e "JSON.parse(...coverage.json...); console.log('coverage JSON: PASS')"`
   → prints `coverage JSON: PASS`, exit 0.
4. `git diff --check` → clean on the frozen tree, exit 0.
5. `git diff --name-only` → owned additions only, exit 0.

Note: an inter-commit diff shows "new blank line at EOF" in the transcript
log file (cosmetic; the acceptance `git diff --check` passes on the clean
frozen tree). Not a source defect; the transcript is a capture artifact.

## Evidence quality

- FINDINGS.md is excellent: maps all **23 matrix rows** to four evidence columns
  (native handler, client reducer/model, presentation op, automated test) with
  current-tip file:line citations, classified **19 COVERED / 2 PARTIAL / 2 RED**.
  It correctly detects matrix drift (stale line anchors, outdated blanket
  "no chronicles test label" claim) and documents it without editing the matrix.
- **Negative controls verified genuine (all literal rg no-match, exit 1):**
  - N-1 (A12 item drop): server emits `world:itemDropped`/`item:change`
    (networking.cpp:997-998) but **zero** references exist in `native/client` —
    the envelopes are dead letters; ground sync rides the `dev:state` poll seam.
  - N-2 (A14 equip): `player:equippedAnItem` unconsumed by name; equip outcome
    inferred from `core:refresh:inventory` diffing.
  - N-3 (B04 successor): `player:chronicles:return` has no client seam /
    ClientCommand type; zero references in `native/client`.
  - N-4 (A18/B05 persistence): no durable cross-process store; `rg
    'ofstream|ifstream|fopen|fwrite' native/src` → no matches.
- The proposed read-only sentinel interface (static, stdlib-only, matches by
  literal envelope names/test labels rather than rotting line numbers, wired to
  `captures/coverage.json`) is concrete and correctly deferred to a successor.
- Machine-readable twin `captures/coverage.json` parses.

## Capsule

Read-only audit respected throughout: no code patched, no ports, no servers,
port 6500 untouched, only owned task-folder paths changed.

## Follow-up

Successor work per the recommendation: A12 consume-or-document the item-drop
channel; A18/B05 durable leg behind `persistence/adapter.hpp` + a restart test;
B04 add a ReturnToHouse intent or demote the envelope; adopt the phase-1
sentinel wired to coverage.json. A docs-owned task should refresh
NATIVE_CLIENT_PROTOCOL_MATRIX.md.
