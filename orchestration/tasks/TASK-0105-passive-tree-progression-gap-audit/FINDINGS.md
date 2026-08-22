# TASK-0105 — Passive-tree and progression authority gap audit (FINDINGS)

Worker: `ox-pc-d` · Base: `42718fbc4340589e606fff94a6eaa3dfbd03ad1c` ·
Branch: `codex/TASK-0105-passive-tree-progression-gap-audit-ox-pc-d` ·
Read-only audit; machine-readable companion:
`captures/progression-matrix.json`.

## Executive summary

Progression authority for the Verdigris passive tree is **split**. The
browser stack owns an authoritative, server-side rebuild of every
allocation against a generated geometric graph
(`server/core/passives/verdigris-authority.js`), with unit specs and
hard-fail playtest scenarios pinning it. The native server currently
ships three seams instead of that authority:

1. an **explicitly non-authoritative +2/attribute approximation**
   (`native/src/networking.cpp:818-821`, `1092-1119`) — recorded here as
   the SPEC's negative control;
2. an **unvalidated save path** — `handle_skilltree_save`
   (`networking.cpp:1157-1166`) stores client snapshots verbatim with no
   schemaVersion/budget/adjacency checks, exactly the trust the browser
   authority removed ("snapshots are no longer trusted save blobs",
   `verdigris-authority.js:24`);
3. **two quest-point counters whose increment sources, caps, and reset
   rules diverge from the browser** (row QP-2).

The "271-node gap" is real but stale: prose still says nine-ring/271
nodes/123 points (`docs/vision.md:19-23`,
`verdigris-authority.js:24`, native stub note), while the shipped engine
generates **331 main-lattice nodes + 34 subtree nodes = 365 total,
974 conduits**, under a 140 = 117 levels + 23 quests economy
(`verdigris-skill-tree.js`). Verified by executing the engine in this
audit. No content was invented and no balance proposed; topology,
node content, and balance remain owner-only (OI-004).

## Frozen invariants honored

- **F-1 Two counters, distinct persistence.** `quests.questPoints`
  (persistent commission-chain record) vs top-level live tree budget are
  kept separate throughout (`docs/rebuild/HANDOFF.md:254-256`,
  `native/include/verdigris/networking.hpp:174-177`).
- **F-2 The +2 approximation is NOT ratified.** Marked
  `approximation / negativeControl` in the matrix; see row AT-2.
- **F-3 Owner-only content.** No topology, node, or balance proposals;
  successor stays scaffold-first.

## Matrix (authoritative vs approximation vs unruled)

| Row | Area | Status | Anchor evidence |
|---|---|---|---|
| LV-1 | Levels & XP curve | authoritative | native `xp_for_level`/`level_from_xp` mirror shared curve (`networking.cpp:1170-1185`); kill XP + level-up `networking.cpp:2045-2060`; session-arc.mjs |
| LV-2 | Level → point budget | authoritative | 140/117/23 constants (`verdigris-skill-tree.js:1-11`; `verdigris-authority.js:10-19`; native `networking.cpp:1125-1126`) |
| AT-1 | Base attributes 10/10/10 | authoritative | `core.cpp:68-84`; `stats-manager.js` base sources |
| AT-2 | Tree → attributes | **approximation (negative control)** | native +2/axis walk `networking.cpp:1092-1119` + STUB NOTE `818-821`; authoritative browser `computeStats()` `verdigris-geometric-tree.js:994-1050`; build-divergence.mjs greens only against browser |
| QP-1 | Chain-record counter | authoritative | `quest-service.js:142-156`; native `quest_points_` `networking.cpp:1373,1082`; survives relogin (quest.mjs:348) |
| QP-2 | Live-budget counter | **approximation/divergent** | browser first-goal-fed, persisted (`first-goal.js:104`, `player.js:57`); native login-reset, commission+goal fed, cap 12 on goal (`networking.cpp:553,1374,1597`) |
| AL-1 | Allocation command/wire | authoritative | `player:skilltree:save {snapshot}` pane→socket→handler; harness `saveSkillTree` |
| AL-2 | Allocation validation | **approximation (negative control)** | browser rebuild+validate `verdigris-authority.js:28-111` (+ sanitise caps `socket-events/index.js:64-85`); native stores raw `networking.cpp:1157-1166` |
| HX-1 | Hex projection | authoritative | axial ids `q,r`; INT `{q:1,r:0}` / STR `{-1,1}` / DEX `{0,-1}`; pixel rotation `toPixel` (`verdigris-geometric-tree.js:101-110,254-265`); native axis scores match axes, magnitude does not |
| GR-1 | "271-node" reference | unruled (stale prose) | executed check: 331 + 34 = 365 nodes, 974 conduits, schemaVersion 2; vision.md/authority-comment/native stub say 271 |
| PR-1 | Persistence/reset rules | authoritative | guest-save-store/SQLite persist tree+questPoints; pre-v2 one-time refund (`verdigris-authority.js:121-128`); native login vs scion-admission reset boundary `networking.cpp:540-579,2632-2653`; permadeath preserved |
| NP-1 | Network payloads | authoritative | login block, dev:state snapshot, `quest:update`, `player:skilltree:update` fields enumerated in matrix |
| CP-1 | Client presentation | unruled | browser SVG pane + Stats.vue aggregation exist; native client renders no tree/attributes surface; presentation parity unspecced |
| TS-1 | Tests | authoritative (browser) / gap (native) | 3 unit specs + 7 playtest scenarios + critic metric vs **zero** native progression tests |

Full per-row citations (file:line, test names, executed-check output)
live in `captures/progression-matrix.json`.

## Key findings

1. **Native trusts what the browser refuses to.** Any client can hand
   the native session an overspent or disconnected tree; the native
   session stores it and later walks it for attributes and spent math.
   The browser rejects each such class with a specific reason string.
2. **QP-2 needs a ruling, not a port.** Browser top-level
   `questPoints`: first-goal-fed, capped 23, persisted. Native live
   counter: commission- AND goal-fed, goal-capped at 12, wiped at every
   login. Both feed the same earned formula. Which semantics are
   canonical across relogins is not frozen anywhere; TASK-0112 must not
   guess.
3. **Stale numbers are load-bearing in comments.** The 271 figure and
   the 123-point economy survive in `vision.md`, the authority-module
   comment, and the native stub note; the executable truth is
   ten-ring/365 nodes/140 points. OI-004's versioned-graph requirement
   exists precisely so prose counts stop mattering.
4. **Native combat already consumes the approximation**
   (`networking.cpp:2014-2021`), so any future authority swap changes
   damage outputs by design; the successor must call this out as an
   intentional behavior change, not a silent retune.

## Scaffold-first successor proposal (TASK-0112 dependency)

Port the deterministic engine headless into `native/` before any
content: graph generation, budget math, allocation validation, and both
counters exposed explicitly in session state; parity-test native
validation against `resolveVerdigrisTree` outputs on shared fixtures;
keep authored seats/effects as data pending OI-004; replace the +2 walk
behind a flag and delete it once green; UI last. Guardrails: no content
in code, no balance embedded in renderers, F-1 preserved, constants
unchanged without owner action.

## Stop condition respected

Per SPEC ("Stop before tree/content/balance choices"), this audit stops
at the boundary above: persistence, wire, and UI contracts are isolated
as rows/evidence, and every open product decision is routed to
OI-004/TASK-0112 rather than decided here.
