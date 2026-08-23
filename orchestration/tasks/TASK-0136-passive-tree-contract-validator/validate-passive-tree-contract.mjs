#!/usr/bin/env node
// TASK-0136 passive-tree contract validator CLI.
//
// Dependency-free Node CLI that validates the accepted TASK-0112 contract
// (`verdigris.passive-tree-authority` schema 1.0.0) and candidate
// graph/allocation/budget/persistence envelopes. Error codes, ranks, and
// ordering are bound to orchestration/tasks/TASK-0112-passive-tree-engine-scaffold/VALIDATION.md:
// batch mode collects every failing check, each reporting the first offending
// element, sorted ascending by (rank, lexicographic element), independent of
// input order. Ranks 1-3 are stop ranks (nothing downstream is meaningful);
// ranks 4-9 accumulate.
//
// Content neutrality: no node/effect/topology/cost/balance value is
// interpreted or authored here. node_id text is opaque; the native +2/axis
// walk is the named negative control `native_plus_two_axis_walk` and this CLI
// never derives meaning from identifier text. Raw snapshots are never trusted
// as persistence: they are refused without validation_provenance and only
// rebuilt through validation (`native_raw_snapshot_save`). The persistent
// commission-chain ledger (quests.questPoints) and the live tree-budget
// ledger remain structurally distinct; merge, alias, or mutual-derivation
// shapes fail closed as COUNTER_CONFUSION.
//
// Counter-confusion detectors (all deterministic, structural):
//   CC-A collapse      a scanned object carries an unsanctioned points-named
//                      key while lacking at least one of the two required
//                      ledger fields (merged ledger shape).
//   CC-B cross-write   `live_tree_points` annotated with the commission wire
//                      identity `quests.questPoints`.
//   CC-C mis-derivation an earned-source marker naming the commission-chain
//                      wire identity instead of a tree-budget counter input.
//   CC-D alias         an alias/derivation marker equating the two ledgers.
//
// Usage:
//   node validate-passive-tree-contract.mjs --contract <contract.json> --fixture <candidates.json> [--json]
// Exit codes: 0 all candidates valid; 1 validation errors; 2 usage/IO error.

import { readFileSync } from "node:fs";
import { argv, cwd, exit } from "node:process";
import { isAbsolute, resolve } from "node:path";
import { pathToFileURL } from "node:url";

export const RANKS = Object.freeze({
  MALFORMED_ALLOCATION: 1,
  UNKNOWN_GRAPH_VERSION: 2,
  UNSUPPORTED_MIGRATION: 3,
  UNKNOWN_NODE: 4,
  DUPLICATE_NODE: 5,
  MALFORMED_EDGE: 6,
  DISCONNECTED_ALLOCATION: 7,
  OVERSPENT: 8,
  COUNTER_CONFUSION: 9,
});

export const REQUIRED_ERROR_CODES = Object.freeze([
  "UNKNOWN_GRAPH_VERSION",
  "UNKNOWN_NODE",
  "DUPLICATE_NODE",
  "DISCONNECTED_ALLOCATION",
  "OVERSPENT",
  "MALFORMED_EDGE",
  "COUNTER_CONFUSION",
  "UNSUPPORTED_MIGRATION",
]);

const ALL_PIPELINE_CODES = Object.freeze(Object.keys(RANKS));
const CONTRACT_ID = "verdigris.passive-tree-authority";
const CONTRACT_OBJECT_SECTIONS = [
  "content_neutrality",
  "counter_separation",
  "graph",
  "node",
  "edge",
  "allocation",
  "budget",
  "validation_result",
  "migration",
  "persistence",
];
const REQUIRED_NEGATIVE_CONTROLS = [
  "native_plus_two_axis_walk",
  "native_raw_snapshot_save",
  "counter_collapse",
];
const LEDGER_FIELDS = ["persistent_commission_points", "live_tree_points"];
const BUDGET_FIELDS = [...LEDGER_FIELDS, "earned", "spent", "unspent"];
const COMMISSION_WIRE_IDENTITY = "quests.questPoints";
const POINTS_KEY_RE = /point/i;
const EARNED_SOURCE_KEYS = ["earned_source", "earned_derived_from", "designated_counter"];
const ALIAS_KEYS = ["alias_of", "derived_from", "merged_with", "same_as"];
const MIGRATION_STRATEGIES = ["revalidate_in_place", "full_refund_reset"];
const MODES = ["allocation", "persistence", "migration_request", "raw_snapshot"];

