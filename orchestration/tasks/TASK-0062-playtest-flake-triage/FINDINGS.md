# TASK-0062 FINDINGS — playtest marginal-timeout flake

Three serialized full suites on `127.0.0.1:6580` with `PLAYTEST_TIMING_LOG=1`.
Flake **did not reproduce** (0 failures / 96 scenario rows). DIAG path proven
with a stub waitFor timeout (see REPORT); no live suite failure to attach a
last-5-envelope capture to.

## Suite totals

| Run | Wall (scenario sum) | Result | Suite p99 lag (end) |
|---|---|---|---|
| 1 | 168.5s | 32/32 | 43.9 ms |
| 2 | 165.6s | 32/32 | 32.2 ms |
| 3 | 186.5s | 32/32 | 32.3 ms |

Raw rows: `timing.jsonl` (96 lines).

## Slowest / most variable (n=3)

| Scenario | min ms | max ms | mean ms | spread ms |
|---|---|---|---|---|
| gear-outcomes | 31740 | 53419 | 41037 | **21679** |
| quest | 23560 | 25816 | 24649 | 2256 |
| session-arc | 12347 | 17962 | 14704 | 5615 |
| zones | 18087 | 18257 | 18150 | 170 |
| respawn | 12318 | 13228 | 12805 | 910 |
| loot | 1064 | 3920 | 2425 | 2856 |

## Ranking

1. **gear-outcomes** — longest wall and largest spread (~32–53s). First
   place to watch if a 31/32 lands without a DIAG line naming the scenario.
2. **session-arc** and **loot** — not the slowest, but 1.5–3.7× spread on a
   quiet machine. The 2026-08-17 sighting named loot; this capture did not
   time out.
3. **quest / zones / respawn** — consistently long, tight spread. Slow
   because the scenario walks a long authored path, not because the wait
   budget is grazing.

## If a 31/32 happens after this lands

The runner now prints `DIAG <name> wall=… waits=[…] lastEnvelopes=[…]`
on the failing scenario. That line is the missing identity from the
2026-08-20 02:26 sighting (counts only). Do not change scenario
timeouts from this file — architect ruling required.
