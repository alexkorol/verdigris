# Native reconstitution handoff

## 2026-08-21 — PC single-lane Ox Alpha surge runway

- Program truth at sweep start: `d2423873`; `origin/master` and
  `origin/codex/native-reconstitution` matched, latest exact-SHA CI was green,
  and no PR, active claim, REVIEW_REQUESTED, or REVISE transition existed.
- D-126 registers only `ox-pc-a` (Windows, ports 6620-6639). The stopped b/c
  tabs shared one OpenCode project, made no claim/write, and are explicitly not
  Verdigris lanes or incidents.
- `RUN_STATUS.md` now exposes 24 effective pairwise path-disjoint READY packets
  plus 12 DRAFT successors. `PROGRAM_GRAPH.md` carries the deeper journey,
  presentation, renderer, campaign, combat, skill, monster, item, progression,
  persistence, replay, performance, tooling, packaging, and polish graph.
- Initial one-at-a-time route: TASK-0081 Gate B wire-contract freeze. The
  worker must use a clean dedicated clone of `codex/native-reconstitution`,
  read `REENTRY-OX-ALPHA-PC.md`, claim by committed STATUS, and never use the
  architect checkout.
- Owner-only decisions are batched under `orchestration/owner-input/`; none
  blocks TASK-0081. This milestone changes coordination only, not gameplay.

## 2026-08-20 — TASK-0070 reference scenes Stage 1 (cursor, REVIEW_REQUESTED)

- Worker `codex/TASK-0070-reference-scenes-cursor` off `27d2be62`.
  `verdigris_client.exe --reference-scene all` writes 10 PNGs (1920x1080 and
  1366x768) and 5 render-list JSON dumps. Two-run JSON must match.
- Gates: `build.ps1 -RunTests` green. Architect eyeballs one scene per
  resolution.

## 2026-08-20 — TASK-0069 remote reconnect/retry (cursor, REVIEW_REQUESTED)

- Worker `codex/TASK-0069-remote-reconnect-cursor` off `1f45eb33`. Unexpected
  drop enters `Retrying` (1s/2s/4s, three attempts), re-logs the same guest,
  and resumes from the login snapshot. `player:session-replaced` stays
  terminal `Disconnected`.
- Gates: `build.ps1 -RunTests` green (reconnect resume + replaced no-retry).

## 2026-08-20 — TASK-0064 remote presentation unify (cursor, REVIEW_REQUESTED)

- Worker `codex/TASK-0064-remote-presentation-unify-cursor` off program tip
  `5c41a048`. `--remote` uses the local `paint_scene` pipeline (billboards,
  FX, HUD, camera2d); the 0061 debug painter is gone. No Simulation in
  remote mode.
- Gates: `build.ps1 -RunTests -RunClientScenarios` green, including new
  `remote-render-list` (Monster/Swing/Drop via paint_scene) and session
  `render-list` ops. Architect still needs to play `--remote` and rescore
  Gate A (no zeroes, ≥9/12).
- Play: N enters tin route (E is Sweep); X take-underfoot; walk stairs to
  extract. Monster/loot positions are inferred until 0063 snapshots.

## 2026-08-15 (latest) — Orchestration program active

- The program is now coordinated through `orchestration/` (protocol, state,
  decisions, task specs). Claude/Fable is architect+reviewer; the Codex
  coordinator with Luna workers implements. Read
  `orchestration/PROTOCOL.md` first.
- The current coordinator snapshot is indexed in
  [`RECONSTITUTION_STATUS.md`](RECONSTITUTION_STATUS.md), including the
  original checklist, WIZARD seams, review-ready tasks, blocked ownership
  questions, and current gate evidence.
- Focused WIZARD seam verification is recorded in
  [`WIZARD_INTEGRATION_VERIFICATION.md`](WIZARD_INTEGRATION_VERIFICATION.md):
  Orbs, inventory/Brands & Bonds, and Cartographer/map tests pass 73/73.
- Historical Delaford-to-Verdigris coverage is mapped in
  [`DELAFORD_COVERAGE_MATRIX.md`](DELAFORD_COVERAGE_MATRIX.md), with PvP and
  resource skills explicitly deferred pending product authority.
- The checklist gap audit is maintained in
  [`VERDIGRIS_GAP_AUDIT.md`](VERDIGRIS_GAP_AUDIT.md), including evidence,
  parity boundaries, and unresolved owner decisions.
- Wave 1 READY: TASK-0001 (native Legends records), TASK-0002 (build/CI
  hardening), TASK-0003 (slice verification harness). DRAFT: TASK-0004
  (client control pass per decision D-007), TASK-0005 (legacy audit).
- `prototypes/founding-slice/` is a committed, verified browser feel-lab
  ("Founding of a House"): serve the folder statically and open
  `index.html`, or rebuild via `node build.mjs`. It answers camera/combat/
  founding presentation questions and has no architectural authority.

## 2026-08-15 (later) — Milestone E first visual pass and a build fix

- Fixed a real Milestone D defect: `build.ps1` never defined
  `VERDIGRIS_NATIVE_WINDOWS`, so the "windowed" client silently compiled the
  console fallback and exited immediately. The define is now passed, the entry
  point is a standard `main()` (console subsystem keeps `--headless` stdout
  working), and `NOMINMAX` protects the std algorithms.
