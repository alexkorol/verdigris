# STATUS — TASK-0097-persistence-durability-audit

state: REVIEW_REQUESTED
lane: ox-pc-bg
model: openrouter/stealth/ox-alpha
base_sha: d2423873c577d299b3b39c56024d1d840993c72b
evidence_commit: a73c5e5f23c7f28dd3a5ac3c06216d85fc19d15b (FINDINGS.md + captures/persistence-contract.json)
frozen_head_sha: this commit is the final commit on the branch; HEAD at push = review request. Evidence pin above. Branch untouched after push.
claim_commit: d298f2d7 (claim(TASK-0097-persistence-durability-audit) [ox-pc-bg])

## Deliverables

- `FINDINGS.md` — full audit report: base/head, field map, save triggers,
  seams, atomicity, stale-data compatibility, reconnect semantics, failure
  modes, negative control, fault matrix pointer, red risks, locking tests.
- `captures/persistence-contract.json` — machine-readable contract (JSON.parse PASS).

## Preflight (at claim)

- `git status --short`: clean; `git rev-parse HEAD` == base d2423873c577d299b3b39c56024d1d840993c72b
- Branch worker/verdigris/pc/ox-pc-bg had no upstream; pushed with `-u origin`.

## Build/test evidence (before acceptance)

Command (literal):

```
powershell -NoProfile -ExecutionPolicy Bypass -File native/build.ps1 -RunTests
```

Exit code: 0. Key output lines:

```
native legacy denylist: PASS
verdigris core tests: PASS
verdigris networking tests: PASS
camera2d tests: PASS
PASS reconnect: Retrying then Ready after server restart
PASS reconnect: same guest identity re-logged in
PASS reconnect: login snapshot is authoritative
PASS replaced: ConnectionLost from player:session-replaced
session tests passed
```

Persistence-specific locks observed green:
`test_persistence_round_trip_and_unknown_fields`,
`test_persistence_d109_mid_instance_and_rng_continuation`,
`test_persistence_recovery_pools`,
`test_persistence_surfaced_recovery_becomes_pending`,
`test_persistence_file_adapter` (all in native/tests/core_tests.cpp:729-866),
plus `test_d106_all_carried_value_is_recoverable` and
`test_d106_recovery_is_ordered_and_deterministic` (core_tests.cpp:1117-1200).

## Acceptance commands (literal) — all exit 0

### 1.

```
rg -n "persist|save|load|profile|serialize|version|reconnect|relic|House|Scion" native/src native/include native/tests
```

Exit code: 0. Output: 488 matching lines across the three trees.
Key lines:

```
native/include\verdigris\persistence.hpp:6:#include "../../persistence/adapter.hpp"
native/include\verdigris\networking.hpp:198:  // chart; cleared wardens persist for the session (dead stays dead).
```

(Confirms the persistence surface audited in FINDINGS.md §1-§2.)

### 2.

```
node -e "JSON.parse(require('fs').readFileSync('orchestration/tasks/TASK-0097-persistence-durability-audit/captures/persistence-contract.json','utf8')); console.log('persistence contract: PASS')"
```

Exit code: 0. Key output line:

```
persistence contract: PASS
```

### 3.

```
git diff --check
```

Exit code: 0 (no output; no whitespace errors).

### 4.

```
git diff --name-only
```

Exit code: 0 (no tracked modifications). `git status --short` before the
evidence commit showed only untracked task-evidence files:

```
?? orchestration/tasks/TASK-0097-persistence-durability-audit/FINDINGS.md
?? orchestration/tasks/TASK-0097-persistence-durability-audit/captures/
```

Expected "only task evidence changes": satisfied. No test weakened or edited;
no file outside owned_paths touched; no real saves opened; port 6500 never used.

## Negative control (SPEC requirement)

Documented in FINDINGS.md §9: external mangling of a House save (cloud-sync /
AV truncation → must fail closed; same-length corruption → silently diverges,
no digest in v1), plus the untested `schemaVersion=2` rejection branch at
native/src/core.cpp:1230-1233. Smallest locking tests L1-L6 specified in
captures/persistence-contract.json → negative_control / fault matrix F3-F5.

## Stop conditions

None triggered: no auth failure, base matches, no owned-path conflict, every
gate passed honestly on first run.
