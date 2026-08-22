# TASK-0137 REPORT — Gate C envelope validator CLI

- worker: ox-pc-e (openrouter / `stealth/ox-alpha`, OpenCode CLI 1.18.21)
- branch: `codex/TASK-0137-gate-c-envelope-validator-ox-pc-e`
- clone root: `Z:\Code\.worktrees\verdigris\ox-pc-e`
- claim commit: `0d175a2f` on routed HEAD `a631cb2e74e2b7463a9f9b3706684be8988b3c09`
- immutable task base: `be6d555688619819084b352660fc0336a90d0ec3` (verified ancestor, exit 0)

## Executive summary

Implemented the dependency-free Node CLI `validate-gate-c-envelope.mjs`, a
16-test suite (`validator.test.mjs`), and three synthetic content-neutral
fixture sets under this task folder only. The validator implements the
13-check deterministic first-match-wins contract from TASK-0130's
`VALIDATION.md`: it rejects unparseable input, unsupported versions,
missing route identity, route-name-only input, missing provenance,
self-contradictory depth, and falsely claimed readiness; honest `MISSING` /
`OWNER_PENDING` decision fields are preserved verbatim and reported as
readiness blockers, never filled or invented. Both SPEC acceptance fixtures
behave exactly as specified: valid-but-incomplete exits 0 with
`ready:false`; route-name-only exits nonzero with `ROUTE_NAME_ONLY`.

## Approach

- Checks 1–5, 12, 13 are fatal (exit 1); checks 6–11 emit per-field blocker
  codes in the fixed order goal → boss → family → depth → branch →
  extraction while remaining structurally valid (exit 0, `ready:false`),
  matching VALIDATION.md's "honest MISSING is structurally valid" rule.
- Provenance accepts both documented shapes: the flat future-envelope shape
  (`authority_source` + `audit_reference` + hex `base_commit`) and TASK-0130's
  narrative `provenance_of_this_contract[]` shape.
- `DERIVABLE-WITHOUT-GAMEPLAY-RULES` fields accept null values within their
  documented scope; `AVAILABLE` fields with null/absent values block readiness.
- Depth contradiction = two differing numbers anywhere inside `depth.value`,
  or a single number conflicting with `route_identity.tier`.
- CLI usage/IO errors exit 2; validation failures exit 1; structural validity
  (with or without readiness) exits 0. `--json` emits one stable result shape;
  plain mode prints `FAIL <index> <CODE>` / `OK` / `BLOCKED` / `OWNER_PENDING`.
- Zero runtime dependencies (`node:fs`, `node:path`, `node:url`,
  `node:process` only).

## Changed files (this task folder only)

- `orchestration/tasks/TASK-0137-gate-c-envelope-validator/STATUS.md` (claim → REVIEW_REQUESTED)
- `orchestration/tasks/TASK-0137-gate-c-envelope-validator/validate-gate-c-envelope.mjs`
- `orchestration/tasks/TASK-0137-gate-c-envelope-validator/validator.test.mjs`
- `orchestration/tasks/TASK-0137-gate-c-envelope-validator/fixtures/valid-incomplete.json`
- `orchestration/tasks/TASK-0137-gate-c-envelope-validator/fixtures/route-name-only.json`
- `orchestration/tasks/TASK-0137-gate-c-envelope-validator/fixtures/negatives.json`
- `orchestration/tasks/TASK-0137-gate-c-envelope-validator/REPORT.md`

No file outside `orchestration/tasks/TASK-0137-gate-c-envelope-validator/**`
was created, modified, or deleted by this work.

## Public interfaces added

- CLI: `node validate-gate-c-envelope.mjs --schema <TASK-0130 schema.json> --fixture <envelope.json> [--json]`
- Exports for tests: `parseArgs(argv)`, `loadJsonFile(path)`,
  `validateRaw(schema, rawText)`, `validateEnvelope(schema, envelopeObj)`,
  `runValidation(schemaPath, fixturePath)`.

## Acceptance commands — literal transcripts

### Gate 1: `node --test orchestration/tasks/TASK-0137-gate-c-envelope-validator/validator.test.mjs`

```text
# tests 16
# pass 16
# fail 0
# cancelled 0
# skipped 0
# todo 0
TEST EXIT: 0
```

(First run was 14/16: two of my own test expectations wrongly kept
`completeness.ready:true` while a field blocked, so first-match-wins check 13
correctly fired `OWNER_PENDING_CONTENT`. Fixed the tests to declare honest
incomplete completeness, as the negative fixtures do; validator semantics
unchanged.)

### Gate 2: valid-incomplete fixture (expected exit 0, `ready:false`)

```text
=== GATE 2: valid-incomplete (expect exit 0, ready:false) ===
{
  "contract_id": "gate-c-decision-envelope",
  ...
  "valid": true,
  "ready": false,
  "error": null,
  "error_index": null,
  "schema_version": "1.0.0",
  "missing_field_codes": ["MISSING_CONCRETE_GOAL", "MISSING_EXPECTED_ITEM_FAMILY"],
  "owner_pending_fields": ["concrete_goal", "expected_item_family"],
  "field_states": {
    "concrete_goal":        { "state": "MISSING", "owner_pending": true },
    "boss_or_danger":       { "state": "AVAILABLE", "owner_pending": false },
    "expected_item_family": { "state": "MISSING", "owner_pending": true },
    "depth":                { "state": "AVAILABLE", "owner_pending": false },
    "branch_consequence":   { "state": "DERIVABLE-WITHOUT-GAMEPLAY-RULES", "owner_pending": false },
    "extraction_or_return": { "state": "AVAILABLE", "owner_pending": false }
  },
  "completeness_ready_claimed": false
}
EXIT: 0
```

