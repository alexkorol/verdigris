#!/usr/bin/env node
/**
 * TASK-0096 evidence generator.
 *
 * Emits captures/graph.json: the measured campaign/zone-graph state of the
 * Verdigris native workspace at the audited head.
 *
 * The `worked_example` section is produced by re-executing the exact
 * deterministic world-web algorithm published at native/src/networking.cpp
 * lines 746-804 (constants copied verbatim: FNV-1a 32-bit seed 2166136261,
 * multiplier 16777619, name tables, width recursion, parent assignment).
 * Re-running this script with the same arguments reproduces byte-identical
 * output. Nothing here invents content: every field cites source lines that
 * exist at the audited head, and owner-only campaign decisions are recorded
 * as MISSING rather than derived.
 *
 * Usage: node tools/build-graph.mjs <audited-head-sha> > ../captures/graph.json
 */
import { writeFileSync, mkdirSync } from 'node:fs';
import { dirname, join } from 'node:path';
import { fileURLToPath } from 'node:url';

const head = process.argv[2];
if (!head || !/^[0-9a-f]{7,40}$/.test(head)) {
  console.error('usage: node tools/build-graph.mjs <audited-head-sha>');
  process.exit(2);
}

const BASE_COMMIT = 'd2423873c577d299b3b39c56024d1d840993c72b';

// ---- verbatim port of native/src/networking.cpp:719-741 --------------------
const ROADS = [
  { id: 'tin',    name: 'The Tin Road',   direction: 'north', blurb: 'North into the old quarry country.',
    pairs: [['dungeon','warren'], ['dungeon','gauntlet'], ['wilds','clearings'], ['dungeon','clearings']] },
  { id: 'salt',   name: 'The Salt Road',  direction: 'east',  blurb: 'East through the fens.',
    pairs: [['marsh','clearings'], ['grove','clearings'], ['marsh','gauntlet'], ['grove','warren']] },
  { id: 'chalk',  name: 'The Chalk Road', direction: 'south', blurb: 'South over the downs and their graves.',
    pairs: [['crypt','warren'], ['crypt','gauntlet'], ['wilds','clearings'], ['crypt','clearings']] },
  { id: 'copper', name: 'The Copper Road',direction: 'west',  blurb: 'West into the burnt hills.',
    pairs: [['crypt','warren'], ['wilds','clearings'], ['crypt','gauntlet'], ['wilds','warren']] },
];
const FIRSTS = [
  ['Hoar','Grey','Whet','Stone','Cold','Scree'],
  ['Eel','Sedge','Rush','Weir','Mere','Fen'],
  ['Barrow','Chalk','Bone','Lych','Grave','Dust'],
  ['Ash','Cinder','Ember','Slag','Copper','Forge'],
];
const SECONDS = [
  ['fell','moor','delf','gate','cleft','howe'],
  ['fen','mere','carr','weir','holm','hythe'],
  ['down','barrow','field','kirk','vault','howe'],
  ['hill','works','kiln','heath','brink','reach'],
];
const ROAD_GATES = [
  { road: 'tin', tile: { x: 37, y: 94 } },
  { road: 'salt', tile: { x: 64, y: 114 } },
  { road: 'chalk', tile: { x: 37, y: 138 } },
  { road: 'copper', tile: { x: 12, y: 115 } },
];

