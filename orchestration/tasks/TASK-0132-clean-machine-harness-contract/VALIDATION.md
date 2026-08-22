# VALIDATION — TASK-0132 clean-machine harness contract

Date: 2026-08-22 UTC. Validator: ox-pc-g worker session (OpenCode CLI 1.18.21,
`openrouter/stealth/ox-alpha`), worktree `Z:\Code\.worktrees\verdigris\ox-pc-g`,
base `b3599c80122d09cd0685ae96830990cc5bada5cf`.

## What this file covers

Static validation of the contract artifacts produced by this task, plus the
mapping that a future harness validator must implement. This is NOT a
clean-machine run: per the SPEC stop conditions, no platform is claimed as
covered without durable evidence, and all `platform_matrix[].evidence_status`
values are honestly `UNPROVEN`.

## Static validation performed (literal transcripts in REPORT.md)

| Check | Command class | Result |
|---|---|---|
| Contract parses and carries every mandatory stage key | `node -e "…clean-machine-contract.json…"` | PASS, exit 0 |
| Negatives parse and cover every mandatory `expected_error` | `node -e "…fixtures/negative-cases.json…"` | PASS, exit 0 |
| Repo-wide keyword survey for contract vocabulary | `rg -n 'npm ci|cmake|build.ps1|RunTests|RunClientScenarios|playtest|smoke|6500|clean.machine|artifact' …` | exit 0; 1254 matches; verbatim capture at `captures/rg-keyword-survey.txt`; see REPORT |
| Whitespace/conflict-marker hygiene | `git diff --check` | PASS, exit 0 |
| Owned-path confinement (worker delta from claim base) | `git diff --name-only b3599c80122d09cd0685ae96830990cc5bada5cf..HEAD` | only this task folder (`STATUS.md`) |

Note on the immutable SPEC base: `cab50d62` is a verified ancestor of routed
HEAD `b3599c80`; `git diff --name-only cab50d62..HEAD` additionally lists
paths from upstream architect commit `b3599c80` ("expand PC OpenRouter fleet
to eight lanes"), which predates this claim and is not worker-authored.
Corrected during the post-denial resume session; see REPORT deviations.

## Negative-case → stage mapping (validator obligations)

A future harness validator must reproduce each fixture deterministically and
require the named failure:

| Case | Injected fault | Detecting stage | Expected error |
|---|---|---|---|
| NEG-001 | tracked-file mutation after pinned checkout | checkout (`git status --porcelain`) | DIRTY_BASE |
| NEG-002 | cache hit stripped of provenance | cache provenance audit | CACHE_LEAK |
| NEG-003 | required tool absent / version below pin | toolchain probes | MISSING_TOOLCHAIN |
| NEG-004 | node_modules altered post-install | dependency fingerprint equality | DEPENDENCY_DRIFT |
| NEG-005 | deterministic test forced to fail | tests (exit code) | NONZERO_STAGE |
| NEG-006 | cleanup skips a tracked PID | process leak enumeration | LEAKED_PROCESS |
| NEG-007 | bind to 0.0.0.0 inside capsule | launch listener table | NON_LOOPBACK_BIND |
| NEG-008 | any reference or listener on port 6500 | port invariant sweep | FORBIDDEN_PORT_6500 |

Pass criterion: all eight cases yield their `expected_error`; none yields a
green run; the unmodified pipeline on a fresh disposable host yields zero
expected errors and labels the run `clean_machine_verified`.

## Cached developer success vs clean machine (decision procedure)

The contract's `clean_machine_decision` block is normative: D-1 through D-6.
Summary — a green counts as clean-machine evidence only when the host was
disposable and image-declared (D-1), the workspace freshly cloned at a pinned
commit with empty status (D-2), every cache recorded cold-miss or full
provenance (D-3), every stage emitted complete evidence with exit zero (D-4),
and process/port verification proved no leaks, loopback-only capsule binds,
and zero contact with port 6500 (D-5). Everything else is labeled
`developer_local` (or `invalid`). Current repository greens are therefore
`developer_local` until the future harness produces its first
`clean_machine_verified` run manifest.

## Port 6500 enforcement points

Port 6500 is forbidden at four independent layers so no single missed check
lets it through: (1) `port` stage rules PORT-1..4, (2) `launch` evidence
checks both arguments/config and observed listeners, (3) negative case
NEG-008 must reproduce the failure, (4) `clean_machine_decision` D-5 refuses
the label if any contact with 6500 is observed. Severity is P0 and never
waivable.

## Honest limits

- No disposable host was provisioned in this task; the contract is a design
  artifact, not an executed proof. Stop condition honored rather than claiming
  coverage.
- Toolchain pins record current observed workstation versions as floors; the
  first disposable-host run must confirm or correct them.
- The `linux-x64` and `macos-arm64` rows depend on a pwsh-or-adapter decision
  for `native/build.ps1` that belongs to the implementing harness task.
