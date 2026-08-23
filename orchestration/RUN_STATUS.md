# Run status — PC Verdigris overnight product wave

## Fleet running — 4 lanes claimed/active; resume loop in place — 2026-08-23 16:15 UTC

- Active fleet (all VALID routes):
  - ox-pc-ba -> TASK-0108 (readable ranged combat, P0): claimed `d9396fb6`,
    running.
  - ox-pc-bb -> TASK-0082 (dual-server matrix): claimed `4ede0d76`, running.
  - ox-pc-bc -> TASK-0097 (persistence durability audit): claimed `c289156a`,
    running.
  - ox-pc-bd -> TASK-0100 (deterministic replay audit): resumed after a
    one-shot stop; claim drafting (STATUS staged), process running.
  - ox-pc-be -> TASK-0165: SHIPPED (independent ACCEPTED + integrated).
- Note: `opencode run --agent build` is one-shot (exits after a turn), so the
  fleet is maintained by RESUMING lanes ("You are resuming... continue where
  you left off") rather than one long-lived process. The lead resumes any lane
  that stops before flipping REVIEW_REQUESTED, and validates + ships each
  REVIEW_REQUESTED head at its exact frozen commit.
- Duplicate-writer watch: some lanes show two matching opencode processes
  (owner launcher + lead relaunch); watching for two writers touching the same
  worktree (INC-015 pattern) — no conflict observed yet.

## Fleet bootstrap corrections + re-pointed lanes claiming — 2026-08-23 16:05 UTC

- Bootstrap review against `Z:\Code\orchestration\FLEET_BOOTSTRAP.md`:
  - START_HERE launch contracts confirmed **gitignored** in the lane worktrees.
  - Stale misrouted origin branches (`worker/verdigris/pc/ox-pc-bb|bc|bd`,
    which pointed to invalid claims on integrated tasks 0148/0157/0158) were
    **deleted** so the re-pointed workers can push fresh, protocol-valid claims
    without a non-fast-forward conflict.
  - Re-pointed lanes are activating: **ox-pc-bb has claimed TASK-0082** (staged
    STATUS, dual-server matrix); ox-pc-bc (TASK-0097) and ox-pc-bd (TASK-0100)
    are starting.
- Open questions from the review (pre-existing, not introduced by the
  re-point): the lane worktrees carry no local `opencode.json` permissions
  config (ba also lacks one), and my relaunches emit to the session log rather
  than `Z:\Code\.fleet\logs\<lane>.jsonl`. Both are noted for the owner; neither
  blocks the running lanes (claims + worktree evidence remain authoritative).
- VALIDATION WATCH unchanged: gate each `REVIEW_REQUESTED` at its exact frozen
  head and ship ACCEPTED work (TASK-0165 pattern).

## Fleet re-pointed + running (4 valid lanes) — 2026-08-23 15:59 UTC

- Lead re-pointed the three P0-misrouted lanes to valid READY tasks and
  relaunched them (auth verified; `OPENROUTER_API_KEY` present, opencode
  `auth.json` has the openrouter provider):
  - `ox-pc-bb` -> **TASK-0082** (dual-server parity matrix; ports 6540-6559) —
    running (PID 11100).
  - `ox-pc-bc` -> **TASK-0097** (persistence durability audit; read-only) —
    running (PID 21044).
  - `ox-pc-bd` -> **TASK-0100** (deterministic replay audit; read-only) —
    running (PID 16340).
  - `ox-pc-ba` -> **TASK-0108** (readable ranged combat, P0) — still running
    (PID 9148).
  - `ox-pc-be` -> TASK-0165 already SHIPPED (independent ACCEPTED +
    integrated at 71b3dd0d).
- Each worktree was reset to the current program tip (`f1180a29`) and given a
  fresh `START_HERE_<lane>.md` packet for its new task. The old misrouted
  claims (bb->0148, bc->0157, bd->0158 on integrated tasks) are abandoned;
  the new workers will push valid claims for the re-pointed tasks.
- VALIDATION WATCH: as lanes flip `REVIEW_REQUESTED`, the independent
  validator will gate each at its exact frozen head and ship ACCEPTED work
  (TASK-0165 pattern).

## SHIPPED TASK-0165 + fleet status — 2026-08-23 15:55 UTC

- **TASK-0165 (input-focus model foundation, ox-pc-be) SHIPPED.** Independent
  validator (deepseek-v4-flash) re-ran the full SPEC gate at frozen head
  `b17d4610`: harness 847 checks PASS (exit 0), denylist PASS, `git diff
  --check` clean, scope = `native/client/input_focus.hpp` + task folder only.
  Verdict ACCEPTED (REVIEW.md); worker branch merged and STATUS -> INTEGRATED.
  Post-integration gate re-ran green at the merged program head `71b3dd0d`.
  This is the first overnight shipment.
- **Fleet lanes:** ox-pc-ba -> TASK-0108 (VALID, impl in progress, dirty
  worktree); ox-pc-be -> TASK-0165 (SHIPPED above); **ox-pc-bb -> TASK-0148,
  ox-pc-bc -> TASK-0157, ox-pc-bd -> TASK-0158 are P0 MISROUTED** — all three
  are INTEGRATED tasks (the stale routes flagged earlier were launched; bd
  even re-implemented TASK-0158 as a duplicate). Owner: stop/repoint bb, bc,
  bd; do not let them implement already-integrated work.
- VALIDATION WATCH: TASK-0108 (ox-pc-ba) is the remaining valid claim; I will
  validate at its frozen head when it flips REVIEW_REQUESTED.

## Fleet LIVE — 3 claims pushed; one P0 MISROUTE — 2026-08-22 22:53 PDT

- Owner fixed the launch auth and relaunched ~22:50 PDT. The fleet is now
  **live**: fresh OpenCode processes (10:50-10:51) and three pushed worker
  branches:
  - `worker/verdigris/pc/ox-pc-ba` -> **TASK-0108** claim `d9396fb6`
    (VALID: base 76368466 = SPEC base; lane ox-pc-ba, model ox-alpha).
  - `worker/verdigris/pc/ox-pc-be` -> **TASK-0165** claim `7c439a27`
    (VALID: base b949b3e4 = SPEC base).
  - `worker/verdigris/pc/ox-pc-bb` -> **TASK-0148** claim `cb2d3f61`
    (**P0 MISROUTED**: TASK-0148 is INTEGRATED / REVIEW ACCEPTED — the stale
    route I flagged earlier was launched anyway, likely because the SPEC
    frontmatter still says READY while the board/REVIEW say INTEGRATED).
    Owner: stop/repoint ox-pc-bb; do not let it implement an integrated task.
- bc/bd did not push claims (still auth-retry or launching).
- Board sentinel remains healthy (25 effective READY, 0 program-branch
  CLAIMED — claims live on worker branches per the worker-branch model).
- VALIDATION WATCH: I will independently validate TASK-0108 (ox-pc-ba) and
  TASK-0165 (ox-pc-be) at their frozen heads the moment each flips
  `REVIEW_REQUESTED`.

## Fleet launch attempt FAILED on provider auth — 2026-08-22 22:45 PDT

- The owner launched the five provisioned lanes (ox-pc-ba..be) at 22:35-22:44
  PDT. **No lane activated**: no STATUS claims pushed, worktrees still clean,
  launcher-exits.txt records `FAIL rc=1` for ba/be, and the session logs carry
  `{"error":{"name":"UnknownError","data":{"message":"Unexpected server
  error"}}}` for bb/be/ba. The launch probe itself returned HTTP **401
  "Missing Authentication header"** (`Z:\Code\.fleet\logs\probe.json`).
- ROOT CAUSE: OpenRouter authentication is not reaching the OpenCode CLI
  sessions — the `OPENROUTER_API_KEY` is missing/not exported for the launch
  environment. This is a centralized launch/auth/provider failure (the launch
  contract allows one restart for this category), NOT a worker or task defect.
- LEAD ACTION: lanes remain PROVISIONED_UNCLAIMED and are NOT capacity.
  Owner action required: fix the `OPENROUTER_API_KEY` export for the launcher
  environment, then relaunch (one allowed restart per OX_CLI_SUBFLEET.md).
  The board is otherwise healthy and the two valid routes (ba->0108,
  be->0165) are unchanged; bb/bc/bd remain stale-to-integrated.

## Lead appointment + TASK-0152 reconciled — 2026-08-23 05:00 PDT

- Owner appointed **deepseek-v4-flash as fleet lead AND independent validator**
  for the Verdigris orchestration: maintain the board/RUN_STATUS, watch for
  `REVIEW_REQUESTED` flips and validate each at its exact frozen head, and keep
  the ox-alpha fleet routing healthy. This replaces the retired PC supervisor
  role; workers still push only their own branches; the lead pushes only
  coordination commits (reviews, RUN_STATUS, releases).
- **TASK-0152 independent re-validation: ACCEPTED** (REVIEW.md revision 3,
  reviewer deepseek-v4-flash) at frozen worker head `d34097d9` in detached
  worktree `Z:\Code\.worktrees\verdigris\review-task0152-d34097d9`. All
  acceptance claims re-run independently: native gate GATE_EXIT=0
  (denylist/core/networking/camera2d/session/presentation-events), fresh
  MSVC `/W4` bench build COMPILE_EXIT=0/LINK_EXIT=0 zero warnings, validator
  matrix 6/6 positives EXIT=0 and 12/12 negatives EXIT=1 (incl. seven tamper
  controls), double-process determinism checksum
  `fnv1a64:9f2964a4df5ac069` identical across fresh processes and the
  committed capture, usage errors EXIT=2, owned-scope-only diff,
  `git diff --check` clean. The worker head is byte-identical to the
  integrated program commit `76837dec`.
- **TASK-0152 STATUS reconciled** `REVIEW_REQUESTED -> INTEGRATED` (it was
  stale: accepted at `d34097d9` and integrated at `76837dec`). Reconciliation
  recorded in the task STATUS.md and pushed as coordination commit `8c71fac7`.
- Board snapshot: program `codex/native-reconstitution` at `8c71fac7`
  (clean/pushed); protected `master` `2d3e92a5` (PR #58). No
  `REVIEW_REQUESTED` tasks. No active CLAIMED lanes (fleet idle awaiting the
  ox swarm; historical superseded TASK-0056 claim preserved, never resumed).
  Highest-priority READY product packet remains **TASK-0108** (base
  `76368466`, ports 7280-7299).
- Route-readiness sweep (INC-016): three READY packets (TASK-0082/0084/0085)
  pinned the garbage-collected base `1f82623d`; refreshed their SPEC
  `base_commit` to `d2423873` (PR #50 merge, verified ancestor — the sibling
  audit base). All 25 effective-READY bases now resolve and are ancestors of
  the current head. Board sentinel healthy (25 READY, 0 claimed, 0
  collisions, exit 0).
- Launch-readiness confirmed: no port-capsule collisions across the 25 READY
  packets (most are read-only/no-ports; TASK-0108 holds the only explicit
  loopback capsule 7280-7299, unique). Board is route-ready for the ox swarm.
- Fleet provisioning observed (2026-08-22 ~22:19 PDT): five provisioned
  worktree lanes `ox-pc-ba`..`ox-pc-be` with launch packets, branches
  `worker/verdigris/pc/ox-pc-*`, all clean, no claims pushed, no worker
  processes yet. Routes: ox-pc-ba -> TASK-0108 (VALID, P0 READY),
  ox-pc-be -> TASK-0165 (VALID, READY), but ox-pc-bb -> TASK-0148,
  ox-pc-bc -> TASK-0157, ox-pc-bd -> TASK-0158 are **STALE routes to
  INTEGRATED tasks** (REVIEW=ACCEPTED, STATUS=INTEGRATED; only the SPEC
  frontmatter still says READY). Owner: do not launch bb/bc/bd as-is; the
  provisioned worktrees are preserved untouched.
- Launch recommendation for the three stale lanes (all READY, bases valid,
  disjoint owned paths): re-point ox-pc-bb -> TASK-0097 (P0 persistence
  durability audit), ox-pc-bc -> TASK-0100 (P0 deterministic replay audit),
  ox-pc-bd -> TASK-0104 (P0 itemization/history audit); TASK-0082 (dual-server
  matrix, base refreshed in INC-016) is also route-ready. ba -> TASK-0108 and
  be -> TASK-0165 stand as provisioned.
- Maintenance observation: `INTEGRATION_LOG.md` is stale — it records only
  TASK-0028 (2026-08-16) while the board shows 120 integrated tasks through
  TASK-0164. The board sentinel is unaffected (it keys on STATUS/REVIEW
  verdicts, not the log), but the log should be backfilled by the
  coordinators or its convention re-confirmed by the owner. Dashboard healthy
  (HTTP 200 on 4737); master `2d3e92a5` unchanged.

## Ship for cloud/other harnesses — 2026-08-22 17:32 PDT

- Owner is leaving the PC Ox launcher and will continue on cloud/other
  harnesses. PR #58 merged: protected `master` `2d3e92a5`, program
  `bb454c3c`. Combined native G6 and PR Native/CI both passed.
- Claimable product packet: TASK-0108 READY, base `76368466`, ports 7280-7299.
- Shared `alexkorol/orchestration` `main` remains Mac-owned. Owner action:
  use the merged `master` / this program SHA as the cloud entrypoint.

## TASK-0161 accepted; readable combat successor READY — 2026-08-22 17:22 PDT

- Independent detached TASK-0161 gate at `9f004d2a` passed (`GATE-EXIT=0`) with
  clean tree and fail-closed capture-root negatives. Integrated at `76368466`.
  Combined program G6 `-RunTests -RunClientScenarios -CaptureRoot` on that
  implementation tip also exited 0; working tree stayed clean (no historical
  capture rewrite).
- TASK-0108 is READY from TASK-0101 W1 on base `76368466`. It owns the
  readable ranged seam but not `session_tests.cpp` (TASK-0162). Ports 7280-7299.
  Owner launches workers; this successor does not implement the packet.
- Board tables now list 0108 under effective READY and drop it from DRAFT
  successors. Owner action: none.

## TASK-0101 accepted; 0161 independent gate next — 2026-08-22 17:12 PDT

- Cursor successor acknowledgement is durable at `5c62c904`. TASK-0101
  revision 1 is independently ACCEPTED (`34ff3137`) and integrated
  (`bdecf037`) from frozen worker head `a742355d`. JSON, final-range diff,
  owned scope, combo absence, and the readable ranged contract all passed.
- TASK-0108 remains DRAFT with W1 selected. READY promotion waits on
  TASK-0161 releasing `native/client/main.cpp`.
- Exact next action: rerun TASK-0161's entire SPEC gate from clean detached
  `Z:\Code\.worktrees\verdigris\review-task0161-9f004d2a` at `9f004d2a`. Do
  not treat the interrupted Gate-B as green. Owner action: none.

## Cursor successor acknowledgement — 2026-08-22 17:05 PDT

- Owner launched a Cursor successor after retiring Codex Sol. This is the
  durable `SUCCESSOR_ACKNOWLEDGED` record required by
  `SUPERVISOR_SUCCESSION.md`. The successor does not implement worker tasks.
- Program: `Z:\Code\Games\delaford\delaford_game` on
  `codex/native-reconstitution`, clean local/remote
  `2e2dece09433bb991b927305eadc52e7f5155556`. Protected `master` `a28ac92f`.
  Broadcast: `Z:\Code\.worktrees\orchestration\pc-overnight-game-wave`
  `3fa4f54c`; Mac-owned `origin/main` `d068012a` untouched.
- Frozen reviews, not capacity: TASK-0161 `9f004d2a` in
  `Z:\Code\.worktrees\verdigris\ox-pc-ah`; TASK-0101 revision 1 `a742355d` in
  `Z:\Code\.worktrees\verdigris\ox-pc-ai`. Detached 0161 review tree remains
  `Z:\Code\.worktrees\verdigris\review-task0161-9f004d2a`.
- Queue: board sentinel healthy, 24 READY, 16 sequenced DRAFT successors, zero
  collisions. Runway hours remain UNKNOWN. Dashboard PID 9604 HTTP 200;
  monitor PID 7364 live. No headless OpenCode writer.
- Exact next action: re-review TASK-0101 `a742355d` (JSON, final-range diff,
  citations, readable ranged contract); independently complete TASK-0161's
  literal gate; integrate only ACCEPTED work; do not promote W1/TASK-0108
  while TASK-0161 still owns `native/client/main.cpp`. Owner action: none.

## Supervisor retirement handoff — 2026-08-22 16:53 PDT

- Owner explicitly retired this Codex supervisor for credit exhaustion and is
  handing control to Cursor or another harness. No OpenCode writer remains
  active. Preserve every worktree; do not infer capacity from the dashboard.
- Authoritative program branch `codex/native-reconstitution` is clean and
  pushed at `1a434371b281494d3f5aa6bdc3e50447e1814855`. Protected `master`
  remains green at `a28ac92f`; current raw worker/program pushes launch zero
  workflows. Board sentinel is healthy at 24 READY / 2 CLAIMED / 16 successors
  / zero collisions.
- TASK-0161 is frozen, clean, and pushed at worker head
  `9f004d2a6802782e2b3a79b895be2b0822f8dbf1` in
  `Z:\Code\.worktrees\verdigris\ox-pc-ah`. Worker evidence reports two green
  full gates. Independent detached review at
  `Z:\Code\.worktrees\verdigris\review-task0161-9f004d2a` was interrupted by
  the retirement request during Gate-B after ordinary succession succeeded and
  the heir hunt reached kill 9; this is **not** an independent pass and no
  REVIEW verdict or integration exists. Exact next action: rerun the literal
  SPEC gate to completion from the clean detached review tree, run negative
  controls, then ACCEPT/REVISE and integrate only if green.
- TASK-0101 revision 1 is frozen, clean, and pushed at worker head
  `a742355d189966f0e344d0c4763e014f87ecb820` in
  `Z:\Code\.worktrees\verdigris\ox-pc-ai`. Program REVIEW at `1a434371`
  rejected the original handoff because final untracked files escaped its diff
  gate and W1 would have routed invisible ranged damage. Revision 1 removes the
  whitespace defects and adds an explicit telegraph/client-visible hit contract.
  Exact next action: independently re-run JSON, final-range diff, scope, and
  source-citation checks; update REVIEW; integrate the audit evidence; only
  then promote its readable combat successor.
- Read-only dashboard PID 9604 serves HTTP 200 at `127.0.0.1:4737`; low-cost
  monitor PID 7364 remains live. Standalone orchestration PC broadcast is clean
  and pushed at `88a4b317`; `origin/main` remains Mac-owned. Owner action:
  start the replacement supervisor with this section and
  `orchestration/SUPERVISOR_SUCCESSION.md` as its first state read.

## Three accepted integrations converged; next two-lane wave claimed — 2026-08-22 16:38 PDT

- TASK-0163 frozen head `a53620ef` is independently ACCEPTED and integrated as
  `acfcbfd1`; a clean detached full native gate plus three consecutive exact
  session runs passed 107/107, and the combined-program gate passed. The
  ordinary House/fall/succession/named-Warden/exact-relic/reconnect release
  journey is no longer blocked by the old load-sensitive driver.
- TASK-0157 accepted audio lineage is now integrated as `287433bf` +
  `a6c33c19` after the Gate-B hold cleared. The combined full native gate and
  60-check audio suite pass; two direct audio runs are byte-identical at
  SHA-256 `87C88BD9...2600C`. This remains a backend-neutral scheduler/cue seam,
  not an audible-playback or final-sound claim.
- TASK-0159 frozen head `7ca17bfe` is independently ACCEPTED and integrated as
  `6cbb1324` + `fbe8f49c`. Four real-GDI 960x600/1366x768 captures were visually
  inspected; the complete combined `-RunTests -RunClientScenarios` gate passes,
  including both Escape contracts and all 12 scenarios. Procedural placeholder
  art remains intentionally non-final.
- TASK-0161 is claimed by `ox-pc-ah` on pushed worker head `7730e49a`, exact
  routed base `610a240e`, ports 7260-7279. The activation controller caught a
  valid but uncommitted claim after the first real session stopped, preserved
  it, and recovered the same session to a durable pushed claim before source
  work continued.
- TASK-0101 is claimed by `ox-pc-ai` on pushed worker head `b083b58b`, exact
  routed base `610a240e`, read-only/no-server. Its first claim used a Markdown
  list the board could not parse; the supervisor stopped source work and the
  same session replaced it with machine-readable YAML before proceeding.
- Board proof is healthy at 24 effective READY, 2 CLAIMED, 16 sequenced DRAFT
  successors, and zero owned-path collisions. Program pre-update head is
  `16c0cd3f`. Dashboard PID 9604 serves HTTP 200; the corrected low-cost
  monitor PID 7364 watches only these two active routes and rejects an active
  worker immediately if its branch carries a raw-push CI trigger. Both current
  branches pass that guard. Protected `master` is green at `a28ac92f`; the two
  current raw claim heads each launched zero Actions runs. Owner action: none.

## Content validation shipped; audio accepted behind Gate-B hold — 2026-08-22 15:46 PDT

- TASK-0164 replacement worker head `cedabc72` is independently ACCEPTED and
  INTEGRATED as program commits `9ffea0b4` + `3961733b`, lifecycle
  `f764600e`. Combined positive seed validation and two deterministic 27/27
  negative-suite runs pass. Cross-collection type mismatches, dangling
  references, and unreachable encounter anchors now fail closed; schema,
  seeds, authored content, and runtime remain unchanged.
- TASK-0157 revision head `bb7b8ceb` is ACCEPTED but intentionally not yet
  integrated. A clean detached checkout proved `build.ps1 -RunTests` now
  creates the audio test executable; that binary passed twice with identical
  output and correct `monster`/`scion` discrimination. The same full run then
  hit the known unrelated Gate-B exact-relic/continuity failure. Integration
  waits for TASK-0163 and a green combined native gate; this is not an audio
  revision request.
- TASK-0163's first process stopped at a clean reasoning boundary with its
  owned session-test correction uncommitted. Its one same-session recovery is
  PID 26948 and preserves the exact dirty worktree; an earlier diagnostic run
  already passed the full Gate-B chain after replacing inconsistent floor
  conversion with the shared tile conversion, and the recovery is removing
  probes and running the literal gates. ox-pc-z/TASK-0159 remains active at
  PID 32.
- Program head `f764600e8b8c57fbeca1ba2251fd011156aa925c` is clean/pushed.
  Queue proof remains healthy at 24 effective READY, 17 sequenced successors,
  and zero collisions. Dashboard PID 26104 serves HTTP 200; monitor PID 2664
  is live. Raw program/worker pushes continue to start zero workflows. Owner
  action required now: none.

## Review truth enforced; stopped lane replaced inside SLA — 2026-08-22 15:26 PDT

- TASK-0157 predecessor handoff `024dabb5` is **REVISE**, not integrated.
  Independent clean-worktree verification proved the report's literal build
  claim was masked by an unrecorded manual CMake prebuild: after
  `native/build.ps1 -RunTests`, the required
  `native/build/verdigris_audio_mixer_tests.exe` did not exist. Source
  inspection also proved every `ActorDied` event was mapped to enemy-kill
  audio even though core emits `text == "scion"` immediately before
  `ScionLost`. Fresh revision lane ox-pc-af pushed valid claim `7ffc8696`,
  ports 7220-7239, and is applying only the two numbered corrections. No audio
  implementation has entered the program branch.
- TASK-0164 predecessor ox-pc-ae pushed claim `949508f5`, then exited dirty
  after OpenCode auto-rejected a global-temp evidence capture. Its prior clean
  activation recovery was already spent, so the claim is released and the
  worktree is quarantined intact. Fresh lane ox-pc-ag, ports 7240-7259, uses
  dedicated temp `Z:\Code\.fleet\tmp\ox-pc-ag`. The low-cost claim validator
  rejected its first `coordinator: ox-alpha` metadata; same-session activation
  recovery repaired it to `coordinator: codex` and pushed exact head
  `dfd40ee2` inside the activation SLA. Program records are `68ec7306` and
  `2404097b`.
- Current live OpenRouter Ox Alpha processes: ox-pc-z/TASK-0159 PID 32,
  ox-pc-ac/TASK-0163 PID 17644, ox-pc-af/TASK-0157 revision PID 26068, and
  ox-pc-ag/TASK-0164 replacement PID 21748. Pushed claims plus fresh activity
  are authoritative; stopped ae/ad and terminal aa are not capacity.
- Claim-aware queue proof is healthy: 24 effective READY, 17 sequenced DRAFT
  successors, zero owned-path collisions, with TASK-0157 surfaced separately
  as REVISE. Board tests pass 22/22. The human dashboard is HTTP 200 at
  `http://127.0.0.1:4737/` on PID 25984; transition monitor PID 2664 remains
  active.
- Program pre-checkpoint head `2404097bca3dc634c3bb9219733070a8df638d81`
  is pushed on `codex/native-reconstitution`. Protected `master` remains green
  at `a28ac92f`. Program/revision/replacement pushes launched zero workflows;
  GitHub Actions remains reserved for PRs targeting `master` and protected
  `master` pushes. Owner action required now: none.

## Pane foundation integrated; four-lane product/reliability wave sustained — 2026-08-22 15:01 PDT

- TASK-0158 frozen worker head
  `016d35a25a86d048ab1c1412fbd23e3dc1f33243` is independently ACCEPTED and
  INTEGRATED. Both the detached worker-head review and combined program head
  passed the 413-check MSVC `/W4` pane-model harness, native legacy denylist,
  scope, ancestry, and diff-hygiene gates. Program lineage is architect review
  `9fd04e98`, implementation `14809519`, evidence `2be77c30`, and lifecycle
  close `b949b3e4`. The pure header remains intentionally unwired until a
  separately accepted painting/input successor.
- Current valid claims: ox-pc-z/TASK-0159 `0e74c185` (PID 32, owned
  `main.cpp` edit active); ox-pc-ac/TASK-0163 `d872687f` (PID 17644, Gate-B
  test-only reliability diagnosis active); and replacement
  ox-pc-ad/TASK-0157 `a41f1013` (PID 25328, owned audio/CMake edits active).
  Historical ox-pc-ab remains dirty, exhausted, quarantined, and is not
  capacity. Completed ox-pc-aa is a terminal reviewed lane, not a live PID.
- Fresh ox-pc-ae is LAUNCH_REQUESTED for TASK-0164 from exact pushed base
  `b949b3e4653961b7f13661f38ef3addfb8af0df4`, branch
  `codex/TASK-0164-native-content-cross-reference-hardening-ox-pc-ae`, ports
  7200-7219, OpenRouter Ox Alpha PID 26840. It does not count as capacity
  until its STATUS claim is committed and pushed. P1
  PROVISIONED_UNCLAIMED is due on the first observation after 15:07:50 PDT;
  P0 ACTIVATION_FAILED is due at 30 minutes or two failed sweeps.
- TASK-0165 adds a concrete, disjoint pure input-focus/close-ordering seam tied
  to the accepted Esc regression and future panes. Board sentinel is healthy
  at 25 effective READY before the TASK-0164 claim, 17 sequenced successors,
  and zero owned-path collisions; after that claim is recorded the floor will
  remain exactly 24.
- Human dashboard is HTTP 200 at `http://127.0.0.1:4737/` (PID 26988), with
  TASK-0158 shown as terminal and ox-pc-ae visible during activation. The
  low-cost transition monitor remains PID 2664. Protected `master` remains
  green at release SHA `a28ac92f`; raw program/worker pushes continue to start
  zero Actions workflows. Owner action required now: none.

## Protected release green; Gate-B reliability blocker routed — 2026-08-22 14:27 PDT

- Protected PR #57 merged the loot reliability, native content schema, and
  progression-visibility wave to `master` at
  `a28ac92f6f4fa5fa5a467ecb8353ad1c55667ecb`. Both PR checks passed. The
  post-merge Native run `32598557565` passed; CI run `32598557570` retained a
  first-attempt `session-arc` named-Warden miss at 31/32, then its single
  bounded rerun passed all 32 scenarios. Raw worker/program pushes launched no
  workflows, so GitHub mail is now reserved for protected release signal.
- TASK-0152 revision head `d34097d9` is independently ACCEPTED: frozen full
  native gate passed, its MSVC `/W4` build was warning-free, all six positive
  captures validated, and all twelve negative captures failed as required.
  Program staging commits are review `3da054b6`, original implementation
  `59ab5083`, and correction `76837dec`.
- The combined program native gate then failed the ordinary-play Gate-B
  heirloom hunt after seven minutes (four kills; Warden never surfaced). One
  bounded exact-session retry failed at the earlier fatal-fall step. These two
  distinct failures do not implicate TASK-0152's measurement-only diff, but
  they block terminal integration truth. TASK-0163 owns only the journey test
  and is routed to remove this nondeterminism without gameplay shortcuts or
  weaker assertions.
- Valid pushed claims now recorded on the program branch: ox-pc-z/TASK-0159
  `0e74c185` and ox-pc-aa/TASK-0158 `d1fd4e40`. The aa first post-claim process
  exited with owned-path edits; its preserved single recovery is PID 26668.
  ox-pc-ab/TASK-0157 claim `c08ad621` and its one dirty recovery both exited
  without a handoff, so the lane is exhausted, preserved, and released for a
  fresh replacement. ox-pc-z remains live at PID 32. Stopped processes are
  alerts, never counted as working capacity.
- Human dashboard is HTTP 200 at `http://127.0.0.1:4737/` (PID 23504); the
  visible low-cost monitor remains PID 2664. Owner action required now: none.

## TASK-0155 accepted; three claims live — 2026-08-22 13:40 PDT

- TASK-0155 worker implementation `3cae8a2d` and frozen handoff `b06f5740`
  passed the mandatory independent architect rerun because this task modifies
  a goal-harness assertion path. Exact-head lint, five focused real-server
  journeys, and the default 32/32 harness passed. After integration, focused
  loot passed in 5885 ms and the combined program harness passed 32/32 in
  196.21 s. Verdict: ACCEPTED; program commits `7771da43` and `46822df2`.
- ox-pc-z stopped after its pushed handoff and is no longer capacity. The
  remaining valid claims are ox-pc-aa/TASK-0156 `e3f4bdf6`,
  ox-pc-ab/TASK-0151 `57627ca3`, and ox-pc-ac/TASK-0152 `5156c33e`; all three
  retain isolated worktrees, branches, ports, provider identity, and fresh
  post-claim activity.
- Claim-aware board proof remains healthy at 24 effective READY, 3 CLAIMED,
  16 sequenced successors, and zero owned-path collisions. The integrated task
  is excluded from both READY and active capacity.
- The owner inbox screenshot is historical failure mail from before the two
  trigger-contract releases. Since Verdigris PR #56's trigger fix began, GitHub
  records 4 protected PR/master runs and 0 failures; since standalone-
  orchestration PR #3's trigger fix began, it records 2 protected PR/main runs
  and 0 failures. Raw worker, program, and PC broadcast pushes remain quiet.
- One ox-pc-ac shell invocation opened the architect checkout's commit editor
  and left an idle `git commit` child even though the worker's valid claim had
  already landed. The supervisor matched and stopped only that exact child;
  both repositories stayed clean and the OpenCode worker continued. Future
  launch packets must use an explicit commit message as well as the no-hooks
  path so unattended workers cannot wait on an interactive editor.
- Dashboard PID 9260 is HTTP 200 and the visible low-cost monitor is PID 2664.
  Owner action required now: none. Historical mail may be archived separately;
  real protected-branch or protected-PR failures remain intentionally visible.

## Four-lane product/reliability activation — 2026-08-22 13:06 PDT

- Queue restock commit `c2b814488278f4f093e754cf695ea9ed749d81fb`
  verifies 28 effective READY packets, 16 sequenced DRAFT successors, and zero
  owned-path collisions. Four current-tip worktrees are provisioned clean at
  that exact SHA; they do not count as fleet capacity before pushed claims.
- `ox-pc-z`: TASK-0155 deterministic loot reliability, branch
  `codex/TASK-0155-deterministic-loot-playtest-reliability-ox-pc-z`, ports
  7100-7119. Removes the recorded second-drop release flake without weakening
  the real goal harness.
- `ox-pc-aa`: TASK-0156 native progression visibility, branch
  `codex/TASK-0156-native-progression-visibility-ox-pc-aa`, ports 7120-7139.
  This is the owner-visible lane: authoritative passive-tree points/allocation
  become legible inside the shipped gear overlay with real GDI evidence.
- `ox-pc-ab`: TASK-0151 native content schema seed, branch
  `codex/TASK-0151-native-content-schema-seed-ox-pc-ab`, ports 7140-7159.
  Content-neutral schema/validator only; no production lore or balance.
- `ox-pc-ac`: TASK-0152 native density benchmark evidence, branch
  `codex/TASK-0152-native-density-benchmark-evidence-ox-pc-ac`, ports
  7160-7179. Reproducible evidence only; no optimization/gameplay change.
- All four route packets require `openrouter/stealth/ox-alpha`, OpenCode CLI
  1.18.21, variant max, an exact pushed STATUS claim within ten minutes, and
  fresh post-claim activity before ACTIVE. Any root/branch/base/task/provider
  mismatch is P0 MISROUTED; no historical dirty/quarantined lane is reused.
- After four valid claims the effective READY floor remains exactly 24. The
  backend-neutral TASK-0157 audio scheduler and TASK-0158 pane model are READY
  replacements but are not routed in this wave.

## CI notification signal hardening — 2026-08-22 12:54 PDT

- Protected PR #56 merged the workflow trigger contract to `master` at
  `81ea0d39ca30fc576509afe3af757bde73ee2401`. Both CI and Native now run only
  for `master` pushes and pull requests whose base is `master`; raw worker,
  program, broadcast, and non-release PR activity cannot fan out Actions mail.
- The program push at `36b9cf2f` dispatched zero workflows. The added
  `tests/unit/ci-signal-hygiene.spec.js` contract passed 2/2 and covers both
  workflow trigger blocks.
- PR Native run `32594398255` passed. PR CI run `32594398265` first recorded a
  load-sensitive existing `loot` scenario miss while 31/32 scenarios passed,
  then its single bounded retry passed the full suite in 6m30s. The failure is
  retained for a deterministic-loot reliability successor; it was not hidden
  and the gate was not weakened.
- Post-merge Native run `32595081869` and CI run `32595081863` both passed.
  Exactly those two protected-branch runs launched; no worker/coordination run
  was created. Owner action required: none.

## PC accepted product integration — 2026-08-22 12:11 PDT

- TASK-0122 frozen worker head `d67129e4` is independently ACCEPTED and
  integrated as `032ae03e`. Frozen-head native gates, 24/24 focused tests,
  31/31 VFX scenario checks, browser playtest 32/32, diff/scope checks, the
  simulation-untouched negative control, and both final images passed.
- TASK-0148 frozen worker head `9f00e198` (implementation `5732367e`) is
  independently ACCEPTED and integrated as `127a540e` + `8d314d5c`. The
  architect rerun proved the complete normal-envelope Gate-B journey through
  ordinary death, successor, exact relic recovery, and same-guest reconnect.
- Combined program native gates pass with all ten client scenarios. The first
  combined browser run missed `session-arc` final death by 384 ms beyond its
  15 s budget; the scenario then passed in 11.7 s and the full rerun passed
  32/32 with `session-arc` at 12.3 s. The timing seam is retained in review
  evidence; no deterministic product regression reproduces.
- ox-pc-r and ox-pc-x are stopped, clean, pushed handoffs and no longer count
  as active capacity. No other OpenCode process is registered as active
  Verdigris capacity. Protected PR #55 merged the verified program head to
  `master` at `f9973df632dbdbee4e8fee24da97627d5400bb50`; post-merge Native
  run `32593413272` and CI run `32593413286` both passed.
- Human dashboard is HTTP 200 at `http://127.0.0.1:4737/`; the visible rows
  identify both lanes as supervisor-reviewed handoffs rather than workers.
- Post-integration board sentinel is healthy: 24 effective READY, zero live
  claims/reviews/revisions, 16 sequenced DRAFT successors, zero collisions,
  and 108 integrated packets. Its 22-test suite passes. The deterministic
  factory still verifies 2,000 graph nodes and 500 reserve packets; autonomous
  runway remains honestly `UNKNOWN` until comparable throughput exists.
- Standalone cross-host broadcast branch is clean/pushed at `004acf1a` on
  `codex/pc-overnight-game-wave`; observed shared `origin/main` remains
  Mac-owned at `d068012a`. The PC dashboard now renders both completed lanes as
  terminal `integrated` with no worker action, HTTP 200 on port 4737. The
  low-cost PowerShell monitor is restored as one process.

## PC live sweep — 2026-08-22 09:58 PDT

- Program branch `codex/native-reconstitution` has independently ACCEPTED and
  integrated TASK-0128 through `882de214`; protected `master` remains green at
  `230a8adf` after the TASK-0153 first-session/Esc release.
- ox-pc-r is the sole active TASK-0148 writer at PID 17640. Supervisor commit
  `3119a08e` added the master-only Native trigger to its pre-hygiene branch
  without touching preserved gameplay work; that push launched zero workflows.
  The ordinary-play heirloom recovery gate remains unresolved, so no gameplay
  handoff is accepted.
- ox-pc-x holds valid TASK-0122 claim `fdbbdca6` and is in its one authorized
  same-session implementation recovery at PID 18144 after a clean context-only
  stop. A second pre-handoff stop releases the lane.
- TASK-0128 worker head `2b469e3d` passed final-head check, 19/19 tests with a
  clean tree, schema/diff/scope gates, and six golden comparisons limited to
  `skipped_folders -> []`. Program-side gates also pass; runway remains honestly
  `hours:null` / `UNKNOWN`.
- Deterministic board sentinel: 24 effective READY, 2 valid CLAIMED, 16
  sequenced successors, zero owned-path collisions, exit 0. Factory target
  remains 2,000 graph nodes and 500 reserve packets.
- The monitor now covers duplicate writers, malformed claims, activation SLA,
  and stale Native branch triggers (`P0_CI_TRIGGER_UNSAFE`). Human dashboard
  health remains HTTP 200 at `http://127.0.0.1:4737/`.

## Morning protected-master release — 2026-08-22 07:12 PDT

- Program branch pushed head: `eb65c08b76c95564d61422a51748dbdb476306a9`.
- Protected `master` release: PR #51 merged at
  `db3fc0467ed0cc978f5152aeec558208825bd0af`; GitHub CI/Native checks were
  still running after merge and are not yet claimed green.
- Exact pre-merge release gate: full native build/tests/all client scenarios
  passed. The merge tree equals the program tree.
- Owner live verdict: playable and materially improved, but still extremely
  janky; repetitive terrain overwhelms the embedded vector motifs. This
  supersedes any interpretation of presence/capture tests as a quality verdict.
- Owner defect: Esc globally exits even with gear open. ox-pc-v now has verified
  TASK-0153 claim `8474ac51` and a binding first-Esc-closes-pane/second-bare-
  Esc-exits scenario contract. Its initial provisioned-but-unclaimed SLA breach
  is recorded as INC-013.
- ox-pc-r was deliberately paused only for the shared-capsule release gate and
  resumed in its exact preserved session at PID 2388. No edits were reset.

## Leader-austerity override — 2026-08-21

The owner adopted event-driven leadership. `LEADER_POLICY.md` keeps the
supervisor focused on transitions, acceptance, collisions, and owner-visible
product convergence. The low-cost same-thread heartbeat and local dashboard
remain the deterministic safety net while `orchd` matures. This does not
release claims, change task ownership, or authorize the architect to implement
worker work.

Automation state: the thread-local `verdigris-surge-supervisor` heartbeat is
ACTIVE every five minutes on the existing supervision thread, with a concise
read-only fleet sweep and deduplicated output. The standalone
`pc-fleet-emergency-monitor` cron is PAUSED because it was creating a new Codex
task on every run. Retire the heartbeat once orchd passes its vertical-slice
acceptance; do not re-enable the chat-spawning cron.

The completed PC wave integrated the first native gameplay objective,
procedural vector presentation, portability correction, and four release/
validator packets. Owner feedback raises tonight's success bar from queue
motion to a visibly better playable C++ game. Program commit `df851cea`
therefore promotes four pairwise-disjoint P0 implementation packets:
TASK-0145 Chronicles owner UI, TASK-0146 first-expedition encounter wave,
TASK-0147 procedural visual polish, and TASK-0148 reconnect-safe Chronicles
runtime. Four additional implementation packets (0149-0152) restock the
claimable reserve without consuming strong workers on audit-only work.

The independent `ox-bootstrap` orchestration worker has pushed M3 head
`82de84ef` after M2 `5627916f`. The standalone repo's Tier-B review contract
still governs acceptance and this lane never counts as Verdigris capacity.

Snapshot: 2026-08-22 05:39 PDT

Sweep base: `df851cead0dadcd96176b370ad132f8344c3c21d`
(`codex/native-reconstitution`; `origin/master` remains at the previously green
`d2423873` merge tip).

GitHub: no open PRs. Latest master CI run 32441409427 passed at `d2423873`;
the coordination-only program pushes did not dispatch a new workflow.
TASK-0081 rev3 is accepted and integrated at `31d21579`.

Orchestrator: **PC Verdigris fleet lead**, architect checkout
`Z:\Code\Games\delaford\delaford_game`. Architect coordinates, reviews,
integrates, and specs; it does not absorb implementation.

## Objective and proof state

1. Server/rules parity: **DONE** — unchanged native attach suite 32/32 twice on
   fresh servers plus post-hotfix verification.
2. Native journey parity: Gate A **GREEN**; Gate B and Gate C remain.
3. Presentation parity: terrain/HUD landed; surface density,
   panels/typography, and Stage-2 renderer choice remain.
4. Post-parity full ARPG graph: active in `PROGRAM_GRAPH.md`; parity is a gate,
   not a stopping point.

Emergency surge floor: at least 24 effective, dependency-free, pairwise
path-disjoint READY packets plus 12 concrete successors. Current valid claims
are excluded from READY accounting. Four accepted-contract validator
implementations restored 28 effective READY. TASK-0080, TASK-0137, TASK-0138,
and TASK-0140 are accepted; TASK-0136 is validly claimed on replacement lane h.
The eight promoted packets add four routed product tasks and four unclaimed
restock tasks. TASK-0136 returned to READY after the ox-pc-h release. Excluding
valid claims leaves **29 effective READY + 17
successors**, above the absolute floors. D-128 supersedes count-only
sufficiency; TASK-0128 worker head `d247638e` fixes the original self-reference
but remains REVISE because its green test run dirties six committed golden
fixtures. The branch is preserved and runway remains UNKNOWN.

## Autonomous runway and factory status

- Autonomous runway: **UNKNOWN / P0 instrumentation gap**. The board has not
  yet been weighted against trailing accepted throughput by full experimental
  unit and packet type; 25+18 must not be reported as adequate runway.
- Target/warning/critical: 72h / <48h / <24h.
- Terminal graph: **2,000 validated concrete nodes** across 20 domains and all
  T1-T8 gates in `backlog-factory/generated/product-graph.nodes.jsonl`.
- Detailed reserve: **500 validated packets**: 100 DRAFT + 400 AUTO_RELEASE in
  `backlog-factory/generated/packet-reserve.jsonl`. Distant packets carry no
  immutable base and remain nonclaimable until current-tip READY promotion.
- Reserve composition: 100 each implementation, integration, presentation,
  hardening, and release; 0 pure audit/research/inventory/evaluation; 25 packets
  (5%) owner-blocked. The immediate board remains audit-heavy, so strong worker
  routes are biased toward implementation and owner-visible convergence.
- Deterministic validation: `node orchestration/backlog-factory/build-manifests.mjs --check`
  passes with 2,000 unique nodes and 500 unique packets.
- Binding contracts: `BACKLOG_FACTORY.md`, `CONTENT_SCALE_MATRIX.md`, and
  D-128. Backlog production continues every sweep and never authorizes Sol to
  implement worker tasks.

## PC Ox Alpha fleet reconciliation

Shared entry: `orchestration/REENTRY-OX-ALPHA-PC.md`.

| Lane | Ports | Repository evidence | Initial route |
|---|---|---|---|
| ox-pc-a | 6620-6639 | TASK-0128 revision `d247638e` preserved; accepted foundation completed by fresh lane ox-pc-y | preserved, not active capacity |
| ox-pc-b | 6640-6659 | TASK-0145 recovered claim `4aa9e0c3` then worker exited dirty; claim released | P0 quarantined, not capacity |
| ox-pc-c | 6660-6679 | TASK-0136 claim released after duplicate-dispatch collision; dirty worktree quarantined | not available |
| ox-pc-d | 6680-6699 | TASK-0146 second post-claim stop after one recovery; claim `7e416ff3` released | P0 quarantined/preserved, not capacity |
| ox-pc-e | 6700-6719 | TASK-0147 second post-claim stop after one recovery; claim `068a1358` released | P0 quarantined/preserved, not capacity |
| ox-pc-f | 6720-6739 | TASK-0138 accepted/integrated at `38942560`; lane available | current-tip successor pending |
| ox-pc-g | 6740-6759 | TASK-0148 second post-claim stop after one recovery; claim `1b058604` released | P0 quarantined/preserved, not capacity |
| ox-pc-h | 6760-6779 | TASK-0136 second post-claim process exit; dirty worktree preserved; claim released | P0 quarantined, not capacity |
| ox-pc-i | 6780-6799 | TASK-0145 frozen head `78dcac60` independently ACCEPTED and integrated at `2df5eac5` | lane complete/available after worker stops |
| ox-pc-j | 6800-6819 | TASK-0149 revision head `a88d307d` independently ACCEPTED and integrated through `8677f021` | lane complete/available |
| ox-pc-k | 6820-6839 | TASK-0150 frozen head `54417592` independently ACCEPTED; implementation integrated at `10039385` | lane complete/available after worker stops |
| ox-pc-l | 6840-6859 | TASK-0146 revision head `086ac07b` independently ACCEPTED; implementation integrated as `c873c5af`, full chain through `5324f13e` | complete/available after worker exit |
| ox-pc-m | 6860-6879 | TASK-0147 second post-claim stop after one recovery; claim `7d092a74` released; dirty worktree preserved | P0 quarantined, not capacity |
| ox-pc-n | 6880-6899 | TASK-0148 clean launch and its one recovery both stopped before claim/write | activation failed; clean preserved, not capacity |
| ox-pc-o | 6900-6919 | TASK-0148 second post-claim stop after one recovery; claim `71a73de8` released; clean worktree preserved | exhausted, not capacity |
| ox-pc-p | 6920-6939 | TASK-0147 frozen head `974ccab6` independently ACCEPTED and integrated at `19be98db`; fresh visual evidence and combined native gates passed | handoff complete; available only after worker process exits |
| ox-pc-q | 6940-6959 | TASK-0148 claim `815a359b` released after recovery proposed a second normal-path bypass (`mutate/select` instead of `create/set-out`); clean claim head preserved | exhausted, not capacity |
| ox-pc-r | 6960-6979 | TASK-0148 frozen head `9f00e198` independently ACCEPTED; implementation integrated as `127a540e`, handoff as `8d314d5c`; full normal-play reconnect gate green | complete; stopped/clean, not active capacity |
| ox-pc-s | 6980-6999 | TASK-0116 corrected audit head `5b007e7e` independently ACCEPTED and integrated through `8eb95893` | audit complete; TASK-0122 successor routed |
| ox-pc-t | 7000-7019 | TASK-0117 frozen head `5a7c22cb` independently ACCEPTED and integrated at `7052feca` | handoff complete; procedural-audio successor sequenced behind current client-main owner |
| ox-pc-u | 7020-7039 | TASK-0119 frozen head `4104e0c8` independently ACCEPTED and integrated at `a5f4133e` | handoff complete; executable onboarding successor ready to route |
| ox-pc-v | 7040-7059 | TASK-0153 final worker head `30ab95fd`; Esc/first-session fix shipped to protected master `230a8adf` with green CI | complete/available |
| ox-pc-w | 7080-7099 | TASK-0154 clean-runner portability head `2dc4bfb2`; protected hotfix master `4e55f4f9` green | complete/available |
| ox-pc-x | 7060-7079 | TASK-0122 frozen head `d67129e4` independently ACCEPTED and integrated as `032ae03e`; native/browser/visual/negative-control gates green | complete; stopped/clean, not active capacity |
| ox-pc-y | none | TASK-0128 fresh-lane head `2b469e3d` independently ACCEPTED and integrated through `882de214`; 19/19 clean | complete/available after worker exit |

The historical stopped `ox-pc-b` and `ox-pc-c` tabs shared one OpenCode project,
stopped before claims/writes, and remain non-incidents. The newly registered
lanes reuse those names only for new independent Z: worktrees and CLI sessions;
they do not retroactively convert the old tabs into capacity.

Legacy PC clones under ChatGPT, Cursor, DeepSeek, kimi, and KimiWork are stale
and/or dirty with preserved user/worker work. They are **not** registered as Ox
clones and must not be reset, cleaned, or silently repurposed. The first
committed Ox claim registers its actual clone path and full scorecard
experimental unit.

OpenCode CLI 1.18.21 is installed and the OpenRouter lanes launch headlessly
with explicit `openrouter/stealth/ox-alpha`; no owner tab opening is required.
`Z:\Code\.fleet\Watch-VerdigrisFleet.ps1` supplies one human-readable five-second
dashboard and transition-deduplicated Windows P1/P0 notifications. Current live
routes include r/x; completed y remains visible for audit. Historical and
quarantined worktrees do not count as active capacity.
An unattended REVISE route is a distinct P1 alert, so a stopped revision worker
cannot be masked by the task-level review verdict.
The worker logs remain under `Z:\Code\.fleet\logs`. Sol does not claim or write
worker STATUS/REPORT and will not take over implementation.

The `ox-pc-e` initial process exited after the repository's yorkie pre-commit
hook found no local `node_modules`, leaving only a staged claim. This was a
centralized environment/permission activation fault, not worker implementation
failure and not an incident. The supervisor installed the locked dependencies
with `npm ci`, granted read-only access only to the lane's external Git metadata,
resumed the preserved session once, and the worker committed and pushed the
claim with hooks enabled at `97580939`. The same narrow read-only Git-metadata
permission is present in every ignored lane-local `opencode.json`; `--auto` is
not enabled.

### Resolved activation alert

- **Lane:** `ox-pc-a`; **prior state:** P0 `MISROUTED` plus P1
  `PROVISIONED_UNCLAIMED`; **launch:** 20:06 PDT; **P1 notification:** 20:16;
  **cleared by valid claim:** 20:21 (`f08131c0`).
- **Expected:** the exact provisioned worktree/branch/base above, TASK-0081,
  and the ignored `START_HERE_OX_PC_A.md` launch packet.
- **Observed transition:** the earlier visible worker-C session was misrouted,
  but a new correctly rooted worker committed and pushed the exact TASK-0081
  claim. It pushed REVIEW_REQUESTED head `0302ea4c` at 20:42 PDT. Independent
  verification passed the literal gates and found two response-shape
  precision defects plus future-dated throughput telemetry; `REVIEW.md` now
  issues a narrow REVISE without rejecting the accepted inventory.
- **Capacity accounting:** the historical alert closed with TASK-0081. It was
  not an incident against the stopped tabs. Five newly isolated CLI lanes are
  now registered; only valid committed claims count as active.

Persistent thread-local supervision is active under `LEADER_POLICY.md`. The
low-cost Luna heartbeat observes transitions until orchd takes over deterministic
session reconciliation and dispatch; it reports in this thread rather than
opening new Codex tasks.

### Cross-project control-plane watch (not Verdigris capacity)

`alexkorol/orchestration` PC main is clean/synced at `b4e8a3aa`. The isolated
`Z:\Code\.worktrees\orchestration\ox-bootstrap` worker pushed valid claim-only
commit `795a9b3` at 20:20 on
`codex/ox-bootstrap-portable-orchestration`; only `bootstrap/CLAIM.md` changed.
Its activation claim remains valid and M3 head `82de84ef` is pushed after M2
`5627916f`. Tier-B acceptance requires the two independent peer reviews recorded
on standalone main before integration.
The earlier no-child-process stall alarm was a false positive because the
OpenCode sidecar was still streaming. Detection now requires corroborating
session, Git/file, backoff, and elapsed evidence. This separate project never
counts toward Verdigris capacity.

### Endpoint identity watch

The existing saved desktop sessions (`ox-pc-a` and `ox-bootstrap`) remain pinned
to their originally observed `opencode/x-preview-f-free` endpoint and are not
relabeled as OpenRouter. The four new CLI sessions explicitly selected
`openrouter/stealth/ox-alpha`, variant `max`; their JSON events contain
OpenRouter reasoning metadata and their committed claims record the same
harness-visible provider/model. These are separate scorecard experimental units.

## Interrupts and authority

- REVISE: **TASK-0128** at `bb67c566`; the committed snapshots bind claim
  commit `0d1898bd` and are stale at their own review head. Lane a must repair
  the evidence/source-revision contract and prove final-head `--check`.
- Accepted/integrated this sweep: **TASK-0080** through `facfd7e4`,
  **TASK-0137** at `83d8f959`, **TASK-0138** at `38942560`, and **TASK-0140**
  at `46574f4e`. Machine-readable terminal states were also restored for the
  earlier accepted PC wave; no implementation content changed in that cleanup.
- Accepted/integrated: **TASK-0150** frozen worker head `54417592`, native
  implementation `ae54f024`, integrated as `10039385` with report/status at
  `ad51287b`. Independent canonical and disposable gates are green; CTest now
  proves 5/5 including camera and all seven client scenarios.
- Released: **TASK-0136** claims `6ea36f5a`/`7b24e5d3`/`7026892e` after a real
  duplicate-dispatch collision. Lane c remains quarantined; the clean lane h
  receives the replacement route after this coordination push.
- Active work: TASK-0146 revision 1 queued on ox-pc-l after exact-head review,
  TASK-0147 replacement claim `3ee9f928` on ox-pc-p, and TASK-0148 replacement
  claim `815a359b` on ox-pc-q is released after its recovery proposed a second
  normal-path bypass; q is clean-preserved and not capacity. TASK-0145 frozen head `78dcac60` and
  TASK-0149 revision head `a88d307d` are independently ACCEPTED and integrated.
  TASK-0147 claim `7d092a74` on ox-pc-m and
  TASK-0148 claim `71a73de8` on ox-pc-o are released after each replacement
  exhausted its one recovery. Fresh independent attempts are requested on
  ox-pc-p and ox-pc-q. All valid claims are excluded from READY accounting.
- Launch requested at 02:49 PDT: TASK-0149 on fresh lane ox-pc-j and TASK-0150
  on fresh lane ox-pc-k. These pairwise-disjoint reliability packets raise
  the product wave to six workers. Both claims landed inside the ten-minute
  SLA: `2d200041` and `ffb51437`. Lane k's first process exited clean before
  writing; its one exact-session recovery restored dependencies for the
  repository hook, pushed the claim, and completed TASK-0150. TASK-0151 and TASK-0152
  stay READY rather than consuming strong overnight workers on lower-immediacy
  schema or benchmark work.
- TASK-0146 and TASK-0147 both reached that second stop. Claims `7e416ff3` and
  `068a1358` are released; dirty d/e worktrees are P0-quarantined and preserved.
  Fresh l/m replacements must implement independently and must not copy the
  unreviewed dirty edits.
- TASK-0148 reached its second post-claim stop after the one permitted recovery.
  Claim `1b058604` is released; the dirty g worktree is P0-quarantined and
  preserved. Fresh lane n must implement independently without copying it. Its
  first clean launch stopped during preflight before any claim/write; its one
  permitted exact-session recovery also stopped before claim/write. Lane n is
  activation-failed and clean-preserved, not capacity. Route the next clean
  independent attempt to ox-pc-o. Lane o then pushed claim `71a73de8`, stopped
  clean, and its one exact-session recovery also stopped clean before an
  implementation commit. Its recovery budget is exhausted; route a new
  independent attempt to ox-pc-q and do not treat recent o log timestamps as
  capacity.
- TASK-0147 replacement lane m likewise stopped dirty and then stopped again
  during its one exact-session recovery before committed handoff. Claim
  `7d092a74` is released and the dirty m worktree is P0-quarantined. Route a
  fresh independent attempt to ox-pc-p; do not inspect or copy m/e edits.
- TASK-0149 used one exact-session recovery, then completed a clean pushed
  REVIEW_REQUESTED handoff at `96f4ccbd`. Independent real-window happy-path
  gates passed, but readiness-timeout/wrong-port exceptions can orphan the
  already-started server because the caller never receives its PID. Verdict is
  REVISE; resume only for that precise correction and negative control.
  Revision 2 is now clean and pushed at `a88d307d`; independently rerun the
  deterministic readiness-fault control, lifecycle selftest, negative guards,
  and full native suite before changing the verdict.
- P0 quarantined: TASK-0136 replacement claim `ddd00857` on ox-pc-h. The first
  process exit received one exact-session recovery; the recovery also exited
  with dirty uncommitted work. RELEASE now returns the task to READY. Preserve
  the worktree and do not recover it again.
- P0 quarantined: TASK-0145 claim `4aa9e0c3` on ox-pc-b. Its one activation
  recovery successfully claimed but later exited dirty without handoff. The
  claim is released and the worktree preserved; replacement routes to ox-pc-i.
- REVISE preserved: TASK-0128 worker head `d247638e` on ox-pc-a. Its exact
  recovery exited after a clean local commit; the supervisor published that
  exact commit, independently found test-induced tracked-output drift, and
  reverted the unpushed trial integration. No further automatic recovery.
  Separate project: orchestration bootstrap claim `795a9b3`, with pushed M3
  head `82de84ef` awaiting its configured Tier-B acceptance path.
- Historical TASK-0056 and legacy clone WIP are superseded/preserved, never
  resumed.
- Historical QUESTION-0007/0008/0009/0010/0011 are resolved by integrated work.
- Port 6500 remains owner-only; every worker uses only its loopback capsule.
- This owner correction explicitly authorizes pushing the architect coordination
  commit to the program branch. Workers still push only their own branches.

## Effective READY — 26 packets before next claims

Every row is dependency-free at this snapshot. Owned paths are pairwise
disjoint; initial routes do not constitute claims. First committed `STATUS.md`
wins after a fresh fetch.

| Pri | Task | Topology / job | Preferred route | Owner-visible contribution |
|---|---|---|---|---|
| P0 | TASK-0097 persistence durability audit | INDEPENDENT / BOUNDED-DESIGN | future after current claim | protects House/Scion/item saves |
| P0 | TASK-0100 deterministic replay audit | INDEPENDENT / BOUNDED-DESIGN | future after current claim | makes divergences reproducible |
| P0 | TASK-0104 itemization/history audit | INDEPENDENT / BOUNDED-DESIGN | future after current claim | stages memorable history-bearing loot |
| P1 | TASK-0082 dual-server matrix runner | INDEPENDENT / BOUNDED-DESIGN | future after current claim | automates unchanged JS/C++ parity evidence |
| P1 | TASK-0115 browser panel/typography inventory | INDEPENDENT / MECHANICAL | future after current claim | freezes presentation delta #4 |
| P1 | TASK-0091 protocol coverage sentinel design | INDEPENDENT / MECHANICAL | future after current claim | catches lost journey wire steps |
| P1 | TASK-0092 owner launch/packaging audit | INDEPENDENT / MECHANICAL | future after current claim | maps double-clickable build gaps |
| P1 | TASK-0093 typography contract audit | INDEPENDENT / BOUNDED-DESIGN | future after current claim | enables readable native panels/text |
| P1 | TASK-0095 content-authoring schema audit | INDEPENDENT / BOUNDED-DESIGN | future after current claim | enables scalable validated content |
| P1 | TASK-0096 campaign graph measurement | INDEPENDENT / MECHANICAL | future after current claim | measures the route to multi-act campaign/endgame |
| P1 | TASK-0098 wire parser robustness audit | INDEPENDENT / MECHANICAL | future after current claim | hardens malformed-client boundaries |
| P1 | TASK-0099 performance budget audit | INDEPENDENT / MECHANICAL | future after current claim | defines dense-ARPG headroom evidence |
| P1 | TASK-0101 combat-depth gap audit | INDEPENDENT / BOUNDED-DESIGN | future after current claim | selects coherent player-visible combat waves |
| P1 | TASK-0102 skill-system gap audit | INDEPENDENT / MECHANICAL | future after current claim | maps LMB/RMB/Q/E/R end to end |
| P1 | TASK-0103 monster/encounter gap audit | INDEPENDENT / BOUNDED-DESIGN | future after current claim | stages packs, rarity, uniques, bosses |
| P1 | TASK-0118 accessibility/options audit | INDEPENDENT / MECHANICAL | future after current claim | stages broad readable and controllable play |
| P1 | TASK-0121 owner content approval matrix | INDEPENDENT / MECHANICAL | future after current claim | batches art/lore/naming/balance/economy/content gates |
| P2 | TASK-0084 reference-capture manifest | INDEPENDENT / MECHANICAL | future after current claim | detects stale/missing visual evidence |
| P2 | TASK-0085 denylist exception audit | INDEPENDENT / MECHANICAL | future after current claim | prepares owner compatibility ruling |
| P2 | TASK-0114 renderer backend evaluation | EXPLORATORY / BOUNDED-DESIGN | future after current claim | feeds cross-platform renderer ADR |
| P2 | TASK-0094 asset provenance audit | INDEPENDENT / MECHANICAL | future after current claim | prevents unshippable assets/fonts |
| P1 | TASK-0151 native content schema seed | INDEPENDENT / IMPLEMENTATION | future clean lane | creates a deterministic content-neutral authoring seam |
| P1 | TASK-0152 native density benchmark evidence | INDEPENDENT / IMPLEMENTATION | future clean lane | measures encounter/presentation headroom reproducibly |
| P1 | TASK-0136 passive-tree contract validator | INDEPENDENT / MECHANICAL | fresh worktree only; quarantined lanes forbidden | fail-closes counter confusion without canonizing content |
| P0 | TASK-0155 deterministic loot reliability | INDEPENDENT / IMPLEMENTATION | fresh isolated lane | prevents load-sensitive second-drop release flakes without weakening gates |
| P0 | TASK-0156 native progression visibility | INDEPENDENT / IMPLEMENTATION | fresh isolated native-client lane | surfaces authoritative passive-tree points in the shipped gear overlay |
| P1 | TASK-0157 procedural audio scheduler | INDEPENDENT / IMPLEMENTATION | future clean lane | freezes backend-neutral deterministic cue behavior before owner backend choice |
| P1 | TASK-0158 pane model foundation | INDEPENDENT / IMPLEMENTATION | future clean lane | creates responsive collision-free pane geometry without choosing final styling |
| P0 | TASK-0159 native HUD/pane readability | INDEPENDENT / IMPLEMENTATION | fresh native-client lane | removes owner-visible text collisions and prototype-like hierarchy defects |
| P1 | TASK-0160 native visual-kit packaging proof | INDEPENDENT / IMPLEMENTATION | future clean lane | proves committed procedural/vector placeholders survive source-to-owner launch reproducibly |
| P0 | TASK-0163 Gate-B ordinary-play journey reliability | INDEPENDENT / IMPLEMENTATION | fresh recovered native-test lane | removes the release-blocking fatal-fall/Warden nondeterminism without runtime shortcuts |
| P1 | TASK-0164 native content cross-reference hardening | INDEPENDENT / IMPLEMENTATION | future clean content-validation lane | rejects dangling zone/encounter seed references before content reaches a shipped runtime |
| P1 | TASK-0165 native input focus model foundation | INDEPENDENT / IMPLEMENTATION | future clean native-client seam lane | prevents future panes from reintroducing global-Esc exits or leaking gameplay input through focused UI |
| P1 | TASK-0161 native scenario capture-output isolation | PIPELINED / IMPLEMENTATION | fresh native-client lane after TASK-0159 | prevents complete gates from rewriting historical visual evidence |
| P1 | TASK-0162 native passive-tree payload hardening | INDEPENDENT / IMPLEMENTATION | future clean session-parser lane after TASK-0163 | rejects malformed progression mirrors without inventing balance |
| P0 | TASK-0108 readable ranged combat successor | INDEPENDENT / IMPLEMENTATION | fresh native combat lane after 0101+0161; ports 7280-7299 | packs mix pressure roles with a telegraph and attributed hit beat |

## HOLD despite historical READY headers

| Task | Release condition | Reason |
|---|---|---|
| TASK-0077 native Chronicles client | TASK-0081 ACCEPTED; architect freezes exact client interfaces | Gate B event/payload contract first |
| TASK-0078 native surface density | TASK-0077 ACCEPTED/integrated | both own `native/client/**`; single-writer rule |
| TASK-0073 renderer backend evaluation | SUPERSEDED by TASK-0114 | surge replacement supplies exact evidence, acceptance, and stop contracts |
| TASK-0079 browser panel inventory | SUPERSEDED by TASK-0115 | surge replacement supplies hard-fail capture and exact gate contracts |

## Sequenced successors — 16 DRAFT

| Task | Dependency / release |
|---|---|
| TASK-0087 native pane shell | 0115 + 0078 accepted; pane model frozen |
| TASK-0088 renderer ADR | 0114 accepted; owner approves any dependency |
| TASK-0089 Gate C native journey | 0077 + 0078 + 0086; missing fields resolved |
| TASK-0090 native progression panes | 0087 + authoritative payload audit |
| TASK-0106 deterministic replay runner | 0100 accepted; record/divergence interfaces frozen |
| TASK-0107 persistence fault matrix | 0097 accepted; disposable profile/fault contract frozen |
| TASK-0109 skill infrastructure | 0102 accepted; content-neutral interfaces frozen |
| TASK-0110 encounter-system wave | 0103 accepted; deterministic pack/rarity seam frozen |
| TASK-0111 item-history lifecycle | 0104 accepted; owner-independent lifecycle frozen |
| TASK-0113 campaign/content tooling | 0095 + 0096 accepted; validators frozen |
| TASK-0123 audio/music runtime | 0117 + backend/license/owner direction |
| TASK-0124 accessibility/options wave | 0118 + pane/input/settings ownership |
| TASK-0125 onboarding first-session wave | 0119 + Gate B/C + copy placeholders |
| TASK-0126 clean-machine release harness | 0092 + 0094 + 0099 + 0120 accepted |
| TASK-0127 save migration matrix | 0097 + 0107 + 0120 accepted |

## Standing sweep and restock

1. Fetch/prune, compare program/master tips, scan Ox branches/timestamps and
   every STATUS/REPORT/REVIEW/RELEASE/QUESTION transition.
2. Review any `REVIEW_REQUESTED` immediately at exact head/base with default
   commands, fresh servers where required, driver preconditions, captures, and
   modified-test inspection.
3. Verdict ACCEPTED/REVISE/BLOCKED/SUPERSEDED with numbered testable findings.
4. Integrate only ACCEPTED work; run affected post-integration gates; restock
   the lane in the same sweep; update full scorecard experimental unit.
5. When no review exists, deepen `PROGRAM_GRAPH.md`, scaffold risky seams, or
   prepare successor packets—never take feature implementation.

## Owner-input queue

Current batched packets are indexed at `orchestration/owner-input/README.md`.
None blocks the three initial Ox routes. Evidence-dependent questions remain
quiet until their prerequisite audit/ADR is accepted.
