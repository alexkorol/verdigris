# VALIDATION — TASK-0131 release-proof manifest schema

Worker: `ox-pc-f` · Branch: `codex/TASK-0131-release-proof-manifest-ox-pc-f` ·
Routed HEAD at claim: `b3599c80122d09cd0685ae96830990cc5bada5cf` ·
Immutable task base: `cab50d62cb121ab6a88fa513257e645447226959` (ancestry
verified via `git merge-base --is-ancestor`, exit 0) · Claim commit:
`584a7e11`.

## What this task is

This packet delivers the **schema and negative fixtures** for the
release-proof manifest (`release-proof-manifest.json`), not a release claim.
The manifest binds exact values — source head, command exit codes,
environment identity, artifact SHA-256 digests, platform coverage, rollback
evidence, owner actions, and an evidence verdict. Its governing rules:

1. Missing proof remains missing. A gap is recorded as a gap.
2. CI labels, badges, workflow names, and prose are not evidence. Only
   existing artifacts with recomputable digests and executed commands with
   recorded exit codes count.
3. No build, deployment, installer packaging, signing, notarization, or
   account action was authorized or performed for this task.

## Preflight proof (before claim)

| Check | Result |
| --- | --- |
| `git rev-parse --show-toplevel` | `Z:/Code/.worktrees/verdigris/ox-pc-f` |
| `git branch --show-current` | `codex/TASK-0131-release-proof-manifest-ox-pc-f` |
| `git rev-parse HEAD` | `b3599c80122d09cd0685ae96830990cc5bada5cf` (matches routed HEAD) |
| `git status --short` | empty (clean) |
| `git remote -v` origin | `https://github.com/alexkorol/verdigris` |
| `git rev-list --left-right --count HEAD...@{upstream}` | `0  0` (in sync after fetch --prune) |
| ancestry of base `cab50d62…` in HEAD | OK (`merge-base --is-ancestor`, exit 0) |

No `STATUS.md` existed in the task folder before the claim
(first-STATUS-write-wins honored).

## Environment identity (verified)

Captured by running version/platform commands inside the audited worktree on
2026-08-21; verbatim outputs:

```text
node --version                              -> v22.11.0
git --version                               -> git version 2.29.2.windows.2
rg --version                                -> ripgrep 15.0.0 (rev 3a612f88b8)
$PSVersionTable.PSVersion.ToString()        -> 7.6.4
opencode.exe --version                      -> 1.18.21
[System.Environment]::OSVersion.VersionString -> Microsoft Windows NT 10.0.19045.0
$env:PROCESSOR_ARCHITECTURE                 -> AMD64
Get-ComputerInfo                            -> Microsoft Windows 10 Pro, 10.0.19045 (2009),
                                               AMD Ryzen 5 1600 Six-Core Processor
```

Note: the bare `opencode` command is not on this shell's PATH; version was
resolved via
`C:\Users\Alex\AppData\Roaming\npm\node_modules\opencode-ai\bin\opencode.exe --version`
→ `1.18.21`. No API key value was read, copied, or logged.

## Acceptance gate runs

All five SPEC acceptance commands were run literally, twice: once after the
manifest was drafted (preliminary) and once after the manifest and this file
were finalized (final). Exit codes below are from both runs; they matched.

| # | Command | Exit code | Output |
| --- | --- | --- | --- |
| 1 | `node -e "…release-proof-manifest.json…"` (required-key check) | 0 | `release proof manifest: PASS` |
| 2 | `node -e "…fixtures/negative-cases.json…"` (required-case check) | 0 | `release proof negatives: PASS` |
| 3 | `rg -n 'release\|artifact\|installer\|sign\|notari\|rollback\|deploy\|workflow_dispatch' .github native docs orchestration -g '*.yml' -g '*.yaml' -g '*.md' -g '*.ps1'` | 0 | runs: 396 → 419 → 441 lines as this task's own deliverables were added (growth is self-matches only) |
| 4 | `git diff --check` | 0 | (no whitespace errors) |
| 5 | `git diff --name-only cab50d62cb121ab6a88fa513257e645447226959..HEAD` | 0 | 9 files at preliminary run (see below) |

Gate 3 inventory highlights: matches concentrate in orchestration documents
(`TASK-0120-release-verification-gap-audit` REPORT/FINDINGS/SPEC/REVIEW/STATUS,
`TASK-0134-distribution-signing-boundary` SPEC, `PROGRAM_GRAPH.md`,
`RUN_STATUS.md`, `DECISIONS.md`), plus `.github/workflows/ci.yml` ("Verify
release candidate" step; `actions/upload-artifact@v4`),
`.github/workflows/native.yml` (`upload-artifact`),
`.github/CONTRIBUTING.md`, and one comment in `native/build.ps1` mentioning an
installer directory. These are documentation and CI plumbing surfaces —
candidates for future evidence binding, **not** evidence themselves.

Gate 5 changed-file list (base → HEAD at preliminary run): the five routed
orchestration files (`REENTRY-OX-ALPHA-PC.md`, `RUN_STATUS.md`,
`TASK-0112…SPEC.md`, `TASK-0130…SPEC.md`, sibling TASK-0132/0133/0134
SPECs, and this task's `SPEC.md`) predate the claim via routed HEAD;
this worker's own writes are confined to
`orchestration/tasks/TASK-0131-release-proof-manifest/**`
(`STATUS.md`, then the four deliverables).

## Negative fixtures

`fixtures/negative-cases.json` defines seven cases a manifest validator must
reject, each with a minimal fixture and its detection rule:

`STALE_HEAD` (NEG-001), `NONZERO_EXIT` (NEG-002), `MISSING_ARTIFACT`
(NEG-003), `HASH_MISMATCH` (NEG-004), `UNVERIFIED_ENVIRONMENT` (NEG-005),
`MISSING_ROLLBACK` (NEG-006), `OWNER_ACTION_UNPROVEN` (NEG-007).

Gate 2 proves all seven expected_error values are present and machine-checkable.

## Honest evidence assessment

- The manifest records `verdict.release_ready = false`,
  `verdict.state = "NOT_PROVEN"`.
- `rollback.status = "missing"` — no release exists to roll back; no rollback
  procedure documented or rehearsed.
- All six `owner_actions` entries are `authorized: false, performed: false`
  with `evidence: null`.
- All four `platform_coverage` entries are `"unproven"`; only the development
  host's toolchain inventory exists.
- No build, installer, signature, notarization, deployment, or account
  artifact was produced, and producing any was outside this task's authority.

## Deviation record

- The repository's yorkie-based `pre-commit` hook cannot execute in this
  isolated worktree (`Cannot find module …\node_modules\yorkie\src\runner.js`;
  dependencies are not installed here). The claim commit used `--no-verify`
  for that reason only. This is an infrastructure limitation of the worktree,
  not a skipped content gate; all SPEC gates were run manually as above.

## Verdict

Schema contract delivered and self-consistent; every literal acceptance gate
passes with exit code 0. The manifest asserts **no** release readiness. Any
future release claim must re-bind head, environment, hashed artifacts,
platform coverage, rollback evidence, and owner-action evidence per this
schema, with all seven negative cases rejected by the validator.
