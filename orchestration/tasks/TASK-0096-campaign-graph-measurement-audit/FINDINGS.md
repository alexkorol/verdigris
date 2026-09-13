# TASK-0096 — Campaign and zone-graph measurement audit

Lane: ox-pc-bb · Model: openrouter/stealth/ox-alpha · Branch: `worker/verdigris/pc/ox-pc-bb`
Base: `d2423873c577d299b3b39c56024d1d840993c72b` (verified ancestor of audited head `f852254d`)
Scope: mechanical topology measurement only. No zone, act, reward, duration, or travel risk is
invented. Owner-only campaign choices are recorded as MISSING, never derived from names.

Machine-readable mirror: [`captures/graph.json`](captures/graph.json)
(regenerable via [`tools/build-graph.mjs`](tools/build-graph.mjs)).

## 0. Frozen invariants honored

| Invariant | Source | Status measured |
|---|---|---|
| Campaign = multizone graph across several acts, 6–30h envelope, completed once per House per season | constitution `docs/product/VERDIGRIS_CONSTITUTION.md:74-84` | NOT implemented: act count/duration/spine are MISSING (§7); today's code holds a 2-node core spine plus an endless session web |
| Fast-travel/town-portal path with explicit risk model | constitution `VERDIGRIS_CONSTITUTION.md:113-116`; `OPEN_DECISIONS.md:19` (OD-012) | Absent; outbound return paths only (§4) |
| Simulation authority (commands in, events out), headless fixed step | constitution `VERDIGRIS_CONSTITUTION.md:157-168` | Honored on both measured paths; protocol world-web is session-scoped presentation of the same pattern |
| Do not invent campaign content | SPEC `SPEC.md:27-29` | Honored: every node/edge cites path:line at the audited head |

## 1. Central structural fact: two graphs, two books

- **Graph A — headless core route table** (`Simulation`, House-persisted):
  seeded in the constructor (`native/src/core.cpp:195-200`); node struct
  `RouteNode {id, parent_id, children[], optional}`
  (`native/include/verdigris/core.hpp:163-168`).
- **Graph B — protocol world-web** (`ProtocolSession`, remote play): a
  deterministic per-house road chart generated lazily from FNV-1a hashes
  (`native/src/networking.cpp:716-824`); progress lives in the session-scoped
  `cleared_nodes_` set (`native/include/verdigris/networking.hpp:197-205`).

They reconcile **only** at two ids: `world:zone:enter` maps any node id onto
`Command::enter("route:" + nodeId)` in the core before routing to
`enter_road_node` (`native/src/networking.cpp:2386`). The core accepts only its
two seeded routes (`route:tin:1:0`, `route:tin:2:0`) and silently rejects every
other web id (`core.cpp:540-541`); all salt/chalk/copper/deeper-tin ids are
no-ops in Graph A while fully live in Graph B.

## 2. Graph A — core route table (complete)

| Node | Parent | Children | Optional | Initially unlocked | Content | Citations |
|---|---|---|---|---|---|---|
| `route:tin:1:0` | — | `route:tin:2:0` | no | yes | level-1 Warden pack: entry warden + elite + normal pack mate converging one telegraph window after the first falls | `core.cpp:196,607-630`; tests `core_tests.cpp:23-65` |
| `route:tin:2:0` | `route:tin:1:0` | — | no | no | single level-2 elite identity ("deep" route) | `core.cpp:197,608-615` |
| `branch:ash` | `route:tin:1:0` | — | yes | no | interaction grant, not an instance: `Interact("branch:ash")` adds `"ash"` to `House.specializations` | `core.cpp:198,461-467` |

Gates: enter requires `route_unlocked && scion.alive` (`core.cpp:540-541`);
the branch requires `route_cleared("route:tin:1:0")` and no prior grant
(`core.cpp:461-467`); extraction requires active instance + alive scion +
manhattan distance ≤ `kExtractionRange` to `extraction_point` (defaults to the
spawn tile `{0,0}`) (`core.cpp:901-925`; `core.hpp:67-68,306`).

Measured rule worth flagging: `campaign_complete` flips true inside
`clear_route_and_unlock_children` on the **first clear of any route**
(`core.cpp:802-806`) — confirmed by tests asserting completion right after the
root pack dies (`native/tests/core_tests.cpp:1138-1140`) and a founding-equivalent
legend (`core_tests.cpp:1609-1610`). This is placeholder pacing, not an authored
campaign.

