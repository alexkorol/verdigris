# TASK-0097 — REPORT (REVIEW_REQUESTED)

- Lane `ox-pc-bc`, model `openrouter/stealth/ox-alpha`, worktree
  `Z:\Code\.worktrees\verdigris\ox-pc-bc`.
- SPEC base commit: `d2423873c577d299b3b39c56024d1d840993c72b`
  (`git merge-base --is-ancestor d2423873... HEAD` → true; branch was
  fast-forwarded from `f1180a29` to program tip `0bee7f1e` before work so the
  lane started on the fleet re-point that routes ox-pc-bc → TASK-0097).
- Claim commit: `c289156ae46979efe32ce53e4d5c928e69f4ce43`.
- Frozen review head: the pushed tip of `worker/verdigris/pc/ox-pc-bc`
  containing this report and the REVIEW_REQUESTED status flip (recorded in
  STATUS.md; validator should gate at that exact SHA).
- Deliverables: `FINDINGS.md` (full audit + fault matrix),
  `captures/persistence-contract.json` (machine-readable contract), this
  `REPORT.md`, flipped `STATUS.md`. Nothing outside
  `orchestration/tasks/TASK-0097-persistence-durability-audit/**` changed.
- Capsule compliance: read-only; no real save opened or mutated; no ports
  bound; port 6500 never touched.

## Acceptance commands — literal transcripts

All four SPEC acceptance commands were run literally in PowerShell at the
pre-commit tree (evidence files present as untracked task-folder files).

### 1. Source/test mapping grep

```text
PS> rg -n "persist|save|load|profile|serialize|version|reconnect|relic|House|Scion" native/src native/include native/tests
<627 matching lines across native/src, native/include, native/tests;
full capture retained by the session tooling. Representative rows:
native/include/verdigris/persistence.hpp:6:#include "../../persistence/adapter.hpp"
native/src/core.cpp:1226:std::vector<std::uint8_t> snapshot(const Simulation& simulation) {
native/src/core.cpp:1299:  if (required_number<std::uint64_t>(fields, "schemaVersion") != kSnapshotSchemaVersion) {
native/src/networking.cpp:2318:    put(fresh, "version", 3);
native/tests/core_tests.cpp:755:void test_persistence_round_trip_and_unknown_fields() {
native/tests/core_tests.cpp:2073:  test_persistence_round_trip_and_unknown_fields();
...
EXIT=0
```

Exit code: **0**. (The full raw output exceeded the console cap and was
preserved to disk by the harness; 627 matched lines counted.)

### 2. Persistence contract JSON parses

```text
PS> node -e "JSON.parse(require('fs').readFileSync('orchestration/tasks/TASK-0097-persistence-durability-audit/captures/persistence-contract.json','utf8')); console.log('persistence contract: PASS')"
persistence contract: PASS
EXIT=0
```

Exit code: **0**.

### 3. Whitespace hygiene

```text
PS> git diff --check
(no output)
EXIT_CHECK=0
```

Exit code: **0**, silent.

### 4. Changed-path scope

```text
PS> git diff --name-only
(no output)
EXIT_NAMES=0
```

Exit code: **0**; empty because every evidence file is a new untracked file.
Companion proof of touched paths:

```text
PS> git status --short
?? orchestration/tasks/TASK-0097-persistence-durability-audit/FINDINGS.md
?? orchestration/tasks/TASK-0097-persistence-durability-audit/REPORT.md
?? orchestration/tasks/TASK-0097-persistence-durability-audit/STATUS.md
?? orchestration/tasks/TASK-0097-persistence-durability-audit/captures/persistence-contract.json
```

Expected "only task evidence changes": confirmed — all four paths live under
the owned folder; zero forbidden paths appear anywhere in this lane's history
since the claim (`git diff --name-only c289156a~1..HEAD` after the final
commit will list exactly these task-folder files).

## Negative control delivered

Named in FINDINGS.md §8 and matrix F-03: stale-version snapshots
(`schemaVersion != 1`) are rejected by implemented code
(`native/src/core.cpp:1299-1301`) but **no current test locks that rejection**
— a realistic rollback/newer-build case with zero coverage. Partial-write
companions F-01 (truncated snapshot) and F-10 (concurrent writers through the
fixed `.tmp` path, `adapter.hpp:25`) are likewise uncovered today.

## Red risks and smallest locking tests

Summarized in FINDINGS.md §7 and §10 (R1/R2 P0: no production save trigger +
no session-layer serialization seam; R3-R5 P1 adapter fsync gap, `.tmp`
collision vector, three inconsistent version policies). Successor entry point:
locking tests L1-L7, all runnable against disposable temp-dir profiles only.

## Stop point honored

No non-disposable save was opened or modified; no server was started; source
and test mapping is complete as of the frozen head.
