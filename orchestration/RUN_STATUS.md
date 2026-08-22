# Run status — PC Verdigris overnight product wave

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
| ox-pc-r | 6960-6979 | TASK-0148 independent claim `837a412f`; workflow guard `3119a08e`; sole OpenCode PID 17640 with dirty owned implementation preserved | ordinary-play recovery active; no handoff accepted |
| ox-pc-s | 6980-6999 | TASK-0116 corrected audit head `5b007e7e` independently ACCEPTED and integrated through `8eb95893` | audit complete; TASK-0122 successor routed |
| ox-pc-t | 7000-7019 | TASK-0117 frozen head `5a7c22cb` independently ACCEPTED and integrated at `7052feca` | handoff complete; procedural-audio successor sequenced behind current client-main owner |
| ox-pc-u | 7020-7039 | TASK-0119 frozen head `4104e0c8` independently ACCEPTED and integrated at `a5f4133e` | handoff complete; executable onboarding successor ready to route |
| ox-pc-v | 7040-7059 | TASK-0153 final worker head `30ab95fd`; Esc/first-session fix shipped to protected master `230a8adf` with green CI | complete/available |
| ox-pc-w | 7080-7099 | TASK-0154 clean-runner portability head `2dc4bfb2`; protected hotfix master `4e55f4f9` green | complete/available |
| ox-pc-x | 7060-7079 | TASK-0122 valid claim `fdbbdca6`; one same-session implementation recovery at PID 18144 | client-only VFX Phase A active |
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

## Effective READY — 25 packets

Every row is dependency-free at this snapshot. Owned paths are pairwise
disjoint; initial routes do not constitute claims. First committed `STATUS.md`
wins after a fresh fetch.

| Pri | Task | Topology / job | Preferred route | Owner-visible contribution |
|---|---|---|---|---|
| P0 | TASK-0097 persistence durability audit | INDEPENDENT / BOUNDED-DESIGN | future after current claim | protects House/Scion/item saves |
| P0 | TASK-0100 deterministic replay audit | INDEPENDENT / BOUNDED-DESIGN | future after current claim | makes divergences reproducible |
| P0 | TASK-0104 itemization/history audit | INDEPENDENT / BOUNDED-DESIGN | future after current claim | stages memorable history-bearing loot |
| P0 | TASK-0119 onboarding/first-session audit | INDEPENDENT / BOUNDED-DESIGN | future after current claim | makes launch through first extraction legible |
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
| P1 | TASK-0117 audio/music runtime audit | INDEPENDENT / BOUNDED-DESIGN | future after current claim | defines combat/UI/ambience/music runtime |
| P1 | TASK-0118 accessibility/options audit | INDEPENDENT / MECHANICAL | future after current claim | stages broad readable and controllable play |
| P1 | TASK-0121 owner content approval matrix | INDEPENDENT / MECHANICAL | future after current claim | batches art/lore/naming/balance/economy/content gates |
| P2 | TASK-0084 reference-capture manifest | INDEPENDENT / MECHANICAL | future after current claim | detects stale/missing visual evidence |
| P2 | TASK-0085 denylist exception audit | INDEPENDENT / MECHANICAL | future after current claim | prepares owner compatibility ruling |
| P2 | TASK-0114 renderer backend evaluation | EXPLORATORY / BOUNDED-DESIGN | future after current claim | feeds cross-platform renderer ADR |
| P2 | TASK-0094 asset provenance audit | INDEPENDENT / MECHANICAL | future after current claim | prevents unshippable assets/fonts |
| P1 | TASK-0149 native owner-launch resilience | INDEPENDENT / IMPLEMENTATION | future clean lane | makes the real client/server launch fail-fast and orphan-free |
| P1 | TASK-0150 native clean-build convergence | INDEPENDENT / IMPLEMENTATION | future clean lane | proves the complete C++ build from a disposable directory |
| P1 | TASK-0151 native content schema seed | INDEPENDENT / IMPLEMENTATION | future clean lane | creates a deterministic content-neutral authoring seam |
| P1 | TASK-0152 native density benchmark evidence | INDEPENDENT / IMPLEMENTATION | future clean lane | measures encounter/presentation headroom reproducibly |
| P1 | TASK-0136 passive-tree contract validator | INDEPENDENT / MECHANICAL | fresh worktree only; quarantined lanes forbidden | fail-closes counter confusion without canonizing content |
| P1 | TASK-0122 native animation/VFX Phase A | INDEPENDENT / IMPLEMENTATION | ox-pc-x, ports 7060-7079 | distinct critical/spawn/lifecycle beats with real captures |

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
| TASK-0108 combat-depth wave | 0101 accepted; scaffold and D-114 table ready |
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
