#!/usr/bin/env node
import { readFileSync } from "node:fs";
import { fileURLToPath } from "node:url";
import { dirname, resolve } from "node:path";

const CODES = [
  "MALFORMED_ALLOCATION",
  "UNKNOWN_GRAPH_VERSION",
  "UNSUPPORTED_MIGRATION",
  "UNKNOWN_NODE",
  "DUPLICATE_NODE",
  "MALFORMED_EDGE",
  "DISCONNECTED_ALLOCATION",
  "OVERSPENT",
  "COUNTER_CONFUSION",
];

const MESSAGE_KEYS = {
  MALFORMED_ALLOCATION: "errors.malformed_allocation",
  UNKNOWN_GRAPH_VERSION: "errors.unknown_graph_version",
  UNSUPPORTED_MIGRATION: "errors.unsupported_migration",
  UNKNOWN_NODE: "errors.unknown_node",
  DUPLICATE_NODE: "errors.duplicate_node",
  MALFORMED_EDGE: "errors.malformed_edge",
  DISCONNECTED_ALLOCATION: "errors.disconnected_allocation",
  OVERSPENT: "errors.overspent",
  COUNTER_CONFUSION: "errors.counter_confusion",
};

const LEDGER_KEYS = ["persistent_commission_points", "live_tree_points"];

const ENVELOPES = [
  "graph",
  "node",
  "edge",
  "allocation",
  "budget",
  "validation_result",
  "migration",
  "persistence",
];

function isPlainObject(value) {
  return typeof value === "object" && value !== null && !Array.isArray(value);
}

function isNonNegativeInt(value) {
  return typeof value === "number" && Number.isInteger(value) && value >= 0;
}

function sortedUnique(values) {
  return [...new Set(values)].sort((a, b) => (a < b ? -1 : a > b ? 1 : 0));
}

function compareElements(a, b) {
  const sa = a === null ? "" : a;
  const sb = b === null ? "" : b;
  return sa < sb ? -1 : sa > sb ? 1 : 0;
}

export function checkContract(contract) {
  const problems = [];
  if (!isPlainObject(contract)) {
    return { ok: false, problems: ["contract is not an object"] };
  }
  if (typeof contract.contract_id !== "string" || contract.contract_id.length === 0) {
    problems.push("contract.contract_id must be a non-empty string");
  }
  if (typeof contract.schema_version !== "string" || contract.schema_version.length === 0) {
    problems.push("contract.schema_version must be a non-empty string");
  }
  if (
    !isPlainObject(contract.counter_separation) ||
    contract.counter_separation.violation_error !== "COUNTER_CONFUSION"
  ) {
    problems.push("contract.counter_separation.violation_error must equal COUNTER_CONFUSION");
  }
  for (const name of ENVELOPES) {
    const section = contract[name];
    if (!isPlainObject(section)) {
      problems.push(`contract.${name} envelope missing`);
      continue;
    }
    if (typeof section.envelope !== "string" || section.envelope.length === 0) {
      problems.push(`contract.${name}.envelope must be a non-empty string`);
    }
  }
  const enumCodes = contract.validation_result?.properties?.errors?.items?.properties?.code?.enum;
  if (!Array.isArray(enumCodes) || CODES.some((code) => !enumCodes.includes(code))) {
    problems.push("contract.validation_result error code enum does not cover all nine required codes");
  }
  return { ok: problems.length === 0, problems };
}

function scanLedgerCollapse(record, prefix, errors) {
  if (!isPlainObject(record)) return;
  for (const key of Object.keys(record).sort()) {
    const value = record[key];
    if (!isPlainObject(value)) continue;
    const path = `${prefix}${key}`;
    if (value.merged_ledger === true) {
      errors.push({ code: "COUNTER_CONFUSION", element: path });
      continue;
    }
    if (typeof value.used_for === "string") {
      const lowered = value.used_for.toLowerCase();
      if (lowered.includes("commission") && lowered.includes("tree")) {
        errors.push({ code: "COUNTER_CONFUSION", element: path });
        continue;
      }
    }
    const aliasSource = value.alias_of ?? value.derived_from;
    if (
      typeof aliasSource === "string" &&
      LEDGER_KEYS.includes(key) &&
      LEDGER_KEYS.includes(aliasSource) &&
      aliasSource !== key
    ) {
      errors.push({ code: "COUNTER_CONFUSION", element: path });
    }
  }
}