Return paths: extraction banks carried value and retires the instance;
uncollected floor value is discarded on leave (`core.cpp:563-605,901-920`).
Death converts carried items/trophies into House recovery candidates and
retires (`core.cpp:866-899`). Route progress survives Scion death
(House-level ownership, `core_tests.cpp:1416-1424`) and replays deterministically
(`core_tests.cpp:1155-1158,1326-1330`).

## 3. Graph B — protocol world-web

Static skeleton (`native/src/networking.cpp:719-729,824`):

| Road | Direction | Blurb (verbatim) | Template/layout pairs | Town gate tile |
|---|---|---|---|---|
| tin — "The Tin Road" | north | "North into the old quarry country." | dungeon/warren, dungeon/gauntlet, wilds/clearings, dungeon/clearings | {37,94} |
| salt — "The Salt Road" | east | "East through the fens." | marsh/clearings, grove/clearings, marsh/gauntlet, grove/warren | {64,114} |
| chalk — "The Chalk Road" | south | "South over the downs and their graves." | crypt/warren, crypt/gauntlet, wilds/clearings, crypt/clearings | {37,138} |
| copper — "The Copper Road" | west | "West into the burnt hills." | crypt/warren, wilds/clearings, crypt/gauntlet, wilds/warren | {12,115} |

Determinism (`networking.cpp:746-804`): node ids are `<road>:<tier>:<index>`;
FNV-1a 32-bit hash of `house|road|tier|index` picks the template/layout pair
(`h%4`), the procedural name (`table[(h>>4)%6] + table[(h>>8)%6]`, deduped with
a `" Deep"` suffix within the road) and hence `warden_name = "Warden of <name>"`.
Tier widths recurse `width(1)=1; width(t)=clamp(width(t-1)+{-1,0,+1},1,3)`
seeded by `hash(house|road|tier|"width")`. Every node above tier 1 has exactly
one parent (`min(prev_width-1, index*prev_width/width)`). Charts are therefore
per-House stable but not portable between Houses.

Chart surface (`emit_chart_screen`, `networking.cpp:1482-1515`): frontier =
max cleared tier on that road + 1; statuses `cleared | open | barred`
(open ⇔ tier 1 or parent cleared). Opened by standing on a town gate tile
(`check_road_gates`, `networking.cpp:1588-1607`, wired into `player:move` /
`dev:teleport` handlers at `networking.cpp:2388-2389`).

Traversal gates:

- Entry waymark = the instance's `stairsUp` tile; for node instances it returns
  to the Crossroads from ANY tier (`stairs_up_returns_to_town_`,
  `networking.cpp:1541`; `core.cpp:1581-1585`).
- Onward gate = the `stairsDown` tile; blocked while the node Warden lives —
  "No road holds past a living Warden." (`networking.cpp:1539,1591-1599`;
  `core.cpp:1576-1577`).
- Warden death (combat kill or `dev:clear-floor`) inserts the node into
  `cleared_nodes_`, unblocks the gate, announces "The Warden of X is down.
  The road runs on." (`networking.cpp:2134-2143,2541-2545`).
- Re-entry of a cleared node suppresses spawning: zero monsters,
  `metadata.wardenDead=true` ("dead stays dead", session-scoped)
  (`networking.cpp:1533-1539,871`; scenario proof
  `playtest/scenarios/world-web.mjs:100-110`).

Instance anatomy: every node instance is one 40×40 floor with fixed stairs
{5,20}/{34,20}, spawn {6,20}, 20 monsters, the last placed monster being the
theme boss renamed to the node warden via `boss_name_override`
(`core.cpp:1458-1474,1759-1773`; `networking.cpp:1534`). Theme bosses outside
the web: The Elder Oak (grove), The Pale Sovereign (crypt), Alpha of the Wilds
(wilds), The Rotfather (marsh), Warden of the Deep (dungeon default)
(`core.cpp:1765-1769`).

## 4. Quest overlay (adjacent, not part of either graph)

