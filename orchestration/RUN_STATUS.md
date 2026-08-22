# Run status — Codex Sol PC surge sweep

## Leader-austerity override — 2026-08-21

The owner adopted event-driven leadership. `LEADER_POLICY.md` supersedes the
standing 15-minute Sol polling model: deterministic local `orchd` becomes the
always-on reconciler, while Sol/Fable wake for Tier C, conflicting evidence,
protected surfaces, owner authority, or repeated unexplained failures. Until
orchd is accepted, monitoring is reduced to an hourly short read-only safety
check. This does not release current claims, change task ownership, or authorize
the architect to implement worker work.

Automation state: the 15-minute Sol heartbeat
`verdigris-surge-supervisor` is PAUSED. Temporary local cron
`pc-fleet-emergency-monitor` is ACTIVE hourly on `gpt-5.6-luna` with minimal
reasoning and a transition-only, no-write scope. Retire it once orchd passes its
vertical-slice acceptance.

The first four-lane OpenRouter wave produced three accepted integrations,
TASK-0129 is now also accepted/integrated at worker head `b138871b`, and
TASK-0080 has a local revision commit `a0419710` pending final STATUS/REPORT
packaging and push. TASK-0083 remains a no-write invalid-base replacement, not
an incident. The accepted TASK-0129 dependency releases TASK-0135 as a
task-folder-only successor for ox-pc-d after this coordination push.

Owner-authorized expansion registers four additional OpenRouter CLI lanes,
ox-pc-f through ox-pc-i, and recycles completed c/e. Six pairwise-disjoint
task-folder architecture packets are launch-requested after this coordination
push: TASK-0112, TASK-0130, TASK-0131, TASK-0132, TASK-0133, and TASK-0134.
Valid pushed claims registered c/e/f/g/h, and all five have already pushed
REVIEW_REQUESTED heads. Lane i hit a pre-claim permission denial followed by
the known missing-`node_modules` Yorkie activation fault; after the supervisor
installed locked dependencies, its hook-valid claim reached origin at
`5b952c08` and implementation resumed.

The independent `ox-bootstrap` orchestration worker has pushed M3 head
`82de84ef` after M2 `5627916f`. The standalone repo's Tier-B review contract
still governs acceptance and this lane never counts as Verdigris capacity.

Snapshot: 2026-08-21 23:05 PDT

Sweep base: `0d40d79db80c53280bb7cfe6f42318b39dab6f4c`
(`codex/native-reconstitution`; `origin/master` remains at the previously green
`d2423873` merge tip).

GitHub: no open PRs. Latest master CI run 32441409427 passed at `d2423873`;
the coordination-only program pushes did not dispatch a new workflow.
TASK-0081 rev3 is accepted and integrated at `31d21579`.

Orchestrator: **Codex Sol**, PC architect checkout
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
are excluded from READY accounting. After five expansion claims and promotion
of TASK-0135 and i's valid claim, d then claimed TASK-0135 at `50cd286f`,
leaving 24. Four accepted-contract validator implementations are now promoted,
  restoring 28 effective READY before c/e/f/g claim them. Their four valid,
  pushed claims left 24; accepted TASK-0135 now promotes TASK-0140, leaving
  **25 effective READY + 17 successors** before d claims it and preserving the
  absolute packet floor. D-128 supersedes count-only sufficiency.

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
  (5%) owner-blocked. The immediate 25 READY board remains audit-heavy and its
  accepted audits must fan out before or at acceptance.
- Deterministic validation: `node orchestration/backlog-factory/build-manifests.mjs --check`
  passes with 2,000 unique nodes and 500 unique packets.
- Binding contracts: `BACKLOG_FACTORY.md`, `CONTENT_SCALE_MATRIX.md`, and
  D-128. Backlog production continues every sweep and never authorizes Sol to
  implement worker tasks.

## PC Ox Alpha fleet reconciliation

Shared entry: `orchestration/REENTRY-OX-ALPHA-PC.md`.

| Lane | Ports | Repository evidence | Initial route |
|---|---|---|---|
| ox-pc-a | 6620-6639 | CLAIMED `0d1898bd`; existing desktop session | TASK-0128 throughput normalization |
| ox-pc-b | 6640-6659 | revision commit `a0419710` local; recovery process packaging/pushing | TASK-0080 board sentinel |
| ox-pc-c | 6660-6679 | CLAIMED and pushed `7b24e5d3`; implementation active | TASK-0136 passive-tree validator |
| ox-pc-d | 6680-6699 | TASK-0135 accepted/integrated at `88092c97`; successor launch requested | TASK-0140 soak evidence validator |
| ox-pc-e | 6700-6719 | CLAIMED and pushed `0d175a2f`; implementation active | TASK-0137 Gate C validator |
| ox-pc-f | 6720-6739 | CLAIMED and pushed `f9458f4e`; implementation active | TASK-0138 release-proof validator |
| ox-pc-g | 6740-6759 | TASK-0139 accepted/integrated at `934863cb`; successor launch requested | TASK-0141 procedural native visual kit |
| ox-pc-h | 6760-6779 | TASK-0133 accepted/integrated at `678c7b80`; successor launch requested | TASK-0142 native client presentation slice |
| ox-pc-i | 6780-6799 | TASK-0134 accepted/integrated at `b7464d94`; successor launch requested | TASK-0143 native gameplay/runtime slice |

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
dashboard and transition-deduplicated Windows P1/P0 activation notifications.
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

