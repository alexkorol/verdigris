# REPORT — TASK-0139 clean-machine evidence manifest validator CLI

## Executive summary

Delivered a dependency-free Node CLI
(`validate-clean-machine-evidence.mjs`), a 26-case `node:test` suite, and two
synthetic fixtures under this task folder only. The CLI validates clean-machine
evidence manifests against the accepted TASK-0132 stage contract
(`clean-machine-contract.json`): pinned clean checkout (D-2), declared
toolchain/dependencies with pin floors and fingerprint equality, cache
provenance per CACHE-1..4, green build/tests/smoke stage evidence with complete
INV-2 records in INV-1 order, no leaked processes (PROC-2), loopback-only binds
inside the declared lane capsule (PORT-1/2), and an absolute P0 ban on port
6500 contact enforced by a deep manifest scan plus capsule containment. The
tool validates evidence only; it provisions nothing and mutates nothing.

All five SPEC acceptance commands were run directly in this session and passed:
the valid synthetic fixture exits 0 `VALID`; the forbidden-port fixture exits 1
with `FORBIDDEN_PORT_6500` (P0). No file outside
`orchestration/tasks/TASK-0139-clean-machine-manifest-validator/` was created or
modified; port 6500 was never bound or contacted.

## Approach

Single-pass structural validator: read the contract's `platform_matrix[0].gates`
as the authoritative ordered stage list and the `evidence_record_schema.required`
fields as the record contract, then evaluate the manifest deterministically.
Every violation yields one typed error with code, severity, JSON path, and
message; verdict is `VALID` only with zero errors. Error codes mirror the
contract's failure modes (`DIRTY_BASE`, `MISSING_TOOLCHAIN`, `DEPENDENCY_DRIFT`,
`CACHE_LEAK`, `NONZERO_STAGE`, `LEAKED_PROCESS`, `NON_LOOPBACK_BIND`,
`FORBIDDEN_PORT_6500`) plus deterministic schema/shape codes (`MISSING_FIELD`,
`MANIFEST_INVALID`, `CONTRACT_INVALID`, `NOT_CLEAN_MACHINE_EVIDENCE`,
`NON_DISPOSABLE_HOST`, `NON_LOCKFILE_INSTALL`, `STAGE_ORDER_VIOLATION`,
`OUTSIDE_CAPSULE`, `CAPSULE_INVALID`, `LISTENER_INVALID`,
`STAGE_HOST_MISMATCH`, `STAGE_TIMESTAMP_INVALID`). The port-6500 ban is
enforced at four independent layers: recursive string/number scan of the whole
manifest (`:6500` / `port=6500` patterns), listener-port equality, capsule-range
containment — all P0 — mirroring the contract's "never waivable" rule. CLI exit
codes: 0 VALID, 1 INVALID, 2 usage/IO error. `validateManifest(manifest,
contract)` is exported for direct test consumption.

## Changed files (all under owned path)

- `validate-clean-machine-evidence.mjs` — new; the CLI + exported validator.
- `validator.test.mjs` — new; 26 node:test cases (fixtures, inline negative
  mutations of the valid manifest, and child-process CLI exit-code checks).
- `fixtures/valid-synthetic.json` — new; complete valid manifest (12 ordered
  stage evidence records, toolchain pins satisfied, cache provenance, loopback
  listener on lane capsule port 6745).
- `fixtures/forbidden-port.json` — new; same manifest mutated so launch runs
  `--port 6500` and a second listener holds port 6500.
- `captures/gate-transcripts.txt` — new; verbatim transcript of all five
  acceptance commands from this session.
- `STATUS.md` — claim commit `5df94011` plus transitions to IMPLEMENTED /
  REVIEW_REQUESTED.

## Public interfaces added/changed

None outside the task folder. New task-local interface:

```
node validate-clean-machine-evidence.mjs --contract <contract.json> --fixture <manifest.json> [--json]
```

JSON output: `{verdict, label, errors[{code, severity, path, message}], summary}`.
Exit codes 0/1/2 as above.

## Acceptance commands and outcomes

Verbatim transcripts: `captures/gate-transcripts.txt`. Summary:

```text
A1 node --test .../validator.test.mjs            -> pass 26 / fail 0   exit=0
A2 CLI --fixture fixtures/valid-synthetic.json   -> verdict VALID      exit=0
A3 CLI --fixture fixtures/forbidden-port.json    -> INVALID,
   errors: FORBIDDEN_PORT_6500 x2 (P0: $.stages[6].command command line,
   $.listeners[1].port) + OUTSIDE_CAPSULE x1 ($.listeners[1].port)   exit=1
A4 git diff --check                              -> clean              exit=0
A5 git diff --name-only be6d5556..HEAD           -> see below          exit=0
```

A5 interpretation: the immutable SPEC base `be6d5556` is an ancestor of the
routed HEAD `a631cb2e` (verified via `git merge-base --is-ancestor`, exit 0).
The diff base..HEAD lists only upstream architect files that predate this claim
(`orchestration/RUN_STATUS.md`, sibling SPECs TASK-0136..0138, this task's
SPEC) from architect commit `a631cb2e` ("orchestration: exclude claimed
TASK-0135 from READY"), none worker-authored. The worker-authored delta is
exactly:

```text
$ git diff --name-only a631cb2e74e2b7463a9f9b3706684be8988b3c09..HEAD
orchestration/tasks/TASK-0139-clean-machine-manifest-validator/STATUS.md
```

with all implementation files still untracked-to-staged inside the owned folder
at REPORT time; nothing outside the owned path was touched.

## Manual verification

- Confirmed no competing `STATUS.md` / `RELEASE.md` existed before claiming;
  fresh `git fetch --prune origin` immediately prior; upstream 0/0.
- Negative-control expectation verified as genuinely nonzero (exit 1), not an
  invocation error (usage/IO would exit 2): parsed JSON confirms `INVALID`
  verdict carrying `FORBIDDEN_PORT_6500`.
- Port hygiene of this session itself: no server started, no listener opened;
  ports 6740–6759 reserved on paper for the lane only; 6500 never contacted.
- Node v22.11.0 observed, satisfying the contract's node floor for running the
  gates (no toolchain was installed or changed).

## Commit SHAs

- `5df94011` — claim commit (STATUS.md only), pushed to origin.
- `<this commit>` — REVIEW_REQUESTED commit carrying validator, tests,
  fixtures, captures, REPORT.md, STATUS transition.

## Deviations

1. Extra capture file (`captures/gate-transcripts.txt`) beyond the named
   deliverables, kept deliberately as durable acceptance evidence, matching the
   TASK-0132 lane precedent.
2. Two additional deterministic shape codes beyond the contract's failure-mode
   vocabulary (`OUTSIDE_CAPSULE`, etc.) are emitted alongside — never instead
   of — the required `FORBIDDEN_PORT_6500` code; the SPEC only pins the
   forbidden-port fixture outcome, which is exact.

## Unresolved questions

None blocking.

## Risks

- Synthetic manifests are static evidence shapes; when the future harness
  (TASK-0126 successor) emits real manifests, schema drift may require a
  contract-versioned revision of this validator (corrections via REVIEW.md).
- The validator trusts the manifest's self-declared host/cache/process facts by
  design; it validates evidence completeness and consistency, not host truth —
  same boundary the contract draws between evidence validation and provisioning.

## Follow-ups

- Wire this validator into the future clean-machine harness pipeline once real
  run manifests exist; consider feeding TASK-0065-style benchmark artifacts as
  additional stages if the contract grows.
