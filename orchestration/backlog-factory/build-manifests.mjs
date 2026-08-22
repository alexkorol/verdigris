import fs from "node:fs";
import path from "node:path";
import { fileURLToPath } from "node:url";

const ROOT = path.dirname(fileURLToPath(import.meta.url));
const OUT = path.join(ROOT, "generated");
const CHECK = process.argv.includes("--check");
const VERSION = "d128-v1";

const domains = [
  ["core-simulation", "Deterministic simulation and architecture", [], "T1", "native/src/**; native/include/**", ["deterministic clock", "command queue", "event stream", "actor model", "stat resolution", "RNG streams", "spatial queries", "collision model", "entity lifecycle", "failure recovery"]],
  ["networking-protocol", "Networking, protocol, and session authority", ["core-simulation"], "T1", "native/src/networking*; native/include/**; server/**", ["transport lifecycle", "protocol version negotiation", "authentication and sessions", "reconnect and resume", "state replication", "input validation", "rate and abuse controls", "party coordination", "latency handling", "protocol migration"]],
  ["persistence-recovery", "Persistence, migration, and recovery", ["core-simulation"], "T1", "native/src/persistence*; native/include/**; server/**", ["account profile saves", "House saves", "Scion saves", "inventory saves", "progression saves", "world-state saves", "save slots", "schema migration", "corruption recovery", "backup and restore"]],
  ["player-entry-journey", "Account-to-play player entry journey", ["networking-protocol", "persistence-recovery"], "T2", "native/src/**; native/include/**; packaging/**", ["launcher and updater", "account entry", "profile selection", "House creation", "Scion creation", "character selection", "initial world load", "settings bootstrap", "reconnect return", "save and relaunch"]],
  ["controls-movement", "Controls, targeting, and movement", ["core-simulation", "player-entry-journey"], "T2", "native/src/**; native/include/**", ["keyboard and mouse input", "controller input", "input rebinding", "target acquisition", "navigation and pathing", "character locomotion", "movement skills", "collision response", "camera coupling", "input accessibility"]],
  ["combat", "Production combat model and feel", ["core-simulation", "controls-movement"], "T4", "native/src/**; native/include/**; server/**", ["attack vocabulary", "hit detection", "damage pipeline", "combat resources", "cooldowns", "status effects", "crowd control", "defenses", "hit reactions", "environmental combat"]],
  ["skills-magic", "Skills, magic, and effect composition", ["combat"], "T4", "native/src/**; native/include/**; content/**", ["input slots", "action interfaces", "targeting shapes", "projectiles", "area effects", "buffs and debuffs", "summons", "channels", "triggers and modifiers", "skill progression and AI use"]],
  ["monsters", "Monster roster and behaviors", ["combat", "skills-magic"], "T4", "native/src/**; native/include/**; content/**", ["monster families", "family variants", "rarity model", "elite packages", "champions", "named uniques", "boss roster", "pack coordination", "spawn rules", "difficulty progression"]],
  ["encounters", "Encounter and boss production", ["monsters"], "T4", "native/src/**; native/include/**; content/**", ["encounter composition", "biome integration", "combat telegraphs", "boss phases", "environmental hazards", "encounter objectives", "world events", "deterministic generation", "pacing", "encounter test matrix"]],
  ["itemization", "Items, identity, loot, and economy surfaces", ["persistence-recovery", "combat"], "T4", "native/src/**; native/include/**; content/**", ["item bases", "item identity", "rarity", "affixes", "item history", "extraction and loss", "recovery", "relics Brands and Bonds", "crafting and stores", "comparison filtering and tooltips"]],
  ["progression", "Character, House, and account progression", ["persistence-recovery", "itemization"], "T4", "native/src/**; native/include/**; content/**", ["character stats", "passive tree", "allocation and respec", "House progression", "Scion progression", "Legends", "unlocks", "milestones and difficulty", "account and seasonal state", "tuning tools"]],
  ["campaign-endgame", "Multi-act campaign, travel, and endgame", ["player-entry-journey", "encounters", "progression"], "T3", "native/src/**; native/include/**; content/**; docs/product/**", ["campaign acts", "regions", "settlements", "zones", "optional branches", "quests", "events", "transitions", "travel and extraction", "endgame and replayability"]],
  ["rendering-presentation", "Renderer, world presentation, and camera", ["core-simulation"], "T5", "native/src/**; native/include/**; assets/**", ["renderer backend", "camera", "terrain", "scenery", "player characters", "monsters", "items and drops", "lighting and materials", "weather and ambience", "resolution and render performance"]],
  ["animation-vfx", "Animation, motion, and visual effects", ["rendering-presentation", "combat"], "T5", "native/src/**; native/include/**; assets/**", ["locomotion animation", "attack animation", "skill animation", "monster animation", "boss animation", "hit and reaction animation", "environment motion", "UI motion", "VFX readability", "animation and VFX pipeline"]],
  ["audio-music", "Sound design, ambience, and music", ["combat"], "T5", "native/src/**; native/include/**; assets/**", ["combat sound", "skill sound", "monster sound", "boss sound", "item sound", "UI sound", "ambience", "music runtime", "mixing and settings", "audio regression"]],
  ["ui-ux", "Player-facing UI and information architecture", ["player-entry-journey", "persistence-recovery"], "T5", "native/src/**; native/include/**; assets/**", ["UI frame system", "HUD", "menus", "inventory", "equipment", "passive tree UI", "map and quest UI", "tooltips", "settings", "onboarding UI"]],
  ["accessibility", "Accessible and adaptable play", ["ui-ux", "controls-movement", "audio-music"], "T5", "native/src/**; native/include/**; docs/**", ["keyboard navigation", "controller navigation", "remapping", "contrast and color", "text sizing", "motion reduction", "subtitles and audio cues", "difficulty assists", "cognitive onboarding", "accessibility test matrix"]],
  ["content-production", "Content authoring and production pipeline", ["core-simulation"], "T4", "tools/**; content/**; docs/**", ["content schemas", "editors", "validators", "import pipelines", "asset provenance", "asset derivatives", "naming and lore workflow", "localization seams", "content review", "build manifests and batches"]],
  ["quality-release", "Quality, packaging, release, and support", ["networking-protocol", "campaign-endgame", "rendering-presentation", "audio-music", "accessibility"], "T7", ".github/**; native/**; playtest/**; packaging/**; docs/**", ["unit verification", "integration verification", "protocol parity", "deterministic replay", "soak testing", "performance testing", "clean-machine testing", "save migration and corruption", "device resolution and accessibility", "packaging signing updates and support"]],
  ["owner-governance", "Owner authority, play verdicts, and correction waves", ["content-production", "campaign-endgame", "rendering-presentation", "quality-release"], "T8", "orchestration/owner-input/**; docs/product/**; content/**", ["art direction", "lore direction", "naming direction", "magic direction", "balance direction", "economy direction", "season and live-content direction", "content approval", "owner play verdicts", "correction waves and support"]],
].map(([id, objective, dependencies, terminalGate, surfaces, subsystems]) => ({ id, objective, dependencies, terminalGate, surfaces: surfaces.split("; "), subsystems }));

