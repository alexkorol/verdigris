# Run status — Codex Sol PC surge sweep

## Leader-austerity override — 2026-08-21

The owner adopted event-driven leadership. `LEADER_POLICY.md` supersedes the
standing 15-minute Sol polling model: deterministic local `orchd` becomes the
always-on reconciler, while Sol/Fable wake for Tier C, conflicting evidence,
protected surfaces, owner authority, or repeated unexplained failures. Until
orchd is accepted, monitoring is reduced to an hourly short read-only safety
check. This does not release current claims, change task ownership, or authorize
the architect to implement worker work.

Both saved PC OpenCode sessions showed fresh streaming/tool activity during
this transition. `ox-pc-a` pushed accepted TASK-0081 rev3 `dff39173`; it was
integrated at merge `31d21579` after green Tier-C review. Its next single-lane
route is mechanical Tier-A TASK-0128 from base `31d21579` on branch
`codex/TASK-0128-accepted-throughput-normalization-ox-pc-a`.
`ox-bootstrap` has pushed orchestration M1 head
`bcd98d04` and is actively writing M2 controller/detector files. Neither lane is
stalled or dark. The orchestration repo's `ORCHD_P0.md` is the next
control-plane acceptance boundary; existing M0 review corrections still apply.

Snapshot: 2026-08-21 20:37 PDT

Sweep base: `a7e9c95f3ac6e3a65864fdfbf3183cb79ff7a9ad`
(`codex/native-reconstitution`; `origin/master` remains at the previously green
`d2423873` merge tip).

GitHub: no open PRs. Latest master CI run 32441409427 passed at `d2423873`;
the coordination-only program pushes did not dispatch a new workflow.
TASK-0081 is in final evidence `REVISE` at worker rev2 head `52a7377b`.

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
path-disjoint READY packets plus 12 concrete successors. TASK-0081's claim
consumed one packet and TASK-0128 restocked the same sweep; current board stays
at **30 effective READY + 18 successors**. D-128 supersedes count-only
sufficiency.

## Autonomous runway and factory status

- Autonomous runway: **UNKNOWN / P0 instrumentation gap**. The board has not
  yet been weighted against trailing accepted throughput by full experimental
  unit and packet type; 30+18 must not be reported as adequate runway.
- Target/warning/critical: 72h / <48h / <24h.
- Terminal graph: **2,000 validated concrete nodes** across 20 domains and all
  T1-T8 gates in `backlog-factory/generated/product-graph.nodes.jsonl`.
- Detailed reserve: **500 validated packets**: 100 DRAFT + 400 AUTO_RELEASE in
  `backlog-factory/generated/packet-reserve.jsonl`. Distant packets carry no
  immutable base and remain nonclaimable until current-tip READY promotion.
- Reserve composition: 100 each implementation, integration, presentation,
  hardening, and release; 0 pure audit/research/inventory/evaluation; 25 packets
  (5%) owner-blocked. The immediate 30 READY board remains audit-heavy and its
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
| ox-pc-a | 6620-6639 | `REVISE`: valid claim `f08131c0`; rev2 head `52a7377b` pushed 21:03 PDT and passes all gates; two wire corrections are accepted, with only exact fall-through wording, rev2 commit/provider evidence, and post-correction transcript remaining | TASK-0081 Gate B final evidence revision |

The stopped `ox-pc-b` and `ox-pc-c` tabs shared the same OpenCode project,
stopped before claims or writes, and are not Verdigris lanes, stalls, dark
capacity, or incidents. The owner reserved that capacity for separate projects;
Verdigris does not direct or monitor it. Route only one Ox Verdigris task at a
time until the owner explicitly adds independent workers.

Legacy PC clones under ChatGPT, Cursor, DeepSeek, kimi, and KimiWork are stale
and/or dirty with preserved user/worker work. They are **not** registered as Ox
clones and must not be reset, cleaned, or silently repurposed. The first
committed Ox claim registers its actual clone path and full scorecard
experimental unit.

Provisioning is complete. The only owner action before claim is to open the
exact Ox worktree as its own OpenCode project and paste: `Read
START_HERE_OX_PC_A.md completely and execute it now.` Sol does not claim or
write worker STATUS/REPORT and will not take over implementation.

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
- **Capacity accounting:** one active local Verdigris Ox lane. This was not an
  incident against stopped `ox-pc-b`/`ox-pc-c`.

Persistent supervision: Codex app heartbeat `verdigris-surge-supervisor` is
ACTIVE every 15 minutes on this architect task. It fetches/scans first,
reviews/integrates/restocks on transitions, enforces D-127/D-128, updates
scorecards, and reports meaningful state changes. At 20:16 its notification
policy was corrected from failed-runs-only to normal notifications; no owner UI
change is required for successful alarm runs.