// ---- verbatim port of native/src/networking.cpp:746-762 --------------------
function webHash(text) {
  let h = 2166136261;
  const bytes = Buffer.from(text, 'utf8');
  for (const c of bytes) {
    h ^= c;
    h = Math.imul(h, 16777619) >>> 0;
  }
  return h >>> 0;
}
function tierWidth(house, road, tier) {
  if (tier <= 1) return 1;
  const previous = tierWidth(house, road, tier - 1);
  const stepPick = webHash(`${house}|${road}|${tier}|width`) % 4;
  const step = stepPick === 0 ? -1 : (stepPick === 3 ? 1 : 0);
  return Math.max(1, Math.min(3, previous + step));
}
// ---- faithful port of native/src/networking.cpp:763-804 --------------------
function roadNodes(house, roadId, maxTier) {
  const ri = ROADS.findIndex(r => r.id === roadId);
  if (ri < 0) return [];
  const nodes = [];
  let previousTier = [];
  const used = new Set();
  for (let tier = 1; tier <= maxTier; ++tier) {
    const width = tierWidth(house, roadId, tier);
    const currentTier = [];
    for (let index = 0; index < width; ++index) {
      const h = webHash(`${house}|${roadId}|${tier}|${index}`);
      const node = {
        id: `${roadId}:${tier}:${index}`,
        tier, index,
        template_id: ROADS[ri].pairs[h % 4][0],
        layout: ROADS[ri].pairs[h % 4][1],
      };
      let name = `${FIRSTS[ri][(h >>> 4) % 6]}${SECONDS[ri][(h >>> 8) % 6]}`;
      while (used.has(name)) name += ' Deep';
      used.add(name);
      node.name = name;
      node.warden_name = `Warden of ${name}`;
      if (previousTier.length) {
        const parentPick = Math.min(previousTier.length - 1, Math.trunc((index * previousTier.length) / width));
        node.parent = nodes[previousTier[parentPick]].id;
      } else {
        node.parent = null;
      }
      currentTier.push(nodes.length);
      nodes.push(node);
    }
    for (const ni of currentTier) {
      const n = nodes[ni];
      if (n.parent != null) {
        for (const cand of nodes) {
          if (cand.id === n.parent) {
            cand.children = cand.children || [];
            cand.children.push(n.id);
            break;
          }
        }
      }
    }
    previousTier = currentTier;
  }
  return nodes;
}

const EXAMPLE_HOUSE = 'house-web-audit-example';
const EXAMPLE_MAX_TIER = 3;

const workedRoads = ROADS.map(r => {
  const nodes = roadNodes(EXAMPLE_HOUSE, r.id, EXAMPLE_MAX_TIER);
  const widths = [];
  for (let t = 1; t <= EXAMPLE_MAX_TIER; ++t) widths.push(nodes.filter(n => n.tier === t).length);
  return { road_id: r.id, widths, nodes };
});

// ---- assembled document ----------------------------------------------------
const coreNodes = [
  {
    graph: 'core_simulation_routes', id: 'route:tin:1:0', kind: 'route', optional: false,
    parent: null, children: ['route:tin:2:0'], initially_unlocked: true,
    content: 'level-1 Warden pack: 1 materialized warden + elite + normal pack mates materializing one telegraph window after the first falls',
    clear_effect: 'first clear records route_cleared; unlocks children; sets House.campaign_complete=true',
    citations: ['native/src/core.cpp:195-200', 'native/src/core.cpp:607-630', 'native/src/core.cpp:787-807'],
  },
  {
    graph: 'core_simulation_routes', id: 'route:tin:2:0', kind: 'route', optional: false,
    parent: 'route:tin:1:0', children: [], initially_unlocked: false,
    content: 'single level-2 elite identity ("deep" route)',
    clear_effect: 'first clear records route_cleared; no children to unlock',
    citations: ['native/src/core.cpp:196-198', 'native/src/core.cpp:608-609', 'native/src/core.cpp:787-807'],
  },
  {
    graph: 'core_simulation_routes', id: 'branch:ash', kind: 'branch', optional: true,
    parent: 'route:tin:1:0', children: [], initially_unlocked: false,
    content: 'not an instance: Interact("branch:ash") grants House specialization "ash"',
    clear_effect: 'BranchUnlocked event + legend branch_unlocked subject=branch:ash detail=specialization=ash',
    citations: ['native/src/core.cpp:198', 'native/src/core.cpp:461-467', 'native/include/verdigris/core.hpp:163-168'],
  },
];