Full JSON transcript captured in session log; elided middle is path echo
(`schema_path`, `fixture_path`). Honest `MISSING`/`owner_pending` states are
preserved verbatim; no value was invented.

### Gate 3: route-name-only fixture (expected nonzero negative control)

```text
=== GATE 3: route-name-only (expect nonzero, ROUTE_NAME_ONLY) ===
{
  "valid": false,
  "ready": false,
  "error": "ROUTE_NAME_ONLY",
  "error_index": 4,
  ... paths elided ...
}
EXIT: 1
```

Human-readable mode for the same input prints `FAIL 4 ROUTE_NAME_ONLY` and
exits 1 (covered by test 16).

### Gate 4: `git diff --check`

```text
DIFF-CHECK EXIT: 0   (no whitespace errors)
```

### Gate 5: `git diff --name-only be6d555688619819084b352660fc0336a90d0ec3..HEAD`

```text
orchestration/RUN_STATUS.md
orchestration/tasks/TASK-0136-passive-tree-contract-validator/SPEC.md
orchestration/tasks/TASK-0137-gate-c-envelope-validator/SPEC.md
orchestration/tasks/TASK-0137-gate-c-envelope-validator/STATUS.md
orchestration/tasks/TASK-0138-release-proof-validator/SPEC.md
orchestration/tasks/TASK-0139-clean-machine-manifest-validator/SPEC.md
NAME-ONLY EXIT: 0
```

The RUN_STATUS/sibling-SPEC entries are the architect's routing commits
(`c7fc703c`, `a631cb2e`) already on the routed branch head before my claim;
my commits touch only the TASK-0137 folder. After the implementation and
REVIEW_REQUESTED commits land, the additional names are exclusively under
`orchestration/tasks/TASK-0137-gate-c-envelope-validator/`.

## Additional coverage beyond SPEC minimums

- All 13 documented codes have triggering synthetic fixtures
  (`fixtures/negatives.json`, 16 cases including absent-version,
  absent-provenance-field, identity-tier-vs-depth-tier conflict, and
  nonempty-declared-missing-with-ready variants).
- Fatal cases proven exit 1 and blocker cases proven exit 0 through the real
  subprocess CLI, not just in-process.
- Determinism proven: repeated in-process runs deep-equal; repeated CLI runs
  byte-equal stdout.
- Usage/IO misuse (missing args, unreadable schema/fixture) exits 2 cleanly.

## Manual verification

- Ran every gate from the repo root exactly as written in SPEC frontmatter.
- Confirmed via `git status --short` that only TASK-0137-folder files are new.
- No server started; port 6500 untouched; ports 6700–6719 unused by this task
  (pure offline CLI work, loopback-only lane discipline maintained).

## Commit SHAs

| Commit | Content |
|---|---|
| `0d175a2f` | CLAIMED status by ox-pc-e |
| `013c7883` | same claim repackaged into canonical STATUS format (lane supervisor, STATUS.md only, pushed while implementation was in progress; preserved intact) |
| `f63c9550` | validator CLI + tests + fixtures + this report |
| `<review-requested>` | STATUS transition to REVIEW_REQUESTED + report commit-SHA table fixup (this commit) |

The branch tip at push (`<review-requested>`) is the authoritative
REVIEW_REQUESTED evidence head.

## Deviations

- None against spec. Interpretive notes: (1) checks 6–11 are reported as
  exit-0 readiness blockers rather than exit-1 errors because VALIDATION.md
  explicitly declares honest MISSING "structurally valid" while blocking
  readiness, and SPEC requires valid-incomplete to exit 0; all other codes are
  exit-1 fatals. (2) The raw TASK-0130 contract JSON doubles as the schema
  document passed to `--schema`; it is not itself an instance envelope, so the
  SPEC fixtures instantiate its exact field-state pattern synthetically.

## Unresolved questions

- None blocking. If the architect prefers MISSING_* blockers to be exit-1
  fatals too, that is a one-line policy change plus fixture updates; the
  current split follows the two governing documents as written.

## Risks and follow-ups

- Future envelope authors must keep `completeness.ready:false` until all six
  fields are AVAILABLE/DERIVABLE-in-scope with no owner_pending flags, else
  check 13 fires — intended fail-closed behavior.
- Successor packets may reuse `runValidation()` for batch validation of many
  envelopes; not in scope here.

## Scope compliance

- Owned paths only: yes. Forbidden paths (`native/**`, `server/**`,
  `src/**`, `playtest/**`, product/campaign/reward values): untouched.
- MISSING / OWNER_PENDING preserved honestly: yes, asserted by tests.
- No merge to program/master, no force-push, port 6500 untouched: confirmed.
- Pushed: only `codex/TASK-0137-gate-c-envelope-validator-ox-pc-e`.
