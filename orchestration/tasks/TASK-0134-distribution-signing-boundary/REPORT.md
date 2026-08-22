# REPORT — TASK-0134 distribution/signing/owner-action boundary contract

Worker: `ox-pc-i` · Branch:
`codex/TASK-0134-distribution-signing-boundary-ox-pc-i` · Base:
`cab50d62cb121ab6a88fa513257e645447226959` · Routed HEAD:
`b3599c80122d09cd0685ae96830990cc5bada5cf` · Claim commit: `5b952c08`.

## Executive summary

Delivered the distribution/signing boundary contract as four deliverables +
retained capture inside the task folder: `distribution-boundary.json` (the
machine-vs-owner boundary contract), `fixtures/negative-cases.json` (seven
negative cases any future validator must reject), `VALIDATION.md`
(preflight/resume proof, environment identity, literal gate runs, negative
controls, honest evidence assessment), this report, and
`captures/gate-3-rg-distribution-surface.txt`. The contract separates what an
implementation worker can prove about release artifacts (build, hash,
verify, drill on fixtures) from actions requiring owner identity
(certificates, signing, notarization, stores, accounts, pricing,
publication), and defines the machine→owner handoff in both directions. No
credential was acquired, no external service contacted, no account or
publication action taken, and nothing claims signing/notarization occurred.
All five SPEC acceptance commands pass with exit code 0; both machine gates
were additionally proven fail-closed via mutated-copy negative controls.

## Approach

- Resumed the pushed claim (`5b952c08`) after checking for `RELEASE.md`
  (none — claim stands per PROTOCOL) and re-running the AGENTS.md preflight.
- Preserved the three dirty partial-work files verbatim from the interrupted
  session; re-validated them instead of regenerating (gates 1–2 pass on the
  preserved bytes; all repo-relative paths cited inside the contract were
  re-checked to exist).
- Modeled the boundary on a four-question classification test (credential?
  third-party identity? legal/economic commitment? reproducible from fresh
  clone?) recorded inside the contract, so future packets inherit the rule
  rather than a one-off judgment.
- Grounded every "current status" claim in existing evidence (TASK-0120
  release-verification-gap-audit findings) and routed each gap to its owning
  successor (`TASK-0126` artifacts/manifests, `TASK-0127` migration/rollback
  evidence, `TASK-0094` license inventory) so the contract creates no
  overlapping ownership.
- Ran all five SPEC acceptance commands literally, twice (preliminary +
  final), plus negative controls proving both machine gates fail closed.

## Changed files

All inside `orchestration/tasks/TASK-0134-distribution-signing-boundary/`:

- `distribution-boundary.json` (preserved from interrupted session)
- `fixtures/negative-cases.json` (preserved from interrupted session)
- `captures/gate-3-rg-distribution-surface.txt` (preserved; final-state run
  retained per VALIDATION.md gate table)
- `VALIDATION.md` (new)
- `REPORT.md` (this file)
- `STATUS.md` (claim → REVIEW_REQUESTED)

No file outside the task folder was created or modified. `forbidden_paths`
(`native/**`, `server/**`, `src/**`, `playtest/**`, credentials, external
accounts, release publication) untouched.

## Public interfaces added/changed

- `distribution-boundary.json` schema v1: required keys `schema_version`,
  `artifacts`, `hashes`, `installer`, `update`, `rollback`,
  `machine_actions` (MA-1..MA-6), `owner_actions` (OA-1..OA-5), `handoff`;
  plus `boundary_rule` (classification test + hard prohibitions),
  `negative_cases_ref`, and `successor_wiring`.
- Machine→owner handoff schema: inputs (`unsigned_artifacts`,
  `hash_manifest`, `negative_case_transcript`, `rollback_drill_status`,
  `asset_license_inventory`, `open_risks`) and outputs expected back
  (`signature_status`, `notarization_status`, `channel_identifiers`,
  `license_rulings`), with exchange rules (repo-file transport only, hashed
  bundles, unsigned artifacts cross read-only).
