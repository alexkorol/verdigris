# REPORT — TASK-0135 server lifecycle soak integration policy contract

Worker: `ox-pc-d` · Branch: `codex/TASK-0135-server-lifecycle-soak-integration-contract-ox-pc-d` · Machine: DESKTOP-TVU7OR7 · 2026-08-21

## Executive summary

Delivered a content-neutral, machine-readable policy (`soak-integration-policy.json`)
defining when and how the accepted TASK-0129 server lifecycle soak runs across
four contexts (local, pre_merge, nightly, release_proof), binding source head,
platform, port capsule, repetition, timeout, artifact retention/integrity,
quarantine, retry, escalation, and verdict-evidence rules. Eight negative-case
fixtures pin the rejection semantics; every case names its enforcing policy
rule. Deterministic failure is permanently distinguished from environmental
port contention, and no retry can convert a failed or missing soak into green.
Nightly scheduling and release-proof interpretation remain `OWNER_PENDING`
with smallest-path options. No CI, native, server, src, playtest, or `.github`
file was touched; port 6500 is encoded as forbidden. All five literal SPEC
gates pass with exit code 0.

## Approach

1. Read the accepted TASK-0129 surface at this worktree to bind real facts, not
   guesses: `native/tools/server_lifecycle_soak.cpp` (cycle/burst constants,
   capsule rotation, watchdog, exit codes 0/1/2/3), `native/build.ps1`
   `-RunServerLifecycleSoak` block (invocation, capture location),
   `orchestration/ACCEPTANCE.md` (staleness rules).
2. Authored the policy around one supreme invariant (non-masking) expressed
   four ways: green preconditions, retry trigger allow/deny lists,
   same-head deterministic-failure lockout, and an explicit never-green list.
3. Environmental contention is classified structurally (binder exhaustion in a
   `server-start`/`burst-server-start` phase with zero attempted upgrades)
   versus anything after a successful server start (deterministic).
4. Fixtures encode eight rejection scenarios; `VALIDATION.md` records machine
   checks including a fixture→policy rule-resolution cross-check so fixtures
   cannot drift from the policy text.
5. Owner authority boundaries kept: nightly host/schedule and release-proof
   activation/interpretation are OWNER_PENDING; cheapest option listed first.

## Changed files

All inside `orchestration/tasks/TASK-0135-server-lifecycle-soak-integration-contract/`:

- `soak-integration-policy.json` (new)
- `fixtures/negative-cases.json` (new)
- `VALIDATION.md` (new)
- `STATUS.md` (new; claim → REVIEW_REQUESTED transitions)
- `REPORT.md` (new; this file)
- `evidence/gate3-rg-transcript.txt` (new; full preserved output of SPEC gate 3)

The `git diff --name-only eef04840…HEAD` listing additionally contains
`orchestration/RUN_STATUS.md`, the task-folder `SPEC.md`, and lane history
files; those entered history via coordinator/architect routing commits
(`aab574b8` and earlier) already on the routed base, not via worker writes.
Worker-authored changes are exactly the six files above.

## Public interfaces added or changed

None outside the task folder. No code, CI, harness, or protocol change.
Machine-readable interface added for future evaluators:

- Policy document schema (`schema_version` `1.0.0`) with required top-level keys
  and verdict evidence-bundle schema (`verdict.evidence_bundle_schema`).
- Negative-fixture schema: `cases[].expected_error` must cover the eight
  canonical codes; each case carries a resolvable `policy_rule` path.

## Acceptance commands and outcomes (literal)

Run from repository root, Windows PowerShell 7, worker branch HEAD `f58cb814`
(implementation commit; REPORT/STATUS flip added on top afterward).

### Gate 1 — policy key presence and forbidden-port binding

Command:

```powershell
node -e "const fs=require('fs');const j=JSON.parse(fs.readFileSync('orchestration/tasks/TASK-0135-server-lifecycle-soak-integration-contract/soak-integration-policy.json','utf8'));for(const k of ['schema_version','contexts','source_head','platform','port_capsule','repetition','timeout','artifacts','retry','quarantine','escalation','verdict'])if(!(k in j))throw Error('missing '+k);if(j.port_capsule.forbidden_ports.indexOf(6500)<0)throw Error('6500 not forbidden');console.log('soak integration policy: PASS')"
```

Output:

```text
soak integration policy: PASS
```

Exit code: `0`

### Gate 2 — negative-case coverage

Command:

```powershell
node -e "const fs=require('fs');const j=JSON.parse(fs.readFileSync('orchestration/tasks/TASK-0135-server-lifecycle-soak-integration-contract/fixtures/negative-cases.json','utf8'));for(const k of ['STALE_SOURCE_HEAD','MISSING_PLATFORM_EVIDENCE','PORT_CAPSULE_COLLISION','FORBIDDEN_PORT_6500','TIMEOUT','NONZERO_SOAK','MISSING_ARTIFACT','RETRY_MASKED_FAILURE'])if(!j.cases.some(x=>x.expected_error===k))throw Error('missing '+k);console.log('soak policy negatives: PASS')"
```

Output:

