# REPORT — TASK-0130 Gate C route-decision envelope and validation contract

Worker: `ox-pc-e` · Branch: `codex/TASK-0130-gate-c-decision-envelope-ox-pc-e` ·
Immutable base: `cab50d62cb121ab6a88fa513257e645447226959` · Routed HEAD at
claim: `b3599c80122d09cd0685ae96830990cc5bada5cf` · Machine: DESKTOP-TVU7OR7 ·
Ports: 6700–6719 (none bound; documentation packet, no server needed) ·
Provider/model/harness: openrouter / stealth/ox-alpha / OpenCode CLI 1.18.21.

## Executive summary

TASK-0086's accepted six-field audit is now an exact, content-neutral wire and
validation contract for a future Gate C route decision. Deliverables inside the
owned task folder: `gate-c-decision-envelope.json` (versioned envelope schema
with per-field states, evidence citations, and an honest
`completeness.ready: false`), `VALIDATION.md` (13 deterministic error codes,
first-match-wins ordering, decision-readiness rule), and
`fixtures/negative-cases.json` (one synthetic content-neutral negative case per
documented code — the nine SPEC-required codes plus `INVALID_JSON`,
`MISSING_ROUTE_IDENTITY`, `CONTRADICTORY_DEPTH`, and `OWNER_PENDING_CONTENT`
for full coverage). MISSING evidence is preserved honestly: concrete goal and
expected item-family remain `MISSING` with null values and owner-pending
flags; no campaign, reward, economy, risk, or balance value was invented; a
route name/tier/blurb alone remains invalid (`ROUTE_NAME_ONLY`); native/browser
node identity is explicitly NOT reconciled (recorded as owner/implementation
ruling, TASK-0086 parity P2).

## Approach

Read TASK-0086's ACCEPTED outputs (FINDINGS.md, captures/gate-c-contract.json,
REVIEW.md verdict at reviewed head
`8ddfb06e16f85c150e9a79ccc9d8bd4932664369`) and translated each audited field
into a contract field carrying: state, wire shape, citations, bounded gaps, and
the smallest future owner path. VALIDATION.md derives its codes directly from
the SPEC-required failure categories plus the minimal structural checks needed
to make evaluation deterministic; every documented code has a fixture. The
envelope's `completeness` block encodes the SPEC's Gate-1 false-complete rule
as check 13 so schema and validator cannot disagree.

## Field → TASK-0086 evidence mapping

| Contract field | TASK-0086 classification | Basis (abridged; full citations live in the envelope JSON) |
|---|---|---|
| `route_identity` | AVAILABLE | road/node/tier/status on both chart builders; name alone fails by authority definition |
| `concrete_goal` | **MISSING** | no goal/objective key on either surface; only route-agnostic first goal exists |
| `boss_or_danger` | AVAILABLE | wardenName + levelHint band + direction/blurb flavor; pack composition visible only after entry |
| `expected_item_family` | **MISSING** | no drop/trophy/family key pre-announced per route on either surface |
| `depth` | AVAILABLE | node tier charted; instances generate depth = tier; stairs metadata tested |
| `branch_consequence` | DERIVABLE-WITHOUT-GAMEPLAY-RULES | immediate next stage computable from statuses/unlock-rule/links; frontier cap bounds visibility (owner ruling needed beyond); C++ rows omit childIds (P1) |
| `extraction_or_return` | AVAILABLE | stairsUp returns to Crossroads; player:extract bank summary; 15-min linger window, 24 h respawn suppression |
| `evidence_provenance` | AVAILABLE | contract-level requirement + this contract's own provenance block (authority doc, accepted audit head, base commit) |

Smallest future owner/implementation paths are recorded verbatim per field in
`gate-c-decision-envelope.json` (`smallest_future_owner_path`). Net effect:
Gate C stays blocked today by exactly `MISSING_CONCRETE_GOAL` and
`MISSING_EXPECTED_ITEM_FAMILY`; everything else is present or derivable within
its documented scope.

## Changed files

All inside owned paths `orchestration/tasks/TASK-0130-gate-c-decision-envelope/**`:

- `STATUS.md` — CLAIMED (claim commit), transitioned to REVIEW_REQUESTED below.
- `gate-c-decision-envelope.json` — new.
- `VALIDATION.md` — new.
- `fixtures/negative-cases.json` — new (13 cases).
- `captures/gate-c-rg-evidence.txt` — new (unabridged Gate 3 rg stdout, 119 lines).
- `REPORT.md` — this file.

No file outside the task folder was created, modified, or deleted by this
worker. Forbidden paths (`native/**`, `server/**`, `src/**`, `playtest/**`,
`docs/product/**`) were read for Gate 3 only, never written.

## Public interfaces added/changed

None in code. This packet adds documentation-only contract artifacts under the
task folder; zero source, protocol, test, or product-authority changes.

## Acceptance commands — literal transcripts and exit codes

Run from repository root with deliverables committed (HEAD `419cf3b1…`).

### Gate 1 — envelope parses, required keys, no false completeness

```
$ node -e "const fs=require('fs');const p='orchestration/tasks/TASK-0130-gate-c-decision-envelope/gate-c-decision-envelope.json';const j=JSON.parse(fs.readFileSync(p,'utf8'));const req=['schema_version','route_identity','concrete_goal','boss_or_danger','expected_item_family','depth','branch_consequence','extraction_or_return','evidence_provenance','completeness'];for(const k of req)if(!(k in j))throw new Error('missing '+k);if(j.completeness.ready===true&&(j.concrete_goal.state==='MISSING'||j.expected_item_family.state==='MISSING'))throw new Error('false complete');console.log('gate-c envelope: PASS')"
gate-c envelope: PASS
EXIT CODE: 0
```

### Gate 2 — negative fixtures cover every required failure code

```
$ node -e "const fs=require('fs');const p='orchestration/tasks/TASK-0130-gate-c-decision-envelope/fixtures/negative-cases.json';const j=JSON.parse(fs.readFileSync(p,'utf8'));const req=['MISSING_CONCRETE_GOAL','MISSING_BOSS_OR_DANGER','MISSING_EXPECTED_ITEM_FAMILY','MISSING_DEPTH','MISSING_BRANCH_CONSEQUENCE','MISSING_EXTRACTION_OR_RETURN','ROUTE_NAME_ONLY','UNSUPPORTED_VERSION','MISSING_PROVENANCE'];for(const k of req)if(!j.cases.some(x=>x.expected_error===k))throw new Error('missing '+k);console.log('gate-c negative fixtures: PASS')"
gate-c negative fixtures: PASS
EXIT CODE: 0
```

### Gate 3 — evidence labels exist across the four surfaces

```
$ rg -n 'world:road:chart|world:zone:enter|nodeId|warden|trophy|depth|stairs|extract' native/src/networking.cpp native/tests/networking_tests.cpp playtest/scenarios/world-web.mjs playtest/scenarios/quest.mjs
EXIT CODE: 0
```

Unabridged stdout (119 matched lines, no elisions) is preserved durably at
`captures/gate-c-rg-evidence.txt` inside this task folder. Matches include the
`world:zone:enter` / `world:road:chart` dispatchers
(`native/src/networking.cpp:2314,:2320`), chart rows (`:1429` wardenName),
instance metadata (`:783-799`, `:864-880` nodeId/stairsUp/stairsDown/entry
gate/wardenDead), extraction (`:939-968`, `:2116-2124`, `:2451`), Warden
refusal (`:1527`), retired trophy event comment (`:1997`), native test labels
(`native/tests/networking_tests.cpp:44,124,143-144,165-171,333-385`), and
scenario assertions (`playtest/scenarios/world-web.mjs:31-110`;
`playtest/scenarios/quest.mjs:255-325`).

### Gates 4 & 5 — whitespace check and base-to-head changed-file list