function cmp(a, b) {
  return a < b ? -1 : a > b ? 1 : 0;
}

function isPlainObject(value) {
  return typeof value === "object" && value !== null && !Array.isArray(value);
}

function isNonEmptyString(value) {
  return typeof value === "string" && value.length > 0;
}

function isLedgerInt(value) {
  return Number.isInteger(value) && value >= 0;
}

function makeError(code, element) {
  if (!(code in RANKS)) throw new Error(`unknown code ${code}`);
  return {
    code,
    element: element ?? null,
    message_key: `error.${code.toLowerCase()}`,
    rank: RANKS[code],
  };
}

function makeMetaError(code, element) {
  const err = { code, element: element ?? null, message_key: `error.${code.toLowerCase()}` };
  err.rank = code === "INVALID_CONTRACT" || code === "INVALID_FIXTURE" ? 0 : RANKS[code];
  return err;
}

function sortErrors(errors) {
  return [...errors].sort((a, b) => a.rank - b.rank || cmp(String(a.element), String(b.element)));
}

function firstLex(candidates) {
  const defined = candidates.filter((c) => c !== undefined && c !== null).map(String);
  if (defined.length === 0) return null;
  return defined.reduce((min, c) => (cmp(c, min) < 0 ? c : min));
}

function usageError(message) {
  const err = new Error(message);
  err.usage = true;
  return err;
}

export function parseArgs(argvSlice) {
  const args = {};
  for (let i = 0; i < argvSlice.length; i++) {
    const a = argvSlice[i];
    if (a === "--json") {
      args.json = true;
    } else if (a === "--contract") {
      args.contract = argvSlice[++i];
    } else if (a === "--fixture") {
      args.fixture = argvSlice[++i];
    } else {
      throw usageError(`unknown argument: ${a}`);
    }
  }
  if (!args.contract) throw usageError("--contract <path> is required");
  if (!args.fixture) throw usageError("--fixture <path> is required");
  return args;
}

function loadJsonFile(path, label) {
  const absolute = isAbsolute(path) ? path : resolve(cwd(), path);
  let text;
  try {
    text = readFileSync(absolute, "utf8");
  } catch (err) {
    const wrapped = new Error(`cannot read ${label} ${absolute}: ${err.message}`);
    wrapped.io = true;
    throw wrapped;
  }
  try {
    return { absolute, json: JSON.parse(text) };
  } catch (err) {
    const wrapped = new Error(`${label} ${absolute} is not valid JSON: ${err.message}`);
    wrapped.io = true;
    throw wrapped;
  }
}

// Validates that the loaded contract document is the accepted TASK-0112
// contract: identity, envelope sections, the intact two-ledger separation,
// and full error-code coverage. Fails closed on any gap.
export function validateContractShape(contract) {
  const problems = [];
  const bad = (element) => problems.push(makeMetaError("INVALID_CONTRACT", element));
  if (!isPlainObject(contract)) {
    bad("contract");
    return { ok: false, errors: problems };
  }
  if (contract.contract_id !== CONTRACT_ID) bad("contract_id");
  if (!isNonEmptyString(contract.schema_version) || !/^\d+\.\d+\.\d+$/.test(contract.schema_version)) {
    bad("schema_version");
  }
  for (const section of CONTRACT_OBJECT_SECTIONS) {
    if (!isPlainObject(contract[section])) bad(section);
  }
  if (!Array.isArray(contract.negative_controls)) bad("negative_controls");
  if (problems.length > 0) return { ok: false, errors: problems };

  const cs = contract.counter_separation;
  if (cs.violation_error !== "COUNTER_CONFUSION") bad("counter_separation.violation_error");
  if (!isNonEmptyString(cs.persistent_commission_points) || !isNonEmptyString(cs.live_tree_points)) {
    bad("counter_separation.ledgers");
  } else if (cs.persistent_commission_points === cs.live_tree_points) {
    bad("counter_separation.collapsed");
  }

  const budget = contract.budget;
  if (
    !isPlainObject(budget.persistent_commission_points) ||
    !isPlainObject(budget.live_tree_points)
  ) {
    bad("budget.ledgers");
  } else if (budget.persistent_commission_points === budget.live_tree_points) {
    bad("budget.counters_collapsed");
  }
  for (const field of BUDGET_FIELDS) {
    if (!isPlainObject(budget.properties) || !isPlainObject(budget.properties[field])) {
      bad(`budget.properties.${field}`);
    }
  }

  const enumCodes =
    contract.validation_result?.properties?.errors?.items?.properties?.code?.enum;
  if (!Array.isArray(enumCodes)) {
    bad("validation_result.errors.code.enum");
  } else {
    for (const code of ALL_PIPELINE_CODES) {
      if (!enumCodes.includes(code)) bad(`validation_result.errors.code.enum.${code}`);
    }
  }

  const controlNames = Array.isArray(contract.negative_controls)
    ? contract.negative_controls.map((c) => (isPlainObject(c) ? c.name : null))
    : [];
  for (const name of REQUIRED_NEGATIVE_CONTROLS) {
    if (!controlNames.includes(name)) bad(`negative_controls.${name}`);
  }

  return { ok: problems.length === 0, errors: problems };
}

