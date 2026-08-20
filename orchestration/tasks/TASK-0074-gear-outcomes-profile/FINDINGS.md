# TASK-0074 gear-outcomes timing profile

Coordinator: `luna-mac`
Worker branch: `codex/TASK-0074-gear-outcomes-profile-luna-mac`
Base commit: `f6d942d597bbb83c9a68e332e767f84980a09331`

## Result

Ten serialized full-suite runs completed successfully on the Luna Mac
capsule, using ports 7001–7010. Every run reported `32/32 scenarios passed`.
The requested `timing.jsonl` contains 320 records: 32 scenarios × 10 runs,
with zero failed records and zero diagnosis records. No code or playtest files
were changed. The runner did append its normal `session-arc` critic telemetry
to `docs/loop-journal.md`; that unrelated, out-of-scope modification is left
uncommitted and is not part of this task branch's task-evidence commit.

Port 7000 was excluded because an unrelated `ControlCe` process already held
it. It was not killed or reused. The ten-run sample therefore uses the next
ten free ports in the assigned 7000–7019 capsule.

## Gear-outcomes distribution

The percentile calculation uses linear interpolation between sorted samples.
Wall times are milliseconds.

| Metric | Value |
|---|---:|
| Minimum | 30,779 ms |
| Median | 33,731.5 ms |
| P90 | 34,283.9 ms |
| Maximum | 34,688 ms |

| Run | Port | gear-outcomes | preceding `gates` | following `house-treasury` |
|---:|---:|---:|---:|---:|
| 1 | 7001 | 34,045 ms | 1,049 ms | 620 ms |
| 2 | 7002 | 33,743 ms | 1,037 ms | 625 ms |
| 3 | 7003 | 33,802 ms | 1,033 ms | 625 ms |
| 4 | 7004 | 34,688 ms | 1,045 ms | 625 ms |
| 5 | 7005 | 34,239 ms | 1,045 ms | 624 ms |
| 6 | 7006 | 33,301 ms | 1,040 ms | 630 ms |
| 7 | 7007 | 30,779 ms | 1,038 ms | 628 ms |
| 8 | 7008 | 33,625 ms | 1,043 ms | 623 ms |
| 9 | 7009 | 33,720 ms | 1,041 ms | 624 ms |
| 10 | 7010 | 32,457 ms | 1,041 ms | 623 ms |

The neighboring scenarios remain short and comparatively stable. Pearson
correlation across the ten aligned runs is `0.381901` for gear time versus
`gates`, `-0.409383` versus `house-treasury`, and `0.161887` versus their
combined duration. With ten samples and no shared failure, this does not
support a specific neighboring-scenario cause. The variance is concentrated
inside `gear-outcomes` itself.

## DIAG and wait labels

No full-suite run failed, so the runner emitted no `DIAG` lines and there are
no dominant wait labels from failure diagnosis to report. The timing log has
zero records with a `diagnosis` field.

The likely waits are nevertheless visible in the scenario source:

1. `playtest/scenarios/gear-outcomes.mjs:7-63` performs multiple serialized
   `kill of comparison monster <uuid>` waits, each with a 30-second bound,
   while a 300 ms pulse repeatedly requests state, heals, teleports, and
   attacks.
2. `playtest/scenarios/gear-outcomes.mjs:83-120` adds bounded drop, pickup,
   and equip waits for the item-level 5 and item-level 65 vessels.
3. `playtest/scenarios/gear-outcomes.mjs:140-185` intentionally performs
   unarmed, low-item-level, deep low-item-level, and high-item-level trials;
   it may perform one bounded deep-trial repeat when the hit-count threshold
   is near its boundary.

### Ranked root-cause hypotheses

1. **Intrinsic scenario work (high confidence).** The scenario deliberately
   runs several authoritative combat trials and item lifecycle waits. Its
   stable 30.8–34.7 s distribution, with all neighboring scenarios under
   1.1 s, is consistent with the scenario's own serialized workload.
