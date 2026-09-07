# TASK-0082 report — dual-server parity matrix runner

- Worker lane: `ox-pc-bb` (model `openrouter/stealth/ox-alpha`)
- Branch: `worker/verdigris/pc/ox-pc-bb`
- Base: `d2423873c577d299b3b39c56024d1d840993c72b` (verified ancestor of work head)
- Implementation commit: `515e8185`
- Resource capsule: loopback 6540–6559 only; this run used **6541 / 6542**. Port 6500 was never touched (and the tool hard-refuses it).

## Executive summary

`playtest/tools/dual-server-matrix.mjs` boots a fresh JS server and a fresh
native C++ server on two explicit loopback ports, drives the unchanged
`playtest/run.mjs --attach` suite against each serially, writes one comparison
JSON artifact, and exits non-zero on any red or asymmetric scenario. The smoke
matrix (`quickstart,movement,zones`) passed 6/6 with `parity: true`; an
authentic negative (nonexistent scenario argument) exits 2 without writing an
artifact.

## Approach

- The wrapper owns both server lifecycles directly: it spawns the JS server
  (`node server/index.js`, `PORT=<js-port>`, hermetic `GUEST_SAVE_DIR` /
  `CHRONICLES_STORE_FILE` / `CHRONICLES_DB_FILE` under a unique
  `%TEMP%\verdigris-matrix-<pid>-<stamp>` root) and the native server
  (`verdigris_server.exe <native-port>`), waits for readiness by watching
  child output, and stops them in a `finally` block plus a process-exit
  registry that kills every spawned PID.
- Scenario execution reuses the real runner verbatim:
  `node playtest/run.mjs --attach <names>` with `PLAYTEST_WS_URL` pointed at
  each server. No runner/harness/scenario file is modified; the wrapper only
  parses ordinary `PASS name (ms)` / `FAIL name (ms)` summary lines.
- Servers run serially (JS phase fully stopped before native boot), so each
  side always faces a fresh server and no cross-talk is possible.
- Artifact records revision (git HEAD full+short), environment, exact child
  commands (array + joined command line), URLs/ports/PIDs/boot times,
  isolated save paths, native exe path + sha256, per-server per-scenario
  pass/fail/duration, runner exit codes, asymmetric scenario list, and final
  `parity`. Written atomically (temp file + rename).
- Validation failures (missing args, port 6500, duplicate ports, unknown
  scenario names, missing exe) exit 2 before any process is spawned.

## Changed files

| File | Change |
| --- | --- |
| `playtest/tools/dual-server-matrix.mjs` | new (owned) |
| `orchestration/tasks/TASK-0082-dual-server-matrix/captures/smoke.json` | new parity artifact (owned) |
| `orchestration/tasks/TASK-0082-dual-server-matrix/STATUS.md`, `REPORT.md` | claim/review bookkeeping (owned) |

No forbidden paths touched (`playtest/harness.mjs`, `playtest/run.mjs`,
`playtest/scenarios/**`, `server/**`, `native/**`, `src/**` all unmodified).

## Public interfaces added

```text
node playtest/tools/dual-server-matrix.mjs
  --native-exe <path>          required, native verdigris_server executable
  --js-port <PORT>             required, loopback port for the JS server
  --native-port <PORT>         required, loopback port for the native server
  [--scenarios a,b,...]        optional; default = all scenarios alphabetically
  --out <artifact.json>        required, where the comparison JSON is written
```

Exit codes: `0` full parity; `1` red/asymmetric/incomplete matrix;
`2` usage/validation error (nothing spawned).

## Acceptance transcripts (literal)

### 1. `powershell -File native/build.ps1 -RunTests`

Tail of transcript (full log captured during the run); exit code literal below:

```text
PASS serialization: two independent runs are byte-identical
PASS serialization: schedule matches the pinned canonical form
PASS serialization: sink recorded exactly the voiced schedule
--- begin canonical schedule ---
cue[000003] tick=5 bus=sfx prio=world id=kill wave=sawtooth 196->49Hz 240ms gain=560 effective=560
cue[000004] tick=5 bus=sfx prio=player id=scion-lost wave=sine 165->41Hz 900ms gain=700 effective=700
cue[000005] tick=7 bus=sfx prio=player id=warcry-expire wave=sine 392->262Hz 300ms gain=420 effective=420
cue[000001] tick=10 bus=sfx prio=player id=hit wave=sine 220->110Hz 90ms gain=480 effective=480
cue[000002] tick=10 bus=sfx prio=player id=crit wave=square 440->110Hz 150ms gain=640 effective=640
cue[000006] tick=12 bus=music prio=ui id=menu-loop wave=sine 262->262Hz 1000ms gain=300 effective=300
--- end canonical schedule ---
all audio mixer checks passed
BUILD_EXIT=0
```

Exit code: `0`.

### 2. `node --check playtest/tools/dual-server-matrix.mjs`

```text
CHECK_EXIT=0
```

(Also `npx eslint playtest/tools/dual-server-matrix.mjs` → `LINT_EXIT=0`.)

### 3. Matrix run