Persistent Sol supervision is paused under `LEADER_POLICY.md`. The temporary
hourly Luna monitor observes transitions until orchd takes over deterministic
session reconciliation and dispatch.

### Cross-project control-plane watch (not Verdigris capacity)

`alexkorol/orchestration` PC main is clean/synced at `4ec2a9d5`. The isolated
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

- REVIEW_REQUESTED: TASK-0112, 0130, 0131, and 0132 were accepted and
  integrated at their exact heads. TASK-0133 `b44ab0ab` received one testable
  REVISE correction: native snapshot v1 remains a candidate, not the selected
  cross-estate target. TASK-0129 remains accepted at `b138871b`.
- REVISE: **TASK-0080** at reviewed head `0ab4e7a5`; revision commit
  `a0419710` exists locally but is not authoritative until pushed.
- Accepted/integrated this sweep: **TASK-0086** at `8ddfb06e`, **TASK-0105**
  at `8e6e42b3`, and **TASK-0120** at `4e0920d4`.
- Active claims: **TASK-0128 by ox-pc-a**; TASK-0080 remains assigned to
  ox-pc-b through revision; TASK-0135 is accepted/integrated at d head
  `88092c97` and releases TASK-0140 to d; TASK-0133/0134/0139 are accepted
  and integrated at `678c7b80`/`b7464d94`/`934863cb` and release the three
  game-facing successors TASK-0141/0142/0143. TASK-0136/0137/0138 remain
  valid pushed implementation claims at `7b24e5d3`/`0d175a2f`/`f9458f4e`.
  Separate project: orchestration bootstrap claim `795a9b3`, with pushed M3
  head `82de84ef` awaiting its configured Tier-B acceptance path.
- Historical TASK-0056 and legacy clone WIP are superseded/preserved, never
  resumed.
- Historical QUESTION-0007/0008/0009/0010/0011 are resolved by integrated work.
- Port 6500 remains owner-only; every worker uses only its loopback capsule.
- This owner correction explicitly authorizes pushing the architect coordination
  commit to the program branch. Workers still push only their own branches.

## Effective READY — 28 packets

Every row is dependency-free at this snapshot. Owned paths are pairwise
disjoint; initial routes do not constitute claims. First committed `STATUS.md`
wins after a fresh fetch.

| Pri | Task | Topology / job | Preferred route | Owner-visible contribution |
|---|---|---|---|---|
| P0 | TASK-0141 procedural native visual kit | INDEPENDENT / IMPLEMENTATION | ox-pc-g after coordination push | removes fallback-only presentation with deterministic vector/SVG-derived assets |
| P0 | TASK-0142 native client presentation slice | INDEPENDENT / IMPLEMENTATION | ox-pc-h after coordination push | makes the owner-facing native window read as a game, not a debug shell |
| P0 | TASK-0143 native gameplay/runtime slice | INDEPENDENT / IMPLEMENTATION | ox-pc-i after coordination push | advances a real C++ combat/extraction loop under the scenario harness |
| P1 | TASK-0140 soak evidence validator | INDEPENDENT / IMPLEMENTATION | ox-pc-d after coordination push | mechanically rejects stale, incomplete, or retry-masked soak proof |
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
| P1 | TASK-0116 animation/VFX contract audit | INDEPENDENT / BOUNDED-DESIGN | future after current claim | stages readable production motion and effects |
| P1 | TASK-0117 audio/music runtime audit | INDEPENDENT / BOUNDED-DESIGN | future after current claim | defines combat/UI/ambience/music runtime |
| P1 | TASK-0118 accessibility/options audit | INDEPENDENT / MECHANICAL | future after current claim | stages broad readable and controllable play |
| P1 | TASK-0121 owner content approval matrix | INDEPENDENT / MECHANICAL | future after current claim | batches art/lore/naming/balance/economy/content gates |
| P2 | TASK-0084 reference-capture manifest | INDEPENDENT / MECHANICAL | future after current claim | detects stale/missing visual evidence |
| P2 | TASK-0085 denylist exception audit | INDEPENDENT / MECHANICAL | future after current claim | prepares owner compatibility ruling |
| P2 | TASK-0114 renderer backend evaluation | EXPLORATORY / BOUNDED-DESIGN | future after current claim | feeds cross-platform renderer ADR |
| P2 | TASK-0094 asset provenance audit | INDEPENDENT / MECHANICAL | future after current claim | prevents unshippable assets/fonts |

## HOLD despite historical READY headers

| Task | Release condition | Reason |
|---|---|---|
| TASK-0077 native Chronicles client | TASK-0081 ACCEPTED; architect freezes exact client interfaces | Gate B event/payload contract first |
| TASK-0078 native surface density | TASK-0077 ACCEPTED/integrated | both own `native/client/**`; single-writer rule |
| TASK-0073 renderer backend evaluation | SUPERSEDED by TASK-0114 | surge replacement supplies exact evidence, acceptance, and stop contracts |
| TASK-0079 browser panel inventory | SUPERSEDED by TASK-0115 | surge replacement supplies hard-fail capture and exact gate contracts |

## Sequenced successors — 17 DRAFT

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
| TASK-0122 animation/VFX system | 0116 + renderer/asset contracts accepted |
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
