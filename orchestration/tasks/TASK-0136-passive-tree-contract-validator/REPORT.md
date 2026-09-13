# TASK-0136 REPORT — Passive-tree contract validator CLI

- worker: ox-pc-bd (openrouter / `stealth/ox-alpha`, OpenCode CLI)
- branch: `worker/verdigris/pc/ox-pc-bd`
- clone/worktree root: `Z:\Code\.worktrees\verdigris\ox-pc-bd`
- claim commit: `f1ffa64b` on routed HEAD `424c315120451fe7f0a16a17ebc1f7e5bdc94694`
- immutable task base: `be6d555688619819084b352660fc0336a90d0ec3` (verified ancestor, exit 0)
- implementation commit: `aab7ad42` (all five acceptance gates executed at this exact HEAD)

## Executive summary

Implemented the dependency-free Node CLI
`validate-passive-tree-contract.mjs`, a 21-test suite (`validator.test.mjs`),
and two synthetic content-neutral fixture sets under this task folder only.
The validator binds the accepted TASK-0112 contract
(`verdigris.passive-tree-authority` schema 1.0.0) and its VALIDATION.md
pipeline: batch-mode errors sorted ascending by `(rank, lexicographic
element)`, stop ranks 1–3 (`MALFORMED_ALLOCATION`, `UNKNOWN_GRAPH_VERSION`,
`UNSUPPORTED_MIGRATION`), accumulating ranks 4–9 (`UNKNOWN_NODE`,
`DUPLICATE_NODE`, `MALFORMED_EDGE`, `DISCONNECTED_ALLOCATION`, `OVERSPENT`,
`COUNTER_CONFUSION`). All eight required error codes fail closed (plus rank-1
`MALFORMED_ALLOCATION`), the two point ledgers
(`persistent_commission_points` = quests.questPoints vs `live_tree_points`)
are preserved as structurally distinct fields in every accepted snapshot, and
both native negative controls are enforced: node-id text is never interpreted
(`native_plus_two_axis_walk`) and raw snapshots are refused without
validation_provenance, rebuilt only through validation
(`native_raw_snapshot_save`). Both SPEC fixtures behave exactly as specified:
valid-synthetic exits 0; counter-confusion exits nonzero emitting
`COUNTER_CONFUSION`.

## Approach

- Fixture format (this task's harness): `authority` (synthetic stand-in for
  the future OI-004 content source: graph_version, migration_floor,
  registered_migrations), one candidate `graph` document, and a `cases[]`
  list with modes `allocation | persistence | migration_request |
  raw_snapshot`. Identifiers reuse the TASK-0112 conventions block verbatim
  (`n:000`, `e:001`, `v:0` style); all numbers are harness context values,
  not authored balance; no topology is asserted.
- Contract preflight fails closed as `INVALID_CONTRACT` unless identity,
  envelope sections, intact counter separation (including the acceptance-gate
  "counters collapsed" object-identity check), full nine-code enum coverage,
  and the three declared negative controls are present.
- Economy checks never clamp: `spent > earned` surfaces `OVERSPENT`;
  `unspent != earned - spent` surfaces envelope malformation; overspend
  supersedes the mismatch report deterministically.
- Counter-confusion detectors are structural and deterministic: CC-A merged/
  collapsed points ledger (unsanctioned points-named key while a required
  ledger field is absent), CC-B cross-write (live ledger annotated with the
  commission wire identity `quests.questPoints`), CC-C wrong-ledger earned
  derivation marker, CC-D alias/equivalence declaration between the ledgers.
  Scans are shallow by design: OWNER_PENDING node content is never
  interpreted.
- Migration machinery implements contract `migration.rules`: current-version
  blobs revalidate directly; other versions require a registered strategy at
  or above the declared floor, else `UNSUPPORTED_MIGRATION` with element
  `"from->to"`; `revalidate_in_place` re-runs stages 4–9 on the migrated form;
  `full_refund_reset` grants an origin-only reset state; both retain
  `audit.pre_migration_graph_version`.
- Raw snapshots: persistence blobs missing `validation_provenance` are
  refused (`MALFORMED_ALLOCATION`, element `validation_provenance`); mode
  `raw_snapshot` validates contents and, only if fully valid, emits a rebuilt
  snapshot carrying authority provenance (`rebuilt_from_raw_snapshot: true`)
  — never verbatim storage.
- CLI usage/IO errors exit 2; validation failures exit 1; success exits 0.
  Zero runtime dependencies (`node:fs`, `node:path`, `node:url`,
  `node:process` only).

## Changed files (this task folder only)

- `orchestration/tasks/TASK-0136-passive-tree-contract-validator/STATUS.md`
  (claim → REVIEW_REQUESTED)
- `orchestration/tasks/TASK-0136-passive-tree-contract-validator/validate-passive-tree-contract.mjs`
- `orchestration/tasks/TASK-0136-passive-tree-contract-validator/validator.test.mjs`
- `orchestration/tasks/TASK-0136-passive-tree-contract-validator/fixtures/valid-synthetic.json`
- `orchestration/tasks/TASK-0136-passive-tree-contract-validator/fixtures/counter-confusion.json`
- `orchestration/tasks/TASK-0136-passive-tree-contract-validator/REPORT.md`