const stages = [
  ["contract", "Freeze a versioned contract and trust boundaries for", "ARCHITECTURE", "CONTRACT_REVIEW"],
  ["implementation", "Implement the production runtime behavior for", "IMPLEMENTATION", "AUTOMATED_AND_NEGATIVE"],
  ["integration", "Integrate end-to-end player and system flows for", "INTEGRATION", "END_TO_END"],
  ["content", "Author representative and scalable production content for", "CONTENT", "SCHEMA_AND_CONTENT_VALIDATION"],
  ["presentation", "Deliver readable owner-visible presentation for", "PRESENTATION", "CAPTURE_AND_USABILITY"],
  ["hardening", "Harden failure, abuse, corruption, and recovery paths for", "HARDENING", "FAULT_INJECTION"],
  ["performance", "Meet measured runtime and content-scale budgets for", "PERFORMANCE", "BENCHMARK_AND_SOAK"],
  ["regression", "Gate deterministic regression coverage for", "VERIFICATION", "REGRESSION_MATRIX"],
  ["owner-play", "Prove representative owner-play journeys and corrections for", "VALIDATION", "OWNER_PLAY_VERDICT"],
  ["release", "Prove clean-machine release, migration, and support readiness for", "RELEASE", "RELEASE_GATE"],
].map(([id, verb, category, acceptanceClass]) => ({ id, verb, category, acceptanceClass }));