// Graph-document preflight for the fixture's synthetic authority graph.
function validateGraphDocument(graph, authority) {
  const errors = [];
  if (!isPlainObject(graph)) {
    errors.push(makeError("MALFORMED_ALLOCATION", "graph"));
    return errors;
  }
  if (!Number.isInteger(graph.graph_version) || graph.graph_version < 1) {
    errors.push(makeError("MALFORMED_ALLOCATION", "graph.graph_version"));
    return errors;
  }
  if (graph.graph_version !== authority.graph_version) {
    errors.push(makeError("UNKNOWN_GRAPH_VERSION", null));
    return errors;
  }
  if (!isNonEmptyString(graph.origin)) {
    errors.push(makeError("MALFORMED_ALLOCATION", "graph.origin"));
  }
  if (!Array.isArray(graph.nodes) || !Array.isArray(graph.edges)) {
    errors.push(makeError("MALFORMED_ALLOCATION", "graph.collections"));
    return sortErrors(errors);
  }
  const seenNodes = new Map();
  for (let i = 0; i < graph.nodes.length; i++) {
    const node = graph.nodes[i];
    if (!isPlainObject(node) || !isNonEmptyString(node.node_id)) {
      errors.push(makeError("MALFORMED_ALLOCATION", `graph.nodes[${i}]`));
      continue;
    }
    seenNodes.set(node.node_id, (seenNodes.get(node.node_id) ?? 0) + 1);
  }
  const dupNodes = [...seenNodes.entries()].filter(([, n]) => n > 1).map(([id]) => id);
  const dupNodeElem = firstLex(dupNodes);
  if (dupNodeElem !== null) errors.push(makeError("DUPLICATE_NODE", dupNodeElem));

  const seenEdges = new Map();
  for (let i = 0; i < graph.edges.length; i++) {
    const edge = graph.edges[i];
    const wellFormed =
      isPlainObject(edge) &&
      isNonEmptyString(edge.edge_id) &&
      isNonEmptyString(edge.from_node) &&
      isNonEmptyString(edge.to_node) &&
      Array.isArray(edge.variants) &&
      edge.variants.length > 0 &&
      edge.variants.every((v) => typeof v === "string");
    if (!wellFormed) {
      errors.push(makeError("MALFORMED_EDGE", isPlainObject(edge) && isNonEmptyString(edge.edge_id) ? edge.edge_id : `graph.edges[${i}]`));
      continue;
    }
    seenEdges.set(edge.edge_id, (seenEdges.get(edge.edge_id) ?? 0) + 1);
    if (!seenNodes.has(edge.from_node) || !seenNodes.has(edge.to_node)) {
      errors.push(makeError("MALFORMED_EDGE", edge.edge_id));
    }
  }
  const dupEdges = [...seenEdges.entries()].filter(([, n]) => n > 1).map(([id]) => id);
  const dupEdgeElem = firstLex(dupEdges);
  if (dupEdgeElem !== null) errors.push(makeError("DUPLICATE_NODE", dupEdgeElem));

  if (isNonEmptyString(graph.origin) && !seenNodes.has(graph.origin)) {
    errors.push(makeError("MALFORMED_ALLOCATION", "graph.origin"));
  }
  return sortErrors(errors);
}