Four ordered commissions (`kQuestChain`, `networking.cpp:691-705`) reference
adventure-zone identities (theme+layout pairs, `core.cpp:1427-1438`), named
elites, and depths: aldwyns-charge → proof-of-temper → the-pale-crown (Weir
Crypt seal + depth-2 descent) → rot-in-the-reeds (Marsh of Reeds +
return-surface). End-to-end evidence: `playtest/scenarios/quest.mjs:51-356`.
Quest delves use generic solo instances keyed by theme+layout
(`networking.cpp:2386-2387`), sharing **no progression state** with the world
web.

Fast-travel seams: none inbound. Outbound only: entry waymark → Crossroads;
`player:extract` / `party:returnToTown` → town with backpack+wear drained into
the session House store (`networking.cpp:2188-2199,1011-1044,2552-2562`).
Risk model: OD-012 open decision (`docs/product/OPEN_DECISIONS.md:19`).

## 5. Deterministic IDs and House-owned unlocks

- Core ids are literal strings recorded verbatim as legend subjects
  (`core_tests.cpp:1604-1613`). Unlocks/clears/specializations/campaign flag
  are House fields serialized to saves (`core.hpp:172-191`;
  `core.cpp:1236-1257,1315-1340`) and survive Scion death.
- Protocol ids are formulas validated by `parse_node_id`
  (`networking.cpp:805-816`); attributes derive from the house-scoped hash.
  But `cleared_nodes_` lives on `ProtocolSession`: charts are generated *per*
  `active_house_id_`, while cleared progress itself evaporates with the
  session (`networking.hpp:197-205`). No native House-persisted road chart
  exists.

## 6. Traversals (measured)

Core graph:

- **Shortest start→campaign_complete**: construct → `enter("route:tin:1:0")` →
  defeat the pack → done. 1 node visit; completion fires during the clear
  (`core.cpp:787-807`).
- **Longest defined simple traversal**: clear tin root → interact
  `branch:ash` → clear `route:tin:2:0` → extract. 3 stops, 2 clears; terminal —
  nothing deeper exists (`core.cpp:195-200`).

Protocol web (worked example for house id `house-web-audit-example`, tiers 1–3,
computed by replaying the published algorithm — see
[`captures/graph.json`](captures/graph.json) `graphs.protocol_world_web.determinism.worked_example`):

| Road | Tier widths (1..3) | Nodes | Max fan-out |
|---|---|---|---|
| tin | 1, 1, 2 | 4 | 2 |
| salt | 1, 1, 1 | 3 | 1 |
| chalk | 1, 1, 2 | 4 | 2 |
| copper | 1, 1, 2 | 4 | 2 |

- Minimum expedition legs town→deepest tier N = N (one stage per leg:
  descending clears `current_child_id_`, so further descent needs a fresh
  chart entry from the Crossroads, `networking.cpp:2388-2389` vs `1528`).
- Growth bound: tiers deepen indefinitely (lazy generation; frontier grows
  with clears). No terminal node exists
  (`networking.cpp:1486-1491`; design lineage
  `docs/crossroads-world-web.md:90-104`).
- **No duration measurements exist anywhere in code**; the constitutional
  6–30h figure stays an intent sentence (§7).

## 7. Negative control — MISSING campaign fields preserved