const domainIndex = new Map(domains.map((domain, index) => [domain.id, index]));
const nodeId = (domainIndexValue, subsystemIndex, stageIndex) =>
  `PG-${String(domainIndexValue + 1).padStart(2, "0")}-${String(subsystemIndex + 1).padStart(2, "0")}-${String(stageIndex + 1).padStart(2, "0")}`;

const graphNodes = [];
for (let d = 0; d < domains.length; d += 1) {
  const domain = domains[d];
  for (let s = 0; s < domain.subsystems.length; s += 1) {
    const subsystem = domain.subsystems[s];
    for (let p = 0; p < stages.length; p += 1) {
      const stage = stages[p];
      const dependencies = [];
      if (p > 0) dependencies.push(nodeId(d, s, p - 1));
      for (const dependencyDomain of domain.dependencies) {
        const dependencyIndex = domainIndex.get(dependencyDomain);
        dependencies.push(nodeId(dependencyIndex, Math.min(s, domains[dependencyIndex].subsystems.length - 1), p));
      }
      const ownerDependency = domain.id === "owner-governance"
        ? "OWNER_APPROVAL_REQUIRED"
        : stage.id === "owner-play"
          ? "OWNER_PLAY_VERDICT_REQUIRED"
          : "NONE_OR_BATCHABLE";
      const terminalGate = domain.id === "owner-governance" && s < 8 ? "T6" : domain.terminalGate;
      graphNodes.push({
        id: nodeId(d, s, p),
        domain: domain.id,
        subsystem,
        stage: stage.id,
        outcome: `${stage.verb} ${subsystem}.`,
        parent_product_objective: domain.objective,
        dependencies: [...new Set(dependencies)].sort(),
        owner_visible_relevance: `Moves ${domain.objective.toLowerCase()} toward terminal gate ${terminalGate} through ${subsystem}.`,
        likely_owning_surface: domain.surfaces,
        acceptance_class: stage.acceptanceClass,
        owner_dependency: ownerDependency,
        category: stage.category,
        terminal_gate: terminalGate,
        generation_provenance: { authority: "D-128", generator: "build-manifests.mjs", version: VERSION },
      });
    }
  }
}

const packetPhases = [
  ["implementation", 1, "IMPLEMENTATION", "BOUNDED_DESIGN"],
  ["integration", 2, "INTEGRATION", "INTEGRATION"],
  ["production-polish", 4, "PRESENTATION", "BOUNDED_DESIGN"],
  ["hardening", 5, "HARDENING", "MECHANICAL"],
  ["release-verification", 9, "RELEASE", "MECHANICAL"],
].map(([id, stageIndex, category, jobType]) => ({ id, stageIndex, category, jobType }));