const exampleNodes = [];
const exampleEdges = [];
for (const r of workedRoads) {
  for (const n of r.nodes) {
    exampleNodes.push({
      graph: 'protocol_world_web_worked_example',
      id: n.id, tier: n.tier, index: n.index,
      name: n.name, warden_name: n.warden_name,
      template: n.template_id, layout: n.layout,
      parent: n.parent, children: n.children || [],
      chart_status_rule: 'cleared if in session cleared_nodes_; open iff tier==1 or parent cleared; else barred',
      citations: ['native/src/networking.cpp:763-804', 'native/src/networking.cpp:1493-1504'],
    });
    if (n.parent != null) {
      exampleEdges.push({
        graph: 'protocol_world_web_worked_example',
        from: n.parent, to: n.id, type: 'child_unlocks_on_parent_warden_death',
        gate: 'onward zoneGate exposed only while parent warden dead; stairs-down blocked while boss alive',
        citations: ['native/src/networking.cpp:786-800', 'native/src/networking.cpp:1591-1599', 'native/src/networking.cpp:2134-2143'],
      });
    }
  }
}

const doc = {
  schema: 'verdigris.task-graph-audit/1',
  task: 'TASK-0096',
  title: 'Campaign and zone-graph measurement audit',
  captured_at: '2026-08-23',
  base_commit: BASE_COMMIT,
  audited_head: head,
  generator: 'tools/build-graph.mjs (this task folder); regenerate with: node orchestration/tasks/TASK-0096-campaign-graph-measurement-audit/tools/build-graph.mjs ' + head,
  citation_format: 'path:line against audited head ' + head,
  scope_note: 'mechanical measurement only; no zones, acts, rewards, duration, or travel risk invented; owner-only campaign choices recorded as MISSING',

  nodes: [...coreNodes, ...exampleNodes],
  edges: [
    ...[
      {
        graph: 'core_simulation_routes', from: null, to: 'route:tin:1:0',
        type: 'initial_unlock', gate: 'seeded in constructor: House.unlocked_routes starts with route:tin:1:0',
        citations: ['native/src/core.cpp:200'],
      },
      {
        graph: 'core_simulation_routes', from: 'route:tin:1:0', to: 'route:tin:2:0',
        type: 'child_unlock_on_clear', gate: 'resolve_enter requires House.route_unlocked && scion alive',
        citations: ['native/src/core.cpp:195-198', 'native/src/core.cpp:540-541', 'native/src/core.cpp:795-800'],
      },
      {
        graph: 'core_simulation_routes', from: 'route:tin:1:0', to: 'branch:ash',
        type: 'side_branch_interaction', gate: 'Interact("branch:ash") requires route_cleared(route:tin:1:0) && specialization "ash" not yet held',
        citations: ['native/src/core.cpp:198', 'native/src/core.cpp:461-467'],
      },
      {
        graph: 'core_simulation_routes', from: 'route:tin:1:0', to: 'House_store',
        type: 'extract_return_path', gate: 'resolve_extract requires instance active, scion alive, manhattan distance to extraction_point <= kExtractionRange',
        citations: ['native/src/core.cpp:901-925', 'native/include/verdigris/core.hpp:67-68'],
      },
    ],
    ...exampleEdges,
  ],

  graphs: {
    core_simulation_routes: {
      authority: 'headless fixed-step Simulation; House-owned progress; deterministic save/load',
      node_struct: 'RouteNode {id, parent_id, children[], optional} at native/include/verdigris/core.hpp:163-168',
      house_state_fields: ['routes', 'unlocked_routes', 'cleared_routes', 'specializations', 'campaign_complete'],
      house_state_citations: ['native/include/verdigris/core.hpp:172-191', 'native/src/core.cpp:1236-1257', 'native/src/core.cpp:1315-1340'],
      campaign_completion_rule: {
        behavior: 'campaign_complete flips true on the FIRST clear of ANY route (not on a final node)',
        citations: ['native/src/core.cpp:802-806'],
        test_evidence: 'last pack kill clears the route and completes campaign progression - native/tests/core_tests.cpp:1138-1140; founding-equivalent legend - native/tests/core_tests.cpp:1609-1610',
      },
      survival_note: 'route progress survives Scion death (House-level ownership)',
      survival_test_evidence: 'native/tests/core_tests.cpp:1416-1424',
      replay_test_evidence: 'cleared/unlocked equality under replay - native/tests/core_tests.cpp:1155-1158,1326-1330',
      traversal_content: {
        enemy_population_level_1: 'entry warden + elite + normal pack mate converging one telegraph window after first fall',
        enemy_population_citations: ['native/src/core.cpp:607-630', 'native/tests/core_tests.cpp:23-65'],
        deep_route_population: 'route:tin:2:0 spawns a single level-2 elite identity',
        extraction_geometry: 'extraction_point defaults {0,0} = player spawn; range kPlayerStepPerTick*kExtractionContactTicks',
        extraction_citations: ['native/include/verdigris/core.hpp:67-68,306', 'native/src/core.cpp:901-925'],
      },
    },

    protocol_world_web: {
      authority: 'ProtocolSession world-web (server-authoritative chart for remote play); progress is SESSION-scoped only',
      session_scope_citation: 'native/include/verdigris/networking.hpp:197-205 (cleared_nodes_ std::set member; "cleared wardens persist for the session")',
      roads_static: ROADS.map((r, i) => ({
        id: r.id, name: r.name, direction: r.direction, blurb: r.blurb,
        template_layout_pairs: r.pairs,
        town_gate_tile: ROAD_GATES[i].tile,
        citations: ['native/src/networking.cpp:720-729', 'native/src/networking.cpp:824'],
      })),
      naming_tables_citation: 'native/src/networking.cpp:730-741',
      determinism: {
        node_id_formula: '<road>:<tier>:<index>',
        hash: 'FNV-1a 32-bit, seed 2166136261, prime 16777619 (web_hash)',
        seeded_fields: 'template/layout pair = h%4; name syllables = table[(h>>4)%6]+table[(h>>8)%6] with in-road dedupe appending " Deep"; warden_name = "Warden of <name>"',
        tier_width_recursion: 'width(1)=1; width(t)=clamp(width(t-1)+step,1,3) with step from hash(house|road|tier|"width")%4 in {-1,0,+1}',
        parent_assignment: 'parent = previous_tier[min(prev_width-1, index*prev_width/width)]; each node >tier 1 has exactly one parent; child_ids accumulate in tier/index scan order so child_ids.front() = leftmost child',
        citations: ['native/src/networking.cpp:746-750', 'native/src/networking.cpp:756-762', 'native/src/networking.cpp:763-804'],
        worked_example: {
          house_id: EXAMPLE_HOUSE,
          max_tier: EXAMPLE_MAX_TIER,
          note: 'illustrative instantiation computed by re-executing the published algorithm; different house ids yield different charts (per-House determinism)',
          roads: workedRoads.map(w => ({ road_id: w.road_id, tier_widths: w.widths, nodes: w.nodes })),
        },
      },
      chart_screen: {
        frontier_rule: 'frontier = max cleared tier on that road + 1; chart renders tiers 1..frontier',
        status_rule: 'cleared | open (tier==1 or parent cleared) | barred',
        citations: ['native/src/networking.cpp:1482-1515'],
        event: 'world:road:chart -> native/src/networking.cpp:2392',
      },
      single_onward_gate_note: {
        finding: 'zoneGates exposes ONLY child_ids.front() even when several children exist; siblings are reachable only by returning to the Crossroads and entering via the chart',
        citations: ['native/src/networking.cpp:1528', 'native/src/networking.cpp:859-872', 'playtest/scenarios/world-web.mjs:43-44,47'],
      },
      unlock_enforcement_note: {
        finding: 'world:zone:enter performs NO cleared-parent check before enter_road_node; unlock enforcement lives in chart presentation (barred status) plus the living-Warden stairs-down hold along an active chain. Direct entry to any well-formed node id is accepted by the protocol.',
        citations: ['native/src/networking.cpp:2386', 'native/src/networking.cpp:1517-1546', 'native/src/networking.cpp:1591-1607'],
      },
      dual_bookkeeping_note: {
        finding: 'world:zone:enter also dispatches Command::enter("route:"+nodeId) into the headless core Simulation; the core accepts only its two seeded routes (route:tin:1:0, route:tin:2:0) and silently rejects every other web node id, so the two books coincide only on the Tin root/deep ids',
        citations: ['native/src/networking.cpp:2386', 'native/src/core.cpp:195-200', 'native/src/core.cpp:540-541'],
      },
      instance_anatomy: {
        geometry: 'every node instance is one 40x40 floor; stairsUp {5,20}, stairsDown {34,20}, spawn {6,20}; 20 monsters; last placed monster is the theme boss (elite) renamed to the node warden via boss_name_override',
        citations: ['native/src/core.cpp:1458-1474', 'native/src/core.cpp:1759-1773', 'native/src/networking.cpp:1534'],
        depth: 'node instances start at depth 1; generic non-node instances descend depth+1 per stairsDown until depth<=1 climbs out',
        depth_citations: ['native/src/core.cpp:1788-1805', 'native/src/core.cpp:1571-1589'],
        cleared_reentry: 'entering a cleared node suppresses spawning: zero monsters, metadata.wardenDead=true (dead stays dead, session-scoped)',
        cleared_reentry_citations: ['native/src/networking.cpp:1533-1539', 'native/src/networking.cpp:871', 'playtest/scenarios/world-web.mjs:100-110'],
      },
      warden_death_paths: {
        combat_kill: 'boss death inserts current_node_id_ into cleared_nodes_, unblocks stairs down, announces "The Warden of <name> is down. The road runs on."',
        dev_clear_floor: 'dev:clear-floor performs the identical insertion when a boss was alive',
        citations: ['native/src/networking.cpp:2134-2143', 'native/src/networking.cpp:2535-2548'],
      },
      named_theme_bosses: {
        grove: 'The Elder Oak', crypt: 'The Pale Sovereign', wilds: 'Alpha of the Wilds',
        marsh: 'The Rotfather', dungeon_default: 'Warden of the Deep',
        citations: ['native/src/core.cpp:1759-1769'],
      },
    },

    quest_overlay: {
      authority: 'ordered commission chain kQuestChain overlays both graphs; objectives reference adventure_zones identities (theme+layout), named elites, and depths',
      quests: [
        { id: 'aldwyns-charge', deed: "Answered Aldwyn's Charge", renown: 5, objectives: ['move', 'attack', 'slay', 'loot', 'delve'] },
        { id: 'proof-of-temper', deed: 'Proved their temper in the old realms', renown: 10, objectives: ['slay-elite(any)', 'loot-vessel', 'equip-vessel'] },
        { id: 'the-pale-crown', deed: "Broke the Pale Sovereign's seal", renown: 15, objectives: ['delve weir-crypt', 'slay-elite The Pale Sovereign (crypt)', 'delve depth>=2 crypt'] },
        { id: 'rot-in-the-reeds', deed: 'Ended the rot beneath the reeds', renown: 20, objectives: ['delve marsh-of-reeds', 'slay-elite The Rotfather (marsh)', 'return-surface marsh-of-reeds'] },
      ],
      quest_citations: ['native/src/networking.cpp:691-705'],
      adventure_zones: [
        { id: 'old-barrow', name: 'The Old Barrow', template: 'dungeon', layout: 'warren' },
        { id: 'verdant-grove', name: 'Verdant Grove', template: 'grove', layout: 'clearings' },
        { id: 'sunken-colonnade', name: 'Sunken Colonnade', template: 'crypt', layout: 'gauntlet' },
        { id: 'weir-crypt', name: 'Weir Crypt', template: 'crypt', layout: 'warren' },
        { id: 'the-wilds', name: 'The Wilds', template: 'wilds', layout: 'clearings' },
        { id: 'marsh-of-reeds', name: 'Marsh of Reeds', template: 'marsh', layout: 'clearings' },
      ],
      adventure_zones_citations: ['native/src/core.cpp:1427-1438'],
      scenario_evidence: 'playtest/scenarios/quest.mjs:51-356 (four commissions, seal/stairs descent to depth 2 at quest.mjs:255-265, return-surface at 325-342)',
      relationship_to_world_web: 'quest delves traverse generic solo instances keyed by theme+layout, NOT world-web chart nodes; the two systems coexist without shared progression state',
      separation_citation: 'native/src/networking.cpp:2386-2387',
    },
  },

  gates: [
    { graph: 'core_simulation_routes', id: 'route_unlock_gate', rule: 'Command::enter accepted iff House.route_unlocked(id) && scion.alive', citations: ['native/src/core.cpp:540-541'] },
    { graph: 'core_simulation_routes', id: 'branch_ash_gate', rule: 'Interact("branch:ash") requires route_cleared(route:tin:1:0) and no prior "ash" specialization', citations: ['native/src/core.cpp:461-467'] },
    { graph: 'core_simulation_routes', id: 'extraction_gate', rule: 'ExtractToHouse requires active instance, alive scion, manhattan(player, extraction_point) <= kExtractionRange', citations: ['native/src/core.cpp:901-925'] },
    ...ROAD_GATES.map(g => ({
      graph: 'protocol_world_web', id: `town_gate:${g.road}`,
      rule: 'standing on the gate tile in town opens that road\'s Wayfinder\'s Chart',
      tile: g.tile,
      citations: ['native/src/networking.cpp:824', 'native/src/networking.cpp:1588-1607', 'playtest/scenarios/world-web.mjs:15-26'],
    })),
    { graph: 'protocol_world_web', id: 'onward_gate_hold', rule: 'stairs-down tile refuses while the node Warden lives: "No road holds past a living Warden."', citations: ['native/src/networking.cpp:1539', 'native/src/core.cpp:1576-1577', 'native/src/networking.cpp:1591-1599', 'playtest/scenarios/world-web.mjs:46-54'] },
    { graph: 'protocol_world_web', id: 'entry_waymark', rule: 'stairsUp tile returns to the Crossroads from ANY node tier (stairs_up_returns_to_town_)', citations: ['native/src/networking.cpp:1541', 'native/src/core.cpp:1581-1585', 'playtest/scenarios/world-web.mjs:89-98'] },
    { graph: 'protocol_world_web', id: 'generic_floor_stairs', rule: 'non-node instances: stairsDown descends depth+1 (blocked while warden lives in node chains); stairsUp climbs until depth<=1 exits to town', citations: ['native/src/core.cpp:1571-1589'] },
  ],

  branches: [
    {
      id: 'branch:ash', graph: 'core_simulation_routes', optional: true,
      grant: 'House.specializations += "ash"', event: 'BranchUnlocked', legend: 'branch_unlocked',
      citations: ['native/src/core.cpp:461-467', 'native/tests/core_tests.cpp:1565-1567'],
    },
    {
      id: 'world_web_width_branching', graph: 'protocol_world_web', optional: 'structural only',
      grant: 'tier width 1..3 produces sibling nodes; no reward/specialization semantics attach to taking vs skipping them',
      citations: ['native/src/networking.cpp:756-762', 'native/src/networking.cpp:786-800'],
    },
  ],

  return_paths: [
    { graph: 'core_simulation_routes', path: 'extract at extraction point banks carried items/trophies to House store; retire_instance discards uncollected floor value', citations: ['native/src/core.cpp:563-605', 'native/src/core.cpp:901-920'] },
    { graph: 'core_simulation_routes', path: 'death converts carried items/trophies to House recovery candidates and retires the instance', citations: ['native/src/core.cpp:866-899'] },
    { graph: 'protocol_world_web', path: 'entry waymark (stairsUp) walks back to the Crossroads from any stage', citations: ['native/src/core.cpp:1581-1585', 'playtest/scenarios/world-web.mjs:89-98'] },
    { graph: 'protocol_world_web', path: 'player:extract / party:returnToTown / surface return all drain backpack+wear into the session House store (player:extract summary)', citations: ['native/src/networking.cpp:2188-2199', 'native/src/networking.cpp:1011-1044', 'native/src/networking.cpp:2552-2562'] },
  ],

  traversals: {
    core_graph: {
      shortest_start_to_campaign_complete: {
        steps: ['Simulation(seed, house)', 'Command::enter("route:tin:1:0")', 'defeat the level-1 warden pack', 'campaign_complete=true fires during clear_route_and_unlock_children'],
        node_visits: 1,
        note: 'completion fires on the FIRST route clear; there is no longer mandatory spine in code',
        citations: ['native/src/core.cpp:787-807', 'native/tests/core_tests.cpp:1138-1140'],
      },
      longest_defined_simple_traversal: {
        steps: ['enter+clear route:tin:1:0', 'interact branch:ash (optional)', 'enter+clear route:tin:2:0', 'extract'],
        node_visits: 3,
        instance_clears: 2,
        terminal_state: 'graph exhausted; no deeper nodes, no post-campaign content seam exists in the core',
        citations: ['native/src/core.cpp:195-200'],
      },
    },
    protocol_worked_example: {
      house_id: EXAMPLE_HOUSE,
      max_tier_measured: EXAMPLE_MAX_TIER,
      per_road: workedRoads.map(w => ({
        road_id: w.road_id,
        tier_widths: w.widths,
        node_count: w.nodes.length,
        max_fan_out: Math.max(0, ...w.nodes.map(n => (n.children || []).length)),
        deepest_chain_legs: EXAMPLE_MAX_TIER,
      })),
      minimum_expedition_legs_town_to_deepest: EXAMPLE_MAX_TIER,
      leg_rule: 'one stage per expedition leg: descending into a child clears current_child_id_, so further descent requires returning to the Crossroads and entering the next node via chart/world:zone:enter',
      leg_citations: ['native/src/networking.cpp:2388-2389 (depth_changed branch)', 'native/src/networking.cpp:1517-1546'],
      growth_bound: 'tiers deepen indefinitely (lazy generation); frontier grows as cleared_nodes_ gains tiers; no terminal node exists',
      growth_citations: ['docs/crossroads-world-web.md:90-104', 'native/src/networking.cpp:1486-1491'],
      duration_warning: 'NO time/duration measurements exist in code; campaign hour targets remain owner-authored (see missing_authoring)',
    },
  },

  missing_authoring: [
    { field: 'campaign.act_count', status: 'MISSING', why: 'constitution requires "several acts"; no act structure, table, or enum exists anywhere in native code', citations: ['docs/product/VERDIGRIS_CONSTITUTION.md:79'], blocked_on: 'owner campaign authoring' },
    { field: 'campaign.target_duration_hours', status: 'MISSING', why: 'constitution states an eventual 6-30h target; no measured or encoded pacing data exists; deliberately NOT derived from node counts or road names', citations: ['docs/product/VERDIGRIS_CONSTITUTION.md:76'], blocked_on: 'owner pacing decision + playtime telemetry' },
    { field: 'campaign.mandatory_spine', status: 'MISSING', why: 'no data distinguishes mandatory campaign nodes from optional ones on the protocol web; core marks exactly one optional flag (branch:ash) and otherwise hardcodes a 2-node spine', citations: ['native/src/core.cpp:195-200'], blocked_on: 'owner campaign authoring' },
    { field: 'campaign.branch_density_targets', status: 'MISSING', why: 'owner-only per SPEC frontmatter (owner_input_dependency); tier widths are structural randomness, not designed density', citations: ['orchestration/tasks/TASK-0096-campaign-graph-measurement-audit/SPEC.md:12'], blocked_on: 'owner' },
    { field: 'fast_travel.risk_model', status: 'MISSING', why: 'OD-012 open decision governs fast travel/town portals; only outbound return paths exist today', citations: ['docs/product/OPEN_DECISIONS.md:19', 'docs/product/VERDIGRIS_CONSTITUTION.md:113-116', 'docs/product/VERDIGRIS_FEATURE_CHECKLIST.md:59'], blocked_on: 'OD-012' },
    { field: 'repeatable_endgame.definition', status: 'MISSING', why: 'no endgame loop, area/goal/mechanic selection, or post-campaign content exists; nearest seams are unbounded web depth and empty re-entry of cleared nodes', citations: ['docs/product/VERDIGRIS_CONSTITUTION.md:81-83', 'native/src/networking.cpp:1533-1539'], blocked_on: 'owner campaign/endgame authoring' },
    { field: 'world_web.persistence_across_sessions', status: 'MISSING', why: 'cleared_nodes_ is session-scoped on ProtocolSession; whether House road charts should persist (and with what weathering/linger rules) is undecided in native', citations: ['native/include/verdigris/networking.hpp:197-205', 'docs/crossroads-world-web.md:106-121'], blocked_on: 'persistence design (adjacent audit tasks)' },
    { field: 'authored_zone_names_and_lore', status: 'MISSING', why: 'zone names come from procedural syllable tables plus six adventure-zone literals; owner holds naming/lore authority', citations: ['native/src/networking.cpp:730-741', 'native/src/core.cpp:1427-1438'], blocked_on: 'owner lore authoring' },
  ],

  delta_map: {
    campaign: {
      today: 'core: hardcoded 2-node linear spine + 1 optional branch; campaign_complete fires after the first clear. protocol: per-house infinite lazy web, no completion state, session-scoped clears.',
      delta_to_constitution: 'constitution requires a multizone multi-act campaign completed once per House per season with later Scions skipping the mandatory route; acts, completion semantics, season linkage, and skip-later-Scions mechanics are all absent',
      citations: ['docs/product/VERDIGRIS_CONSTITUTION.md:74-84', 'native/src/core.cpp:802-806'],
    },
    optional_branches: {
      today: 'one specialization-granting interaction (branch:ash) gated behind the first core clear; web width branching exists structurally with zero reward semantics',
      delta: 'branches must eventually grant specialization directions, item access, knowledge, league mechanics, routes, or starting opportunities (constitution line 80-82)',
      citations: ['native/src/core.cpp:461-467', 'docs/product/VERDIGRIS_CONSTITUTION.md:79-82'],
    },
    repeatable_endgame: {
      today: 'none implemented; re-entering cleared nodes yields empty floors (spawn suppressed); adventure-zone solo instances are infinitely re-enterable but featureless beyond the quest overlay',
      delta: 'endgame must let players choose areas, goals, mechanics, item targets, trophy targets, and build experiments',
      citations: ['docs/product/VERDIGRIS_CONSTITUTION.md:81-83', 'playtest/scenarios/world-web.mjs:100-110'],
    },
    fast_travel: {
      today: 'outbound return seams only: node entry waymark -> Crossroads; player:extract / party:returnToTown -> town + bank drain; no inbound fast travel, portal, or waypoint exists',
      delta: 'fast-travel/town-portal path with explicit risk model is constitution-required and owned by OD-012',
      citations: ['native/src/core.cpp:1581-1585', 'native/src/networking.cpp:2188-2199', 'docs/product/OPEN_DECISIONS.md:19'],
    },
  },

  negative_control: {
    requirement: 'SPEC: preserve at least one MISSING campaign field rather than deriving it from a route name',
    preserved: [
      'missing_authority[campaign.target_duration_hours]: left MISSING; the string "6-30 hours" appears only as constitutional intent, never as measured data, and was NOT copied into any measured field',
      'missing_authority[campaign.act_count]: left MISSING; the four road NAMES (Tin/Salt/Chalk/Copper) were NOT promoted into an act structure',
    ],
  },

  successor_tool_contract: {
    purpose: 'how a campaign-authoring successor consumes this audit',
    consume: [
      'Treat graphs.core_simulation_routes and graphs.protocol_world_web as the complete measured topology; every node/edge/gate cites path:line at the audited head.',
      'Recompute per-house charts only through the published algorithm (port above or call into networking.cpp); never persist derived node lists as content.',
      'Author campaign layers (acts, mandatory spine, durations, branch rewards, fast travel) as NEW data tables; do not encode them into route names or road blurbs.',
    ],
    extend_points: [
      'core RouteNode table (constructor-seeded today) is the natural seat for a data-driven campaign spine',
      'emit_chart_screen rows already carry id/status/tier/template/layout/parentId - an act/stage column slots there',
      'world:zone:enter is the single entry seam where unlock authorization would centralize',
    ],
    regeneration: 'node orchestration/tasks/TASK-0096-campaign-graph-measurement-audit/tools/build-graph.mjs <head-sha>',
    determinism_guarantee: 'same inputs produce byte-identical output; worked example depends only on EXAMPLE_HOUSE + EXAMPLE_MAX_TIER constants declared in this file',
  },
};

mkdirSync(join(dirname(fileURLToPath(import.meta.url)), '..', 'captures'), { recursive: true });
const outPath = join(dirname(fileURLToPath(import.meta.url)), '..', 'captures', 'graph.json');
writeFileSync(outPath, JSON.stringify(doc, null, 2) + '\n');
console.log(`wrote ${outPath}: ${doc.nodes.length} nodes, ${doc.edges.length} edges`);
