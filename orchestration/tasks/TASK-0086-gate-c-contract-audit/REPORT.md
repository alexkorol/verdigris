# REPORT — TASK-0086 Gate C campaign-decision contract audit

Worker: `ox-pc-c` · Branch: `codex/TASK-0086-gate-c-contract-audit-ox-pc-c`
· Base: `42718fbc4340589e606fff94a6eaa3dfbd03ad1c` · Route/base-refresh head
at claim: `039dcfa7f12497aa79c3677873a06a96c231a13d` · Machine:
DESKTOP-TVU7OR7 · Ports: 6660-6679 (none bound; no server needed) ·
Provider/model/harness: openrouter / stealth/ox-alpha / OpenCode CLI 1.18.21.

## Executive summary

Gate C (route decision on concrete info) is **not satisfiable from today's
surface**: of the six required fields, **boss/danger**, **depth**, and
**extraction/return condition** are AVAILABLE; **branch consequence** is
DERIVABLE-WITHOUT-GAMEPLAY-RULES for the immediate next stage only;
**concrete goal** and **expected trophy/material/item family** are MISSING on
both the browser-authoritative server and the N6 C++ server. A route name
(plus blurb/tier) alone still fails the contract. No reward/economy/balance/
risk value was invented; both MISSING fields are marked per stop conditions.
Five parity deltas between the browser and native chart surfaces are recorded
(P1-P5 in FINDINGS.md), including the documented intentional node-identity
divergence.

## Approach

Read-only audit. The six Gate C fields from
`docs/rebuild/NATIVE_PRODUCT_CONVERGENCE.md:68-72` were mapped against every
current producer/consumer/test of the world-web chart and zone surface:
browser handlers/services (`world-web.js`, `zone-service.js`,
`first-goal.js`), native N6 server (`native/src/networking.cpp` chart/zone/
extract paths), client consumer (`Chart.vue`), and the three test surfaces.
Each field cites event/payload/source/test labels, carries a single honest
classification, and names the smallest future owner path. Deliverables:
`FINDINGS.md` (narrative + citations) and `captures/gate-c-contract.json`
(machine-evaluable mapping).

## Changed files

All inside owned paths `orchestration/tasks/TASK-0086-gate-c-contract-audit/**`:

- `STATUS.md` — CLAIMED at `2ed4799a`, now REVIEW_REQUESTED.
- `FINDINGS.md` — new.
- `captures/gate-c-contract.json` — new.
- `captures/gate-c-rg-evidence.txt` — new (revision r2): unabridged Gate 2 rg stdout.

No file outside the task folder was created, modified, or deleted by this
worker (verified below). Forbidden paths (`native/**`, `server/**`, `src/**`,
`playtest/**`, `docs/product/**`) untouched.

## Public interfaces added/changed

None. Pure audit packet; zero code or protocol changes.

## Acceptance commands — literal transcripts and exit codes

### Gate 1 — contract JSON parses

```
$ node -e "JSON.parse(require('fs').readFileSync('orchestration/tasks/TASK-0086-gate-c-contract-audit/captures/gate-c-contract.json','utf8')); console.log('gate-c contract JSON: PASS')"
gate-c contract JSON: PASS
EXIT CODE: 0
```

### Gate 2 — evidence labels exist across the four audited surfaces

```
$ rg -n 'world:road:chart|world:zone:enter|nodeId|warden|trophy|depth|stairs|extract' native/src/networking.cpp native/tests/networking_tests.cpp playtest/scenarios/world-web.mjs playtest/scenarios/quest.mjs
EXIT CODE: 0
```

Unabridged stdout (119 matched lines, no elisions) is preserved verbatim in
`captures/gate-c-rg-evidence.txt` inside this task folder (revision r2). The
matches cover, across all four files: the `world:road:chart` /
`world:zone:enter` handlers (`native/src/networking.cpp:2314,2320`), chart
rows (`:1429` wardenName), zone metadata (`:783-799`, `:864-880` nodeId /
stairsUp / stairsDown / entryGate / zoneGates / wardenDead), extraction
(`:939-968`, `:2116-2124`, `:2451`, `:2486`), Warden refusal (`:1527`),
test labels (`native/tests/networking_tests.cpp:44,124,143-144,165-171,333-385`),
and scenario assertions (`playtest/scenarios/world-web.mjs:31,34,40-41,54,74-79,101,104,110`;
`playtest/scenarios/quest.mjs:255-260,325`).