const packetReserve = [];
let packetCounter = 0;
for (let d = 0; d < domains.length; d += 1) {
  const domain = domains[d];
  for (let s = 0; s < 5; s += 1) {
    const subsystem = domain.subsystems[s];
    let previousPacket = null;
    for (const phase of packetPhases) {
      packetCounter += 1;
      const id = `PKT-${String(packetCounter).padStart(4, "0")}`;
      const graphNode = nodeId(d, s, phase.stageIndex);
      const dependencies = previousPacket ? [previousPacket] : [];
      const state = previousPacket ? "AUTO_RELEASE" : "DRAFT";
      packetReserve.push({
        id,
        state,
        graph_node_id: graphNode,
        outcome: `${phase.id.replaceAll("-", " ")} for ${subsystem} in ${domain.objective}.`,
        topology: previousPacket ? "SEQUENTIAL" : "INDEPENDENT_AFTER_GRAPH_PREREQUISITES",
        job_type: phase.jobType,
        category: phase.category,
        dependencies,
        likely_owned_paths: domain.surfaces,
        forbidden_paths: ["peer task evidence", "architect checkout branch state", "owner-only decisions", "port 6500"],
        interface: `Consume the accepted ${subsystem} contract and preserve versioned boundaries; freeze exact symbols and payloads only at READY promotion.`,
        acceptance_approach: `${stages[phase.stageIndex].acceptanceClass}: require default-path positive proof, authentic negative control, changed-test inspection, and pack-specific regression gates.`,
        evidence_requirements: ["base and head SHA", "literal commands and exit codes", "changed paths", "interface inventory", "negative control", "full experimental-unit telemetry"],
        owner_input_dependency: domain.id === "owner-governance" ? "BATCHED_OWNER_APPROVAL" : phase.id === "production-polish" ? "OWNER_DIRECTION_IF_TASTE_BEARING" : "NONE_OR_NONBLOCKING",
        fallback_path: "If current interfaces, owner authority, collision, or acceptance cannot be validated, return to DRAFT with evidence; do not guess or auto-promote.",
        successor_generation_rule: `On acceptance, analyze 3-10 integration, edge-case, performance, content-use, presentation, regression, and release successors for ${subsystem}; suppress filler with exhaustion evidence.`,
        release_condition: previousPacket ? { event: "PACKET_ACCEPTED", packet_id: previousPacket, plus: [`GRAPH_NODE_ACCEPTED:${nodeId(d, s, Math.max(0, phase.stageIndex - 1))}`, "CURRENT_TIP_VALIDATION_GREEN", "OWNED_PATH_COLLISION_CLEAR", "RESOURCE_CAPSULE_AVAILABLE"] } : null,
        refresh_condition: "Refresh before READY when program tip, interface evidence, dependency verdict, owner ruling, path ownership, resource capsule, or acceptance harness changes.",
        immutable_base_sha: null,
        likely_lane: domain.id === "owner-governance" ? "ARCHITECT_OR_OWNER_INPUT" : "FUTURE_REGISTERED_IMPLEMENTATION_LANE",
        terminal_gate: domain.id === "owner-governance" && s < 5 ? "T6" : domain.terminalGate,
        generation_provenance: { authority: "D-128", generator: "build-manifests.mjs", version: VERSION, parent_graph_node: graphNode },
      });
      previousPacket = id;
    }
  }
}

const allNodeIds = new Set(graphNodes.map((node) => node.id));
const allPacketIds = new Set(packetReserve.map((packet) => packet.id));
if (graphNodes.length !== 2000 || allNodeIds.size !== 2000) throw new Error(`Expected 2,000 unique graph nodes, got ${graphNodes.length}/${allNodeIds.size}`);
if (packetReserve.length !== 500 || allPacketIds.size !== 500) throw new Error(`Expected 500 unique packets, got ${packetReserve.length}/${allPacketIds.size}`);
for (const node of graphNodes) for (const dependency of node.dependencies) if (!allNodeIds.has(dependency)) throw new Error(`Missing graph dependency ${dependency} for ${node.id}`);
const graphById = new Map(graphNodes.map((node) => [node.id, node]));
const visiting = new Set();
const visited = new Set();
const visitGraphNode = (id) => {
  if (visited.has(id)) return;
  if (visiting.has(id)) throw new Error(`Graph dependency cycle at ${id}`);
  visiting.add(id);
  for (const dependency of graphById.get(id).dependencies) visitGraphNode(dependency);
  visiting.delete(id);
  visited.add(id);
};
for (const id of allNodeIds) visitGraphNode(id);
const observedGates = new Set(graphNodes.map((node) => node.terminal_gate));
for (let gate = 1; gate <= 8; gate += 1) if (!observedGates.has(`T${gate}`)) throw new Error(`Terminal gate T${gate} has no graph coverage`);
for (const packet of packetReserve) {
  if (!allNodeIds.has(packet.graph_node_id)) throw new Error(`Missing graph node ${packet.graph_node_id} for ${packet.id}`);
  for (const dependency of packet.dependencies) if (!allPacketIds.has(dependency)) throw new Error(`Missing packet dependency ${dependency} for ${packet.id}`);
  if (packet.immutable_base_sha !== null) throw new Error(`Distant packet ${packet.id} must not freeze a base SHA`);
  if (packet.state === "AUTO_RELEASE" && (!packet.release_condition || packet.dependencies.length === 0)) throw new Error(`AUTO_RELEASE packet ${packet.id} lacks an exact predecessor release condition`);
}