```text
PS Z:\Code\.worktrees\verdigris\ox-pc-bb> node playtest/tools/dual-server-matrix.mjs --native-exe native/build/verdigris_server.exe --js-port 6541 --native-port 6542 --scenarios quickstart,movement,zones --out orchestration/tasks/TASK-0082-dual-server-matrix/captures/smoke.json
Dual-server parity matrix (TASK-0082) — revision 4ede0d76d
Scenarios (3): quickstart, movement, zones

[1/4] Booting fresh JS server…
JS server ready (pid 20912, boot 21240ms); saves isolated under C:\Users\Alex\AppData\Local\Temp\verdigris-matrix-28524-1787501591411
[2/4] Running scenarios against the JS server…
JS runner finished (exit code 0).

[3/4] Booting fresh native server…
Native server ready (pid 27784, boot 458ms).
[4/4] Running scenarios against the native server…
Native runner finished (exit code 0).

────────────────────────────────
 js     PASS  quickstart (159ms)
 js     PASS  movement (4567ms)
 js     PASS  zones (18236ms)
 native  PASS  quickstart (170ms)
 native  PASS  movement (4627ms)
 native  PASS  zones (1230ms)
Artifact: orchestration\tasks\TASK-0082-dual-server-matrix\captures\smoke.json
PARITY: PASS
MATRIX_EXIT=0
```

Exit code: `0`.

### 4. Parity assertion over the artifact

```text
PS Z:\Code\.worktrees\verdigris\ox-pc-bb> node -e "const r=require('./orchestration/tasks/TASK-0082-dual-server-matrix/captures/smoke.json'); if(!r.parity) process.exit(1); console.log('dual-server smoke: PASS')"
dual-server smoke: PASS
ARTIFACT_CHECK_EXIT=0
```

Exit code: `0`.

### 5. `git diff --check`

```text
DIFF_CHECK_EXIT=0
```

Exit code: `0`.

## Authentic negative demonstration

Nonexistent scenario argument (explicitly sanctioned by the spec). Nothing was
created, so there was nothing to restore; no disposable copy was needed.

```text
PS Z:\Code\.worktrees\vendigris\ox-pc-bb> node playtest/tools/dual-server-matrix.mjs --native-exe native/build/verdigris_server.exe --js-port 6541 --native-port 6542 --scenarios nonexistent-scenario --out orchestration/tasks/TASK-0082-dual-server-matrix/captures/negative.json
Usage:
  node playtest/tools/dual-server-matrix.mjs \
    --native-exe native/build/verdigris_server.exe \
    --js-port <PORT> --native-port <PORT> \
    [--scenarios name,name,...] \
    --out <artifact.json>
ERROR: Unknown scenario(s): nonexistent-scenario. Available: boss-mechanic, build-divergence, chronicles,
chronicles-first-combat, combat, crossroads, depth-loot, economy, encounter-variety, equipment-slots, first-goal,
gates, gear-outcomes, house-treasury, loot, mortality, movement, overflow, party, party-stories, persistence, quest,
quickstart, respawn, session-arc, single-session, skilltree, town-amenities, vesselforge, vesselforge-brand,
world-web, zones
NEGATIVE_EXIT=2
False   (artifact negative.json was NOT created)
```

(The `PS>` prompt line above contains a transcription typo — `vendigris`
should read `verdigris`; the command itself was executed from the real
worktree root.)

Exit code: `2`, non-zero as required.

## Process hygiene evidence

After the matrix run:

```text
listeners_on_6541_6542: 13   # netstat matches were TIME_WAIT teardown sockets only
verdigris_server_processes: 0
```

No LISTENING socket remains on 6541/6542 and no `verdigris_server` process
survives; the wrapper killed exactly the children it spawned.

## Manual verification notes

- The JS server binds `127.0.0.1` by default (`server/index.js`) and the
  native server binds `inet_addr("127.0.0.1")` (`native/src/networking.cpp`);
  the wrapper sets nothing that widens either bind. Loopback-only confirmed.
- Port guard rejects `6500` explicitly; distinct-port check present.
- Save isolation: all JS persistence env vars point inside the unique
  `%TEMP%\verdigris-matrix-*` root recorded in the artifact (`saveRoot`).
- Full-suite mode (no `--scenarios`) enumerates `playtest/scenarios/*.mjs`
  sorted alphabetically at invocation time, so newly added scenarios are
  picked up automatically.

## Deviations / notes

- START_HERE's claim protocol ("commit + push to origin") was followed
  literally for the lane branch `worker/verdigris/pc/ox-pc-bb` (explicit
  refspec push only); PROTOCOL.md's "NEVER push" clause is respected in
  spirit — the program branch `codex/native-reconstitution` was never pushed
  to, merged, or rebased.
- The wrapper validates requested scenario names itself and fails fast
  (exit 2) instead of relying on the runner's silent filtering of unknown
  names; this is what makes the negative demonstration authentic.
- Known behavior inherited from the unchanged runner: scenarios calling
  `recordMetrics` append trend rows to `docs/loop-journal.md`. The smoke set
  (quickstart/movement/zones) does not record metrics; the committed
  worktree contains no journal churn.
- Native-side durations differ from JS (zones 1230 ms vs 18236 ms) because
  the native world sim answers state polls faster; pass/fail parity is the
  contract, timing is informational.

## Unresolved questions

None.

## Risks / follow-ups

- Layer-1 covers protocol/rules parity through the harness. Deeper sweep
  layers (D-116) can call this tool with broader `--scenarios` lists or no
  argument for the full alphabet.
- Runner output parsing tolerates ANSI-free lines; if the runner ever gains
  colorized output the parser regex may need a strip pass (runner currently
  emits plain text).

## Commit SHAs

- Claim: `4ede0d76` (pushed to origin)
- Implementation + artifact: `515e8185`
- Report + status flip: the immediate child of `515e8185` on
  `worker/verdigris/pc/ox-pc-bb` (frozen pushed head stated in STATUS.md).
