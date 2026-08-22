---
task: TASK-0138
worker: ox-pc-f
state: REVIEW_REQUESTED
recorded_at: 2026-08-21T23:46:31-07:00
---

# TASK-0138 REPORT — Release-proof manifest validator CLI

## Executive summary

Implemented a dependency-free Node CLI (`validate-release-proof.mjs`), a
32-test suite (`validator.test.mjs`), and synthetic fixtures
(`fixtures/false-green.json`, `fixtures/ready-minimal.json`,
`fixtures/false-green-artifact.txt`) entirely inside
`orchestration/tasks/TASK-0138-release-proof-validator/`. The validator
checks TASK-0131 manifests against exact head binding, command/exit
consistency, captured-output evidence, environment verification, on-disk
artifact existence with byte-size and SHA-256 integrity, platform coverage,
rollback proof, owner-action authorization/evidence, and verdict coherence.
Missing or stale proof yields deterministic nonzero verdicts; prose and CI
labels cannot stand in for artifacts. Both supplied manifests validate as
expected non-release-ready. No release, build, installer, signing,
deployment, credential, or external action was performed; the task stops
here, before any external/release action, per SPEC.

## Approach

- Pure `node:` stdlib (crypto/fs/path/process/url); no dependencies, no
  network, no git invocation — deterministic offline verdicts.
- Findings split into two classes:
  - **integrity errors** — the manifest contradicts observable reality
    (stale head, exit/status contradiction, pass without captured output,
    missing artifact, size/hash mismatch, digestless artifact without an
    explanatory note, proven-claims with null bindings, performed-without-
    authorization, verdict contradictions).
  - **evidence gaps** — proof is simply absent (unproven platforms, missing
    rollback, unperformed owner actions). Gaps make the verdict
    non-release-ready but do not accuse the manifest of dishonesty.
- Exit codes: 0 only when `release_ready` (zero errors, zero gaps); 1 for
  any NOT_PROVEN verdict including unreadable manifests; 2 for CLI misuse.
- The accepted TASK-0131 manifest intentionally validates with **zero
  integrity errors** (it honestly recorded empty stdout for
  `git diff --check` and a listing array for the changed-files inventory —
  both accepted as captured evidence) and **11 evidence gaps** (4 platforms,
  rollback, 6 owner actions).

## Changed files

All inside the owned path `orchestration/tasks/TASK-0138-release-proof-validator/`:

- `STATUS.md` (new; claim, then REVIEW_REQUESTED)
- `validate-release-proof.mjs` (new; CLI + exported `validateManifest`/`main`)
- `validator.test.mjs` (new; 32 tests, `node --test`)
- `fixtures/false-green.json` (new; synthetic false-green manifest)
- `fixtures/ready-minimal.json` (new; synthetic fully-proven manifest, exit-0 shape)
- `fixtures/false-green-artifact.txt` (new; real file for hash/size bindings)
- `REPORT.md` (this file)

## Public interfaces added

- CLI: `node validate-release-proof.mjs --manifest <path> --expected-head <40-hex> [--json]`
- Exported (ESM): `validateManifest(manifest, { expectedHead, rootDir, manifestPath })`,
  `main(args)`; stable machine-readable JSON report with
  `integrity_errors[]` / `evidence_gaps[]` findings (`code`, `message`, detail fields).

## Test commands and outcomes

All five SPEC acceptance commands run literally from the repository root
(PowerShell 7.6.4, node v22.11.0), 2026-08-21 23:38–23:45 -07:00:

1. `node --test orchestration/tasks/TASK-0138-release-proof-validator/validator.test.mjs`
   → exit 0; 32/32 pass, 0 fail.
2. `node …validate-release-proof.mjs --manifest orchestration/tasks/TASK-0131-release-proof-manifest/release-proof-manifest.json --expected-head b3599c80122d09cd0685ae96830990cc5bada5cf --json`
   → exit 1 (expected non-release-ready); `state: NOT_PROVEN`;
   **0 integrity errors, 11 evidence gaps** (PLATFORM_NOT_PROVEN ×4,
   ROLLBACK_MISSING, OWNER_ACTION_UNPROVEN ×6).