const countBy = (items, field) => Object.fromEntries([...items.reduce((map, item) => map.set(item[field], (map.get(item[field]) ?? 0) + 1), new Map()).entries()].sort());
const productShareCategories = new Set(["IMPLEMENTATION", "INTEGRATION", "CONTENT", "PRESENTATION", "HARDENING", "PERFORMANCE", "RELEASE"]);
const pureAuditCategories = new Set(["AUDIT", "RESEARCH", "INVENTORY", "EVALUATION"]);
const productPackets = packetReserve.filter((packet) => productShareCategories.has(packet.category)).length;
const auditPackets = packetReserve.filter((packet) => pureAuditCategories.has(packet.category)).length;
const ownerBlockedPackets = packetReserve.filter((packet) => packet.owner_input_dependency === "BATCHED_OWNER_APPROVAL").length;
const summary = {
  schema_version: VERSION,
  authority: "D-128",
  graph_nodes: graphNodes.length,
  detailed_packet_reserve: packetReserve.length,
  packet_states: countBy(packetReserve, "state"),
  packet_categories: countBy(packetReserve, "category"),
  graph_domains: countBy(graphNodes, "domain"),
  terminal_gates: countBy(graphNodes, "terminal_gate"),
  graph_acyclic: visited.size === graphNodes.length,
  implementation_integration_content_presentation_polish_release_share: productPackets / packetReserve.length,
  pure_audit_research_inventory_evaluation_share: auditPackets / packetReserve.length,
  owner_blocked_packets: ownerBlockedPackets,
  owner_blocked_share: ownerBlockedPackets / packetReserve.length,
  emergency_ready_floor: 24,
  emergency_successor_floor: 12,
  autonomous_runway: { hours: null, confidence: "UNKNOWN", reason: "Comparable trailing accepted-throughput samples by full experimental unit and packet type have not yet been normalized." },
  runway_policy: { target_hours: 72, warning_below_hours: 48, critical_below_hours: 24 },
  completion_claim: false,
};

const files = new Map([
  ["product-graph.nodes.jsonl", `${graphNodes.map((item) => JSON.stringify(item)).join("\n")}\n`],
  ["packet-reserve.jsonl", `${packetReserve.map((item) => JSON.stringify(item)).join("\n")}\n`],
  ["summary.json", `${JSON.stringify(summary, null, 2)}\n`],
]);

if (!CHECK) fs.mkdirSync(OUT, { recursive: true });
for (const [name, expected] of files) {
  const target = path.join(OUT, name);
  if (CHECK) {
    if (!fs.existsSync(target)) throw new Error(`Missing generated file: ${target}`);
    const actual = fs.readFileSync(target, "utf8");
    if (actual !== expected) throw new Error(`Generated file is stale: ${target}`);
  } else {
    fs.writeFileSync(target, expected, "utf8");
  }
}

console.log(`D-128 manifests ${CHECK ? "verified" : "generated"}: ${graphNodes.length} graph nodes, ${packetReserve.length} detailed packets, runway ${summary.autonomous_runway.confidence}`);
