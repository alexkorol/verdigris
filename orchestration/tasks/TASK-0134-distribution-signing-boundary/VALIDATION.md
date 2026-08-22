# VALIDATION — TASK-0134 distribution/signing/owner-action boundary contract

Worker: `ox-pc-i` · Branch:
`codex/TASK-0134-distribution-signing-boundary-ox-pc-i` · Routed HEAD at
claim: `b3599c80122d09cd0685ae96830990cc5bada5cf` · Immutable task base:
`cab50d62cb121ab6a88fa513257e645447226959` (ancestry re-verified on resume
via `git merge-base --is-ancestor`, exit 0) · Claim commit: `5b952c08`.

## What this task is

This packet delivers the **boundary contract** separating machine-verifiable
distribution evidence from owner-only certificate, signing, notarization,
store, account, pricing, and publication actions:

1. `distribution-boundary.json` — the contract itself (artifacts, hashes,
   installer, update, rollback, machine actions MA-1..MA-6, owner actions
   OA-1..OA-5, machine→owner handoff inputs/outputs).
2. `fixtures/negative-cases.json` — seven declarative negative cases
   (MISSING_ARTIFACT, HASH_MISMATCH, UNSIGNED_ARTIFACT, MISSING_LICENSE,
   OWNER_CREDENTIAL_REQUIRED, NOTARIZATION_UNPROVEN, ROLLBACK_UNPROVEN) any
   future validator must reject.
3. `captures/` — retained gate output.
4. `VALIDATION.md` (this file) and `REPORT.md`.

No credentials were acquired, no external service was contacted, no account
or publication action was taken, and nothing in this packet claims that
signing or notarization occurred — the contract records only their absence
and their owner-lane disposition.

## Resume provenance

This is a resume of the claim in `STATUS.md` (commit `5b952c08`, pushed).
On resume:

- No `RELEASE.md` existed in the task folder (checked first, per PROTOCOL
  claim-release rule); the claim stands.
- Three dirty implementation files were found untracked in the task folder
  from the interrupted session: `distribution-boundary.json`,
  `fixtures/negative-cases.json`, `captures/gate-3-rg-distribution-surface.txt`.
  All three were preserved verbatim — not regenerated, not edited — after
  re-validating them (gates 1 and 2 pass against the preserved bytes; every
  repo-relative path referenced inside the contract was re-checked to exist).
- `VALIDATION.md` and `REPORT.md` were the missing deliverables; they are
  new in this session. No file outside
  `orchestration/tasks/TASK-0134-distribution-signing-boundary/**` was
  created or modified.

## Preflight proof (resume session, 2026-08-21)

| Check | Result |
| --- | --- |
| `git status --short` (pre-work) | only the three untracked task-folder paths above |
| `git branch --show-current` | `codex/TASK-0134-distribution-signing-boundary-ox-pc-i` |
| `git remote -v` origin | `https://github.com/alexkorol/verdigris` |
| `git fetch --prune origin` | done; `git status -sb` in sync `0  0` with origin worker branch |
| ancestry of base `cab50d62…` in HEAD | OK (`merge-base --is-ancestor`, exit 0) |
| `RELEASE.md` present? | no — claim not released |

## Environment identity (verified)

Captured inside this worktree on 2026-08-21; verbatim outputs:

```text
node --version                              -> v22.11.0
git --version                               -> git version 2.29.2.windows.2
rg --version                                -> ripgrep 15.0.0 (rev 3a612f88b8)
$PSVersionTable.PSVersion.ToString()        -> 7.6.4
opencode.exe --version                      -> 1.18.21
[System.Environment]::OSVersion.VersionString -> Microsoft Windows NT 10.0.19045.0
$env:PROCESSOR_ARCHITECTURE                 -> AMD64
```

Note: bare `opencode` is not on this shell's PATH; version was resolved via
`C:\Users\Alex\AppData\Roaming\npm\node_modules\opencode-ai\bin\opencode.exe --version`
→ `1.18.21`. No API key value was read, copied, or logged.

## Acceptance gate runs

All five SPEC acceptance commands were run literally, twice: once against the
preserved partial work before writing the remaining deliverables
(preliminary), and once after `VALIDATION.md` and `REPORT.md` were finalized
(final). Exit codes matched across runs.