- `fixtures/negative-cases.json` contract: `cases[]` entries with `id`,
  `simulated_condition`, `expected_error`, `boundary_response`,
  `machine_verifiable`; consumers assert observed error equals
  `expected_error` and fail closed.
- Error vocabulary for future validators: `MISSING_ARTIFACT`,
  `HASH_MISMATCH`, `UNSIGNED_ARTIFACT`, `MISSING_LICENSE`,
  `OWNER_CREDENTIAL_REQUIRED`, `NOTARIZATION_UNPROVEN`, `ROLLBACK_UNPROVEN`.

## Test commands + outcomes

All five SPEC acceptance commands run literally from the worktree root,
twice (preliminary and final runs identical in exit codes):

| Command | Exit | Output |
| --- | --- | --- |
| boundary required-key check (node -e) | 0 | `distribution boundary: PASS` |
| negative-cases required-case check (node -e) | 0 | `distribution negatives: PASS` |
| `rg -n 'installer\|sign\|…' .github native docs orchestration …` | 0 | 530 lines preliminary; final state captured verbatim in `captures/gate-3-rg-distribution-surface.txt` |
| `git diff --check` | 0 | clean |
| `git diff --name-only cab50d62…..HEAD` | 0 | 9 routed files; worker writes confined to task folder |

Negative controls (disposable temp copies; real deliverables untouched):
deleting `rollback` from a boundary copy → gate exits 1 (`missing rollback`);
removing the `ROLLBACK_UNPROVEN` case from a fixtures copy → gate exits 1
(`missing ROLLBACK_UNPROVEN`). Gates are sensitive to exactly the conditions
they check. Full detail in `VALIDATION.md`.

## Manual verification

- Resume checks: no `RELEASE.md` in task folder; preflight
  (`status/remote/fetch --prune/status -sb/upstream count 0 0`);
  `git merge-base --is-ancestor cab50d62… HEAD` exit 0.
- Preserved-file validation: gates 1–2 against unmodified dirty bytes;
  existence re-check of every repo-relative path referenced by the contract
  (TASK-0120 evidence capture, TASK-0126 SPEC, constitution, build script,
  deployment doc).
- Environment identity captured (node v22.11.0, git 2.29.2.windows.2,
  ripgrep 15.0.0, PowerShell 7.6.4, Windows 10.0.19045 AMD64, harness
  opencode 1.18.21) — see `VALIDATION.md`.

## Commit SHAs

- Claim: `5b952c08` (pushed before interruption).
- Implementation + validation record: the commit carrying this file
  (exact SHA recorded in `STATUS.md` transition log).
- REVIEW_REQUESTED status update: the follow-up commit on this branch
  (exact SHA recorded in `STATUS.md` transition log).

## Deviations

None. Dependencies are installed in this worktree, so the yorkie pre-commit
runs normally (and selects none of these `.json`/`.md`/`.txt` files); no
`--no-verify` was used.

## Unresolved questions

None blocking acceptance. One deliberate scoping note: the contract fixes
SHA-256 as the only hash algorithm and a sha256sum-compatible manifest line
format; if TASK-0126 wants a different manifest container (e.g., JSON), it
must extend this contract rather than silently diverge — flag in REVIEW.md
if disagreeable.

## Risks

- The contract is prose+data only until TASK-0126 wires validators; nothing
  enforces MA/OA discipline mechanically yet. Mitigated by the negative-case
  vocabulary being machine-checkable once wired.
- Owner-lane actions (OA-1..OA-5) stay open indefinitely until the owner
  acts; downstream tasks must treat signature/notarization as absent, never
  assume them.

## Follow-ups

- TASK-0126: produce first real artifacts + manifests consuming MA-1..MA-3
  and the handoff input schema.
- TASK-0127: supply migration/rollback evidence clearing
  `ROLLBACK_UNPROVEN`.
- TASK-0094: asset license inventory feeding OA-4 and clearing
  `MISSING_LICENSE`.