function checkBudgetContext(budget, errors) {
  if (!isPlainObject(budget)) {
    errors.push({ code: "MALFORMED_ALLOCATION", element: "context.budget" });
    return false;
  }
  for (const ledgerKey of LEDGER_KEYS) {
    if (!isNonNegativeInt(budget[ledgerKey])) {
      errors.push({ code: "MALFORMED_ALLOCATION", element: `context.budget.${ledgerKey}` });
    }
  }
  if (!isNonNegativeInt(budget.earned)) {
    errors.push({ code: "MALFORMED_ALLOCATION", element: "context.budget.earned" });
  }
  const source = budget.designated_earned_source;
  if (typeof source !== "string" || !LEDGER_KEYS.includes(source)) {
    errors.push({ code: "COUNTER_CONFUSION", element: "context.budget.earned" });
  }
  const costRule = budget.cost_rule;
  if (
    !isPlainObject(costRule) ||
    !isNonNegativeInt(costRule.per_non_origin_node) ||
    !isNonNegativeInt(costRule.per_edge_choice)
  ) {
    errors.push({ code: "MALFORMED_ALLOCATION", element: "context.budget.cost_rule" });
  }
  scanLedgerCollapse(budget, "context.budget.", errors);
  return true;
}

function checkGraphContext(graph, errors) {
  if (!isPlainObject(graph)) {
    errors.push({ code: "MALFORMED_ALLOCATION", element: "context.graph" });
    return;
  }
  if (!isNonNegativeInt(graph.graph_version) || graph.graph_version < 1) {
    errors.push({ code: "MALFORMED_ALLOCATION", element: "context.graph.graph_version" });
  }
  if (typeof graph.origin !== "string" || graph.origin.length === 0) {
    errors.push({ code: "MALFORMED_ALLOCATION", element: "context.graph.origin" });
  }
  if (!Array.isArray(graph.nodes) || !graph.nodes.every((n) => isPlainObject(n) && typeof n.node_id === "string")) {
    errors.push({ code: "MALFORMED_ALLOCATION", element: "context.graph.nodes" });
    return;
  }
  if (!Array.isArray(graph.edges)) {
    errors.push({ code: "MALFORMED_ALLOCATION", element: "context.graph.edges" });
    return;
  }
  const nodeIds = graph.nodes.map((n) => n.node_id);
  if (new Set(nodeIds).size !== nodeIds.length) {
    errors.push({ code: "MALFORMED_ALLOCATION", element: "context.graph.nodes" });
  }
  const nodeIdSet = new Set(nodeIds);
  const edgeIds = [];
  for (const edge of graph.edges) {
    if (
      !isPlainObject(edge) ||
      typeof edge.edge_id !== "string" ||
      typeof edge.from_node !== "string" ||
      typeof edge.to_node !== "string" ||
      !Array.isArray(edge.variants) ||
      edge.variants.length < 1 ||
      !edge.variants.every((v) => typeof v === "string")
    ) {
      errors.push({ code: "MALFORMED_ALLOCATION", element: "context.graph.edges" });
      continue;
    }
    edgeIds.push(edge.edge_id);
    if (!nodeIdSet.has(edge.from_node) || !nodeIdSet.has(edge.to_node)) {
      errors.push({ code: "MALFORMED_ALLOCATION", element: `context.graph.edges.${edge.edge_id}` });
    }
  }
  if (new Set(edgeIds).size !== edgeIds.length) {
    errors.push({ code: "MALFORMED_ALLOCATION", element: "context.graph.edges" });
  }
  if (typeof graph.origin === "string" && !nodeIdSet.has(graph.origin)) {
    errors.push({ code: "MALFORMED_ALLOCATION", element: "context.graph.origin" });
  }
}

function checkShape(allocation, errors) {
  if (!isPlainObject(allocation)) {
    errors.push({ code: "MALFORMED_ALLOCATION", element: null });
    return false;
  }
  if (!isNonNegativeInt(allocation.graph_version) || allocation.graph_version < 1) {
    errors.push({ code: "MALFORMED_ALLOCATION", element: "graph_version" });
  }
  if (!Array.isArray(allocation.allocated_nodes) || !allocation.allocated_nodes.every((n) => typeof n === "string")) {
    errors.push({ code: "MALFORMED_ALLOCATION", element: "allocated_nodes" });
  }
  const choicesOk =
    Array.isArray(allocation.edge_choices) &&
    allocation.edge_choices.every(
      (c) => isPlainObject(c) && typeof c.edge_id === "string" && typeof c.variant === "string"
    );
  if (!choicesOk) {
    errors.push({ code: "MALFORMED_ALLOCATION", element: "edge_choices" });
  }
  if (allocation.selected_node !== undefined && allocation.selected_node !== null && typeof allocation.selected_node !== "string") {
    errors.push({ code: "MALFORMED_ALLOCATION", element: "selected_node" });
  }
  if (
    allocation.calling_order !== undefined &&
    (!Array.isArray(allocation.calling_order) || !allocation.calling_order.every((n) => typeof n === "string"))
  ) {
    errors.push({ code: "MALFORMED_ALLOCATION", element: "calling_order" });
  }
  return (
    isNonNegativeInt(allocation.graph_version) &&
    allocation.graph_version >= 1 &&
    Array.isArray(allocation.allocated_nodes) &&
    allocation.allocated_nodes.every((n) => typeof n === "string") &&
    choicesOk
  );
}

