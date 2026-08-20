# TASK-0065 RESULTS — N7 entity-density benchmark

Host: Windows 10, MSVC 2019 x64. Cadence: 1000 `Melee` dispatches after
`Command::enter("route:tin:1:0")` + `Simulation::spawn_monster` to N.
Real-time budget is 20 ticks/s (`kSimulationTickMs = 50`).

Opt-in (not in default gates):

```
powershell -File native/build.ps1 -RunDensityBench
```

## Three-run table (2026-08-20)

| N | run | ticks/s | p99 tick (ms) | monsters start/end |
|---|---|---|---|---|
| 50 | 1 | 1,178,689 | 0.0011 | 50 / 50 |
| 50 | 2 | 1,645,549 | 0.0009 | 50 / 50 |
| 50 | 3 | 1,815,541 | 0.0006 | 50 / 50 |
| 200 | 1 | 441,365 | 0.0045 | 200 / 200 |
| 200 | 2 | 540,774 | 0.0030 | 200 / 200 |
| 200 | 3 | 736,052 | 0.0018 | 200 / 200 |
| 500 | 1 | 310,791 | 0.0056 | 500 / 500 |
| 500 | 2 | 343,619 | 0.0039 | 500 / 500 |
| 500 | 3 | 231,235 | 0.0052 | 500 / 500 |
| 1000 | 1 | 175,753 | 0.0102 | 1000 / 1000 |
| 1000 | 2 | 182,126 | 0.0075 | 1000 / 1000 |
| 1000 | 3 | 184,043 | 0.0054 | 1000 / 1000 |

Median ticks/s: **N=50 → 1.65e6**, **N=200 → 5.41e5**, **N=500 → 3.11e5**,
**N=1000 → 1.82e5**. N=1000 median p99 is **0.0075 ms** vs the 50 ms tick
budget (~6,600× headroom). JSON: `captures/density-n*-run*.json`.

Monsters stay alive across 1000 ticks (Melee cooldown + one-target nearest
hit); the measured cost is spawn_monster density through `advance_tick` /
`enemy_turn` / scripted `dispatch(Melee)` every tick.

## JS comparison (not implemented)

The JS server has **no** `spawn_monster` / `dev:spawn` seam (`grep` of
`server/` is empty). Matching N in one instance would need a new server
handler. SPEC: implement the JS side only if it needs no `server/`
changes — so this is a note, not a harness script.

Apples-to-apples shape, if a later task adds a spawn verb:

1. Attach (`PLAYTEST_WS_URL`) to one instance.
2. Spawn N trash monsters at melee contact.
3. `player:skill:trigger` in a 1000-iteration loop, timestamp each reply.
4. Write the same JSON fields (`n`, `run`, `ticks`, `ticks_per_sec`,
   `p99_tick_ms`).
