import test from "node:test";
import assert from "node:assert/strict";
import { spawnSync } from "node:child_process";
import { readFileSync } from "node:fs";
import { fileURLToPath } from "node:url";
import { dirname, join } from "node:path";
import { checkContract, validate } from "./validate-passive-tree-contract.mjs";

const here = dirname(fileURLToPath(import.meta.url));
const taskDir = here;
const contractPath = join(taskDir, "..", "TASK-0112-passive-tree-engine-scaffold", "passive-tree-contract.json");
const negativeCasesPath = join(
  taskDir,
  "..",
  "TASK-0112-passive-tree-engine-scaffold",
  "fixtures",
  "negative-cases.json"
);
const validFixturePath = join(taskDir, "fixtures", "valid-synthetic.json");
const counterConfusionFixturePath = join(taskDir, "fixtures", "counter-confusion.json");
const cliPath = join(taskDir, "validate-passive-tree-contract.mjs");

const contract = JSON.parse(readFileSync(contractPath, "utf8"));
const negativeCases = JSON.parse(readFileSync(negativeCasesPath, "utf8"));
const validFixture = JSON.parse(readFileSync(validFixturePath, "utf8"));

function contextFor(caseEntry) {
  const synthetic = negativeCases.conventions.synthetic_graph;
  return {
    approved_graph_versions: [synthetic.graph_version],
    graph: {
      graph_version: synthetic.graph_version,
      origin: synthetic.origin,
      nodes: synthetic.nodes.map((node_id) => ({ node_id })),
      edges: synthetic.edges,
    },
    budget: {
      persistent_commission_points: 0,
      live_tree_points: 99,
      earned: caseEntry.context?.earned ?? 99,
      designated_earned_source: "live_tree_points",
      cost_rule: { per_non_origin_node: 1, per_edge_choice: 1 },
    },
    registered_migrations: {},
  };
}

function codesOf(result) {
  return result.errors.map((error) => error.code);
}

test("accepted TASK-0112 contract passes structural sanity", () => {
  const check = checkContract(contract);
  assert.equal(check.ok, true, check.problems.join("; "));
});

test("malformed contracts fail closed", () => {
  assert.equal(checkContract(null).ok, false);
  assert.equal(checkContract({}).ok, false);
  const truncated = { ...contract };
  delete truncated.persistence;
  assert.equal(checkContract(truncated).ok, false);
  const weakened = JSON.parse(JSON.stringify(contract));
  weakened.validation_result.properties.errors.items.properties.code.enum = ["MALFORMED_ALLOCATION"];
  assert.equal(checkContract(weakened).ok, false);
});

test("every TASK-0112 negative case surfaces its expected error code", () => {
  for (const caseEntry of negativeCases.cases) {
    const fixture = { context: contextFor(caseEntry), candidate: null };
    if (caseEntry.case_id === "NEG-007") {
      fixture.candidate = { kind: "persistence", payload: caseEntry.payload };
    } else if (caseEntry.case_id === "NEG-008") {
      fixture.candidate = { kind: "persistence", payload: caseEntry.payload };
    } else if (caseEntry.case_id === "NEG-010") {
      fixture.candidate = { kind: "allocation", payload: caseEntry.payload.raw_snapshot };
    } else {
      fixture.candidate = { kind: "allocation", payload: caseEntry.payload };
    }
    const result = validate(fixture);
    assert.equal(
      result.ok,
      false,
      `${caseEntry.case_id} must not validate as ok`
    );
    assert.ok(
      codesOf(result).includes(caseEntry.expected_error),
      `${caseEntry.case_id}: expected ${caseEntry.expected_error} in ${codesOf(result).join(",")}`
    );
  }
});

test("valid synthetic fixture validates clean and emits an accepted snapshot", () => {
  const result = validate(validFixture);
  assert.equal(result.ok, true, JSON.stringify(result.errors));
  assert.equal(result.errors.length, 0);
  assert.equal(result.accepted_snapshot.graph_version, 2);
  assert.equal(result.accepted_snapshot.spent, 2);
  assert.equal(result.accepted_snapshot.unspent, 0);
  assert.deepEqual(result.accepted_snapshot.allocated_nodes, ["n:000", "n:001"]);
  assert.deepEqual(result.accepted_snapshot.ledgers.designated_earned_source, "live_tree_points");
});

test("counter-confusion fixture fails closed with COUNTER_CONFUSION", () => {
  const fixture = JSON.parse(readFileSync(counterConfusionFixturePath, "utf8"));
  const result = validate(fixture);
  assert.equal(result.ok, false);
  assert.ok(codesOf(result).includes("COUNTER_CONFUSION"), JSON.stringify(result.errors));
  assert.equal(result.accepted_snapshot, undefined);
});

test("errors are ordered by validation rank then element, independent of input order", () => {
  const fixture = structuredClone(validFixture);
  fixture.candidate.payload = {
    graph_version: 2,
    allocated_nodes: ["n:002", "n:000", "n:404"],
    edge_choices: [
      { edge_id: "e:001", variant: "v:9" },
      { edge_id: "e:999", variant: "v:0" },
    ],
  };
  fixture.context.budget.earned = 0;
  const shuffled = structuredClone(fixture);
  shuffled.candidate.payload.allocated_nodes.reverse();
  shuffled.candidate.payload.edge_choices.reverse();
  const resultA = validate(fixture);
  const a = codesOf(resultA);
  const b = codesOf(validate(shuffled));
  assert.deepEqual(a, [
    "UNKNOWN_NODE",
    "MALFORMED_EDGE",
    "DISCONNECTED_ALLOCATION",
    "OVERSPENT",
  ]);
  assert.deepEqual(
    resultA.errors.map((error) => error.element),
    ["n:404", "e:001", "e:001", "budget"]
  );
  assert.deepEqual(a, b, "input order must not change emitted order");
});