// Stage-1 envelope shapes shared by every mode. Reports the first violation
// in a fixed deterministic order; null means the container is usable.
function firstEnvelopeFault(allocation, budget) {
  if (!isPlainObject(allocation)) return makeError("MALFORMED_ALLOCATION", null);
  if (!Number.isInteger(allocation.graph_version) || allocation.graph_version < 1) {
    return makeError("MALFORMED_ALLOCATION", "graph_version");
  }
  if (!Array.isArray(allocation.allocated_nodes)) {
    return makeError("MALFORMED_ALLOCATION", "allocated_nodes");
  }
  for (let i = 0; i < allocation.allocated_nodes.length; i++) {
    if (typeof allocation.allocated_nodes[i] !== "string") {
      return makeError("MALFORMED_ALLOCATION", `allocated_nodes[${i}]`);
    }
  }
  if (!Array.isArray(allocation.edge_choices)) {
    return makeError("MALFORMED_ALLOCATION", "edge_choices");
  }
  for (let i = 0; i < allocation.edge_choices.length; i++) {
    const choice = allocation.edge_choices[i];
    if (!isPlainObject(choice) || typeof choice.edge_id !== "string" || typeof choice.variant !== "string") {
      return makeError("MALFORMED_ALLOCATION", `edge_choices[${i}]`);
    }
  }
  if (allocation.selected_node !== undefined && allocation.selected_node !== null && typeof allocation.selected_node !== "string") {
    return makeError("MALFORMED_ALLOCATION", "selected_node");
  }
  if (allocation.calling_order !== undefined) {
    if (!Array.isArray(allocation.calling_order)) {
      return makeError("MALFORMED_ALLOCATION", "calling_order");
    }
    for (let i = 0; i < allocation.calling_order.length; i++) {
      if (typeof allocation.calling_order[i] !== "string") {
        return makeError("MALFORMED_ALLOCATION", `calling_order[${i}]`);
      }
    }
  }
  if (!isPlainObject(budget)) return makeError("MALFORMED_ALLOCATION", "budget");
  for (const field of BUDGET_FIELDS) {
    if (!isLedgerInt(budget[field])) return makeError("MALFORMED_ALLOCATION", `budget.${field}`);
  }
  return null;
}

function budgetEconomyErrors(budget) {
  if (budget.spent > budget.earned) return [makeError("OVERSPENT", null)];
  if (budget.unspent !== budget.earned - budget.spent) {
    return [makeError("MALFORMED_ALLOCATION", "budget.unspent")];
  }
  return [];
}

// Counter-confusion scan over one object's own keys (shallow by design:
// node content is OWNER_PENDING opaque data and is never interpreted).
function counterConfusionCandidates(obj) {
  const offenders = [];
  if (!isPlainObject(obj)) return offenders;
  const keys = Object.keys(obj);

  const strayPointKeys = keys.filter((k) => POINTS_KEY_RE.test(k) && !BUDGET_FIELDS.includes(k));
  const hasBothLedgers = LEDGER_FIELDS.every((f) => keys.includes(f));
  if (strayPointKeys.length > 0 && !hasBothLedgers) {
    offenders.push(firstLex(strayPointKeys));
  }
  if (isPlainObject(obj.live_tree_points)) {
    for (const k of ["source", "wire_identity"]) {
      if (typeof obj.live_tree_points[k] === "string" && obj.live_tree_points[k].includes(COMMISSION_WIRE_IDENTITY)) {
        offenders.push("live_tree_points");
      }
    }
  }
  for (const k of EARNED_SOURCE_KEYS) {
    if (typeof obj[k] === "string" && obj[k].includes(COMMISSION_WIRE_IDENTITY)) {
      offenders.push(k);
    }
  }
  for (const k of ALIAS_KEYS) {
    const v = obj[k];
    if (typeof v !== "string") continue;
    const namesBoth = v.includes(LEDGER_FIELDS[0]) && v.includes(LEDGER_FIELDS[1]);
    const equalsALedger = LEDGER_FIELDS.includes(v);
    if (namesBoth || equalsALedger) offenders.push(k);
  }
  return offenders;
}

function counterConfusionErrors(scannedObjects) {
  const all = scannedObjects.flatMap(counterConfusionCandidates);
  if (all.length === 0) return [];
  return [makeError("COUNTER_CONFUSION", firstLex(all))];
}