```text
soak policy negatives: PASS
```

Exit code: `0`

### Gate 3 — cross-repository reference sweep (informational)

Command:

```powershell
rg -n 'RunServerLifecycleSoak|server_lifecycle_soak|nightly|workflow_dispatch|timeout|artifact|quarantine|retry|6500' native .github orchestration -g '*.ps1' -g '*.cpp' -g '*.yml' -g '*.yaml' -g '*.md' -g '*.json'
```

Output: 395 matching lines; exit code `0`. Full transcript preserved verbatim at
`evidence/gate3-rg-transcript.txt`. First lines:

```text
native\build.ps1:6:  [switch]$RunServerLifecycleSoak
native\build.ps1:140: if ($RunServerLifecycleSoak) {
native\build.ps1:141:   $soakObj = Join-Path $buildRoot "server_lifecycle_soak.obj"
```

Last line:

```text
orchestration\tasks\TASK-0129-server-lifecycle-soak\REVIEW.md:14: Architect verification on Windows completed with exit 0 for `-RunTests -RunClientScenarios`; … Verdict: **ACCEPTED**.
```

Observation relevant to review: no `.yml`/`.yaml` workflow defines a nightly
soak or `workflow_dispatch` hook for it — consistent with the policy leaving
nightly hosting OWNER_PENDING and authorizing zero CI change.

### Extra self-check — fixture-to-policy rule resolution

Command documented in `VALIDATION.md` §1.3; output:

```text
negative-case rule resolution: PASS (8 cases)
```

Exit code: `0`

### Gate 4 — whitespace hygiene

Command: `git diff --check`

Output: (empty)

Exit code: `0`

### Gate 5 — diff scope since SPEC base

Command: `git diff --name-only eef04840465f8b3e0400be182ff29506a520eb60..HEAD`

Output at implementation commit:

```text
orchestration/RUN_STATUS.md
orchestration/tasks/TASK-0135-server-lifecycle-soak-integration-contract/SPEC.md
orchestration/tasks/TASK-0135-server-lifecycle-soak-integration-contract/STATUS.md
orchestration/tasks/TASK-0135-server-lifecycle-soak-integration-contract/VALIDATION.md
orchestration/tasks/TASK-0135-server-lifecycle-soak-integration-contract/fixtures/negative-cases.json
orchestration/tasks/TASK-0135-server-lifecycle-soak-integration-contract/soak-integration-policy.json
```

Exit code: `0`

Scope audit: `orchestration/RUN_STATUS.md` and the folder `SPEC.md` differ from
the SPEC base solely through coordinator routing commit `aab574b8` ("integrate
TASK-0129 and route successor"), which is the immutable routed base of this
worker branch — not a worker write. Every worker-authored path is inside
`owned_paths`.

## Manual verification

- Confirmed `native/build.ps1` still compiles/drives nothing new: no build was
  run because no native file changed; verified by inspection only.
- Confirmed both JSON deliverables parse under Node's strict `JSON.parse`.
- Walked each of the eight negative cases against the policy text and confirmed
  the enforcing rule exists at the exact JSON path recorded in the fixture
  (scripted check, §Gate extra above).
- Verified no file outside `orchestration/tasks/TASK-0135-…/` was modified by
  this worker (`git status --short` clean apart from task-folder paths before
  each commit).

## Commit SHAs

- `50cd286f` — CLAIMED: STATUS.md claim, pushed.
- `f58cb814` — IMPLEMENTED: policy, fixtures, VALIDATION.md.
- REVIEW_REQUESTED commit — this commit (REPORT.md, evidence transcript, and
  STATUS flip); pushed to this worker branch only. Its exact SHA is visible as
  the worker branch head on origin at review time.

## Deviations

None from the SPEC. Notes within spec freedom:

- Added one extra self-check command beyond the five gates (fixture rule
  resolution); additive evidence, recorded in VALIDATION.md.
- Full gate-3 output (395 lines) is preserved in
  `evidence/gate3-rg-transcript.txt` rather than inlining all of it here;
  first/last lines and count are quoted above with exit code.

## Unresolved questions

None blocking. OWNER_PENDING items are recorded in the policy with smallest
future owner paths: nightly schedule/host choice, release-proof activation and
interpretation, long-term artifact storage, unsupported-platform execution.

## Risks

- The policy is advisory-by-contract until an evaluator adopts it; a future
  packet could wire it into review checklists or (owner-approved) automation.
- Structural contention classification relies on phase/error strings emitted by
  the TASK-0129 binder; if the tool's error text changes, the classification
  rule needs a one-line update (bound by `source_head.staleness_rules` anyway).

## Follow-ups

- Owner decision on nightly Option A vs Option B (policy §contexts.nightly).
- Optional successor packet: a tiny evaluator script that consumes
  `verdict.evidence_bundle_schema` and the negative fixtures to grade submitted
  bundles mechanically (task-folder work; would need its own READY spec).

## Stop-condition review

No stop condition hit: no CI mutation, no paid-runner selection, no weakened
failure, no credential contact, no writes outside the task folder, port 6500
untouched.