test("unknown graph versions fail closed before any structural stage", () => {
  const fixture = structuredClone(validFixture);
  fixture.candidate.payload.graph_version = 9999;
  const result = validate(fixture);
  assert.deepEqual(codesOf(result), ["UNKNOWN_GRAPH_VERSION"]);
});

test("tie-breaks report the first offending element in lexicographic order", () => {
  const fixture = structuredClone(validFixture);
  fixture.candidate.payload = {
    graph_version: 2,
    allocated_nodes: ["n:000", "n:900", "n:100"],
    edge_choices: [],
  };
  const result = validate(fixture);
  const unknownNode = result.errors.find((error) => error.code === "UNKNOWN_NODE");
  assert.equal(unknownNode.element, "n:100");
});

test("duplicate nodes and duplicate edge choices both raise DUPLICATE_NODE", () => {
  const dupNode = structuredClone(validFixture);
  dupNode.candidate.payload.allocated_nodes.push("n:000");
  assert.ok(codesOf(validate(dupNode)).includes("DUPLICATE_NODE"));
  const dupEdge = structuredClone(validFixture);
  dupEdge.candidate.payload.edge_choices.push({ edge_id: "e:001", variant: "v:0" });
  assert.ok(codesOf(validate(dupEdge)).includes("DUPLICATE_NODE"));
});

test("missing origin allocation is DISCONNECTED_ALLOCATION", () => {
  const fixture = structuredClone(validFixture);
  fixture.candidate.payload.allocated_nodes = ["n:001"];
  const result = validate(fixture);
  assert.ok(codesOf(result).includes("DISCONNECTED_ALLOCATION"));
});

test("overspend is reported, never clamped silently", () => {
  const fixture = structuredClone(validFixture);
  fixture.candidate.payload.allocated_nodes = ["n:000", "n:001", "n:002", "n:003"];
  fixture.candidate.payload.edge_choices.push(
    { edge_id: "e:002", variant: "v:0" },
    { edge_id: "e:003", variant: "v:0" }
  );
  const result = validate(fixture);
  const overspent = result.errors.find((error) => error.code === "OVERSPENT");
  assert.ok(overspent, JSON.stringify(result.errors));
  assert.equal(overspent.element, "budget");
});

test("unregistered migration requests fail closed with UNSUPPORTED_MIGRATION", () => {
  const fixture = structuredClone(validFixture);
  fixture.candidate = {
    kind: "persistence",
    payload: {
      blob: {
        graph_version: 1,
        allocation: {
          graph_version: 1,
          allocated_nodes: ["legacy:n:000"],
          edge_choices: [],
        },
      },
      migration_request: { from_version: 1, to_version: 2 },
    },
  };
  const result = validate(fixture);
  assert.ok(codesOf(result).includes("UNSUPPORTED_MIGRATION"), JSON.stringify(result.errors));
  assert.equal(result.accepted_snapshot, undefined);
});

test("raw client snapshots are validated before any persistence trust", () => {
  const fixture = structuredClone(validFixture);
  fixture.context.budget.earned = 1;
  fixture.candidate = {
    kind: "persistence",
    payload: {
      raw_snapshot: {
        graph_version: 2,
        allocated_nodes: ["n:000", "n:001", "n:002", "n:003"],
        edge_choices: [{ edge_id: "e:001", variant: "v:0" }],
      },
    },
  };
  const result = validate(fixture);
  assert.equal(result.ok, false);
  assert.ok(codesOf(result).includes("MALFORMED_ALLOCATION"), JSON.stringify(result.errors));
});

test("output is byte-deterministic across repeated runs", () => {
  const a = JSON.stringify(validate(validFixture));
  const b = JSON.stringify(validate(structuredClone(validFixture)));
  assert.equal(a, b);
});

test("CLI exits 0 on the valid SPEC fixture", () => {
  const run = spawnSync(process.execPath, [
    cliPath,
    "--contract",
    contractPath,
    "--fixture",
    validFixturePath,
    "--json",
  ]);
  assert.equal(run.status, 0, run.stdout.toString());
  const body = JSON.parse(run.stdout.toString());
  assert.equal(body.ok, true);
  assert.deepEqual(body.errors, []);
});

test("CLI exits nonzero on the counter-confusion SPEC fixture and emits COUNTER_CONFUSION", () => {
  const run = spawnSync(process.execPath, [
    cliPath,
    "--contract",
    contractPath,
    "--fixture",
    counterConfusionFixturePath,
    "--json",
  ]);
  assert.notEqual(run.status, 0);
  const body = JSON.parse(run.stdout.toString());
  assert.equal(body.ok, false);
  assert.ok(
    body.errors.some((error) => error.code === "COUNTER_CONFUSION"),
    run.stdout.toString()
  );
});

test("CLI exits 2 on usage and unreadable-input errors", () => {
  const noArgs = spawnSync(process.execPath, [cliPath]);
  assert.equal(noArgs.status, 2);
  const missing = spawnSync(process.execPath, [
    cliPath,
    "--contract",
    contractPath,
    "--fixture",
    join(taskDir, "fixtures", "does-not-exist.json"),
    "--json",
  ]);
  assert.equal(missing.status, 2);
});