// Stages 4-8 over a well-formed allocation against the resolved graph.
function allocationContentErrors(allocation, ctx, budget) {
  const errors = [];
  const knownNodes = ctx.nodeIds;

  // Rank 4: unknown nodes.
  const unknown = allocation.allocated_nodes.filter((id) => !knownNodes.has(id));
  const unknownElem = firstLex(unknown);
  if (unknownElem !== null) errors.push(makeError("UNKNOWN_NODE", unknownElem));

  // Rank 5: duplicate node ids and duplicate edge choices.
  const nodeCounts = new Map();
  for (const id of allocation.allocated_nodes) {
    nodeCounts.set(id, (nodeCounts.get(id) ?? 0) + 1);
  }
  const dupCandidates = [...nodeCounts.entries()].filter(([, n]) => n > 1).map(([id]) => id);
  const choiceCounts = new Map();
  for (const choice of allocation.edge_choices) {
    choiceCounts.set(choice.edge_id, (choiceCounts.get(choice.edge_id) ?? 0) + 1);
  }
  for (const [id, n] of choiceCounts) if (n > 1) dupCandidates.push(id);
  const dupElem = firstLex(dupCandidates);
  if (dupElem !== null) errors.push(makeError("DUPLICATE_NODE", dupElem));

  // Rank 6: malformed edge choices (unknown edge or unknown variant).
  const validChoices = [];
  const malformedCandidates = [];
  for (const choice of allocation.edge_choices) {
    const edge = ctx.edgesById.get(choice.edge_id);
    if (!edge || !edge.variants.includes(choice.variant)) {
      malformedCandidates.push(choice.edge_id);
      continue;
    }
    validChoices.push({ ...choice, from_node: edge.from_node, to_node: edge.to_node });
  }
  const malformedElem = firstLex(malformedCandidates);
  if (malformedElem !== null) errors.push(makeError("MALFORMED_EDGE", malformedElem));

  // Rank 7: connectivity through allocated nodes only.
  const disconnectedCandidates = [];
  const allocatedSet = new Set(allocation.allocated_nodes);
  if (!allocatedSet.has(ctx.graph.origin)) {
    disconnectedCandidates.push(ctx.graph.origin);
  }
  for (const choice of validChoices) {
    if (!allocatedSet.has(choice.from_node) || !allocatedSet.has(choice.to_node)) {
      disconnectedCandidates.push(choice.edge_id);
    }
  }
  const reachable = new Set();
  if (allocatedSet.has(ctx.graph.origin) && knownNodes.has(ctx.graph.origin)) {
    const adjacency = new Map();
    for (const choice of validChoices) {
      for (const [a, b] of [[choice.from_node, choice.to_node], [choice.to_node, choice.from_node]]) {
        if (!allocatedSet.has(a) || !allocatedSet.has(b)) continue;
        if (!adjacency.has(a)) adjacency.set(a, []);
        adjacency.get(a).push(b);
      }
    }
    const queue = [ctx.graph.origin];
    reachable.add(ctx.graph.origin);
    while (queue.length > 0) {
      const current = queue.shift();
      for (const next of adjacency.get(current) ?? []) {
        if (!reachable.has(next)) {
          reachable.add(next);
          queue.push(next);
        }
      }
    }
  }
  for (const id of allocation.allocated_nodes) {
    if (knownNodes.has(id) && !reachable.has(id)) disconnectedCandidates.push(id);
  }
  for (const entry of allocation.calling_order ?? []) {
    if (!allocatedSet.has(entry)) disconnectedCandidates.push(entry);
  }
  const disconnectedElem = firstLex(disconnectedCandidates);
  if (disconnectedElem !== null) errors.push(makeError("DISCONNECTED_ALLOCATION", disconnectedElem));

  // Rank 8: economy, never clamped silently.
  errors.push(...budgetEconomyErrors(budget));

  return sortErrors(errors);
}

