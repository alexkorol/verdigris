# VALIDATION — TASK-0140 soak evidence-bundle validator

Record of design decisions and gate runs for `validate-soak-evidence.mjs`.

## Contract

- Inputs: `--policy <path>` (the accepted TASK-0135
  `soak-integration-policy.json`) and `--bundle <path>` (a submitted evidence
  bundle). Optional `--tip-under-judgment <40-hex sha>`.
- Stdout carries exactly one stable JSON result document; stderr carries
  human-readable diagnostics. No file writes, no network, no ports, no
  subprocesses, no clock reads: evaluation is a pure function of the two
  documents (plus the optional tip argument).
- Exit codes: `0` only when a bundle is accepted as a policy-valid PASS;
  `1` for a deterministic rejection or a consistent FAIL /
  BLOCKED_ENVIRONMENTAL submission (only PASS exits zero, per SPEC); `2`
  for usage, parse, schema, unsupported-policy, and unsupported-context
  errors.

## Evaluation model

Two layers, evaluated fail-closed in a fixed priority order:

1. Session validity (unconditional): `RETRY_MASKED_FAILURE` (undisclosed
   failed attempts, session-attempt-count mismatch, duplicate artifact
   references), forbidden-port scan of attempt disclosures
   (`FORBIDDEN_PORT_6500`, `PORT_CAPSULE_OUT_OF_RANGE`,
   `NON_LOOPBACK_BIND`), platform completeness (`MISSING_PLATFORM_EVIDENCE`)
   apply to every submission regardless of its conclusion.
2. Conclusion adjudication:
   - PASS submissions must additionally survive `STALE_SOURCE_HEAD`,
     `UNSUPPORTED_PLATFORM`, `TIMEOUT`, `NONZERO_SOAK`,
     `MISSING_ARTIFACT`, collision reclassification
     (`PORT_CAPSULE_COLLISION` -> BLOCKED_ENVIRONMENTAL when every attempt
     collided or quarantine limits were exceeded), deep artifact
     verification (readable, parsable, all `artifacts.required_fields`
     present, sha256 binding, clean-pass shape, watchdog/context budgets,
     capsule safety), independence counting
     (`INSUFFICIENT_INDEPENDENT_RUNS`), and release-proof extras
     (`FRESH_PROCESS_UNDISCLOSED`, `RUN_INTERVAL_OVERLAP`).
   - FAIL submissions are accepted only when the disclosed evidence
     substantiates failure (timeout / deterministic / nonzero / missing-
     artifact / collision facts); otherwise `CONCLUSION_UNSUPPORTED`.
   - BLOCKED_ENVIRONMENTAL submissions are accepted only on pure
     environmental collisions without a clean re-run inside quarantine
     limits; deterministic contamination yields `CONCLUSION_UNSUPPORTED`.
- Disabled contexts (`nightly`, `release_proof` in the accepted policy) are
  refused with exit 2 `unsupported_context` before any judgment: judging
  them would decide an OWNER_PENDING scheduling/hosting question.
- Policies are refused with exit 2 `unsupported_policy` unless they carry
  the accepted `policy_id`, a `1.x` `schema_version`, all twelve required
  sections, and still forbid port 6500. The validator never evaluates a
  policy that would permit the owner-reserved port.

Error codes: all eight TASK-0135 canonical codes are judged and covered by
tests (STALE_SOURCE_HEAD, MISSING_PLATFORM_EVIDENCE,
PORT_CAPSULE_COLLISION, FORBIDDEN_PORT_6500, TIMEOUT, NONZERO_SOAK,
MISSING_ARTIFACT, RETRY_MASKED_FAILURE) plus documented fail-closed
extensions (UNSUPPORTED_PLATFORM, INCOMPLETE_RUN, ARTIFACT_HASH_MISMATCH,
ARTIFACT_INTERVAL_INVALID, INSUFFICIENT_INDEPENDENT_RUNS,
RUN_INTERVAL_OVERLAP, FRESH_PROCESS_UNDISCLOSED, CONCLUSION_UNSUPPORTED,
PORT_CAPSULE_OUT_OF_RANGE, NON_LOOPBACK_BIND).

## Artifact integrity under git normalization

`.gitattributes` sets `* text=auto eol=lf`; committed sha256 bindings stay
valid across checkouts because fixtures/artifacts JSON files are written
minified with no line endings at all, so eol conversion cannot alter their
bytes. Bundles themselves are not hashed and are pretty-printed.
`fixtures/generate-fixtures.mjs` regenerates every fixture byte-stable
(fixed timestamps, sorted key insertion) and recomputes bindings.

## Gate transcript summary

Run 2026-08-22 from repository root `Z:\Code\.worktrees\verdigris\ox-pc-d`
on branch `codex/TASK-0140-soak-evidence-bundle-validator-ox-pc-d`;
literal transcripts live in `REPORT.md`.

- `node --test validate-soak-evidence.test.mjs`: 33/33 pass, exit 0.
- valid-pass CLI gate: `evaluated=PASS accepted=true`, exit 0.
- retry-masked negative control: `evaluated=FAIL accepted=false`, emits
  `RETRY_MASKED_FAILURE` on stdout and stderr, exit 1 (nonzero negative
  control; a zero exit here would fail the gate).
- `git diff --check`: exit 0.
- Changed-path audit against base `6a10e862`: worker-authored paths are
  confined to `orchestration/tasks/TASK-0140-soak-evidence-bundle-validator/**`;
  the remaining two paths (`orchestration/RUN_STATUS.md`, task `SPEC.md`)
  are the architect's READY-promotion commit `9aa43b7a`, which precedes the
  claim.