function runGraphStages(allocation, graph, approvedVersions, errors) {
  if (!approvedVersions.includes(allocation.graph_version)) {
    errors.push({ code: "UNKNOWN_GRAPH_VERSION", element: "graph_version" });
    return;
  }
  const nodeIds = sortedUnique(graph.nodes.map((n) => n.node_id));
  const allocated = allocation.allocated_nodes;
  const allocatedSet = new Set(allocated);
  const sortedAllocated = sortedUnique(allocated);

  const unknown = sortedAllocated.find((id) => !nodeIds.includes(id));
  if (unknown !== undefined) {
    errors.push({ code: "UNKNOWN_NODE", element: unknown });
  }

  const nodeCounts = new Map();
  for (const id of allocated) nodeCounts.set(id, (nodeCounts.get(id) ?? 0) + 1);
  const dupNode = sortedAllocated.find((id) => nodeCounts.get(id) > 1);
  if (dupNode !== undefined) {
    errors.push({ code: "DUPLICATE_NODE", element: dupNode });
  } else {
    const edgeCounts = new Map();
    for (const choice of allocation.edge_choices) {
      edgeCounts.set(choice.edge_id, (edgeCounts.get(choice.edge_id) ?? 0) + 1);
    }
    const dupEdge = sortedUnique([...allocation.edge_choices.map((c) => c.edge_id)]).find(
      (id) => edgeCounts.get(id) > 1
    );
    if (dupEdge !== undefined) {
      errors.push({ code: "DUPLICATE_NODE", element: dupEdge });
    }
  }

  const graphEdges = sortedUnique(graph.edges.map((e) => e.edge_id));
  const variantsByEdge = new Map(graph.edges.map((e) => [e.edge_id, e.variants]));
  const endpointsByEdge = new Map(graph.edges.map((e) => [e.edge_id, [e.from_node, e.to_node]]));
  const sortedChoices = [...allocation.edge_choices].sort((a, b) =>
    compareElements(a.edge_id, b.edge_id)
  );
  const malformedEdge = sortedChoices.find(
    (c) => !graphEdges.includes(c.edge_id) || !(variantsByEdge.get(c.edge_id) ?? []).includes(c.variant)
  );
  if (malformedEdge !== undefined) {
    errors.push({ code: "MALFORMED_EDGE", element: malformedEdge.edge_id });
  }

  const disconnectedEdge = sortedChoices.find((c) => {
    const endpoints = endpointsByEdge.get(c.edge_id);
    if (!endpoints) return false;
    return endpoints.some((endpoint) => !allocatedSet.has(endpoint));
  });
  if (disconnectedEdge !== undefined) {
    errors.push({ code: "DISCONNECTED_ALLOCATION", element: disconnectedEdge.edge_id });
  } else if (!allocatedSet.has(graph.origin)) {
    errors.push({ code: "DISCONNECTED_ALLOCATION", element: graph.origin });
  } else {
    const adjacency = new Map();
    for (const choice of allocation.edge_choices) {
      const endpoints = endpointsByEdge.get(choice.edge_id);
      if (!endpoints) continue;
      const [a, b] = endpoints;
      if (!adjacency.has(a)) adjacency.set(a, []);
      if (!adjacency.has(b)) adjacency.set(b, []);
      adjacency.get(a).push(b);
      adjacency.get(b).push(a);
    }
    const seen = new Set([graph.origin]);
    const queue = [graph.origin];
    while (queue.length > 0) {
      const current = queue.shift();
      for (const neighbor of adjacency.get(current) ?? []) {
        if (!seen.has(neighbor)) {
          seen.add(neighbor);
          queue.push(neighbor);
        }
      }
    }
    const unreachable = sortedAllocated.find((id) => !seen.has(id));
    if (unreachable !== undefined) {
      errors.push({ code: "DISCONNECTED_ALLOCATION", element: unreachable });
    }
  }
}