| # | Command | Exit code | Output |
| --- | --- | --- | --- |
| 1 | `node -e "…distribution-boundary.json…"` (required-key check) | 0 | `distribution boundary: PASS` |
| 2 | `node -e "…fixtures/negative-cases.json…"` (required-case check) | 0 | `distribution negatives: PASS` |
| 3 | `rg -n 'installer\|sign\|certificate\|notari\|distribution\|publish\|release\|license\|rollback\|update' .github native docs orchestration -g '*.yml' -g '*.yaml' -g '*.md' -g '*.ps1'` | 0 | 530 lines preliminary; final run retained verbatim in `captures/gate-3-rg-distribution-surface.txt` (growth over the preserved capture is this task's own deliverables self-matching) |
| 4 | `git diff --check` | 0 | (no whitespace errors) |
| 5 | `git diff --name-only cab50d62cb121ab6a88fa513257e645447226959..HEAD` | 0 | 9 routed orchestration files (see below) |

Gate 3 inventory highlights (full capture retained): matches concentrate in
orchestration planning documents (`TASK-0120-release-verification-gap-audit`,
`PROGRAM_GRAPH.md`, `RUN_STATUS.md`, `DECISIONS.md`,
`TASK-0131/0133/0134` SPECs, this task's own deliverables), plus
`.github/workflows/ci.yml` ("Verify release candidate"),
`.github/CONTRIBUTING.md`, `native/build.ps1` (one installer-directory
comment), and `docs/deployment.md` (Caddy TLS certificates; git-pull update
recipe). No workflow, script, or doc in the scan performs signing,
notarization, store upload, or publication — the distribution surface is
documentation and CI plumbing only, which is exactly what the contract
records.

Gate 5 changed-file list (base → HEAD): the nine routed orchestration files
(`REENTRY-OX-ALPHA-PC.md`, `RUN_STATUS.md`, TASK-0112/0130/0131/0132/0133
SPECs, this task's `SPEC.md` + claim `STATUS.md`) predate the claim via the
routed HEAD; this worker's writes are confined to
`orchestration/tasks/TASK-0134-distribution-signing-boundary/**`.

## Negative controls (gate sensitivity proof)

Both machine gates were proven to fail closed by mutating disposable copies
under the session temp directory (real deliverables untouched):

- Copy of `distribution-boundary.json` with `rollback` deleted → gate 1
  snippet exits 1 with `Error: missing rollback`.
- Copy of `fixtures/negative-cases.json` with the `ROLLBACK_UNPROVEN` case
  removed → gate 2 snippet exits 1 with `Error: missing ROLLBACK_UNPROVEN`.

Positive runs exit 0; mutated runs exit 1 — the gates detect exactly the
missing-key / missing-case conditions they claim to.

## Honest evidence assessment

- No release artifact, hash manifest, installer, signature, notarization,
  store listing, or rollback drill exists; the contract records each as
  absent and routes it to its owner (`TASK-0126`, `TASK-0127`, `TASK-0094`,
  OA-1..OA-5).
- `machine_actions` MA-1..MA-5 are defined but not yet executed by this
  packet — executing them is TASK-0126's job; MA-6 is marked never
  machine-performable.
- All seven negative cases are declarative fixtures; no validator exists yet
  to execute them (first wiring is TASK-0126's), and this packet runs none.
- Every repo-relative path cited in the contract was re-checked to exist on
  resume (TASK-0120 evidence capture, TASK-0126 SPEC, constitution,
  `native/build.ps1`, `ecosystem.config.cjs`, `docs/deployment.md`).

## Deviation record

None. Unlike sibling lanes, `node_modules` is installed in this worktree, so
the yorkie `pre-commit` (`lint-staged`: eslint/stylelint on `*.{js,vue}`,
`*.vue`) runs normally; this packet adds only `.json`/`.md`/`.txt` files,
which lint-staged does not select. No `--no-verify` was used or needed.

## Verdict

Boundary contract delivered and self-consistent; every literal acceptance
gate passes with exit code 0, and both gates are proven fail-closed by
negative controls. The packet asserts no signing, notarization, publication,
or release readiness — it only fences those actions into the owner lane and
specifies the machine→owner handoff that must precede them.