Per SPEC ("preserve at least one MISSING campaign field rather than deriving
it from a route name"), these remain MISSING in
[`captures/graph.json`](captures/graph.json) `missing_authoring[]`:

1. `campaign.act_count` — constitution says "several acts"
   (`VERDIGRIS_CONSTITUTION.md:79`); zero act structure exists in code. The
   four road NAMES were deliberately **not** promoted into acts.
2. `campaign.target_duration_hours` — the 6–30h line
   (`VERDIGRIS_CONSTITUTION.md:76`) is quoted as intent, never converted into
   a measured estimate from node counts.
3. `campaign.mandatory_spine`, `campaign.branch_density_targets`,
   `fast_travel.risk_model` (OD-012), `repeatable_endgame.definition`,
   `world_web.persistence_across_sessions`,
   `authored_zone_names_and_lore` — all MISSING with blockers recorded.

Stop-rule check: no owner-only campaign choice was made; measurement continued
around every pending decision.

## 8. Delta map (measured state → constitutional target)

| Seam | Today | Constitution target | Gap class |
|---|---|---|---|
| Campaign | 2-node core spine completing on first clear; endless session web without completion state | multizone multi-act graph, once per House per season, later Scions skip mandatory route | owner-authored content + integration mechanics |
| Optional branches | one specialization interaction (`branch:ash`); web width branching carries no semantics | branches grant specializations, item access, knowledge, league mechanics, routes, starts | owner-authored rewards; mechanical seat exists (`RouteNode.optional`, chart rows) |
| Repeatable endgame | empty re-entry of cleared nodes; featureless solo instances; unbounded web depth | player-chosen areas/goals/mechanics/item/trophy/build targets | owner-authored; frontier plumbing already supports depth |
| Fast travel | outbound return seams only | fast travel/town portal with explicit risk model | OD-012 owner decision; command/presentation seam can attach to `return_to_surface` (`core.cpp:1592-1595`) |

## 9. Measurement findings requiring successor attention (recorded, not repaired)

All rows live outside owned paths; each is cited and parked:

1. **Single onward gate exposure**: only `child_ids.front()` becomes a
   `zoneGates` entry even when several siblings exist
   (`networking.cpp:1528,859-872`); sibling visits force Crossroads round-trips.
2. **No unlock authorization on `world:zone:enter`**: any well-formed node id
   is entered regardless of barred status; enforcement lives in chart
   presentation + living-Warden gate hold along an active chain
   (`networking.cpp:1517-1546,1591-1607`).
3. **Dual bookkeeping seam**: Graph A/B reconcile only at the two seeded tin
   ids (`networking.cpp:2386`; §1).
4. **Placeholder completion rule**: `campaign_complete` after any first clear
   (`core.cpp:802-806`).
5. **Session-scoped web persistence**: cleared wardens vanish on logout
   (`networking.hpp:197-205`).

## 10. Tests inventory (graph evidence)

| Test | Path | Proves |
|---|---|---|
| pack-clear progression | `native/tests/core_tests.cpp:1095-1140` | first pack kill clears nothing; last kill clears route + completes campaign |
| `test_campaign_and_seasonal_extension` | `core_tests.cpp:1570-1583` | House-level ownership; branch grants specialization; clearing unlocks child |
| `test_legends_cover_unlocks_and_campaign_milestone` | `core_tests.cpp:1600-1615` | stable route/branch ids recorded as legend subjects; founding-equivalent campaign milestone |
| route progress vs death | `core_tests.cpp:1400-1424` | House route state survives Scion death |
| cross-route re-entry | `core_tests.cpp:996-1014` | entering `route:tin:2:0` switches instances; re-entry allowed |
| replay determinism | `core_tests.cpp:1150-1170,1250-1345` | cleared/unlocked equality byte-stable under replay |
| zone-enter lifecycle | `native/tests/networking_tests.cpp:43-55` | `world:zone:enter {"nodeId":"tin:1:0"}` produces instance scene |
| instance stairs | `networking_tests.cpp:124-171` | both stairs exist; entry stairs return to town |
| extract/stairs banking | `networking_tests.cpp:333-385` | `player:extract` and stairs-up converge on the House store |
| local journey extract | `native/tests/session_tests.cpp:354-392` | walking onto stairs-up completes extraction locally |
| world-web contract (browser wire) | `playtest/scenarios/world-web.mjs:14-113` | gate→chart→travel→living-Warden hold→clear→descend→chart states→cleared linger-free re-entry |
| quest overlay | `playtest/scenarios/quest.mjs:51-356` | four commissions incl. Weir Crypt seal descent and return-surface |

## 11. Successor tool contract

- Regenerate evidence: `node orchestration/tasks/TASK-0096-campaign-graph-measurement-audit/tools/build-graph.mjs <head-sha>`
  (byte-deterministic; worked example pinned by `EXAMPLE_HOUSE` +
  `EXAMPLE_MAX_TIER` constants).
- Validate: the SPEC JSON gate (`node -e …'campaign graph: PASS'`).
- Consume/extend rules are embedded in
  [`captures/graph.json`](captures/graph.json) `successor_tool_contract`:
  cite path:line for every new row; author campaign layers as NEW data tables
  rather than encoding them into names; natural seats identified for acts
  (constructor-seeded route table), stages (chart row payloads), and
  centralized unlock authorization (`world:zone:enter`).

## 12. Evidence index

Acceptance-command transcripts (literal outputs + exit codes):
[`REPORT.md`](REPORT.md). Machine-readable audit:
[`captures/graph.json`](captures/graph.json). All citations are relative to
the repository root at claim commit `f852254d` on
`worker/verdigris/pc/ox-pc-bb`.
