# REPORT — TASK-0140 server lifecycle soak evidence-bundle validator

## Executive summary

Implemented a deterministic, side-effect-free Node CLI
(`validate-soak-evidence.mjs`) that evaluates a submitted lifecycle-soak
evidence bundle against the accepted TASK-0135 policy and fails closed on
every canonical failure family. Exit 0 only for an accepted PASS; exit 1 for
deterministic rejections and consistent FAIL/BLOCKED_ENVIRONMENTAL
submissions; exit 2 for usage/parse/schema/unsupported-policy/context errors.
Positive PASS and BLOCKED_ENVIRONMENTAL fixtures plus negative coverage for
all eight TASK-0135 canonical error codes are included, each exercised by the
33-test suite. All five SPEC acceptance gates are green, including the
required nonzero negative control. Work is confined to this task folder; no
soak ran, no port was bound or probed (6500 untouched), TASK-0135 is
untouched, and CI/product source is untouched.

## Approach

- Policy-driven evaluation: required artifact fields, capsule bounds,
  forbidden ports, repetition counts, watchdog/context budgets, quarantine
  limits, supported platforms, and verdict values are all read from the
  submitted policy document; the CLI refuses (`unsupported_policy`, exit 2)
  any policy that is not the accepted TASK-0135 policy shape or that no
  longer forbids port 6500.
- Two-layer fail-closed model: unconditional session-validity checks
  (masking, forbidden ports, platform completeness), then conclusion
  adjudication (PASS must survive staleness, support, timeout, nonzero,
  missing-artifact, collision-reclassification, deep artifact verification,
  independence counting, and release-proof extras; FAIL/BLOCKED submissions
  must be substantiated by disclosed facts). Priority order is fixed and
  documented in `VALIDATION.md`.
- OWNER_PENDING protection: disabled contexts (`nightly`, `release_proof`)
  are refused with exit 2 before judgment; the validator never decides
  schedule/hosting/release questions.
- Artifact sha256 bindings are robust to git eol normalization because
  fixture artifacts are minified newline-free JSON;
  `fixtures/generate-fixtures.mjs` regenerates them byte-stable.

## Changed files

All under `orchestration/tasks/TASK-0140-soak-evidence-bundle-validator/`:

- `validate-soak-evidence.mjs` (new) — the CLI.
- `validate-soak-evidence.test.mjs` (new) — 33 node:test cases spawning the
  real CLI as a subprocess.
- `fixtures/valid-pass.json` (new) — positive PASS fixture.
- `fixtures/valid-blocked-environmental.json` (new) — positive
  BLOCKED_ENVIRONMENTAL fixture (three quarantined collisions).
- `fixtures/retry-masked-failure.json` (new) — SPEC-mandated negative
  control.
- `fixtures/artifacts/lifecycle-soak-clean-pass-a.json` (new) — minified
  soak artifact fixture bound by sha256.
- `fixtures/generate-fixtures.mjs` (new) — byte-stable fixture generator.
- `VALIDATION.md` (new) — design decisions, code inventory, normalization
  strategy.
- `STATUS.md` (claim commit `2a1a1d9c`; flipped to REVIEW_REQUESTED in the
  final commit).
- `REPORT.md` (this file).

## Public interfaces added

- CLI: `node validate-soak-evidence.mjs --policy <path> --bundle <path>
  [--tip-under-judgment <sha>]` (also `--help`). Stable stdout JSON result:
  `{validator, policy{id,revision,schema_version}, bundle, context,
  source_head, submitted_conclusion, evaluated_conclusion, accepted,
  errors[], findings[], notes[]}`; error-class documents use
  `{result:"ERROR", error_class, message}`. Exit contract 0/1/2 as above.
- Bundle conventions consumed: optional top-level `undisclosed_failed_attempts`
  and `session_attempt_count` feed RETRY_MASKED_FAILURE detection; optional
  attempt fields `process_exit_code`, `timed_out`, `phase`, `listener_port`
  /`port`/`client_port`/`server_port`, `host`, `fresh_process`,
  `tip_under_judgment` feed the corresponding rules.

## Test commands and outcomes

Run from repository root on Windows (PowerShell 7, Node v22.11.0). Literal
transcripts:

Gate 1 — unit suite:

```
node --test orchestration/tasks/TASK-0140-soak-evidence-bundle-validator/validate-soak-evidence.test.mjs
... 33 subtests ...
# tests 33
# pass 33
# fail 0
EXIT=0
```

