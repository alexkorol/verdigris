# REPORT — TASK-0132 clean-machine harness contract

## Executive summary

Delivered the complete design artifact set for the future disposable-host
clean-machine execution harness: `clean-machine-contract.json` (12 ordered
stages, evidence-record schema, invariants, platform matrix, and the D-1..D-6
`clean_machine_decision` procedure separating clean-machine proof from cached
developer success), `fixtures/negative-cases.json` (eight deterministic
negative controls NEG-001..008), `VALIDATION.md` (honest static validation;
all `platform_matrix[].evidence_status` left `UNPROVEN` per SPEC stop
conditions), and this REPORT.

All five acceptance commands were run directly in this session and passed.
No file outside `orchestration/tasks/TASK-0132-clean-machine-harness-contract/`
was created or modified; no CI, machine, service, or system-software state was
touched. Port 6500 was never bound or contacted.

Session note: this session resumed after a tool-isolation denial in an earlier
attempt. Every command and capture in this session ran inside the worktree,
with captures kept inside the owned TASK-0132 folder only.

## Approach

The contract is written as a normative stage pipeline (checkout → toolchain →
dependencies → cache → build → tests → launch → smoke → process → port →
cleanup → artifacts) where every stage must emit a complete evidence record;
a stage without its record fails regardless of exit code. The cached-developer-
vs-clean-machine distinction is enforced structurally: cache rules CACHE-1..4,
the D-1..D-6 decision procedure, and negative control NEG-002 (CACHE_LEAK)
make it impossible for a warm-cache developer green to be relabeled as
clean-machine evidence. Port 6500 is forbidden at four independent layers:
port-stage rules PORT-1..4, launch-stage evidence checks, negative case
NEG-008, and decision step D-5 (severity P0, never waivable). Negative cases
are specified as reproducible injection recipes with detection predicates so a
future validator can be tested against them mechanically.

## Changed files (all under owned path)

- `clean-machine-contract.json` — new; the contract itself.
- `fixtures/negative-cases.json` — new; eight negative-control fixtures.
- `VALIDATION.md` — new; static validation record + validator obligations.
- `REPORT.md` — this file.
- `captures/rg-keyword-survey.txt` — verbatim capture of acceptance command A3
  (kept inside the owned folder; extra deliverable beyond the four named ones,
  retained as run evidence).
- `STATUS.md` — claim (commit `d6aa02d2`) plus state transitions to
  IMPLEMENTED / REVIEW_REQUESTED.

## Public interfaces added/changed

None. No product code, CI configuration, or build/test scripts were touched.
The contract JSON is a proposed interface for a future harness implementation
task; it changes nothing until that task consumes it.

## Acceptance commands and outcomes (literal transcripts)

### A1 — contract mandatory keys

```text
$ node -e "const fs=require('fs');const j=JSON.parse(fs.readFileSync('orchestration/tasks/TASK-0132-clean-machine-harness-contract/clean-machine-contract.json','utf8'));for(const k of ['schema_version','checkout','toolchain','dependencies','build','tests','launch','smoke','cleanup','artifacts','platform_matrix'])if(!(k in j))throw Error('missing '+k);console.log('clean-machine contract: PASS')"
clean-machine contract: PASS
exit=0
```

### A2 — negatives mandatory expected_error coverage

```text
$ node -e "const fs=require('fs');const j=JSON.parse(fs.readFileSync('orchestration/tasks/TASK-0132-clean-machine-harness-contract/fixtures/negative-cases.json','utf8'));for(const k of ['DIRTY_BASE','CACHE_LEAK','MISSING_TOOLCHAIN','DEPENDENCY_DRIFT','NONZERO_STAGE','LEAKED_PROCESS','NON_LOOPBACK_BIND','FORBIDDEN_PORT_6500'])if(!j.cases.some(x=>x.expected_error===k))throw Error('missing '+k);console.log('clean-machine negatives: PASS')"
clean-machine negatives: PASS
exit=0
```

### A3 — repo-wide keyword survey

```text
$ rg -n 'npm ci|cmake|build.ps1|RunTests|RunClientScenarios|playtest|smoke|6500|clean.machine|artifact' .github native docs package.json orchestration -g '*.yml' -g '*.yaml' -g '*.md' -g '*.ps1' -g '*.json'
exit=0
matches=1254
```

Full verbatim capture: `captures/rg-keyword-survey.txt` (1254 lines).
Per-top-level-prefix breakdown: `.github`=6, `native`=34, `docs`=142,
`package.json`=8, `orchestration`=1064.

Representative excerpt:

