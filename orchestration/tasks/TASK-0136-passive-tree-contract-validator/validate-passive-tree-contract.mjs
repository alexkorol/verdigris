#!/usr/bin/env node

import fs from 'node:fs';
import path from 'node:path';
import process from 'node:process';
import { pathToFileURL } from 'node:url';

export const CODE_RANK = Object.freeze({
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

const REQUIRED_CONTRACT_CODES = [
  'UNKNOWN_GRAPH_VERSION',
  'UNSUPPORTED_MIGRATION',
  'UNKNOWN_NODE',
  'DUPLICATE_NODE',
  'MALFORMED_EDGE',
  'DISCONNECTED_ALLOCATION',
  'OVERSPENT',
  'COUNTER_CONFUSION',
];

const COMMISSION_MARKER = /questpoints|commission/i;
const TREE_MARKER = /live[_-]?tree|tree[_-]?budget/i;
const LEDGER_KEYS = ['persistent_commission_points', 'live_tree_points'];

function err(code, element) {
  return { code, element: element === undefined ? null : element, message_key: code };
}

function cmpStr(a, b) {
  const sa = a === null || a === undefined ? '' : String(a);
  const sb = b === null || b === undefined ? '' : String(b);
  return sa < sb ? -1 : sa > sb ? 1 : 0;
}

function sortErrors(errors) {
  return [...errors].sort(
    (a, b) =>
      CODE_RANK[a.code] - CODE_RANK[b.code] ||
      cmpStr(a.element, b.element) ||
      cmpStr(a.case_id, b.case_id),
  );
}

function isPlainObject(v) {
  return typeof v === 'object' && v !== null && !Array.isArray(v);
}

function isNonNegativeInt(v) {
  return Number.isInteger(v) && v >= 0;
}

export function validateContractDocument(contract) {
  const problems = [];
  if (!isPlainObject(contract)) {
    return { ok: false, contract_errors: ['contract document is not a JSON object'] };
  }
  for (const key of ['schema_version', 'graph', 'allocation', 'budget', 'validation_result', 'migration', 'persistence']) {
    if (!(key in contract)) problems.push(`contract missing required section '${key}'`);
  }
  if (isPlainObject(contract.budget)) {
    for (const ledger of LEDGER_KEYS) {
      if (!isPlainObject(contract.budget[ledger])) {
        problems.push(`contract budget missing distinct '${ledger}' descriptor`);
      }
    }
    if (
      isPlainObject(contract.budget.persistent_commission_points) &&
      isPlainObject(contract.budget.live_tree_points) &&
      contract.budget.persistent_commission_points === contract.budget.live_tree_points
    ) {
      problems.push('contract counters collapsed: persistent_commission_points and live_tree_points are the same object');
    }
  }
  const enumCodes = contract?.validation_result?.errors?.items?.properties?.code?.enum;
  if (!Array.isArray(enumCodes)) {
    problems.push("contract validation_result.errors enum missing");
  } else {
    for (const code of REQUIRED_CONTRACT_CODES) {
      if (!enumCodes.includes(code)) problems.push(`contract error enum missing required code '${code}'`);
    }
  }
  if (contract?.counter_separation?.violation_error !== 'COUNTER_CONFUSION') {
    problems.push("contract counter_separation.violation_error must be 'COUNTER_CONFUSION'");
  }
  return { ok: problems.length === 0, contract_errors: problems };
}

function buildGraphIndex(graphDoc) {
  const errors = [];
  if (!isPlainObject(graphDoc)) {
    return { errors: [err('MALFORMED_ALLOCATION', null)], index: null };
  }
  const { graph_version, origin, nodes, edges } = graphDoc;
  if (!Number.isInteger(graph_version) || graph_version < 1) {
    errors.push(err('MALFORMED_ALLOCATION', null));
  }
  if (typeof origin !== 'string') {
    errors.push(err('MALFORMED_ALLOCATION', null));
  }
  if (!Array.isArray(nodes)) {
    errors.push(err('MALFORMED_ALLOCATION', null));
  }
  if (!Array.isArray(edges)) {
    errors.push(err('MALFORMED_ALLOCATION', null));
  }
  if (errors.length > 0) {
    return { errors: sortErrors(errors), index: null };
  }
  const nodeSet = new Set();
  const seenNodes = new Map();
  for (const node of nodes) {
    if (!isPlainObject(node) || typeof node.node_id !== 'string') {
      errors.push(err('MALFORMED_ALLOCATION', null));
      continue;
    }
    if (seenNodes.has(node.node_id)) {
      seenNodes.set(node.node_id, true);
      continue;
    }
    seenNodes.set(node.node_id, false);
  }
  for (const [id, dup] of seenNodes) {
    if (dup) errors.push(err('DUPLICATE_NODE', id));
    else nodeSet.add(id);
  }
  const edgeIndex = new Map();
  const seenEdges = new Set();
  for (const edge of edges) {
    if (!isPlainObject(edge) || typeof edge.edge_id !== 'string') {
      errors.push(err('MALFORMED_EDGE', null));
      continue;
    }
    const id = edge.edge_id;
    if (
      typeof edge.from_node !== 'string' ||
      typeof edge.to_node !== 'string' ||
      !Array.isArray(edge.variants) ||
      edge.variants.length < 1 ||
      !edge.variants.every((v) => typeof v === 'string')
    ) {
      errors.push(err('MALFORMED_EDGE', id));
      continue;
    }
    if (seenEdges.has(id)) {
      errors.push(err('MALFORMED_EDGE', id));
      continue;
    }
    seenEdges.add(id);
    edgeIndex.set(id, { from_node: edge.from_node, to_node: edge.to_node, variants: new Set(edge.variants) });
  }
  for (const edge of edgeIndex.values()) {
    for (const endpoint of [edge.from_node, edge.to_node]) {
      if (!nodeSet.has(endpoint)) errors.push(err('UNKNOWN_NODE', endpoint));
    }
  }
  if (!nodeSet.has(origin)) errors.push(err('UNKNOWN_NODE', origin));
  return { errors: sortErrors(errors), index: { version: graphDoc.graph_version, origin, nodeSet, edgeIndex } };
}

function createResolver(contentSource) {
  const graphs = isPlainObject(contentSource) && Array.isArray(contentSource.graphs) ? contentSource.graphs : [];
  const cache = new Map();
  let currentVersion = null;
  const versions = new Set();
  for (const g of graphs) {
    if (isPlainObject(g) && Number.isInteger(g.graph_version)) versions.add(g.graph_version);
  }
  if (versions.size === 1) currentVersion = [...versions][0];
  return {
    resolve(version) {
      if (!versions.has(version)) return null;
      if (!cache.has(version)) {
        const doc = graphs.find((g) => isPlainObject(g) && g.graph_version === version);
        cache.set(version, buildGraphIndex(doc));
      }
      return cache.get(version);
    },
    getCurrentVersion() {
      return currentVersion;
    },
  };
}

function subtreeStrings(value, out) {
  if (typeof value === 'string') {
    out.push(value);
  } else if (Array.isArray(value)) {
    for (const item of value) subtreeStrings(item, out);
  } else if (isPlainObject(value)) {
    for (const key of Object.keys(value).sort()) subtreeStrings(value[key], out);
  }
  return out;
}

function normalizedKey(key) {
  return key.toLowerCase().replace(/[_-]/g, '');
}

function looksLikePointsLedger(key) {
  const n = normalizedKey(key);
  if (LEDGER_KEYS.map(normalizedKey).includes(n)) return false;
  if (n === 'earned' || n === 'spent' || n === 'unspent' || n === 'designatedearningsource' || n === 'designatedearnedsources') {
    return false;
  }
  return n.includes('point');
}

function scanCounterConfusion(root, rootPath) {
  const findings = [];
  const walk = (value, path, ledgerContext) => {
    if (Array.isArray(value)) {
      for (const item of value) walk(item, `${path}[#]`, ledgerContext);
      return;
    }
    if (!isPlainObject(value)) return;
    const direct = [];
    for (const key of Object.keys(value).sort()) {
      if (typeof value[key] === 'string') direct.push(value[key]);
    }
    if (direct.some((s) => COMMISSION_MARKER.test(s)) && direct.some((s) => TREE_MARKER.test(s))) {
      findings.push(err('COUNTER_CONFUSION', path));
    }
    if (typeof value.designated_earned_source === 'string') {
      if (COMMISSION_MARKER.test(value.designated_earned_source)) {
        findings.push(err('COUNTER_CONFUSION', joinPath(path, 'designated_earned_source')));
      } else if (!TREE_MARKER.test(value.designated_earned_source)) {
        findings.push(err('COUNTER_CONFUSION', joinPath(path, 'designated_earned_source')));
      }
    }
    if ('earned' in value) {
      for (const sourceKey of ['source', 'derived_from', 'input']) {
        if (typeof value[sourceKey] === 'string' && COMMISSION_MARKER.test(value[sourceKey])) {
          findings.push(err('COUNTER_CONFUSION', joinPath(path, 'earned')));
        }
      }
    }
    const hasCommissionLedger = 'persistent_commission_points' in value;
    const hasTreeLedger = 'live_tree_points' in value;
    for (const key of Object.keys(value).sort()) {
      const child = value[key];
      if (looksLikePointsLedger(key) && !(hasCommissionLedger && hasTreeLedger)) {
        const numeric = isNonNegativeInt(child) || (isPlainObject(child) && isNonNegativeInt(child.value));
        if (numeric) findings.push(err('COUNTER_CONFUSION', joinPath(path, key)));
      }
      if (key === 'persistent_commission_points' && isPlainObject(child)) {
        const strs = subtreeStrings(child, []).join('\n');
        if (TREE_MARKER.test(strs) && !COMMISSION_MARKER.test(strs)) {
          findings.push(err('COUNTER_CONFUSION', joinPath(path, key)));
        }
      }
      if (key === 'live_tree_points' && isPlainObject(child)) {
        const strs = subtreeStrings(child, []).join('\n');
        if (COMMISSION_MARKER.test(strs) && !TREE_MARKER.test(strs)) {
          findings.push(err('COUNTER_CONFUSION', joinPath(path, key)));
        }
      }
      walk(child, joinPath(path, key), ledgerContext);
    }
  };
  walk(root, rootPath, null);
  return findings;
}

function joinPath(base, key) {
  return base ? `${base}.${key}` : key;
}

function checkEnvelopeShape(payload) {
  const errors = [];
  if (!isPlainObject(payload)) {
    errors.push(err('MALFORMED_ALLOCATION', null));
    return { errors, ok: false };
  }
  if (!Number.isInteger(payload.graph_version) || payload.graph_version < 1) {
    errors.push(err('MALFORMED_ALLOCATION', null));
  }
  if (!Array.isArray(payload.allocated_nodes) || !payload.allocated_nodes.every((n) => typeof n === 'string')) {
    errors.push(err('MALFORMED_ALLOCATION', null));
  }
  if (!Array.isArray(payload.edge_choices)) {
    errors.push(err('MALFORMED_ALLOCATION', null));
  } else {
    for (const choice of payload.edge_choices) {
      if (!isPlainObject(choice)) {
        errors.push(err('MALFORMED_ALLOCATION', null));
      } else if (typeof choice.edge_id !== 'string' || typeof choice.variant !== 'string') {
        errors.push(err('MALFORMED_EDGE', typeof choice.edge_id === 'string' ? choice.edge_id : null));
      }
    }
  }
  return { errors, ok: errors.length === 0 };
}

function checkBudgetContext(context) {
  const errors = [];
  if (!isPlainObject(context)) {
    errors.push(err('MALFORMED_ALLOCATION', null));
    return { errors, ok: false };
  }
  for (const field of ['persistent_commission_points', 'live_tree_points', 'earned']) {
    if (!isNonNegativeInt(context[field])) errors.push(err('MALFORMED_ALLOCATION', null));
  }
  if (
    typeof context.designated_earned_source !== 'string' ||
    !/live[_-]?tree|tree[_-]?budget/i.test(context.designated_earned_source)
  ) {
    errors.push(err('COUNTER_CONFUSION', 'context.designated_earned_source'));
  }
  return { errors, ok: errors.every((e) => e.code !== 'MALFORMED_ALLOCATION') };
}

function validateAllocationClaim(payload, context, resolver) {
  const shape = checkEnvelopeShape(payload);
  if (!shape.ok) return { errors: sortErrors(shape.errors), snapshot: null };
  const budget = checkBudgetContext(context);
  const shapeOnlyBudgetErrors = budget.errors.filter((e) => e.code !== 'COUNTER_CONFUSION');
  if (shapeOnlyBudgetErrors.length > 0) {
    return { errors: sortErrors([...shape.errors, ...shapeOnlyBudgetErrors]), snapshot: null };
  }

  const resolved = resolver.resolve(payload.graph_version);
  if (resolved === null || resolved.index === null) {
    return { errors: [err('UNKNOWN_GRAPH_VERSION', null)], snapshot: null };
  }
  const graph = resolved.index;
  const errors = [];

  const allocated = [...new Set(payload.allocated_nodes)];
  for (const nodeId of allocated.sort(cmpStr)) {
    if (!graph.nodeSet.has(nodeId)) errors.push(err('UNKNOWN_NODE', nodeId));
  }
  if (errors.length > 0) return { errors: sortErrors(errors), snapshot: null };

  const counts = new Map();
  for (const nodeId of payload.allocated_nodes) counts.set(nodeId, (counts.get(nodeId) ?? 0) + 1);
  for (const [nodeId, count] of counts) {
    if (count > 1) errors.push(err('DUPLICATE_NODE', nodeId));
  }
  const edgeChoiceCounts = new Map();
  for (const choice of payload.edge_choices) {
    edgeChoiceCounts.set(choice.edge_id, (edgeChoiceCounts.get(choice.edge_id) ?? 0) + 1);
  }
  for (const [edgeId, count] of edgeChoiceCounts) {
    if (count > 1) errors.push(err('DUPLICATE_NODE', edgeId));
  }
  if (errors.length > 0) return { errors: sortErrors(errors), snapshot: null };

  const sortedChoices = [...payload.edge_choices].sort((a, b) => cmpStr(a.edge_id, b.edge_id));
  for (const choice of sortedChoices) {
    const edge = graph.edgeIndex.get(choice.edge_id);
    if (edge === undefined) {
      errors.push(err('MALFORMED_EDGE', choice.edge_id));
    } else if (!edge.variants.has(choice.variant)) {
      errors.push(err('MALFORMED_EDGE', choice.edge_id));
    }
  }
  if (errors.length > 0) return { errors: sortErrors(errors), snapshot: null };

  const allocatedSet = new Set(allocated);
  if (!allocatedSet.has(graph.origin)) {
    errors.push(err('DISCONNECTED_ALLOCATION', graph.origin));
  }
  for (const choice of sortedChoices) {
    const edge = graph.edgeIndex.get(choice.edge_id);
    if (edge && (!allocatedSet.has(edge.from_node) || !allocatedSet.has(edge.to_node))) {
      errors.push(err('DISCONNECTED_ALLOCATION', choice.edge_id));
    }
  }
  if (allocatedSet.has(graph.origin)) {
    const adjacency = new Map();
    for (const choice of sortedChoices) {
      const edge = graph.edgeIndex.get(choice.edge_id);
      if (!edge) continue;
      if (!adjacency.has(edge.from_node)) adjacency.set(edge.from_node, []);
      if (!adjacency.has(edge.to_node)) adjacency.set(edge.to_node, []);
      adjacency.get(edge.from_node).push(edge.to_node);
      adjacency.get(edge.to_node).push(edge.from_node);
    }
    const visited = new Set([graph.origin]);
    const queue = [graph.origin];
    while (queue.length > 0) {
      const current = queue.shift();
      for (const neighbor of adjacency.get(current) ?? []) {
        if (!visited.has(neighbor) && allocatedSet.has(neighbor)) {
          visited.add(neighbor);
          queue.push(neighbor);
        }
      }
    }
    for (const nodeId of allocated.sort(cmpStr)) {
      if (!visited.has(nodeId)) errors.push(err('DISCONNECTED_ALLOCATION', nodeId));
    }
  }
  if (errors.length > 0) return { errors: sortErrors(errors), snapshot: null };

  const earned = context.earned;
  const spent = allocated.length - 1 + payload.edge_choices.length;
  if (spent > earned) {
    errors.push(err('OVERSPENT', null));
    return { errors: sortErrors(errors), snapshot: null };
  }

  if (budget.errors.length > 0) {
    return { errors: sortErrors(budget.errors), snapshot: null };
  }

  const snapshot = {
    graph_version: payload.graph_version,
    origin: graph.origin,
    selected_node:
      typeof payload.selected_node === 'string' && allocatedSet.has(payload.selected_node)
        ? payload.selected_node
        : graph.origin,
    allocated_nodes: allocated.sort(cmpStr),
    edge_choices: sortedChoices
      .map((c) => ({ edge_id: c.edge_id, variant: c.variant }))
      .sort((a, b) => cmpStr(a.edge_id, b.edge_id)),
    budget: {
      persistent_commission_points: context.persistent_commission_points,
      live_tree_points: context.live_tree_points,
      designated_earned_source: context.designated_earned_source,
      earned,
      spent,
      unspent: earned - spent,
    },
  };
  return { errors: [], snapshot };
}

function validatePersistenceClaim(payload, context, resolver) {
  if (!isPlainObject(payload)) {
    return { errors: [err('MALFORMED_ALLOCATION', null)], snapshot: null };
  }
  if (isPlainObject(payload.raw_snapshot)) {
    return validateAllocationClaim(payload.raw_snapshot, context, resolver);
  }
  const blob = payload.blob;
  const errors = [];
  if (!isPlainObject(blob)) {
    errors.push(err('MALFORMED_ALLOCATION', null));
    return { errors: sortErrors(errors), snapshot: null };
  }
  if (!Number.isInteger(blob.graph_version) || blob.graph_version < 1 || !isPlainObject(blob.allocation)) {
    errors.push(err('MALFORMED_ALLOCATION', null));
  }
  const provenance = blob.validation_provenance;
  if (
    !isPlainObject(provenance) ||
    typeof provenance.authority !== 'string' ||
    !isPlainObject(provenance.result) ||
    typeof provenance.result.ok !== 'boolean' ||
    !Array.isArray(provenance.result.errors)
  ) {
    errors.push(err('MALFORMED_ALLOCATION', null));
  }
  if (errors.some((e) => e.code === 'MALFORMED_ALLOCATION')) {
    return { errors: sortErrors(errors), snapshot: null };
  }

  const currentVersion = resolver.getCurrentVersion();
  if (currentVersion === null || blob.graph_version !== currentVersion) {
    return { errors: [err('UNSUPPORTED_MIGRATION', null)], snapshot: null };
  }

  const inner = validateAllocationClaim(blob.allocation, context, resolver);
  if (inner.errors.length > 0) return { errors: inner.errors, snapshot: null };

  return { errors: [], snapshot: inner.snapshot };
}

function validateMigrationClaim(payload, context, resolver) {
  const errors = [];
  if (!isPlainObject(payload) || !isPlainObject(payload.migration_request) || !isPlainObject(payload.blob)) {
    return { errors: [err('MALFORMED_ALLOCATION', null)], snapshot: null };
  }
  const request = payload.migration_request;
  if (
    !Number.isInteger(request.from_version) ||
    request.from_version < 1 ||
    !Number.isInteger(request.to_version) ||
    request.to_version < 1
  ) {
    errors.push(err('MALFORMED_ALLOCATION', null));
    return { errors, snapshot: null };
  }
  const currentVersion = resolver.getCurrentVersion();
  if (currentVersion === null || request.from_version !== currentVersion || request.to_version !== currentVersion) {
    return { errors: [err('UNSUPPORTED_MIGRATION', null)], snapshot: null };
  }
  return validatePersistenceClaim({ blob: payload.blob }, context, resolver);
}

function validateCase(caseEntry, resolver) {
  const caseId = isPlainObject(caseEntry) && typeof caseEntry.case_id === 'string' ? caseEntry.case_id : null;
  const fail = (errors) => ({ case_id: caseId, ok: false, errors: sortErrors(errors), snapshot: null });
  if (!isPlainObject(caseEntry)) return fail([err('MALFORMED_ALLOCATION', null)]);
  const { kind, payload, context } = caseEntry;
  if (typeof kind !== 'string' || !isPlainObject(payload)) return fail([err('MALFORMED_ALLOCATION', null)]);
  let result;
  if (kind === 'allocation') {
    result = validateAllocationClaim(payload, context, resolver);
  } else if (kind === 'persistence') {
    result = validatePersistenceClaim(payload, context, resolver);
  } else if (kind === 'migration') {
    result = validateMigrationClaim(payload, context, resolver);
  } else {
    result = { errors: [err('MALFORMED_ALLOCATION', null)], snapshot: null };
  }
  const counterFindings = scanCounterConfusion({ payload, context }, null);
  const seen = new Set();
  const merged = [];
  for (const e of sortErrors([...result.errors, ...counterFindings])) {
    const key = `${e.code}|${e.element}`;
    if (!seen.has(key)) {
      seen.add(key);
      merged.push(e);
    }
  }
  const withCase = merged.map((e) => ({ ...e, case_id: caseId }));
  return {
    case_id: caseId,
    ok: merged.length === 0,
    errors: withCase,
    snapshot: merged.length === 0 ? result.snapshot : null,
  };
}

export function validateFixture(contractDoc, fixtureDoc) {
  const contractCheck = validateContractDocument(contractDoc);
  const output = {
    validator: 'validate-passive-tree-contract.mjs',
    contract_schema_version: isPlainObject(contractDoc) ? (contractDoc.schema_version ?? null) : null,
    ok: false,
    errors: [],
    cases: [],
    contract_errors: contractCheck.contract_errors,
  };
  if (!contractCheck.ok) {
    output.ok = false;
    return output;
  }
  if (!isPlainObject(fixtureDoc) || !isPlainObject(fixtureDoc.content_source) || !Array.isArray(fixtureDoc.cases)) {
    output.errors = [{ ...err('MALFORMED_ALLOCATION', null), case_id: null }];
    return output;
  }
  const resolver = createResolver(fixtureDoc.content_source);
  const results = fixtureDoc.cases.map((caseEntry) => validateCase(caseEntry, resolver));
  const allErrors = [];
  for (const result of results) {
    output.cases.push({
      case_id: result.case_id,
      ok: result.ok,
      errors: result.errors,
      accepted_snapshot: result.ok ? result.snapshot : undefined,
    });
    allErrors.push(...result.errors);
  }
  output.cases = output.cases.map((c) =>
    c.accepted_snapshot === undefined ? { case_id: c.case_id, ok: c.ok, errors: c.errors } : c,
  );
  output.errors = sortErrors(allErrors);
  output.ok = allErrors.length === 0;
  return output;
}

export function runValidation(contractDoc, fixtureDoc) {
  const contractCheck = validateContractDocument(contractDoc);
  if (!contractCheck.ok) return { status: 'invalid_contract', report: validateFixture(contractDoc, fixtureDoc) };
  const report = validateFixture(contractDoc, fixtureDoc);
  return { status: report.ok ? 'valid' : 'invalid_payload', report };
}

function parseArgs(argv) {
  const args = { contract: null, fixture: null, json: false };
  for (let i = 0; i < argv.length; i += 1) {
    const arg = argv[i];
    if (arg === '--json') args.json = true;
    else if (arg === '--contract') args.contract = argv[(i += 1)] ?? null;
    else if (arg === '--fixture') args.fixture = argv[(i += 1)] ?? null;
    else return null;
  }
  if (typeof args.contract !== 'string' || typeof args.fixture !== 'string') return null;
  return args;
}

export function main(argv) {
  const args = parseArgs(argv);
  if (args === null) {
    process.stderr.write(
      'usage: node validate-passive-tree-contract.mjs --contract <passive-tree-contract.json> --fixture <fixture.json> [--json]\n',
    );
    return 2;
  }
  let contractDoc;
  let fixtureDoc;
  try {
    contractDoc = JSON.parse(fs.readFileSync(args.contract, 'utf8'));
  } catch (error) {
    process.stderr.write(`cannot read contract: ${error.message}\n`);
    return 2;
  }
  try {
    fixtureDoc = JSON.parse(fs.readFileSync(args.fixture, 'utf8'));
  } catch (error) {
    process.stderr.write(`cannot read fixture: ${error.message}\n`);
    return 2;
  }
  const { status, report } = runValidation(contractDoc, fixtureDoc);
  if (args.json) {
    process.stdout.write(`${JSON.stringify(report, null, 2)}\n`);
  } else {
    for (const problem of report.contract_errors) process.stdout.write(`CONTRACT: ${problem}\n`);
    for (const result of report.cases) {
      const label = result.case_id ?? '(unnamed)';
      if (result.ok) process.stdout.write(`CASE ${label}: OK\n`);
      else
        for (const e of result.errors) {
          process.stdout.write(`CASE ${label}: ${e.code} @ ${e.element === null ? '(envelope)' : e.element}\n`);
        }
    }
    process.stdout.write(`RESULT: ${status} errors=${report.errors.length}\n`);
  }
  if (status === 'invalid_contract') return 2;
  return status === 'valid' ? 0 : 1;
}

const isMain =
  process.argv[1] !== undefined &&
  import.meta.url === pathToFileURL(path.resolve(process.argv[1])).href;
if (isMain) {
  process.exit(main(process.argv.slice(2)));
}