function buildAcceptedSnapshot(allocation, effectiveVersion, budget, ctx, extras) {
  const snapshot = {
    envelope: "verdigris.passive-tree-authority/allocation@1",
    graph_version: effectiveVersion,
    allocated_nodes: [...allocation.allocated_nodes],
    edge_choices: allocation.edge_choices.map((c) => ({ edge_id: c.edge_id, variant: c.variant })),
    selected_node:
      allocation.selected_node === undefined
        ? null
        : allocation.selected_node !== null && allocation.allocated_nodes.includes(allocation.selected_node)
          ? allocation.selected_node
          : ctx.graph.origin,
    calling_order: allocation.calling_order === undefined ? [] : [...allocation.calling_order],
    budget: {
      persistent_commission_points: budget.persistent_commission_points,
      live_tree_points: budget.live_tree_points,
      earned: budget.earned,
      spent: budget.spent,
      unspent: budget.unspent,
    },
    validation_provenance: {
      authority: ctx.authorityId,
      result: { ok: true, errors: [] },
    },
  };
  return Object.assign(snapshot, extras ?? {});
}

function blobProvenanceFault(blob) {
  const provenance = blob.validation_provenance;
  if (!isPlainObject(provenance)) return "validation_provenance";
  if (!isNonEmptyString(provenance.authority)) return "validation_provenance.authority";
  if (
    !isPlainObject(provenance.result) ||
    typeof provenance.result.ok !== "boolean" ||
    !Array.isArray(provenance.result.errors)
  ) {
    return "validation_provenance.result";
  }
  return null;
}

function migrationFor(fromVersion, toVersion, authority) {
  if (!Number.isInteger(fromVersion) || fromVersion < 1) return { unsupported: true, transition: `${fromVersion}->${toVersion}` };
  if (fromVersion < authority.migration_floor) {
    return { unsupported: true, transition: `${fromVersion}->${toVersion}` };
  }
  if (toVersion !== authority.graph_version) {
    return { unsupported: true, transition: `${fromVersion}->${toVersion}` };
  }
  const registered = authority.registered_migrations.find(
    (m) => m.from_version === fromVersion && m.to_version === toVersion,
  );
  if (!registered) return { unsupported: true, transition: `${fromVersion}->${toVersion}` };
  return { strategy: registered.strategy };
}

function validateAuthority(authority) {
  const faults = [];
  if (!isPlainObject(authority)) return ["authority"];
  if (!Number.isInteger(authority.graph_version) || authority.graph_version < 1) faults.push("authority.graph_version");
  if (!Number.isInteger(authority.migration_floor) || authority.migration_floor < 1) faults.push("authority.migration_floor");
  if (!Array.isArray(authority.registered_migrations)) {
    faults.push("authority.registered_migrations");
  } else {
    for (let i = 0; i < authority.registered_migrations.length; i++) {
      const m = authority.registered_migrations[i];
      const ok =
        isPlainObject(m) &&
        Number.isInteger(m.from_version) &&
        m.from_version >= 1 &&
        Number.isInteger(m.to_version) &&
        m.to_version >= 1 &&
        MIGRATION_STRATEGIES.includes(m.strategy);
      if (!ok) faults.push(`authority.registered_migrations[${i}]`);
    }
  }
  return faults;
}