```text
package.json:17:    "dev:server": "cross-env NODE_ENV=development PORT=6500 nodemon --watch server --ext js,json --exec \"node --enable-source-maps server/index.js\"",
package.json:36:    "smoke:browser": "npm run build && start-server-and-test start:e2e http://127.0.0.1:6500 \"playwright test tests/e2e/browser-critical-loop.spec.mjs\"",
orchestration\ACCEPTANCE.md:12:| Full playtest | `npm run playtest` | 32/32, default flags | ~3.5 min; serialize per machine; boots :6510 |
native\tools\play-native.ps1:16:  throw "play-native: port 6500 is reserved for the historical browser server. Use 6520-6539."
```

Observation relevant to the contract's P0 invariant: all seven `native/**`
matches for `6500` are guardrails or documentation explicitly reserving or
forbidding it (`native\README.md:43`, `native\networking\README.md:11,16`,
`native\tools\play-native.ps1:15,16,78,79`). The remaining 6500 references
live in the historical browser reference and orchestration docs, which this
task does not own and did not modify. No harness stage exists yet, so there is
no current violation; the contract now makes any future one a P0 failure.

### A4 — whitespace/conflict-marker hygiene

```text
$ git diff --check
(no output)
exit=0
```

### A5 — owned-path confinement

```text
$ git diff --name-only cab50d62cb121ab6a88fa513257e645447226959..HEAD
orchestration/REENTRY-OX-ALPHA-PC.md
orchestration/RUN_STATUS.md
orchestration/tasks/TASK-0112-passive-tree-engine-scaffold/SPEC.md
orchestration/tasks/TASK-0130-gate-c-decision-envelope/SPEC.md
orchestration/tasks/TASK-0131-release-proof-manifest/SPEC.md
orchestration/tasks/TASK-0132-clean-machine-harness-contract/SPEC.md
orchestration/tasks/TASK-0132-clean-machine-harness-contract/STATUS.md
orchestration/tasks/TASK-0133-save-migration-rollback-contract/SPEC.md
orchestration/tasks/TASK-0134-distribution-signing-boundary/SPEC.md
exit=0
```

Interpretation: the immutable SPEC base `cab50d62` is an ancestor of the
routed HEAD `b3599c80` (verified via `git merge-base --is-ancestor`, exit 0).
The extra paths above are entirely from upstream architect commit `b3599c80`
("expand PC OpenRouter fleet to eight lanes"), which predates this claim and
is not worker-authored. The worker-authored delta is:

```text
$ git diff --name-only b3599c80122d09cd0685ae96830990cc5bada5cf..HEAD
orchestration/tasks/TASK-0132-clean-machine-harness-contract/STATUS.md
```

i.e. confined to the owned task folder, as required.

## Manual verification

- Contract completeness reviewed against the SPEC outcome list: checkout,
  dependencies, toolchain, cache, build, tests, launch, smoke, process, port,
  cleanup, artifacts stages plus `platform_matrix` and
  `clean_machine_decision` are all present (A1 checks the mandatory subset).
- Each of NEG-001..008 maps to exactly one detecting stage and one
  `expected_error`; mapping table reproduced in `VALIDATION.md`.
- Confirmed no `RELEASE.md` exists in the task folder before resuming, and
  origin branch was in sync (0/0 ahead/behind) after `git fetch --prune`.

## Commit SHAs

- `d6aa02d2` — claim commit (STATUS.md only), pushed.
- `<this commit>` — REVIEW_REQUESTED commit carrying the contract, fixtures,
  VALIDATION.md, captures/, REPORT.md, and STATUS transition.

## Deviations

1. **Resume-after-denial re-run:** the prior attempt was denied by tool
   isolation (writes outside the worktree). This session re-executed every
   acceptance command directly and confined all captures to the owned task
   folder. No results from the denied attempt are relied upon.
2. **VALIDATION.md confinement-row correction:** the original row claimed
   `cab50d62..HEAD` touched "only this task folder"; the actual diff includes
   the upstream architect commit `b3599c80`. Corrected during resume to cite
   the worker delta `b3599c80..HEAD` (owned folder only). No other content
   changed.
3. **Extra capture file** (`captures/rg-keyword-survey.txt`) beyond the four
   named deliverables, kept deliberately as durable acceptance evidence.

## Unresolved questions

None blocking. The pwsh-or-bash-adapter question for running
`native/build.ps1` on linux-x64/macos-arm64 is recorded in the contract's
platform notes as belonging to the implementing harness task.

## Risks

- The contract is unexecuted by design: no disposable host was provisioned
  (SPEC stop condition). First real runs may force corrections to toolchain
  pins (currently workstation-observed floors) and host-image declarations.
- Until the successor harness produces its first `clean_machine_verified`
  manifest, all repository greens remain labeled `developer_local` by D-6.

## Follow-ups

- Successor implementation task: build the harness validator that reproduces
  NEG-001..008 deterministically (each must yield its `expected_error`, none
  may go green) and executes the positive control on a fresh disposable host.
- Decide pwsh vs adapter strategy for non-Windows native builds when that
  task claims the work.