function runBudgetStage(allocation, budget, errors) {
  const allocatedSet = new Set(allocation.allocated_nodes);
  const nonOriginCount = [...allocatedSet].filter((id) => id !== budget.origin_id).size;
  const spent =
    budget.cost_rule.per_non_origin_node * nonOriginCount +
    budget.cost_rule.per_edge_choice * allocation.edge_choices.length;
  if (budget.spent !== undefined) {
    if (!isNonNegativeInt(budget.spent)) {
      errors.push({ code: "MALFORMED_ALLOCATION", element: "context.budget.spent" });
    } else if (budget.spent !== spent) {
      errors.push({ code: "MALFORMED_ALLOCATION", element: "context.budget.spent" });
    }
  }
  if (budget.unspent !== undefined && (!isNonNegativeInt(budget.unspent) || budget.unspent !== budget.earned - spent)) {
    errors.push({ code: "MALFORMED_ALLOCATION", element: "context.budget.unspent" });
  }
  if (spent > budget.earned) {
    errors.push({ code: "OVERSPENT", element: "budget" });
  }
  return { spent, unspent: budget.earned - spent };
}

function runMigrationStage(blob, migrationRequest, context, errors) {
  const approvedVersions = context.approved_graph_versions;
  if (approvedVersions.includes(blob.graph_version)) {
    return { allocation: blob.allocation, graphVersion: blob.graph_version };
  }
  if (!isPlainObject(migrationRequest)) {
    errors.push({ code: "UNSUPPORTED_MIGRATION", element: "blob" });
    return null;
  }
  const { from_version, to_version } = migrationRequest;
  if (
    !isNonNegativeInt(from_version) ||
    !isNonNegativeInt(to_version) ||
    from_version < 1 ||
    to_version < 1
  ) {
    errors.push({ code: "MALFORMED_ALLOCATION", element: "migration_request" });
    return null;
  }
  if (from_version !== blob.graph_version) {
    errors.push({ code: "MALFORMED_ALLOCATION", element: "migration_request.from_version" });
    return null;
  }
  const registered = context.registered_migrations?.[`${from_version}->${to_version}`];
  if (registered === undefined || !approvedVersions.includes(to_version)) {
    errors.push({ code: "UNSUPPORTED_MIGRATION", element: `migration:${from_version}->${to_version}` });
    return null;
  }
  if (registered === "full_refund_reset") {
    return { allocation: null, graphVersion: to_version, refundReset: true };
  }
  if (registered !== "revalidate_in_place") {
    errors.push({ code: "UNSUPPORTED_MIGRATION", element: `migration:${from_version}->${to_version}` });
    return null;
  }
  return { allocation: blob.allocation, graphVersion: to_version };
}

function validateAllocationClaim(payload, context, errors) {
  if (!checkShape(payload, errors)) return null;
  const graph = context.graph;
  runGraphStages(payload, graph, context.approved_graph_versions, errors);
  const budget = { ...context.budget, origin_id: graph.origin };
  if (
    !errors.some(
      (e) => e.code === "MALFORMED_ALLOCATION" && e.element === "context.budget.cost_rule"
    )
  ) {
    return runBudgetStage(payload, budget, errors);
  }
  return null;
}

function validatePersistence(payload, context, errors) {
  const blob = payload?.blob;
  if (!isPlainObject(blob)) {
    errors.push({ code: "MALFORMED_ALLOCATION", element: "blob" });
    return;
  }
  if (
    !isPlainObject(blob.validation_provenance) ||
    typeof blob.validation_provenance.authority !== "string" ||
    !isPlainObject(blob.validation_provenance.result)
  ) {
    errors.push({ code: "MALFORMED_ALLOCATION", element: "blob.validation_provenance" });
  }
  scanLedgerCollapse(blob, "blob.", errors);
  const migrated = runMigrationStage(blob, payload.migration_request ?? blob.migration_request, context, errors);
  if (migrated === null) return;
  if (migrated.refundReset) {
    return;
  }
  validateAllocationClaim(migrated.allocation, context, errors);
}

