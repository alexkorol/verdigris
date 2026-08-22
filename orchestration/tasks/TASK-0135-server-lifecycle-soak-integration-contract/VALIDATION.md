# VALIDATION — TASK-0135 soak integration policy

Validation record for `soak-integration-policy.json` and
`fixtures/negative-cases.json`. Literal acceptance-gate transcripts and exit
codes live in `REPORT.md`; this document records what was validated, how, and
the conformance mapping.

## 1. Machine validation performed

### 1.1 Policy key presence and forbidden-port binding (SPEC gate 1)

Command (from repository root):

```powershell
node -e "const fs=require('fs');const j=JSON.parse(fs.readFileSync('orchestration/tasks/TASK-0135-server-lifecycle-soak-integration-contract/soak-integration-policy.json','utf8'));for(const k of ['schema_version','contexts','source_head','platform','port_capsule','repetition','timeout','artifacts','retry','quarantine','escalation','verdict'])if(!(k in j))throw Error('missing '+k);if(j.port_capsule.forbidden_ports.indexOf(6500)<0)throw Error('6500 not forbidden');console.log('soak integration policy: PASS')"
```

Observed: `soak integration policy: PASS`, exit code 0.

Result: all twelve required top-level keys present; `port_capsule.forbidden_ports`
contains 6500; `port_capsule.allowed_range` is `[6680, 6699]` (this lane's
capsule); `host` is loopback `127.0.0.1`.

### 1.2 Negative-case coverage (SPEC gate 2)

Command (from repository root):

```powershell
node -e "const fs=require('fs');const j=JSON.parse(fs.readFileSync('orchestration/tasks/TASK-0135-server-lifecycle-soak-integration-contract/fixtures/negative-cases.json','utf8'));for(const k of ['STALE_SOURCE_HEAD','MISSING_PLATFORM_EVIDENCE','PORT_CAPSULE_COLLISION','FORBIDDEN_PORT_6500','TIMEOUT','NONZERO_SOAK','MISSING_ARTIFACT','RETRY_MASKED_FAILURE'])if(!j.cases.some(x=>x.expected_error===k))throw Error('missing '+k);console.log('soak policy negatives: PASS')"
```

Observed: `soak policy negatives: PASS`, exit code 0.

Result: all eight required negative error codes are covered by at least one
fixture case.

### 1.3 Fixture-to-policy rule resolution (extra self-check)

Every fixture case carries a `policy_rule` JSON path into the policy. This
check fails if any path does not resolve, so the fixtures cannot drift away
from the policy text:

```powershell
node -e "const fs=require('fs');const p=JSON.parse(fs.readFileSync('orchestration/tasks/TASK-0135-server-lifecycle-soak-integration-contract/soak-integration-policy.json','utf8'));const f=JSON.parse(fs.readFileSync('orchestration/tasks/TASK-0135-server-lifecycle-soak-integration-contract/fixtures/negative-cases.json','utf8'));const get=(o,k)=>k.split('.').reduce((a,c)=>a&&a[c],o);for(const c of f.cases){if(!get(p,c.policy_rule))throw Error('unresolved policy_rule '+c.policy_rule)}console.log('negative-case rule resolution: PASS ('+f.cases.length+' cases)')"
```

Observed: `negative-case rule resolution: PASS (8 cases)`, exit code 0.

## 2. Negative-case to enforcement mapping

| Case | expected_error | Enforcing policy rule | Expected conclusion |
|---|---|---|---|
| NEG-001 | STALE_SOURCE_HEAD | `source_head.staleness_rules` | FAIL |
| NEG-002 | MISSING_PLATFORM_EVIDENCE | `platform.rule` | FAIL |
| NEG-003 | PORT_CAPSULE_COLLISION | `quarantine.persistent_collision_outcome` | BLOCKED_ENVIRONMENTAL |
| NEG-004 | FORBIDDEN_PORT_6500 | `port_capsule.on_forbidden_port_use` | FAIL |
| NEG-005 | TIMEOUT | `timeout.on_breach` | FAIL |
| NEG-006 | NONZERO_SOAK | `retry.non_masking_invariants` | FAIL |
| NEG-007 | MISSING_ARTIFACT | `artifacts.missing_rule` | FAIL |
| NEG-008 | RETRY_MASKED_FAILURE | `retry.non_masking_invariants` | FAIL |

Note the deliberate asymmetry: deterministic failures (NEG-006) fix the head's
verdict at FAIL with no same-head retry path, while environmental contention
(NEG-003) routes through quarantine and can end only in a clean full re-run,
PASS, or BLOCKED_ENVIRONMENTAL — never in a green produced by the collided
attempt itself.

## 3. Non-masking property walkthrough

The policy satisfies the SPEC's core invariant ("never convert a failed or
missing soak into green by retry") through four mutually reinforcing rules:

1. `verdict.green_requires_all` demands every cited artifact be complete,
   parseable, and internally green (`passed=true`, `failures=[]`,
   `cyclesCompleted==cyclesRequested`, `burstPassed=true`) on the exact tip.
2. `retry.allowed_triggers` admits only environmental port contention;
   `forbidden_triggers` explicitly excludes deterministic failure, unproven
   timeouts, missing artifacts, and any failed/incomplete run.
3. `retry.non_masking_invariants` requires all attempts to stay on disk and be
   disclosed; a same-head deterministic failure permanently fixes that head's
   verdict at FAIL.
4. `verdict.never_green_when` restates each failure mode as an explicit
   anti-green condition, so no aggregation or presentation rule can override
   it.

The `supreme_non_masking_invariant` at the policy root states the property in
one sentence for any future evaluator.

## 4. Boundary conformance

- **CI untouched:** the policy authorizes no CI change (`content_neutrality.ci_change_authorized: false`); pre-merge enforcement is a review-evidence contract only.
- **Port 6500:** listed in `forbidden_ports`; any selection, probe, or bind of it is a P0 protocol violation with immediate FAIL, halt, report, and no retry.
- **No release declaration:** release_proof context defines evidence only; activation and interpretation are `OWNER_PENDING`.
- **OWNER_PENDING items** carry the smallest future owner path (local Task Scheduler option before paid hosted runners).
- **Content neutrality:** the policy binds execution and evidence only; it names no gameplay content and modifies no simulation behavior.

## 5. Source-of-truth bindings used

Facts bound into the policy were read from the accepted TASK-0129 implementation at this worktree:

- Tool constants (100 cycles, 8 burst clients, 5000 ms login, 15000 ms burst, 15-minute watchdog): `native/tools/server_lifecycle_soak.cpp`
- Gate invocation and capture location: `native/build.ps1` `-RunServerLifecycleSoak` block
- Exit-code semantics 0/1/2/3: `server_lifecycle_soak.cpp` `main()`
- Staleness rules: `orchestration/ACCEPTANCE.md` universal invalidation section
- Lane capsule 6680–6699: `orchestration/REENTRY-OX-ALPHA-PC.md`

No native, server, src, playtest, or `.github` file was modified while
producing this packet.