2. **Scheduler and development-control cadence (medium confidence).** The
   300 ms pulse and repeated state/teleport/attack requests can add scheduler
   gaps to the combat stopwatch. The source comments explicitly describe
   rate-bucket and full-suite CPU-starvation handling, but no run crossed a
   timeout or emitted a diagnosis proving this as the primary cause.
3. **Neighbor-induced contention (low confidence).** `gates` precedes and
   `house-treasury` follows the scenario, but their aligned durations are
   short and the correlation with their combined duration is only `0.161887`.
   The sample does not identify a specific preceding scenario to fix.

## Literal command transcripts

### Port preflight

```text
$ PLAYTEST_PORT=7000 PLAYTEST_TIMING_LOG=1 PLAYTEST_TIMING_LOG_PATH=/Users/alexkorol/Code/verdigris/orchestration/tasks/TASK-0074-gear-outcomes-profile/timing.jsonl npm run playtest
> verdigris@0.2.0 playtest
> node playtest/run.mjs
Booting playtest server on :7000…
Error: No playable server at ws://localhost:7000
exit code: 1

$ lsof -nP -iTCP:7000 -sTCP:LISTEN
COMMAND   PID      USER   FD   TYPE             DEVICE SIZE/OFF NODE NAME
ControlCe 650 alexkorol    9u  IPv4 0x978db2e53eda7a9d      0t0  TCP *:7000 (LISTEN)
ControlCe 650 alexkorol   10u  IPv6 0x978db2e53eda7a9d      0t0  TCP *:7000 (LISTEN)
```

### Full-suite runs

Each block below is the literal command and final runner output for one
serialized run. The runner's complete per-scenario timing records are in
`timing.jsonl`.