export function validate(fixture) {
  const errors = [];
  let snapshot = null;
  if (!isPlainObject(fixture)) {
    errors.push({ code: "MALFORMED_ALLOCATION", element: null });
    return { ok: false, errors: finalize(errors) };
  }
  const context = fixture.context;
  if (!isPlainObject(context)) {
    errors.push({ code: "MALFORMED_ALLOCATION", element: "context" });
    return { ok: false, errors: finalize(errors) };
  }
  if (
    !Array.isArray(context.approved_graph_versions) ||
    !context.approved_graph_versions.every(isNonNegativeInt)
  ) {
    errors.push({ code: "MALFORMED_ALLOCATION", element: "context.approved_graph_versions" });
  }
  checkGraphContext(context.graph, errors);
  checkBudgetContext(context.budget, errors);
  const candidate = fixture.candidate;
  if (!isPlainObject(candidate) || typeof candidate.kind !== "string") {
    errors.push({ code: "MALFORMED_ALLOCATION", element: "candidate" });
    return { ok: false, errors: finalize(errors) };
  }
  let budgetOutcome = null;
  if (candidate.kind === "allocation") {
    budgetOutcome = validateAllocationClaim(candidate.payload, context, errors);
  } else if (candidate.kind === "persistence" || candidate.kind === "migration") {
    validatePersistence(candidate.payload, context, errors);
  } else {
    errors.push({ code: "MALFORMED_ALLOCATION", element: "candidate.kind" });
  }
  const finalized = finalize(errors);
  if (finalized.length === 0 && budgetOutcome !== null) {
    snapshot = {
      graph_version: candidate.payload.graph_version,
      allocated_nodes: sortedUnique(candidate.payload.allocated_nodes),
      edge_choices: [...candidate.payload.edge_choices].sort((a, b) =>
        compareElements(a.edge_id, b.edge_id)
      ),
      earned: context.budget.earned,
      spent: budgetOutcome.spent,
      unspent: budgetOutcome.unspent,
      ledgers: {
        persistent_commission_points: context.budget.persistent_commission_points,
        live_tree_points: context.budget.live_tree_points,
        designated_earned_source: context.budget.designated_earned_source,
      },
      validation_provenance: {
        authority: "TASK-0136-passive-tree-contract-validator",
        contract_schema_version: "1.0.0",
      },
    };
  }
  return { ok: finalized.length === 0, errors: finalized, ...(snapshot ? { accepted_snapshot: snapshot } : {}) };
}

function finalize(errors) {
  const rankOf = (code) => CODES.indexOf(code) + 1;
  const seen = new Set();
  const deduped = [];
  for (const error of errors) {
    const key = `${error.code}|${error.element}`;
    if (!seen.has(key)) {
      seen.add(key);
      deduped.push(error);
    }
  }
  return deduped
    .map((error) => ({
      code: error.code,
      element: error.element ?? null,
      message_key: MESSAGE_KEYS[error.code],
    }))
    .sort((a, b) => rankOf(a.code) - rankOf(b.code) || compareElements(a.element, b.element));
}

function usage() {
  return "usage: node validate-passive-tree-contract.mjs --contract <path> --fixture <path> [--json]";
}

function main(argv) {
  const args = { json: false };
  for (let i = 0; i < argv.length; i += 1) {
    const arg = argv[i];
    if (arg === "--json") {
      args.json = true;
    } else if (arg === "--contract" || arg === "--fixture") {
      const value = argv[i + 1];
      if (value === undefined) {
        process.stderr.write(`${usage()}\n`);
        return 2;
      }
      args[arg.slice(2)] = value;
      i += 1;
    } else {
      process.stderr.write(`${usage()}\n`);
      return 2;
    }
  }
  if (!args.contract || !args.fixture) {
    process.stderr.write(`${usage()}\n`);
    return 2;
  }
  let contract;
  let fixture;
  try {
    contract = JSON.parse(readFileSync(resolve(args.contract), "utf8"));
  } catch (error) {
    process.stderr.write(`contract unreadable: ${error.message}\n`);
    return 2;
  }
  try {
    fixture = JSON.parse(readFileSync(resolve(args.fixture), "utf8"));
  } catch (error) {
    process.stderr.write(`fixture unreadable: ${error.message}\n`);
    return 2;
  }
  const contractCheck = checkContract(contract);
  if (!contractCheck.ok) {
    const body = {
      ok: false,
      contract_error: { element: "contract", problems: contractCheck.problems },
      errors: [],
    };
    process.stdout.write(`${JSON.stringify(body, null, 2)}\n`);
    return 2;
  }
  const result = validate(fixture);
  process.stdout.write(`${JSON.stringify(result, null, 2)}\n`);
  return result.ok ? 0 : 1;
}

const isMain =
  process.argv[1] !== undefined && fileURLToPath(import.meta.url) === resolve(process.argv[1]);

if (isMain) {
  process.exit(main(process.argv.slice(2)));
}