No file outside
`orchestration/tasks/TASK-0136-passive-tree-contract-validator/**` was
created, modified, or deleted by this work.

## Public interfaces added

- CLI: `node validate-passive-tree-contract.mjs --contract <TASK-0112 contract.json> --fixture <candidates.json> [--json]`
- Exports for tests/successors: `RANKS`, `REQUIRED_ERROR_CODES`,
  `parseArgs(argv)`, `validateContractShape(contract)`,
  `evaluateFixture(contract, fixtureDoc)` (pure), `runValidation(contractPath, fixturePath)`.

## Acceptance commands — literal transcripts

Executed from the repository root exactly as written in SPEC frontmatter, at
implementation commit `aab7ad42`.

### Gate 1: `node --test orchestration/tasks/TASK-0136-passive-tree-contract-validator/validator.test.mjs`

```text
1..21
# tests 21
# suites 0
# pass 21
# fail 0
# cancelled 0
# skipped 0
# todo 0
# duration_ms 1057.0338
TEST EXIT: 0
```

(First run was 12/21: my contract shape check wrongly required
`negative_controls` to be an object when the accepted TASK-0112 contract
declares it as an array, cascading to every real-contract CLI case; and one
test expectation missed that batch mode correctly accumulates
`DISCONNECTED_ALLOCATION` behind `MALFORMED_EDGE` when every edge choice is
invalid. Fixed the checker and the test expectation; validator pipeline
semantics unchanged and now proven by tests.)

### Gate 2: valid-synthetic fixture (expected exit 0)

```text
=== GATE 2: valid-synthetic (expect exit 0) ===
{
  "tool": "validate-passive-tree-contract",
  "task": "TASK-0136",
  "contract": { "contract_id": "verdigris.passive-tree-authority", "schema_version": "1.0.0", ... path elided ... },
  "fixture_set": "verdigris.passive-tree-authority/task-0136-valid-synthetic",
  "ok": true,
  "results": [
    { "case_id": "VAL-001-direct-claim",       "mode": "allocation",        "ok": true, "errors": [], "accepted_snapshot": { ... } },
    { "case_id": "VAL-002-persistence-load",   "mode": "persistence",       "ok": true, "errors": [], "accepted_snapshot": { ... } },
    { "case_id": "VAL-003-supported-migration","mode": "migration_request", "ok": true, "errors": [], "accepted_snapshot": { ..., "audit": { "pre_migration_graph_version": 1, "strategy": "revalidate_in_place" } } },
    { "case_id": "VAL-004-raw-snapshot-rebuilt","mode": "raw_snapshot",     "ok": true, "errors": [], "accepted_snapshot": { ..., "rebuilt_from_raw_snapshot": true } }
  ],
  "errors": [],
  ... paths elided ...
  "contract_ok": true
}
EXIT: 0
```

Every accepted snapshot carries both ledgers as distinct integer fields, e.g.
VAL-001 budget: `"persistent_commission_points": 7, "live_tree_points": 5,
"earned": 5, "spent": 4, "unspent": 1`. Full byte-exact JSON preserved in the
session log; nothing elided above carries validation semantics except paths.

### Gate 3: counter-confusion fixture (expected nonzero negative control)

```text
=== GATE 3: counter-confusion (expect nonzero, COUNTER_CONFUSION) ===
{
  ...
  "fixture_set": "verdigris.passive-tree-authority/task-0136-counter-confusion",
  "ok": false,
  "results": [
    { "case_id": "CC-001-merged-ledger-blob",        "mode": "persistence", "ok": false,
      "errors": [ { "code": "COUNTER_CONFUSION", "element": "points",         "message_key": "error.counter_confusion", "rank": 9 } ] },
    { "case_id": "CC-002-wrong-ledger-earned-source","mode": "allocation",  "ok": false,
      "errors": [ { "code": "COUNTER_CONFUSION", "element": "earned_source",  "message_key": "error.counter_confusion", "rank": 9 } ] },
    { "case_id": "CC-003-ledger-alias-declaration",  "mode": "allocation",  "ok": false,
      "errors": [ { "code": "COUNTER_CONFUSION", "element": "merged_with",    "message_key": "error.counter_confusion", "rank": 9 } ] }
  ],
  "errors": [ ...same three COUNTER_CONFUSION errors, same order... ],
  ... paths elided ...
  "contract_ok": true
}
EXIT: 1
```

### Gate 4: `git diff --check`

```text
DIFF-CHECK EXIT: 0   (no whitespace errors)
```

### Gate 5: `git diff --name-only be6d555688619819084b352660fc0336a90d0ec3..HEAD`

```text
NAME-ONLY EXIT: 0    (546 names total)
```

The list is dominated by branch history integrated before this lane's claim
(native/, docs/rebuild/HANDOFF.md, orchestration logs, prior task folders
TASK-0080..TASK-0165). The TASK-0136 entries at gate time are exactly:

```text
orchestration/tasks/TASK-0136-passive-tree-contract-validator/RELEASE.md
orchestration/tasks/TASK-0136-passive-tree-contract-validator/SPEC.md
orchestration/tasks/TASK-0136-passive-tree-contract-validator/STATUS.md
orchestration/tasks/TASK-0136-passive-tree-contract-validator/fixtures/counter-confusion.json
orchestration/tasks/TASK-0136-passive-tree-contract-validator/fixtures/valid-synthetic.json
orchestration/tasks/TASK-0136-passive-tree-contract-validator/validate-passive-tree-contract.mjs
orchestration/tasks/TASK-0136-passive-tree-contract-validator/validator.test.mjs
```

After the REVIEW_REQUESTED commit lands, the only additional names are
`REPORT.md` and the updated `STATUS.md` in this same folder.

## Additional coverage beyond SPEC minimums

- All eight required codes plus `MALFORMED_ALLOCATION` proven reachable and
  fail-closed programmatically, each reporting its deterministic first
  offending element (test 5).
- Stop ranks proven to short-circuit: version-9999 input carrying duplicate +
  unknown + malformed-edge + overspend defects reports exactly
  `UNKNOWN_GRAPH_VERSION` (tests 5, 6).
- Ordering proven against a five-defect allocation: exact sequence
  `[UNKNOWN_NODE n:404, DUPLICATE_NODE n:001, MALFORMED_EDGE e:999,
  DISCONNECTED_ALLOCATION n:003, OVERSPENT null]` per VALIDATION.md ranks,
  verified in-process and through the real CLI subprocess (tests 7, 18).
- Migration machinery covered positively and negatively: supported
  revalidate_in_place with audit retention, full_refund_reset origin-only
  reset state, below-floor/unregistered `UNSUPPORTED_MIGRATION`
  `"1->2"` (tests 8–10).
- Raw-snapshot trust boundary: missing-provenance refusal,
  NEG-010-style overspent+disconnected raw snapshot validated then refused
  with no accepted snapshot, internally valid raw snapshot rebuilt with
  authority provenance (tests 11–13).
- Opaque-identifier control: nodes named `o:root` / `axis:+2hex` allocate
  normally with zero invented attribute/effect/bonus fields; accepted-snapshot
  key set asserted exactly (test 14).
- Two-ledger preservation asserted across every accepted snapshot; serialized
  output contains no bare `"points"` / `"questPoints"` key (test 15).
- Determinism: repeated in-process runs deep-equal; reordered cases produce
  identical per-case outcomes; repeated CLI runs byte-equal stdout (test 17).
- Tampered contracts (collapsed counters, removed section, altered violation
  error, stripped enum code, emptied negative controls) all fail closed as
  `INVALID_CONTRACT` (test 4).
- Usage/IO misuse exits 2 cleanly, including invalid JSON inputs (tests 19–20);
  human-readable mode prints `CASE FAIL <id> <CODE> <element>` lines and a
  `RESULT ok=false cases=3 error_count=3` summary (test 21).

## Manual verification

- Ran every gate from the repo root exactly as written in SPEC frontmatter.
- Confirmed via `git status --short` that only TASK-0136-folder files changed.
- No server started; port 6500 untouched; task-folder-only executable, no
  ports opened.

## Commit SHAs

| Commit | Content |
|---|---|
| `f1ffa64b` | CLAIMED status by ox-pc-bd |
| `aab7ad42` | validator CLI + tests + fixtures (all five gates executed at this HEAD) |
| `<review-requested>` | STATUS transition to REVIEW_REQUESTED + this report |

The branch tip at push (`<review-requested>`) is the authoritative
REVIEW_REQUESTED evidence head; it differs from `aab7ad42` only by
REPORT.md and STATUS.md inside the owned folder.

## Interpretive notes (no spec deviations)

- The eight required SPEC codes are a subset of VALIDATION.md's nine-rank
  pipeline; rank-1 `MALFORMED_ALLOCATION` is implemented alongside them so
  the pipeline matches the normative table exactly.
- The fixture harness carries an explicit synthetic `authority` declaration
  (graph_version, migration floor, registered migrations). This stands in
  for the future OI-004 approved content source because the acceptance
  command line passes only `--contract` and `--fixture`; it asserts no
  gameplay content, and UNSUPPORTED_MIGRATION remains owner-policy driven.
- Spent/earned are checked as declared envelope numbers under the contract's
  invariants (`spent <= earned`, `unspent == earned - spent`); the validator
  embeds no cost constant, honoring the contract's prohibition on authored
  balance.

## Scope compliance

- Owned paths only: yes. Forbidden paths (`native/**`, `server/**`,
  `src/**`, `playtest/**`, any content/balance decision): untouched.
- Two point ledgers preserved everywhere; COUNTER_CONFUSION fails closed.
- Negative controls honored: no identifier-text interpretation anywhere;
  raw snapshots never trusted verbatim.
- No merge to program/master, no force-push, port 6500 untouched: confirmed.
- Pushed: only `worker/verdigris/pc/ox-pc-bd`.