```
$ git diff --check
EXIT CODE: 0            (empty output)
$ git diff --name-only cab50d62cb121ab6a88fa513257e645447226959..HEAD
orchestration/REENTRY-OX-ALPHA-PC.md
orchestration/RUN_STATUS.md
orchestration/tasks/TASK-0112-passive-tree-engine-scaffold/SPEC.md
orchestration/tasks/TASK-0130-gate-c-decision-envelope/SPEC.md
orchestration/tasks/TASK-0130-gate-c-decision-envelope/STATUS.md
orchestration/tasks/TASK-0130-gate-c-decision-envelope/VALIDATION.md
orchestration/tasks/TASK-0130-gate-c-decision-envelope/fixtures/negative-cases.json
orchestration/tasks/TASK-0130-gate-c-decision-envelope/gate-c-decision-envelope.json
orchestration/tasks/TASK-0131-release-proof-manifest/SPEC.md
orchestration/tasks/TASK-0132-clean-machine-harness-contract/SPEC.md
orchestration/tasks/TASK-0133-save-migration-rollback-contract/SPEC.md
orchestration/tasks/TASK-0134-distribution-signing-boundary/SPEC.md
EXIT CODE: 0
```

### Path-boundary proof

`git log --oneline cab50d62..b3599c80` shows exactly one commit between base
and routed HEAD — the architect's coordination refresh `b3599c80`
("orchestration: expand PC OpenRouter fleet to eight lanes"), which alone
accounts for every non-task-folder path above (verified via
`git diff --name-only cab50d62..b3599c80`: REENTRY/RUN_STATUS plus five lane
SPECs). Worker-authored changes never leave
`orchestration/tasks/TASK-0130-gate-c-decision-envelope/**`.

## Manual verification

No server/client runs were required by the SPEC (documentation packet; ports
6700–6719 stayed unbound). Additional self-checks performed: both JSON files
re-parse cleanly; fixture coverage was cross-checked against ALL 13 documented
codes in VALIDATION.md (superset of the nine required); each negative case was
reviewed against the ordering table so it triggers exactly its intended code
and no earlier check; the false-complete fixture (neg-13) mirrors Gate 1's
rejected pattern.

## Commit SHAs

- `30b851a72e59f7a6989c375abe2c0c167bf57f8f` — claim (STATUS.md only).
- `419cf3b116699823a83dc0b9eb0feeaef1d28de5` — gate-c-decision-envelope.json,
  VALIDATION.md, fixtures/negative-cases.json.
- `<REVIEW_REQUESTED commit>` — captures/gate-c-rg-evidence.txt, REPORT.md,
  STATUS transition to REVIEW_REQUESTED (exact SHA appended below after
  committing, per handoff discipline; never amended).

## Deviations

- None against SPEC or PROTOCOL. Pre-commit lint-staged ran with zero matching
  files for these markdown/JSON-only commits (hook output preserved in session;
  commits succeeded normally).

## Stop-condition compliance

Evidence is absent for concrete goal and expected item family; both remain
`MISSING` with null values, owner-pending flags, and their blocking codes.
The contract contains no product values, proposes none, and does not reconcile
native/browser node identity. All work stayed inside the task folder; no
source or product-authority modification was needed to represent TASK-0086's
classifications.

## Unresolved questions

None requiring escalation. Owner decisions this contract stages (goal content,
loot family tables, frontier policy beyond deepest-cleared+1, node-identity
reconciliation P2, native chart pane P5) belong to future owner-authority
packets, as recorded in `completeness.owner_pending_content`.

## Risks

- The envelope cites line numbers from TASK-0086's audit; they drift if
  `networking.cpp` / `world-web.js` move. The audit capture records its head;
  this contract records its base.
- `DERIVABLE-WITHOUT-GAMEPLAY-RULES` readiness for `branch_consequence` holds
  only for the immediate next stage; consumers must respect the documented
  scope (enforced by VALIDATION.md's readiness rule wording, not machine-checkable
  beyond state inspection).

## Follow-ups (routing suggestions, not claims)

- A future owner packet supplies concrete goal content and item-family tables;
  implementation then flips the two MISSING fields through both chart builders
  and re-evaluates `completeness.ready`.
- Native parity touch-ups (P1 levelHint/childIds, P5 chart pane) remain the
  client-side path to actually presenting a Gate C decision natively.

## Revision r1 — record REVIEW_REQUESTED head

Appended after pushing per the REPORT placeholder above.

- REVIEW_REQUESTED evidence commit: `<appended post-push>`