- `native/client/main.cpp` now carries the Milestone E visual foundation:
  an adjustable 2.5D camera (wheel zoom, PgUp/PgDn pitch, -/= perspective,
  Home reset), back-to-front depth sorting, contact shadows, a projected
  ground grid and extraction pad, billboard actors with enemy life bars,
  client-side loot scatter around kill sites, and procedural event-driven
  effects (swing arcs, impact flashes, death rings, dust, loot sparkles),
  all double-buffered.
- Verified end to end on Windows: MSVC build clean, the eleven core tests and
  the legacy denylist gate pass, `--headless` completes the extraction loop,
  and a driven input pass (PostMessage WASD/LMB/P/E/X against the live window,
  PrintWindow captures) shows the fight, drops, route unlocks, and extraction
  rendering correctly.

## 2026-08-15 — Milestones A–D first runnable slice

- Repository was synchronized to `origin/master` (`882dd81`) and work moved to
  `codex/native-reconstitution`.
- The browser game remains intact as historical reference.
- Product authority, open decisions, WIZARD Arcane Lattice evidence, legacy
  matrix/denylist, canonical `AGENTS.md`, sprint map, and C++ ADR are complete.
- `native/` is a dependency-free C++20 workspace with a fixed-step command/event
  simulation. It proves House and Scion creation, shared player/monster stats,
  movement, melee, damage/death, generated item and trophy drops, pickup/equip,
  extraction, loss and relic candidacy, House-owned route/branch progression,
  and an external seasonal objective/reward.
- `native/client/main.cpp` is a first runnable client: Win32 opens a native
  placeholder-shape window with WASD, mouse actions, Space dash, pickup/equip,
  and extraction controls; `--headless` provides a self-terminating smoke run.
- The core test executable covers the eleven requested architectural behaviors.
- Explicit `platform/`, `renderer/`, `networking/`, `persistence/`, and
  `content/` seams are documented without coupling them into the simulation.
- WIZARD integration intent is now explicit: orbs and Splash feed the renderer
  or menu presentation, Brands & Bonds/inventory feeds the item/UI path, and
  Cartographer is a candidate deterministic map-content adapter.
- The Claude demo source and 22 external PNG plates are now inventoried in
  [`CLAUDE_DEMO_ASSET_INTAKE.md`](CLAUDE_DEMO_ASSET_INTAKE.md). The binaries
  remain outside the checkout pending asset provenance, size, and packaging
  approval.
- The incomplete product checklist is recorded in
  [`VERDIGRIS_FEATURE_CHECKLIST.md`](../product/VERDIGRIS_FEATURE_CHECKLIST.md),
  with economy, Legends, branch-length, travel-risk, and UI-setting questions
  promoted into `OPEN_DECISIONS.md`.

## Next exact steps

1. Run a manual Win32 play pass and capture control/feel notes.
2. Decide whether the next renderer experiment uses the existing Canvas 2.5D
   reference ideas or a focused native billboard layer.
3. Keep networking, persistence, complete magic, and production art out of the
   core until the first playable loop has been evaluated.

## 2026-08-17 — Native parity wave N2 in progress

- TASK-0044 is claimed by Kimi Code in the external worktree
  `C:\Users\Alex\Documents\KimiWork\verdigris`; the WIP adds native world,
  movement, solo-zone, metadata, monster, and stair-return seams without
  changing the unchanged browser harness.
- Coordinator evidence is kept in
  [`TASK-0044 BASELINE.md`](../../orchestration/tasks/TASK-0044-native-protocol-n2/BASELINE.md)
  and the provisional
  [`REPORT.md`](../../orchestration/tasks/TASK-0044-native-protocol-n2/REPORT.md).
- The current worker WIP now passes the native denylist, core tests, and
  networking tests. A rebuilt server also passes unchanged `movement` and
  `zones` attach scenarios 2/2, including all six zones and saved-position
  restoration. Coordinator captures are preserved in the TASK-0044 folder;
  Kimi committed the six native files as `d476788`, so the task is now
  `REVIEW_REQUESTED` pending Fable's architect rerun and acceptance decision.

## 2026-08-17 — N3 combat parity boundary prepared

- The coordinator completed a read-only audit of the native/browser combat
  seams and recorded the executable handoff in
  [`N3_PARITY_IMPLEMENTATION_BRIEF.md`](N3_PARITY_IMPLEMENTATION_BRIEF.md).
  It maps the existing `player:move` and `player:skill:trigger` wire events
  into the deterministic core and lists the unchanged `combat` and
  `encounter-variety` acceptance matrix.
- The committed N2 server intentionally fails those two scenarios at the
  N2 boundary (18 monsters instead of the combat scenario's minimum 20, and
  no authored melee/ranged/buffer roles). The raw negative transcript is
  preserved in the TASK-0044 captures; no assertion was weakened and no N2
  source was changed.
- This is a handoff and evidence checkpoint, not an N3 claim. Native combat
  implementation must wait for Fable to issue a READY N3 task/spec.