function validateCase(caseDef, index, ctx) {
  const caseId = isNonEmptyString(caseDef.case_id) ? caseDef.case_id : `cases[${index}]`;
  const result = { case_id: caseId, mode: caseDef.mode ?? null, ok: false, errors: [] };

  if (!MODES.includes(caseDef.mode)) {
    result.mode = null;
    result.errors = [makeMetaError("INVALID_FIXTURE", `${caseId}.mode`)];
    return result;
  }

  const usesAllocationDirectly = caseDef.mode === "allocation" || caseDef.mode === "raw_snapshot";
  const claimSource = usesAllocationDirectly
    ? caseDef.mode === "allocation"
      ? caseDef.allocation
      : caseDef.raw_snapshot
    : caseDef.blob;

  // Stage 1: envelope shapes (stop rank).
  const envelopeFault = firstEnvelopeFault(
    usesAllocationDirectly ? claimSource : isPlainObject(claimSource) ? claimSource.allocation : undefined,
    caseDef.budget,
  );
  if (envelopeFault) {
    result.errors = [envelopeFault];
    return result;
  }
  const allocation = usesAllocationDirectly ? claimSource : claimSource.allocation;
  const budget = caseDef.budget;

  // Persistence-specific stage 1: trusted-blob provenance (raw snapshots are
  // never accepted as persistence without it).
  let provenanceFault = null;
  if (caseDef.mode === "persistence" || caseDef.mode === "migration_request") {
    if (!isPlainObject(claimSource)) {
      result.errors = [makeError("MALFORMED_ALLOCATION", "blob")];
      return result;
    }
    if (!Number.isInteger(claimSource.graph_version) || claimSource.graph_version < 1) {
      result.errors = [makeError("MALFORMED_ALLOCATION", "blob.graph_version")];
      return result;
    }
    provenanceFault = blobProvenanceFault(claimSource);
    if (provenanceFault) {
      result.errors = [makeError("MALFORMED_ALLOCATION", provenanceFault)];
      return result;
    }
    if (caseDef.mode === "migration_request") {
      const request = caseDef.migration_request;
      if (
        !isPlainObject(request) ||
        !Number.isInteger(request.from_version) ||
        request.from_version < 1 ||
        !Number.isInteger(request.to_version) ||
        request.to_version < 1
      ) {
        result.errors = [makeError("MALFORMED_ALLOCATION", "migration_request")];
        return result;
      }
      if (request.from_version !== claimSource.graph_version) {
        result.errors = [makeError("MALFORMED_ALLOCATION", "migration_request.from_version")];
        return result;
      }
    }
  }

  // Stage 2 / 3: version resolution and migration applicability (stop ranks).
  let effectiveAllocation = allocation;
  let effectiveVersion = ctx.authority.graph_version;
  let extras;
  if (caseDef.mode === "allocation" || caseDef.mode === "raw_snapshot") {
    if (allocation.graph_version !== ctx.authority.graph_version) {
      result.errors = [makeError("UNKNOWN_GRAPH_VERSION", null)];
      return result;
    }
  } else {
    const blobVersion = claimSource.graph_version;
    if (blobVersion !== ctx.authority.graph_version) {
      const requested =
        caseDef.mode === "migration_request"
          ? { from_version: caseDef.migration_request.from_version, to_version: caseDef.migration_request.to_version }
          : { from_version: blobVersion, to_version: ctx.authority.graph_version };
      const outcome = migrationFor(requested.from_version, requested.to_version, ctx.authority);
      if (outcome.unsupported) {
        result.errors = [makeError("UNSUPPORTED_MIGRATION", outcome.transition)];
        return result;
      }
      if (outcome.strategy === "full_refund_reset") {
        effectiveAllocation = {
          graph_version: ctx.authority.graph_version,
          allocated_nodes: [ctx.graph.origin],
          edge_choices: [],
        };
      } else {
        effectiveAllocation = { ...claimSource.allocation, graph_version: ctx.authority.graph_version };
      }
      extras = { audit: { pre_migration_graph_version: blobVersion, strategy: outcome.strategy } };
    }
    // A current-version blob re-validates its stored allocation form directly.
  }
  if (effectiveAllocation.graph_version !== ctx.authority.graph_version) {
    result.errors = [makeError("UNKNOWN_GRAPH_VERSION", String(effectiveAllocation.graph_version))];
    return result;
  }

  // Stages 4-8 on the effective form, then the cross-cutting rank-9 scan.
  const contentErrors = allocationContentErrors(effectiveAllocation, ctx, budget);
  const ccErrors = counterConfusionErrors([
    budget,
    ...(caseDef.mode === "persistence" || caseDef.mode === "migration_request" ? [claimSource] : []),
    ...(caseDef.mode === "raw_snapshot" ? [claimSource] : []),
  ]);
  result.errors = sortErrors([...contentErrors, ...ccErrors]);
  if (result.errors.length === 0) {
    result.ok = true;
    result.accepted_snapshot = buildAcceptedSnapshot(
      effectiveAllocation,
      effectiveVersion,
      budget,
      ctx,
      caseDef.mode === "raw_snapshot"
        ? { ...(extras ?? {}), rebuilt_from_raw_snapshot: true }
        : extras,
    );
  }
  return result;
}