Coverage includes every TASK-0135 canonical code (STALE_SOURCE_HEAD incl.
`--tip-under-judgment`, MISSING_PLATFORM_EVIDENCE, PORT_CAPSULE_COLLISION
reclassification of a submitted PASS, FORBIDDEN_PORT_6500 via attempt
disclosure and inside cited artifacts, TIMEOUT via watchdog exit 3 and
context-budget breach, NONZERO_SOAK via same-head deterministic masking and
bare exit 1, MISSING_ARTIFACT via absent/unparsable/field-incomplete
artifacts, RETRY_MASKED_FAILURE via the SPEC fixture) plus extensions
(UNSUPPORTED_PLATFORM, ARTIFACT_HASH_MISMATCH, INCOMPLETE_RUN,
PORT_CAPSULE_OUT_OF_RANGE, NON_LOOPBACK_BIND, INSUFFICIENT_INDEPENDENT_RUNS,
FRESH_PROCESS_UNDISCLOSED, RUN_INTERVAL_OVERLAP, CONCLUSION_UNSUPPORTED),
usage/parse/schema/unsupported-policy exit-2 classes, disabled-context
refusal, release_proof independence once enabled, honest-FAIL consistency,
and byte-level determinism across repeated runs.

Gate 2 — valid PASS fixture (exit must be 0):

```
node ... validate-soak-evidence.mjs --policy .../soak-integration-policy.json --bundle .../fixtures/valid-pass.json
{ "evaluated_conclusion": "PASS", "accepted": true, "errors": [], ... }
verdigris.validate-soak-evidence: evaluated=PASS accepted=true exit=0
EXIT=0
```

Gate 3 — retry-masked nonzero negative control (exit must be 1):

```
node ... validate-soak-evidence.mjs --policy .../soak-integration-policy.json --bundle .../fixtures/retry-masked-failure.json
{ "evaluated_conclusion": "FAIL", "accepted": false,
  "errors": [ { "code": "RETRY_MASKED_FAILURE", "detail": "bundle discloses 1 undisclosed failed attempt(s)" } ], ... }
verdigris.validate-soak-evidence: error RETRY_MASKED_FAILURE: bundle discloses 1 undisclosed failed attempt(s)
verdigris.validate-soak-evidence: evaluated=FAIL accepted=false exit=1
EXIT=1
```

Gate 4 — whitespace audit: `git diff --check` -> no output, `EXIT=0`.

Gate 5 — changed-path audit:
`git diff --name-only 6a10e862cc40a5aeb09694baa8d8446257df5382..HEAD` ->
`orchestration/RUN_STATUS.md`,
`orchestration/tasks/TASK-0140-soak-evidence-bundle-validator/SPEC.md`,
`orchestration/tasks/TASK-0140-soak-evidence-bundle-validator/STATUS.md`
(plus, after the implementation commit, only files under
`orchestration/tasks/TASK-0140-soak-evidence-bundle-validator/`). The first
two paths are the architect's READY promotion commit `9aa43b7a`, which
precedes this claim; every worker-authored path is inside the owned task
folder.

## Manual verification

- Ran the three CLI fixtures by hand from the repository root exactly as the
  SPEC invokes them and confirmed stdout/stderr separation, stable key
  order, and exit codes 0/1/1.
- Confirmed `--help` prints usage and exits 0; unknown/duplicate flags and
  missing values exit 2.
- Confirmed the validator performs no writes: no new files appear outside
  `os.tmpdir()` during runs (tests clean their temp dirs), and the repo
  working tree shows only the intended task-folder additions.

## Commit SHAs

- `2a1a1d9c` — claim (STATUS.md CLAIMED), pushed.
- `d9910a6d` — implementation deliverables, pushed.
- final REVIEW_REQUESTED commit — this report plus STATUS flip (SHA recorded
  in the STATUS transition log).

## Deviations

- None from the SPEC's outcome or gate requirements. One interpretive
  decision worth review: per SPEC "exit 0 only for a policy-valid PASS
  bundle", consistent FAIL/BLOCKED_ENVIRONMENTAL submissions also exit 1
  (with `accepted: true` in the result body); this is documented in
  VALIDATION.md and the `--help` text.

## Unresolved questions

- None blocking. If the architect prefers distinct exit codes for
  consistent-but-non-green submissions versus rejected bundles, that is a
  one-line follow-up revision.

## Risks

- The validator trusts the policy document it is pointed at; mitigation is
  the strict `unsupported_policy` refusal for anything but the accepted
  TASK-0135 policy shape (including mandatory 6500 forbiddance).

## Follow-ups

- If TASK-0129's emitter ever adds fields to its artifacts, extend
  verification there; nothing else is coupled.