- The requested authority choice and task issuance are tracked in
  [`QUESTION-0009`](../../orchestration/questions/QUESTION-0009-native-n3-authority-bridge.md);
  no source workaround is authorized while it remains open.

## 2026-08-17 — Current coordinator evidence refresh

- The disposable combined parity candidate `codex/integration-parity-candidate-v3`
  (`3636b729`) applies the complete TASK-0043 correction chain and TASK-0044
  `d476788` directly onto coordinator tip `9fea5668`.
- That candidate passes browser unit `122/779`, browser playtest `31/31`, the
  native denylist/core/networking/client gate, and the unchanged native attach
  matrix `quickstart`, `single-session`, `movement`, `zones` at `4/4`.
- TASK-0043 is now accepted and integrated at `1f470e3` on program tip
  `50ca60ad`; Fable's architect review `1f833081` records the default-mode
  31/31 gate and loopback-bind guidance. TASK-0044 is now accepted and
  integrated at `5b84f51e` on program tip `71b6b207`; review `5b2ee5b6`
  records the personal 4/4 rerun and accepts the documented N3 stubs.
- A dependency-complete rerun of that exact staged timing correction was
  recorded at coordinator commit `9e5d9fd8`: a fresh worktree installed
  dependencies with normal install scripts, then passed browser unit `122/779`
  and `PLAYTEST_PORT=6538 npm run playtest` at `31/31` (`loadMode:false`,
  p99/max event-loop lag `32.178/109.642ms`). This strengthens the evidence
  package and is retained as coordinator provenance for the accepted
  correction.
- The WIZARD seam rerun is recorded at coordinator commit `6cb7b366`:
  Orbs, Brands & Bonds/inventory, and Cartographer/map tests remain `73/73`;
  Verdigris Splash remains intentionally presentation/reference-only.

## 2026-08-17 — Current-tip N2 candidate refresh

- The pre-integration coordinator tip `27db1611` is intentionally still N1:
  its unchanged native attach baseline is 2/4 (`quickstart` and
  `single-session` pass; `movement` and `zones` time out).
- Disposable candidate `f602dab4` cherry-picks N2 worker commit `d476788`
  onto that tip and passes the native denylist/core/networking/client gate plus
  unchanged `quickstart`, `single-session`, `movement`, and `zones` at 4/4.
- This supersedes the earlier candidate reference for handoff purposes but
  accepted implementation and loopback-bind correction are integrated.

## 2026-08-18 — N3 combat parity review handoff

TASK-0045 is `REVIEW_REQUESTED` on
`codex/TASK-0045-native-protocol-n3` at `6d39565c`. The worker owns only
`native/**`; `playtest/**`, `server/**`, and `src/**` remain read-only. Native
denylist/core/networking gates, unchanged seven-scenario attach, combat unit
coverage, and the authentic telegraph-radius negative are captured. The
architect must rebuild and rerun the attach before acceptance; no N3 source is
integrated into the program branch yet.

## 2026-08-18 — Current-master playability evaluation review handoff

TASK-0046 is `REVIEW_REQUESTED` on
`codex/TASK-0046-playability-reevaluation` at `1de6e45b`, based on current
program tip `45846af7`. It owns only task-folder evidence. The report records
two approximately ten-minute arcs, first-minute page-context `window.ws.url`
proofs on disposable ports, and a new disposition/ranking. Guest produces
readable melee kills/XP/gold; the mortal-oath Chronicles arc remains blocked
at a visually present but mechanically silent opener. Architect review is
pending.

## 2026-08-20 — Server/rules parity COMPLETE (32/32 attach)

D-122 axis 1 is done: the unchanged 32-scenario playtest harness passes
against the native C++ server, verified twice consecutively on fresh
servers (PR #45, hotfix PR #46). Native gates (build + unit + session +
client scenarios) all green; session tests survive 8/8 under heavy CPU
load after the reader-thread join fix.

Load-bearing findings for successors:

- Target selection: `start_player_attack` now does a true nearest scan
  with direction-aware tie-breaks. The old scan silently locked onto
  the first spawned monster; anything combat-adjacent that "worked"
  before 08-20 may have depended on that bug.
- Session semantics mirror JS exactly (proven by probing the live JS
  server): one shared anonymous guest account; concurrent anonymous
  login REPLACES the old session; adoption rebuilds a fresh Player
  while loot/levels/bank/tree/quest-record persist; permadeath
  survives reconnect; the commission chain resets on scion admission
  only. Two quest-point counters exist on purpose: quests.questPoints
  (persistent chain record) vs top-level questPoints (live tree
  budget, resets per login).
- WebSocketServer::stop() now joins per-connection reader threads.
  Never revert to detach: a reader waking after `delete server`
  dereferences the freed object (the 08-20 Native CI segfault).
- Ops: kill servers with PowerShell Stop-Process, never Git Bash
  pkill (MSYS pkill cannot kill native Windows exes; a stale server
  produced a bogus 26/32).

Remaining axes: presentation deltas #3/#4 (surface density TASK-0078,
panels/typography unspecced), Gate B Chronicles client (TASK-0077).
