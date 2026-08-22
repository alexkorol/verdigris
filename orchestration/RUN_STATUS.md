# Run status — Codex Sol PC surge sweep

Snapshot: 2026-08-21 19:58 PDT

Sweep base: `7f2716911e0136ad4c89d552f6ca0c33fd185eb4`
(`codex/native-reconstitution`; `origin/master` remains at the previously green
`d2423873` merge tip).

GitHub: no open PRs. Latest master CI run 32441409427 passed at `d2423873`;
the coordination-only `7f271691` push did not dispatch a new workflow. No
current REVIEW_REQUESTED or REVISE transition exists.

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

Surge floor: at least 24 effective, dependency-free, pairwise path-disjoint
READY packets plus 12 concrete successors. Current board: **24 READY + 12
successors**.

## PC Ox Alpha fleet reconciliation

Shared entry: `orchestration/REENTRY-OX-ALPHA-PC.md`.

| Lane | Ports | Repository evidence | Initial route |
|---|---|---|---|
| ox-pc-a | 6620-6639 | PROVISIONED, UNCLAIMED: `Z:\Code\.worktrees\verdigris\ox-pc-a`, branch `codex/TASK-0081-gate-b-wire-contract-ox-pc-a`, base `7f271691`; local START packet ignored | TASK-0081 Gate B wire freeze |

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

Persistent supervision: Codex app heartbeat `verdigris-surge-supervisor` is
ACTIVE every 15 minutes on this architect task. It fetches/scans first,
reviews/integrates/restocks on transitions, updates scorecards, and alerts only
on meaningful state changes.

## Interrupts and authority

- REVIEW_REQUESTED: **none**.
- REVISE: **none**.
- Active claims: **none**.
- Historical TASK-0056 and legacy clone WIP are superseded/preserved, never
  resumed.
- Historical QUESTION-0007/0008/0009/0010/0011 are resolved by integrated work.
- Port 6500 remains owner-only; every worker uses only its loopback capsule.
- This owner correction explicitly authorizes pushing the architect coordination
  commit to the program branch. Workers still push only their own branches.

## Effective READY — 24 packets

Every row is dependency-free at this snapshot. Owned paths are pairwise
disjoint; initial routes do not constitute claims. First committed `STATUS.md`
wins after a fresh fetch.

| Pri | Task | Topology / job | Preferred route | Owner-visible contribution |
|---|---|---|---|---|
| P0 | TASK-0081 Gate B wire-contract freeze | INDEPENDENT / MECHANICAL | ox-pc-a first | safely unlocks Chronicles client journey |
| P0 | TASK-0083 server lifecycle soak | INDEPENDENT / BOUNDED-DESIGN | future after current claim | guards the reader-thread crash fix |
| P0 | TASK-0086 Gate C contract audit | INDEPENDENT / MECHANICAL | future after current claim | exposes missing route-decision information |
| P0 | TASK-0080 board sentinel | INDEPENDENT / MECHANICAL | future after current claim | machine-enforces queue and collision truth |
| P0 | TASK-0097 persistence durability audit | INDEPENDENT / BOUNDED-DESIGN | future after current claim | protects House/Scion/item saves |
| P0 | TASK-0100 deterministic replay audit | INDEPENDENT / BOUNDED-DESIGN | future after current claim | makes divergences reproducible |
| P0 | TASK-0104 itemization/history audit | INDEPENDENT / BOUNDED-DESIGN | future after current claim | stages memorable history-bearing loot |
| P0 | TASK-0105 passive-tree authority audit | INDEPENDENT / MECHANICAL | future after current claim | replaces approximation with an evidence path |
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

## Sequenced successors — 12 DRAFT

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
