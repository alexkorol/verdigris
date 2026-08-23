# TASK-0082-dual-server-matrix — STATUS

- state: REVIEW_REQUESTED
- lane: ox-pc-bf
- model: openrouter/stealth/ox-alpha
- base SHA: d2423873c577d299b3b39c56024d1d840993c72b (matched at claim time; verified via `git rev-parse HEAD`)
- claimed at (UTC): 2026-08-23T09:05Z (approx; claim commit 9e92a4f65b57a1527342bc6c57e8c4a6aee22d03)
- branch: worker/verdigris/pc/ox-pc-bf
- ports capsule: acceptance run used 6541 (JS) / 6542 (native) per SPEC; never 6500
- frozen head: 34486f121461544b28310bf44739a1b05db3179e — the deliverable
  commit (tool + captures + evidence). A commit cannot contain its own SHA, so
  it is recorded here in this final review-request commit; the branch is not
  touched after this push. Verify content at that SHA.

## Deliverable

`playtest/tools/dual-server-matrix.mjs` — self-terminating wrapper that:

- spawns a fresh JS server (`node server/index.js`) on `--js-port` with
  loopback pinned (`VERDIGRIS_BIND_HOST=127.0.0.1`), hermetic temp save paths
  (`GUEST_SAVE_DIR`, `CHRONICLES_STORE_FILE`, `CHRONICLES_DB_FILE`,
  `PLAYER_SAVE_COOLDOWN_MS=999999999`), mirroring the runner's own isolation;
- spawns a fresh native server (`--native-exe <port>` argv, `VERDIGRIS_PORT`
  scrubbed from env) on `--native-port`;
- probes readiness with a core-module WebSocket upgrade handshake against
  `ws://127.0.0.1:<port>` (45 s deadline per server);
- runs the UNCHANGED `playtest/run.mjs --attach <names…>` serially against each
  server (no harness/scenario/assertion changes);
- writes one comparison JSON artifact recording revision, executable path +
  sha256 for both servers, URLs, exact child commands + env deltas,
  per-server scenario pass/fail/duration, asymmetries, and final `parity`;
- exits non-zero on any red or asymmetric scenario or boot failure (validation
  errors exit 2 before anything spawns); always kills only its own children
  (tracked Set + SIGINT/SIGTERM/SIGBREAK/uncaughtException handlers).
- With no `--scenarios`, runs all current scenarios alphabetically (mirrors the
  runner's discovery). Unknown scenario names are rejected up front.

## Environment note

Worktree had no node_modules and npm 12 blocked install scripts by default.
Ran `npm ci --no-audit --no-fund`, then `npm install-scripts approve
better-sqlite3` + `npm rebuild better-sqlite3` so the JS server can boot.
The `allowScripts` entry npm added to package.json was reverted before commit
(outside owned_paths); the built binding remains local to this worktree.

## Acceptance transcripts (literal)

Resume note: the implementing session died before committing. On resume
(2026-08-23), every acceptance gate below was re-run from scratch against the
uncommitted working tree and passed again; the transcripts below are from the
resume re-verification run.

### 1. `powershell -File native/build.ps1 -RunTests`

exit code: `0`

key output tail:

```
PASS reconnect: Retrying then Ready after server restart
...
PASS render-list: Monster op recorded from remote model
PASS render-list: Swing op recorded from AttackStarted
PASS render-list: Drop op recorded from kill loot
session tests passed
EXIT_CODE=0
```

Produces `native/build/verdigris_server.exe` (sha256 recorded in artifact).

### 2. `node --check playtest/tools/dual-server-matrix.mjs`

exit code: `0` (silent).

### 3. Matrix run

```
node playtest/tools/dual-server-matrix.mjs --native-exe native/build/verdigris_server.exe --js-port 6541 --native-port 6542 --scenarios quickstart,movement,zones --out orchestration/tasks/TASK-0082-dual-server-matrix/captures/smoke.json
```

exit code: `0`. Key output lines:

```
[dual-server-matrix] revision 9e92a4f65b57a1527342bc6c57e8c4a6aee22d03
[dual-server-matrix] scenarios: movement, quickstart, zones
[js] ready at ws://127.0.0.1:6541
[native] ready at ws://127.0.0.1:6542
[js]  PASS  movement (4584ms)
[js]  PASS  quickstart (159ms)
[js]  PASS  zones (18103ms)
[js] 3/3 scenarios passed
[native]  PASS  movement (4642ms)
[native]  PASS  quickstart (169ms)
[native]  PASS  zones (1211ms)
[native] 3/3 scenarios passed
 PASS  movement  js=4584ms native=4642ms
 PASS  quickstart  js=159ms native=169ms
 PASS  zones  js=18103ms native=1211ms
[dual-server-matrix] parity: PASS
[dual-server-matrix] artifact: orchestration\tasks\TASK-0082-dual-server-matrix\captures\smoke.json
MATRIX_EXIT_CODE=0
```

Durable evidence is the artifact JSON (`captures/smoke.json`): it contains
revision, both executable sha256 hashes, exact child commands/env deltas,
per-scenario results and durations for both servers, empty asymmetries, and
`parity: true`.

Post-run orphan check: `netstat -ano | grep -E "65(41|42)"` showed only
TIME_WAIT sockets, no LISTENERS — wrapper killed exactly its spawned children.

### 4. Artifact parity gate

```
node -e "const r=require('./orchestration/tasks/TASK-0082-dual-server-matrix/captures/smoke.json'); if(!r.parity) process.exit(1); console.log('dual-server smoke: PASS')"
```

exit code: `0`, output: `dual-server smoke: PASS`

### 5. `git diff --check`

exit code: `0` (no output).

## Authentic negative demonstration

Nonexistent scenario argument (no disposable copy needed — validation rejects
before any process spawn, nothing to restore):

```
node playtest/tools/dual-server-matrix.mjs --native-exe native/build/verdigris_server.exe --js-port 6541 --native-port 6542 --scenarios quickstart,no-such-scenario --out orchestration/tasks/TASK-0082-dual-server-matrix/captures/negative.json
ERROR: unknown scenario(s): no-such-scenario. Available: boss-mechanic, build-divergence, chronicles, chronicles-first-combat, combat, crossroads, depth-loot, economy, encounter-variety, equipment-slots, first-goal, gates, gear-outcomes, house-treasury, loot, mortality, movement, overflow, party, party-stories, persistence, quest, quickstart, respawn, session-arc, single-session, skilltree, town-amenities, vesselforge, vesselforge-brand, world-web, zones
NEGATIVE_EXIT_CODE=2
```

Non-zero exit captured literally; no artifact written; no processes spawned.

## Stop conditions reviewed

No harness/scenario/assertion changes (runner invoked unchanged in attach
mode); loopback only (`127.0.0.1` pinned/verified, URLs use `ws://127.0.0.1`);
port 6500 explicitly rejected by the tool; only self-spawned children killed.
No stop condition triggered.

## Files touched (owned paths only)

- `playtest/tools/dual-server-matrix.mjs` (new)
- `orchestration/tasks/TASK-0082-dual-server-matrix/STATUS.md` (this file)
- `orchestration/tasks/TASK-0082-dual-server-matrix/captures/smoke.json` (new)