### Gates 3 & 4 — whitespace check and changed-file list (run with deliverables on disk)

```
$ git diff --check
EXIT CODE: 0
$ git diff --name-only
EXIT CODE: 0
```

Honest note: both `git diff` gates are empty because this packet's
deliverables are NEW untracked files at run time (`git status --short`:
`?? orchestration/tasks/TASK-0086-gate-c-contract-audit/FINDINGS.md`,
`?? orchestration/tasks/TASK-0086-gate-c-contract-audit/captures/`), and
`git diff` covers tracked modifications only. After the REVIEW_REQUESTED
commit the same two commands remain exit 0 with empty output over a clean
tree.

## Manual verification

No server/client runs were required by the SPEC (MECHANICAL audit; no
acceptance servers, no port binds). Verification was line-level source and
test reading; every JSON citation was re-checked against the working tree at
the audited head. Cross-checks performed: absence greps for goal/reward keys
in both chart builders; absence of `levelHint`/`childIds`/`world:chart:updated`
in `native/src/networking.cpp`; absence of any chart pane under
`native/client/`.

## Base-to-head path boundary

`git diff --name-only 42718fbc..HEAD` shows only the architect's
coordination-only refresh `039dcfa7` (ORCHESTRATION/REENTRY/RUN_STATUS plus
four lane SPECs) and this worker's single commit `2ed4799a` touching exactly
`orchestration/tasks/TASK-0086-gate-c-contract-audit/STATUS.md`; the pending
REVIEW_REQUESTED commit adds FINDINGS.md, captures/gate-c-contract.json,
REPORT.md, and the STATUS transition — all inside owned paths. Worker-authored
changes never leave `orchestration/tasks/TASK-0086-gate-c-contract-audit/**`.

## Commit SHAs

- `2ed4799a5385f2d6237697a31305f5671b437e93` — claim (STATUS.md only).
- `d8ab6670df742d3946886de91913be728bb535ed` — REVIEW_REQUESTED: FINDINGS.md, captures/gate-c-contract.json, REPORT.md, STATUS.md transition (architect-reviewed head).

## Deviations

- Pre-commit hook initially failed (`yorkie` runner missing because the
  isolated worktree had no `node_modules`); resolved by running `npm install`
  locally (gitignored) rather than skipping hooks. lint-staged then ran with
  zero matching files for markdown-only commits.
- None otherwise. Variant/reasoning settings were not observable in-session
  and are recorded as absent in STATUS per packet rule.

## Unresolved questions

None requiring escalation. Owner decisions this audit stages (goal content;
trophy/material family tables; whether to reconcile cross-server chart
identity P2/P3) belong to future owner-authority packets, not questions.

## Risks

- P2/P5 mean Gate C cannot be driven natively until a native chart pane exists
  and node identity is either reconciled or consciously accepted as
  per-server (already documented acceptable in code at
  `networking.cpp:644-646`).
- FINDINGS line references are exact at the audited tree but will drift if
  `networking.cpp`/`world-web.js` move; the JSON records the audit head.

## Follow-ups (routing suggestions, not claims)

- TASK-0089 (Gate C native journey) consumes this audit; its spec already
  requires "missing fields resolved".
- Candidate owners for the MISSING fields per RUN_STATUS board: TASK-0104
  itemization/history audit, TASK-0103 monster/encounter gap audit, plus an
  owner ruling on chart-goal content.

## Revision r2 — evidence-only corrections per REVIEW.md (REVISE)

Architect review at head `d8ab6670df742d3946886de91913be728bb535ed` required
exactly two evidence corrections; both applied additively with all substantive
findings and classifications preserved:

1. Gate 2's rg transcript is now the unabridged, unelided stdout saved
   durably at `captures/gate-c-rg-evidence.txt` (119 lines, exit 0); REPORT
   cites the artifact instead of the session transcript.
2. The `<this commit>` placeholder is replaced with the exact reviewed head
   `d8ab6670df742d3946886de91913be728bb535ed`; this revision's SHA is appended
   below after pushing. STATUS remains REVIEW_REQUESTED at the new head.

Post-correction reruns at the revision head: Gate 1 PASS (exit 0), Gate 2
exit 0, `git diff --check` exit 0, `git diff --name-only` empty (exit 0),
and `git diff --name-only 039dcfa7..HEAD` contains only this task folder's
files plus the architect's coordination refresh — owned-path boundary holds.

- Revision r2 evidence commit: PENDING-SHA