```text
$ PLAYTEST_PORT=7001 PLAYTEST_TIMING_LOG=1 PLAYTEST_TIMING_LOG_PATH=/Users/alexkorol/Code/verdigris/orchestration/tasks/TASK-0074-gear-outcomes-profile/timing.jsonl npm run playtest
PASS gear-outcomes (34045ms)
32/32 scenarios passed
Timing diagnostics: {"loadMode":false,"p99EventLoopLagMs":22.593535,"maxEventLoopLagMs":65.535999}
exit code: 0

$ PLAYTEST_PORT=7002 PLAYTEST_TIMING_LOG=1 PLAYTEST_TIMING_LOG_PATH=/Users/alexkorol/Code/verdigris/orchestration/tasks/TASK-0074-gear-outcomes-profile/timing.jsonl npm run playtest
PASS gear-outcomes (33743ms)
32/32 scenarios passed
Timing diagnostics: {"loadMode":false,"p99EventLoopLagMs":22.151167,"maxEventLoopLagMs":43.220991}
exit code: 0

$ PLAYTEST_PORT=7003 PLAYTEST_TIMING_LOG=1 PLAYTEST_TIMING_LOG_PATH=/Users/alexkorol/Code/verdigris/orchestration/tasks/TASK-0074-gear-outcomes-profile/timing.jsonl npm run playtest
PASS gear-outcomes (33802ms)
32/32 scenarios passed
Timing diagnostics: {"loadMode":false,"p99EventLoopLagMs":22.233087,"maxEventLoopLagMs":45.809663}
exit code: 0

$ PLAYTEST_PORT=7004 PLAYTEST_TIMING_LOG=1 PLAYTEST_TIMING_LOG_PATH=/Users/alexkorol/Code/verdigris/orchestration/tasks/TASK-0074-gear-outcomes-profile/timing.jsonl npm run playtest
PASS gear-outcomes (34688ms)
32/32 scenarios passed
Timing diagnostics: {"loadMode":false,"p99EventLoopLagMs":22.593535,"maxEventLoopLagMs":46.563327}
exit code: 0

$ PLAYTEST_PORT=7005 PLAYTEST_TIMING_LOG=1 PLAYTEST_TIMING_LOG_PATH=/Users/alexkorol/Code/verdigris/orchestration/tasks/TASK-0074-gear-outcomes-profile/timing.jsonl npm run playtest
PASS gear-outcomes (34239ms)
32/32 scenarios passed
Timing diagnostics: {"loadMode":false,"p99EventLoopLagMs":22.560767,"maxEventLoopLagMs":47.710207}
exit code: 0

$ PLAYTEST_PORT=7006 PLAYTEST_TIMING_LOG=1 PLAYTEST_TIMING_LOG_PATH=/Users/alexkorol/Code/verdigris/orchestration/tasks/TASK-0074-gear-outcomes-profile/timing.jsonl npm run playtest
PASS gear-outcomes (33301ms)
32/32 scenarios passed
Timing diagnostics: {"loadMode":false,"p99EventLoopLagMs":22.593535,"maxEventLoopLagMs":46.137343}
exit code: 0

$ PLAYTEST_PORT=7007 PLAYTEST_TIMING_LOG=1 PLAYTEST_TIMING_LOG_PATH=/Users/alexkorol/Code/verdigris/orchestration/tasks/TASK-0074-gear-outcomes-profile/timing.jsonl npm run playtest
PASS gear-outcomes (30779ms)
32/32 scenarios passed
Timing diagnostics: {"loadMode":false,"p99EventLoopLagMs":22.544383,"maxEventLoopLagMs":46.596095}
exit code: 0

$ PLAYTEST_PORT=7008 PLAYTEST_TIMING_LOG=1 PLAYTEST_TIMING_LOG_PATH=/Users/alexkorol/Code/verdigris/orchestration/tasks/TASK-0074-gear-outcomes-profile/timing.jsonl npm run playtest
PASS gear-outcomes (33625ms)
32/32 scenarios passed
Timing diagnostics: {"loadMode":false,"p99EventLoopLagMs":22.560767,"maxEventLoopLagMs":47.710207}
exit code: 0

$ PLAYTEST_PORT=7009 PLAYTEST_TIMING_LOG=1 PLAYTEST_TIMING_LOG_PATH=/Users/alexkorol/Code/verdigris/orchestration/tasks/TASK-0074-gear-outcomes-profile/timing.jsonl npm run playtest
PASS gear-outcomes (33720ms)
32/32 scenarios passed
Timing diagnostics: {"loadMode":false,"p99EventLoopLagMs":22.544383,"maxEventLoopLagMs":57.868287}
exit code: 0

$ PLAYTEST_PORT=7010 PLAYTEST_TIMING_LOG=1 PLAYTEST_TIMING_LOG_PATH=/Users/alexkorol/Code/verdigris/orchestration/tasks/TASK-0074-gear-outcomes-profile/timing.jsonl npm run playtest
PASS gear-outcomes (32457ms)
32/32 scenarios passed
Timing diagnostics: {"loadMode":false,"p99EventLoopLagMs":22.577151,"maxEventLoopLagMs":46.268415}
exit code: 0
```

### Timing-analysis transcript

```text
$ node --input-type=module - <<'NODE' ... NODE
records=320
ports=7001,7002,7003,7004,7005,7006,7007,7008,7009,7010
scenario_counts={"gear-outcomes":10,"gates":10,"house-treasury":10}
failed_records=0
gear_stats={"min":30779,"median":33731.5,"p90":34283.9,"max":34688}
run,port,gear_ms,gates_ms,house_treasury_ms
1,7001,34045,1049,620
2,7002,33743,1037,625
3,7003,33802,1033,625
4,7004,34688,1045,625
5,7005,34239,1045,624
6,7006,33301,1040,630
7,7007,30779,1038,628
8,7008,33625,1043,623
9,7009,33720,1041,624
10,7010,32457,1041,623
diag_records=0
exit code: 0

$ rg -n DIAG orchestration/tasks/TASK-0074-gear-outcomes-profile/timing.jsonl
exit code: 0
```