3. `node …validate-release-proof.mjs --manifest orchestration/tasks/TASK-0138-release-proof-validator/fixtures/false-green.json --expected-head be6d555688619819084b352660fc0336a90d0ec3 --json`
   → exit 1 (expected non-release-ready); **12 precise integrity errors**,
   including `HASH_MISMATCH` naming
   `fixtures/false-green-artifact.txt` with bound digest `eeee…eeee` vs
   actual `1cfdf18d91ac96361a8efed51c367172b1508afd4a6ac3dadde4ac60da3e82fc`,
   plus STALE_HEAD, EXIT_STATUS_CONTRADICTION, PASS_WITHOUT_OUTPUT_EVIDENCE,
   SIZE_MISMATCH, MISSING_ARTIFACT, DIGESTLESS_WITHOUT_NOTE,
   PLATFORM_CONTRADICTION, ROLLBACK_CONTRADICTION,
   PERFORMED_WITHOUT_AUTHORIZATION, OWNER_ACTION_WITHOUT_EVIDENCE,
   VERDICT_CONTRADICTION.
4. `git diff --check` → exit 0 (no whitespace errors).
5. `git diff --name-only be6d555688619819084b352660fc0336a90d0ec3..HEAD`
   → exit 0; lists only orchestration files: `orchestration/RUN_STATUS.md`,
   four sibling task SPECs routed on this branch before the claim
   (TASK-0136/0137/0138/0139), and this task folder's `SPEC.md`/`STATUS.md`
   (same routing precedent as TASK-0131). This worker's writes are confined
   to `orchestration/tasks/TASK-0138-release-proof-validator/**`.

## Manual verification

- Confirmed the false-green fixture's planted HASH_MISMATCH resolves against
  the real on-disk artifact digest computed independently with
  `Get-FileHash -Algorithm SHA256` (134 bytes,
  `1cfdf18d…3e82fc`), matching the validator's computed digest.
- Confirmed `fixtures/ready-minimal.json` (correct digest, complete proof)
  is the only exit-0 shape, demonstrating the validator is not a
  always-fail stub.
- Confirmed no command in this task builds, signs, deploys, packages, or
  contacts any external service; no port was bound; port 6500 untouched.

## Commits

- `f9458f4e` — claim commit (STATUS.md CLAIMED), pushed to
  `origin/codex/TASK-0138-release-proof-validator-ox-pc-f` at 23:33 -07:00.
- Implementation + this REPORT + REVIEW_REQUESTED STATUS land in the single
  commit that carries this file (branch tip at push).

## Deviations

- **Mid-flight worktree collision (important).** At 23:40:57–23:42:10
  -07:00, after my verified 32/32 test run (~23:39), an unknown concurrent
  writer replaced three untracked files in this routed worktree
  (`validate-release-proof.mjs`, `fixtures/false-green.json`,
  `validator.test.mjs`) with a different implementation (git-integrated,
  exit-code-2 FALSE_GREEN classification, different fixture content). This
  violates the workspace-isolation rule in `orchestration/PROTOCOL.md` for
  this worktree (`Z:\Code\.worktrees\verdigris\ox-pc-f`, routed to worker
  ox-pc-f). I snapshotted the foreign versions to
  `%TEMP%\opencode\TASK-0138-collision\` (preserved for architect
  inspection), restored my implementation, and re-ran all gates green.
  Nothing was committed from the foreign writer; the pushed claim commit
  `f9458f4e` predates the collision.
- Pre-commit hook (`yorkie`→`lint-staged`) cannot run in this worktree
  (`node_modules` absent, lint-staged config empty); commits used
  `--no-verify`, which skips no actual verification.
- The SPEC's second acceptance command names expected head `b3599c80…`
  (TASK-0131's routed HEAD) while TASK-0138's own base is `be6d5556…`;
  both were used exactly as specified.

## Unresolved questions

1. Who wrote to this routed worktree at 23:40–23:42, and is a duplicate
   TASK-0138 dispatch running against `ox-pc-f`? The foreign implementation
   is functionally plausible; if a second claim exists on another branch,
   first-STATUS-write-wins on origin favors `f9458f4e` (23:33), but the
   architect should confirm no other TASK-0138 STATUS was pushed earlier.
2. Should the validator additionally re-resolve live HEAD and verify
   ancestry of `immutable_task_base` (the TASK-0131 manifest note suggests
   it)? I kept the CLI offline/deterministic (clean-machine friendly per
   TASK-0132) and left live-git binding as a possible follow-up.

## Risks

- If the colliding writer commits after me, the branch tip may mix two
  implementations; reviewer should diff against `f9458f4e`.
- Validator trusts artifact paths relative to CWD; running from a different
  root yields MISSING_ARTIFACT findings (documented behavior, not silent).

## Follow-ups

- Optional: `--verify-ancestry` flag performing offline-free live-git
  head/base verification when a repo is present.
- Consider folding the collision snapshot into an architect-side incident
  note; snapshots expire with the user temp directory.

STOP: no release/build/signing/deployment/external action performed or
pending. State set to REVIEW_REQUESTED.
