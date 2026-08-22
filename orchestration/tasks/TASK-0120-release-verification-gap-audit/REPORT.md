# TASK-0120 REPORT — release verification gap audit (ox-pc-e)

## Executive summary

Audited every release-verification surface named in the SPEC against
committed repository evidence at base `42718fbc` (+ coordination-only route
commit `039dcfa7`). Deliverables: `FINDINGS.md` (narrative separating
proven / partial / missing / owner-only gates) and
`captures/release-gates.json` (machine-verifiable inventory, 18 surfaces,
negative control embedded). Verdict: **not safely releasable yet** — CI,
protocol regression, and native server shutdown are proven; clean-machine
harness, long soak, performance budget, save migration, rollback, asset
manifest, macOS coverage, installer, and signing are partial, missing, or
owner-only. All four literal acceptance gates pass. Only this task folder
changed.

## Approach

1. Read AGENTS.md, orchestration/PROTOCOL.md, ORCHESTRATION.md,
   LEADER_POLICY.md, RUN_STATUS.md, ACCEPTANCE.md, STANDING-LOOP.md, SPEC.
2. Claimed first (STATUS.md committed alone, pushed) after fresh fetch
   confirmed no competing claim/RELEASE/route on origin.
3. Read-only evidence pass over `.github/workflows/{ci,native}.yml`,
   `native/tools/ci-native.ps1`, `native/tools/play-native.ps1`,
   `native/build.ps1`, `native/README.md`, `native/persistence/README.md`,
   `native/platform/README.md`, `native/tests/session_tests.cpp`,
   `native/src/core.cpp`, `package.json`, `ecosystem.config.cjs`,
   `docs/deployment.md`, playtest harness (`playtest/soak.mjs`),
   orchestration graph/backlog records — plus targeted greps for soak,
   migration/schemaVersion, shutdown, notar/codesign/installer,
   rollback/replay/support-bundle.
4. Classified each of the 18 SPEC surfaces by its strongest committed,
   machine-verifiable evidence; recorded discharge owners among existing
   READY/DRAFT successor packets (0083, 0094, 0099, 0100/0106, 0126, 0127).
5. Ran the literal gates, preserved transcripts in `captures/`, wrote
   FINDINGS + JSON inventory, then this REPORT and the REVIEW_REQUESTED flip.

## Changed files

- `orchestration/tasks/TASK-0120-release-verification-gap-audit/STATUS.md` (claim → REVIEW_REQUESTED)
- `orchestration/tasks/TASK-0120-release-verification-gap-audit/FINDINGS.md`
- `orchestration/tasks/TASK-0120-release-verification-gap-audit/REPORT.md`
- `orchestration/tasks/TASK-0120-release-verification-gap-audit/captures/release-gates.json`
- `orchestration/tasks/TASK-0120-release-verification-gap-audit/captures/gate-1-rg-release-surface.txt`

No file outside the owned path was created, modified, or deleted. No public
interfaces added or changed (documentation/evidence packet only).

## Test commands + outcomes (literal gate transcripts)

All commands run from repository root, pwsh 7, ripgrep 15.0.0, Node v22.11.0.

### Gate 1 — release-surface search

```
rg -n "CI|clean|build|package|launcher|soak|migration|upgrade|rollback|sign|notar|installer|release|artifact|crash|save" .github native docs orchestration --glob "*.md" --glob "*.yml" --glob "*.yaml" --glob "*.ps1" --glob "*.json"
```

- exit code: **0**
- full output preserved at `captures/gate-1-rg-release-surface.txt`
  (**1,692** matching lines). Note: an initial run before deliverables
  existed matched 1,575 lines; the preserved transcript is the final-state
  rerun which additionally matches this task's own new `*.md`/`*.json`
  files.

### Gate 2 — machine-verifiable inventory parses

```
node -e "JSON.parse(require('fs').readFileSync('orchestration/tasks/TASK-0120-release-verification-gap-audit/captures/release-gates.json','utf8')); console.log('release gates: PASS')"
```

- stdout: `release gates: PASS`
- exit code: **0**

### Gate 3 — whitespace check

```
git diff --check
```

- no output; exit code: **0**

### Gate 4 — changed-path scope proof

```
git diff --name-only
git status --porcelain=v1   # supplementary, because deliverables are new untracked files
```

- `git diff --name-only`: empty (no tracked-file modifications), exit **0**
- `git status --porcelain=v1`: only
  `?? orchestration/tasks/TASK-0120-release-verification-gap-audit/FINDINGS.md`
  and `?? orchestration/tasks/TASK-0120-release-verification-gap-audit/captures/`
  — i.e. **only this folder changes**, as the SPEC expects.

### Negative control (SPEC-required)

Recorded in both FINDINGS.md and `captures/release-gates.json`
(`negative_control` key): the deployment.md upgrade recipe is claimed safe
but has **no** clean-machine or migration artifact behind it — schemaVersion
mismatch fails restore closed, browser SQLite has no versioned migration,
and no rollback procedure exists anywhere. Discharge path: TASK-0126 +
TASK-0127 successors.

## Manual verification

- Confirmed base-to-head boundary: `git merge-base --is-ancestor
  42718fbc HEAD` true; exactly one commit (`039dcfa7`, "orchestration:
  register four-lane Ox CLI wave") sits between immutable base and route
  head, matching the START_HERE description of a coordination-only refresh.
- Cross-checked each "proven" classification against the exact enforcing
  command/workflow rather than prose claims; no narrow test promoted to
  broad release proof.
- Verified absence claims (installer/signing/notarization/rollback/migration
  paths) with dedicated searches, not just the broad Gate 1 pattern.
- Resource capsule respected: read-only audit; no installer executed, no
  account touched, no ports bound, port 6500 untouched throughout.

## Commit SHAs

- Claim commit: `97580939` — "TASK-0120: claim release verification gap audit (ox-pc-e)" (STATUS.md only), pushed to origin.
- Implementation/review-requested commit(s): see branch log; contains
  FINDINGS.md, captures/, REPORT.md, STATUS REVIEW_REQUESTED flip — this
  folder only.

## Deviations

- None from the SPEC's literal contract. One environment note, resolved
  under owner guidance during claim staging: the yorkie/lint-staged
  pre-commit hook initially failed in this fresh worktree because
  `node_modules` was absent; the owner restored dependencies via `npm ci`
  and the claim commit was completed with hooks enabled (no `--no-verify`
  used anywhere; shared Git metadata never edited).

## Unresolved questions

- None blocking review. If the architect wants `playtest/soak.mjs` wired
  into any standing gate before TASK-0083 lands, that is a spec decision
  above this packet's ownership.

## Risks

- Findings reflect evidence at base `42718fbc`/route `039dcfa7`; greens are
  environment-bound per ACCEPTANCE.md and go stale after surface/base/
  evaluator changes (G6 revalidation applies post-integration).
- The rg transcript embeds this task's own files (unavoidable self-match);
  counts are stated for both runs so the delta is auditable.

## Follow-ups (successor routing, not owned here)

- TASK-0083 (native lifecycle soak), TASK-0094 (asset provenance),
  TASK-0099 (performance budget), TASK-0100→0106 (replay record/runner):
  already READY; this audit confirms they are correctly sequenced.
- TASK-0126 clean-machine release harness and TASK-0127 save-migration
  matrix are the discharge path for the negative control; both depend on
  this packet being ACCEPTED.
- Owner-only lane remains open for signing/notarization/installer/distribution
  decisions once packaging work begins.