### Cross-project control-plane watch (not Verdigris capacity)

`alexkorol/orchestration` PC main is clean/synced at `a57e21c`. The isolated
`Z:\Code\.worktrees\orchestration\ox-bootstrap` worker pushed valid claim-only
commit `795a9b3` at 20:20 on
`codex/ox-bootstrap-portable-orchestration`; only `bootstrap/CLAIM.md` changed.
Its activation claim remains valid and M0 head `b91db15` pushed at 21:05.
Independent review passes build/typecheck/lint/22 tests/CLI, but clean
`format:check` fails and three-OS CI run `32550754204` is red; M0 is `REVISE`.
The earlier no-child-process stall alarm was a false positive because the
OpenCode sidecar was still streaming. Detection now requires corroborating
session, Git/file, backoff, and elapsed evidence. This separate project never
counts toward Verdigris capacity.

### Endpoint identity watch

Both saved OpenCode session records identify the selected endpoint as
`providerID: opencode`, `modelID: x-preview-f-free`, variant `max`. That proves
the current harness-visible provider is OpenCode, not OpenRouter; any upstream
inference behind the free alias remains unknown. The bootstrap claim's
`Provider: OpenRouter` field is contradicted by this authoritative local
selection and must be corrected in its first reviewable milestone. Work may
continue and must not be discarded, but neither run may be aggregated under
OpenRouter. If the owner wants the next turn to be an OpenRouter experiment,
they must explicitly choose OpenRouter -> Ox Alpha in each OpenCode model
picker; that provider change starts a distinct experimental unit.

## Interrupts and authority

- REVIEW_REQUESTED: **none** (TASK-0081 rev2 was reviewed at `52a7377b`).
- REVISE: **TASK-0081**, final three-item evidence correction in `REVIEW.md`;
  rev2's mortal-oath and relaunch response-shape corrections are preserved.
- Active claims: **TASK-0081 by ox-pc-a** (`f08131c0`; revision takes priority).
  Separate project: orchestration bootstrap
  claim `795a9b3`, also active locally pending its first milestone commit.
- Historical TASK-0056 and legacy clone WIP are superseded/preserved, never
  resumed.
- Historical QUESTION-0007/0008/0009/0010/0011 are resolved by integrated work.
- Port 6500 remains owner-only; every worker uses only its loopback capsule.
- This owner correction explicitly authorizes pushing the architect coordination
  commit to the program branch. Workers still push only their own branches.

## Effective READY — 30 packets

Every row is dependency-free at this snapshot. Owned paths are pairwise
disjoint; initial routes do not constitute claims. First committed `STATUS.md`
wins after a fresh fetch.

| Pri | Task | Topology / job | Preferred route | Owner-visible contribution |
|---|---|---|---|---|
| P0 | TASK-0083 server lifecycle soak | INDEPENDENT / BOUNDED-DESIGN | future after current claim | guards the reader-thread crash fix |
| P0 | TASK-0086 Gate C contract audit | INDEPENDENT / MECHANICAL | future after current claim | exposes missing route-decision information |
| P0 | TASK-0080 board sentinel | INDEPENDENT / MECHANICAL | future after current claim | machine-enforces queue and collision truth |
| P0 | TASK-0097 persistence durability audit | INDEPENDENT / BOUNDED-DESIGN | future after current claim | protects House/Scion/item saves |
| P0 | TASK-0100 deterministic replay audit | INDEPENDENT / BOUNDED-DESIGN | future after current claim | makes divergences reproducible |
| P0 | TASK-0104 itemization/history audit | INDEPENDENT / BOUNDED-DESIGN | future after current claim | stages memorable history-bearing loot |
| P0 | TASK-0105 passive-tree authority audit | INDEPENDENT / MECHANICAL | future after current claim | replaces approximation with an evidence path |
| P0 | TASK-0119 onboarding/first-session audit | INDEPENDENT / BOUNDED-DESIGN | future after current claim | makes launch through first extraction legible |
| P0 | TASK-0120 release verification audit | INDEPENDENT / MECHANICAL | future after current claim | freezes clean-machine/migration/soak release proof |
| P0 | TASK-0128 accepted-throughput normalization | INDEPENDENT / MECHANICAL | ox-pc-a next only after TASK-0081 transition | makes adaptive autonomous-runway hours reproducible without guessed telemetry |
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

## Sequenced successors — 18 DRAFT

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
| TASK-0112 passive-tree scaffold | 0105 accepted; owner source or schema-only fallback |
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