// Pure core: validate an already-parsed fixture document against an
// already-checked contract. Deterministic: same inputs, same output.
export function evaluateFixture(contract, fixture) {
  const base = {
    tool: "validate-passive-tree-contract",
    task: "TASK-0136",
    contract: {
      contract_id: contract.contract_id,
      schema_version: contract.schema_version,
    },
    fixture_set: isPlainObject(fixture) && isNonEmptyString(fixture.fixture_set) ? fixture.fixture_set : null,
    ok: false,
    results: [],
    errors: [],
  };

  const authorityFaults = validateAuthority(fixture?.authority);
  if (authorityFaults.length > 0) {
    base.errors = authorityFaults.map((f) => makeMetaError("INVALID_FIXTURE", f));
    return base;
  }
  const authority = fixture.authority;

  const graphErrors = validateGraphDocument(fixture.graph, authority);
  if (graphErrors.length > 0) {
    base.results = [{ case_id: "__graph_document__", mode: "graph", ok: false, errors: graphErrors }];
    base.errors = [...graphErrors];
    return base;
  }

  const ctx = {
    authority,
    authorityId: `task-0136-validator@${contract.schema_version}`,
    graph: fixture.graph,
    nodeIds: new Set(fixture.graph.nodes.map((n) => n.node_id)),
    edgesById: new Map(fixture.graph.edges.map((e) => [e.edge_id, e])),
  };

  if (!Array.isArray(fixture.cases)) {
    base.errors = [makeMetaError("INVALID_FIXTURE", "cases")];
    return base;
  }

  base.results = fixture.cases.map((caseDef, index) =>
    validateCase(isPlainObject(caseDef) ? caseDef : {}, index, ctx),
  );
  const aggregated = [];
  base.results.forEach((r, caseIndex) => {
    for (const err of r.errors) aggregated.push({ case_index: caseIndex, ...err });
  });
  aggregated.sort((a, b) => a.case_index - b.case_index || a.rank - b.rank || cmp(String(a.element), String(b.element)));
  base.errors = aggregated.map(({ case_index, ...err }) => err);
  base.ok = base.results.every((r) => r.ok);
  return base;
}

export function runValidation(contractPath, fixturePath) {
  const contractLoaded = loadJsonFile(contractPath, "contract");
  const contractCheck = validateContractShape(contractLoaded.json);
  if (!contractCheck.ok) {
    return {
      tool: "validate-passive-tree-contract",
      task: "TASK-0136",
      contract: {
        contract_id: isPlainObject(contractLoaded.json) ? contractLoaded.json.contract_id ?? null : null,
        schema_version: isPlainObject(contractLoaded.json) ? contractLoaded.json.schema_version ?? null : null,
        path: contractLoaded.absolute,
      },
      fixture_set: null,
      fixture_path: null,
      contract_ok: false,
      ok: false,
      results: [],
      errors: sortErrors(contractCheck.errors),
    };
  }
  const fixtureLoaded = loadJsonFile(fixturePath, "fixture");
  const result = evaluateFixture(contractLoaded.json, fixtureLoaded.json);
  result.contract.path = contractLoaded.absolute;
  result.fixture_path = fixtureLoaded.absolute;
  result.contract_ok = true;
  return result;
}

function printHuman(result) {
  console.log(
    `CONTRACT ${result.contract_ok ? "OK" : "INVALID"} ${result.contract.contract_id}@${result.contract.schema_version}`,
  );
  for (const r of result.results) {
    if (r.ok) {
      console.log(`CASE OK ${r.case_id} ${r.mode}`);
    } else {
      for (const err of r.errors) {
        console.log(`CASE FAIL ${r.case_id} ${err.code}${err.element === null ? "" : ` ${err.element}`}`);
      }
    }
  }
  console.log(`RESULT ok=${result.ok} cases=${result.results.length} error_count=${result.errors.length}`);
}

function main() {
  let args;
  try {
    args = parseArgs(argv.slice(2));
  } catch (err) {
    console.error(`usage error: ${err.message}`);
    console.error(
      "usage: node validate-passive-tree-contract.mjs --contract <contract.json> --fixture <candidates.json> [--json]",
    );
    return exit(2);
  }
  let result;
  try {
    result = runValidation(args.contract, args.fixture);
  } catch (err) {
    console.error(err.message);
    return exit(2);
  }
  if (args.json) {
    console.log(JSON.stringify(result, null, 2));
  } else {
    printHuman(result);
  }
  return exit(result.ok ? 0 : 1);
}

const invokedDirectly =
  argv[1] !== undefined && import.meta.url === pathToFileURL(resolve(argv[1])).href;

if (invokedDirectly) main();
