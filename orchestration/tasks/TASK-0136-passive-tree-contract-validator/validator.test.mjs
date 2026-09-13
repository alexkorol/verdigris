// TASK-0136 validator tests. Runs the real CLI as a subprocess for the SPEC
// acceptance fixtures and exercises the TASK-0112 validation pipeline
// programmatically against synthetic content-neutral negatives. No
// dependencies beyond node:*.

import test from "node:test";
import assert from "node:assert/strict";
import { spawnSync } from "node:child_process";
import { dirname, join, resolve } from "node:path";
import { fileURLToPath } from "node:url";
import { readFileSync, writeFileSync, rmSync } from "node:fs";
import {
  REQUIRED_ERROR_CODES,
  RANKS,
  evaluateFixture,
  parseArgs,
  runValidation,
  validateContractShape,
} from "./validate-passive-tree-contract.mjs";

const here = dirname(fileURLToPath(import.meta.url));
const repoRoot = resolve(here, "../../..");
const contractPath = resolve(
  here,
  "../TASK-0112-passive-tree-engine-scaffold/passive-tree-contract.json",
);
const contract = JSON.parse(readFileSync(contractPath, "utf8"));
const cliPath = join(here, "validate-passive-tree-contract.mjs");
const validFixturePath = join(here, "fixtures/valid-synthetic.json");
const ccFixturePath = join(here, "fixtures/counter-confusion.json");
const validFixture = JSON.parse(readFileSync(validFixturePath, "utf8"));
const ccFixture = JSON.parse(readFileSync(ccFixturePath, "utf8"));

function runCli(...args) {
  return spawnSync(process.execPath, [cliPath, ...args], {
    encoding: "utf8",
    cwd: repoRoot,
  });
}

function runCliJson(...args) {
  const r = runCli(...args, "--json");
  assert.equal(r.stderr, "", `unexpected stderr: ${r.stderr}`);
  return { status: r.status, stdout: r.stdout, json: JSON.parse(r.stdout) };
}

const baseAuthority = () => ({
  role: "synthetic stand-in scoped to this test run",
  graph_version: 2,
  migration_floor: 1,
  registered_migrations: [{ from_version: 1, to_version: 2, strategy: "revalidate_in_place" }],
});

const baseGraph = () => ({
  envelope: "verdigris.passive-tree-authority/graph@1",
  graph_version: 2,
  origin: "n:000",
  nodes: [{ node_id: "n:000" }, { node_id: "n:001" }, { node_id: "n:002" }, { node_id: "n:003" }],
  edges: [
    { edge_id: "e:001", from_node: "n:000", to_node: "n:001", variants: ["v:0"] },
    { edge_id: "e:002", from_node: "n:001", to_node: "n:002", variants: ["v:0", "v:1"] },
    { edge_id: "e:003", from_node: "n:002", to_node: "n:003", variants: ["v:0"] },
  ],
});

const baseBudget = () => ({
  persistent_commission_points: 7,
  live_tree_points: 5,
  earned: 5,
  spent: 4,
  unspent: 1,
});

const baseAllocation = () => ({
  graph_version: 2,
  allocated_nodes: ["n:000", "n:001", "n:002"],
  edge_choices: [
    { edge_id: "e:001", variant: "v:0" },
    { edge_id: "e:002", variant: "v:1" },
  ],
});

function makeFixture(overrides = {}, cases = []) {
  return {
    fixture_set: "task-0136-test-harness/synthetic",
    schema_version: "1.0.0",
    purpose: "content-neutral structural negatives; no topology or balance claims",
    authority: overrides.authority ?? baseAuthority(),
    graph: overrides.graph ?? baseGraph(),
    cases,
  };
}

function blob(graphVersion, allocationOverrides = {}) {
  return {
    graph_version: graphVersion,
    allocation: { graph_version: graphVersion, allocated_nodes: ["n:000"], edge_choices: [], ...allocationOverrides },
    validation_provenance: {
      authority: "task-0136-synthetic-authority",
      result: { ok: true, errors: [] },
    },
  };
}

function codesOf(result) {
  return result.errors.map((e) => e.code);
}

// --- SPEC acceptance gates -------------------------------------------------

test("SPEC gate: valid-synthetic fixture exits 0 fully valid", () => {
  const { status, json } = runCliJson("--contract", contractPath, "--fixture", validFixturePath);
  assert.equal(status, 0);
  assert.equal(json.contract_ok, true);
  assert.equal(json.ok, true);
  assert.deepEqual(json.errors, []);
  assert.equal(json.results.length, 4);
  for (const r of json.results) {
    assert.equal(r.ok, true, r.case_id);
    assert.ok(r.accepted_snapshot, `${r.case_id} must produce an accepted snapshot`);
  }
});

test("SPEC gate: counter-confusion fixture exits nonzero emitting COUNTER_CONFUSION", () => {
  const { status, json } = runCliJson("--contract", contractPath, "--fixture", ccFixturePath);
  assert.equal(status, 1);
  assert.equal(json.ok, false);
  assert.equal(json.results.length, 3);
  for (const r of json.results) {
    assert.deepEqual(codesOf(r), ["COUNTER_CONFUSION"], r.case_id);
    assert.equal(r.accepted_snapshot, undefined, `${r.case_id} must not be accepted`);
  }
  assert.deepEqual(json.results[0].errors[0].element, "points");
  assert.deepEqual(json.results[1].errors[0].element, "earned_source");
  assert.deepEqual(json.results[2].errors[0].element, "merged_with");
});

// --- Contract binding ------------------------------------------------------

test("accepted TASK-0112 contract passes the contract shape check", () => {
  const check = validateContractShape(contract);
  assert.equal(check.ok, true);
  assert.deepEqual(check.errors, []);
});

test("tampered contracts fail closed as INVALID_CONTRACT", () => {
  const tampered = (mutate) => {
    const clone = JSON.parse(JSON.stringify(contract));
    mutate(clone);
    return validateContractShape(clone);
  };
  const collapsed = tampered((c) => {
    c.budget.persistent_commission_points = c.budget.live_tree_points;
  });
  assert.equal(collapsed.ok, false);
  assert.ok(collapsed.errors.some((e) => e.element === "budget.counters_collapsed"));

  const sectionless = tampered((c) => delete c.persistence);
  assert.equal(sectionless.ok, false);

  const wrongViolation = tampered((c) => {
    c.counter_separation.violation_error = "SOMETHING_ELSE";
  });
  assert.equal(wrongViolation.ok, false);

  const strippedEnum = tampered((c) => {
    const en = c.validation_result.properties.errors.items.properties.code.enum;
    en.splice(en.indexOf("OVERSPENT"), 1);
  });
  assert.equal(strippedEnum.ok, false);

  const noControls = tampered((c) => {
    c.negative_controls = [];
  });
  assert.equal(noControls.ok, false);

  for (const check of [collapsed, sectionless, wrongViolation, strippedEnum, noControls]) {
    assert.ok(check.errors.every((e) => e.code === "INVALID_CONTRACT"));
  }
});

// --- Every required code fails closed --------------------------------------

test("all eight required error codes plus MALFORMED_ALLOCATION are reachable", () => {
  const seen = new Set();
  const expectSingle = (fixture, code, element) => {
    const result = evaluateFixture(contract, fixture);
    assert.deepEqual(codesOf(result), [code], `${code}: got ${JSON.stringify(result.errors)}`);
    assert.equal(result.ok, false);
    if (element !== undefined) assert.equal(result.errors[0].element, element, code);
    seen.add(code);
    return result;
  };

  // UNKNOWN_GRAPH_VERSION (rank 2, stop).
  expectSingle(makeFixture({}, [{ case_id: "T", mode: "allocation", allocation: { ...baseAllocation(), graph_version: 9999 }, budget: baseBudget() }]), "UNKNOWN_GRAPH_VERSION");

  // UNKNOWN_NODE (rank 4).
  expectSingle(
    makeFixture({}, [{ case_id: "T", mode: "allocation", allocation: { ...baseAllocation(), allocated_nodes: ["n:000", "n:404"], edge_choices: [] }, budget: { ...baseBudget(), earned: 5, spent: 0, unspent: 5 } }]),
    "UNKNOWN_NODE",
    "n:404",
  );

  // DUPLICATE_NODE (rank 5).
  expectSingle(
    makeFixture({}, [{ case_id: "T", mode: "allocation", allocation: { ...baseAllocation(), allocated_nodes: ["n:000", "n:001", "n:001"], edge_choices: [{ edge_id: "e:001", variant: "v:0" }] }, budget: baseBudget() }]),
    "DUPLICATE_NODE",
    "n:001",
  );

  // DISCONNECTED_ALLOCATION (rank 7): island allocation.
  expectSingle(
    makeFixture({}, [{ case_id: "T", mode: "allocation", allocation: { graph_version: 2, allocated_nodes: ["n:000", "n:003"], edge_choices: [] }, budget: { ...baseBudget(), earned: 5, spent: 1, unspent: 4 } }]),
    "DISCONNECTED_ALLOCATION",
    "n:003",
  );

  // OVERSPENT (rank 8), never clamped.
  expectSingle(
    makeFixture({}, [
      {
        case_id: "T",
        mode: "allocation",
        allocation: baseAllocation(),
        budget: { persistent_commission_points: 0, live_tree_points: 2, earned: 2, spent: 5, unspent: 0 },
      },
    ]),
    "OVERSPENT",
    null,
  );

  // MALFORMED_EDGE (rank 6): unknown edge and unknown variant; lex-first wins.
  // Both choices fail rank 6, so no edge is traversable and rank 7 also fires
  // (batch mode accumulates ranks 4-9 independently).
  expectSingle(
    makeFixture({}, [
      {
        case_id: "T",
        mode: "allocation",
        allocation: { graph_version: 2, allocated_nodes: ["n:000"], edge_choices: [{ edge_id: "e:999", variant: "v:0" }, { edge_id: "e:001", variant: "v:7" }] },
        budget: { ...baseBudget(), earned: 5, spent: 0, unspent: 5 },
      },
    ]),
    "MALFORMED_EDGE",
    "e:001",
  );
  const bothEdgeFaults = evaluateFixture(contract, makeFixture({}, [
    {
      case_id: "T",
      mode: "allocation",
      allocation: { graph_version: 2, allocated_nodes: ["n:000", "n:001"], edge_choices: [{ edge_id: "e:999", variant: "v:0" }, { edge_id: "e:001", variant: "v:7" }] },
      budget: { ...baseBudget(), earned: 5, spent: 0, unspent: 5 },
    },
  ]));
  assert.deepEqual(codesOf(bothEdgeFaults), ["MALFORMED_EDGE", "DISCONNECTED_ALLOCATION"]);
  assert.equal(bothEdgeFaults.errors[0].element, "e:001");

  // COUNTER_CONFUSION (rank 9): earned derived from the commission ledger.
  expectSingle(
    makeFixture({}, [
      { case_id: "T", mode: "allocation", allocation: baseAllocation(), budget: { ...baseBudget(), earned_source: "quests.questPoints" } },
    ]),
    "COUNTER_CONFUSION",
    "earned_source",
  );

  // UNSUPPORTED_MIGRATION (rank 3, stop): no registered path below the floor.
  expectSingle(
    makeFixture({ authority: { ...baseAuthority(), migration_floor: 2, registered_migrations: [] } }, [
      { case_id: "T", mode: "persistence", blob: blob(1), budget: baseBudget() },
    ]),
    "UNSUPPORTED_MIGRATION",
    "1->2",
  );

  // MALFORMED_ALLOCATION (rank 1, stop): required collections missing.
  expectSingle(makeFixture({}, [{ case_id: "T", mode: "allocation", allocation: { graph_version: 2 }, budget: baseBudget() }]), "MALFORMED_ALLOCATION", "allocated_nodes");

  for (const code of [...REQUIRED_ERROR_CODES, "MALFORMED_ALLOCATION"]) {
    assert.ok(seen.has(code), `no negative exercised ${code}`);
  }
  assert.equal(REQUIRED_ERROR_CODES.length, 8);
  assert.equal(Object.keys(RANKS).length, 9);
});

test("stop ranks short-circuit everything downstream", () => {
  const messy = makeFixture({}, [
    {
      case_id: "T",
      mode: "allocation",
      allocation: { graph_version: 9999, allocated_nodes: ["n:001", "n:001", "n:404"], edge_choices: [{ edge_id: "e:999", variant: "v:0" }] },
      budget: { persistent_commission_points: 0, live_tree_points: 0, earned: 0, spent: 9, unspent: 0 },
    },
  ]);
  const result = evaluateFixture(contract, messy);
  assert.deepEqual(codesOf(result), ["UNKNOWN_GRAPH_VERSION"]);
});

// --- Deterministic ordering --------------------------------------------------

test("batch errors are ordered by VALIDATION.md rank then lexicographic element", () => {
  const fixture = makeFixture({}, [
    {
      case_id: "ORDER",
      mode: "allocation",
      allocation: {
        graph_version: 2,
        allocated_nodes: ["n:000", "n:001", "n:001", "n:003", "n:404"],
        edge_choices: [{ edge_id: "e:001", variant: "v:0" }, { edge_id: "e:999", variant: "v:0" }],
      },
      budget: { persistent_commission_points: 0, live_tree_points: 1, earned: 1, spent: 5, unspent: 0 },
    },
  ]);
  const result = evaluateFixture(contract, fixture);
  assert.deepEqual(
    result.errors.map((e) => [e.code, e.element]),
    [
      ["UNKNOWN_NODE", "n:404"],
      ["DUPLICATE_NODE", "n:001"],
      ["MALFORMED_EDGE", "e:999"],
      ["DISCONNECTED_ALLOCATION", "n:003"],
      ["OVERSPENT", null],
    ],
  );
});

// --- Migration machinery ------------------------------------------------------

test("supported migration revalidates in place and retains the pre-migration audit", () => {
  const fixture = makeFixture({}, [
    {
      case_id: "MIG",
      mode: "migration_request",
      migration_request: { from_version: 1, to_version: 2 },
      blob: blob(1),
      budget: baseBudget(),
    },
  ]);
  const result = evaluateFixture(contract, fixture);
  assert.equal(result.ok, true, JSON.stringify(result.errors));
  const snap = result.results[0].accepted_snapshot;
  assert.equal(snap.graph_version, 2);
  assert.deepEqual(snap.audit, { pre_migration_graph_version: 1, strategy: "revalidate_in_place" });
});

test("full_refund_reset discards the old allocation and grants origin-only reset", () => {
  const fixture = makeFixture(
    {
      authority: {
        ...baseAuthority(),
        registered_migrations: [{ from_version: 1, to_version: 2, strategy: "full_refund_reset" }],
      },
    },
    [
      {
        case_id: "REFUND",
        mode: "persistence",
        blob: blob(1, { allocated_nodes: ["legacy:n:000"] }),
        budget: { ...baseBudget(), spent: 0, unspent: 5 },
      },
    ],
  );
  const result = evaluateFixture(contract, fixture);
  assert.equal(result.ok, true, JSON.stringify(result.errors));
  const snap = result.results[0].accepted_snapshot;
  assert.deepEqual(snap.allocated_nodes, ["n:000"]);
  assert.deepEqual(snap.edge_choices, []);
  assert.deepEqual(snap.audit.strategy, "full_refund_reset");
});

test("implicit unsupported migration on persisted load fails closed", () => {
  const fixture = makeFixture({ authority: { ...baseAuthority(), registered_migrations: [] } }, [
    { case_id: "OLD", mode: "persistence", blob: blob(1), budget: baseBudget() },
  ]);
  const result = evaluateFixture(contract, fixture);
  assert.deepEqual(codesOf(result), ["UNSUPPORTED_MIGRATION"]);
  assert.equal(result.errors[0].element, "1->2");
});

// --- Raw-snapshot trust boundary (native_raw_snapshot_save control) ---------

test("persisted blobs without validation_provenance are refused as raw snapshots", () => {
  const rawBlob = blob(2);
  delete rawBlob.validation_provenance;
  const fixture = makeFixture({}, [{ case_id: "RAW", mode: "persistence", blob: rawBlob, budget: baseBudget() }]);
  const result = evaluateFixture(contract, fixture);
  assert.deepEqual(codesOf(result), ["MALFORMED_ALLOCATION"]);
  assert.equal(result.errors[0].element, "validation_provenance");
  assert.equal(result.results[0].accepted_snapshot, undefined);
});

test("NEG-010 style raw snapshot is validated then refused, never stored verbatim", () => {
  const fixture = makeFixture({}, [
    {
      case_id: "NEG010",
      mode: "raw_snapshot",
      raw_snapshot: {
        graph_version: 2,
        allocated_nodes: ["n:000", "n:001", "n:002", "n:003"],
        edge_choices: [{ edge_id: "e:001", variant: "v:0" }],
      },
      budget: { persistent_commission_points: 0, live_tree_points: 1, earned: 1, spent: 4, unspent: 0 },
    },
  ]);
  const result = evaluateFixture(contract, fixture);
  assert.deepEqual(
    result.errors.map((e) => [e.code, e.element]),
    [
      ["DISCONNECTED_ALLOCATION", "n:002"],
      ["OVERSPENT", null],
    ],
  );
  assert.equal(result.results[0].accepted_snapshot, undefined);
});

test("internally valid raw snapshots are rebuilt through validation with provenance", () => {
  const fixture = makeFixture({}, [
    {
      case_id: "REBUILD",
      mode: "raw_snapshot",
      raw_snapshot: { graph_version: 2, allocated_nodes: ["n:000", "n:001"], edge_choices: [{ edge_id: "e:001", variant: "v:0" }] },
      budget: { persistent_commission_points: 1, live_tree_points: 2, earned: 2, spent: 2, unspent: 0 },
    },
  ]);
  const result = evaluateFixture(contract, fixture);
  assert.equal(result.ok, true, JSON.stringify(result.errors));
  const snap = result.results[0].accepted_snapshot;
  assert.equal(snap.rebuilt_from_raw_snapshot, true);
  assert.equal(snap.validation_provenance.authority, "task-0136-validator@1.0.0");
  assert.deepEqual(snap.validation_provenance.result, { ok: true, errors: [] });
});

// --- Negative control: identifier text is never interpreted -----------------

test("+2/axis-walk control: opaque node ids gain no derived attributes or effects", () => {
  const fixture = {
    fixture_set: "task-0136-test-harness/opaque-ids",
    schema_version: "1.0.0",
    authority: baseAuthority(),
    graph: {
      graph_version: 2,
      origin: "o:root",
      nodes: [{ node_id: "o:root" }, { node_id: "axis:+2hex" }],
      edges: [{ edge_id: "x:1", from_node: "o:root", to_node: "axis:+2hex", variants: ["v:0"] }],
    },
    cases: [
      {
        case_id: "OPAQUE",
        mode: "allocation",
        allocation: {
          graph_version: 2,
          allocated_nodes: ["o:root", "axis:+2hex"],
          edge_choices: [{ edge_id: "x:1", variant: "v:0" }],
        },
        budget: { persistent_commission_points: 0, live_tree_points: 2, earned: 2, spent: 2, unspent: 0 },
      },
    ],
  };
  const result = evaluateFixture(contract, fixture);
  assert.equal(result.ok, true, JSON.stringify(result.errors));
  const snap = result.results[0].accepted_snapshot;
  assert.deepEqual(snap.allocated_nodes, ["o:root", "axis:+2hex"]);
  const serialized = JSON.stringify(result);
  for (const banned of ['"attributes"', '"effects"', '"bonus"', '"score"']) {
    assert.ok(!serialized.includes(banned), `validator invented ${banned}`);
  }
  assert.deepEqual(Object.keys(snap), [
    "envelope",
    "graph_version",
    "allocated_nodes",
    "edge_choices",
    "selected_node",
    "calling_order",
    "budget",
    "validation_provenance",
  ]);
});

// --- Two-ledger preservation --------------------------------------------------

test("both point ledgers stay structurally distinct in every accepted snapshot", () => {
  const result = evaluateFixture(contract, validFixture);
  assert.equal(result.ok, true);
  for (const r of result.results) {
    const b = r.accepted_snapshot.budget;
    assert.notEqual(b.persistent_commission_points, undefined);
    assert.notEqual(b.live_tree_points, undefined);
    assert.notEqual(b.persistent_commission_points, b.live_tree_points, r.case_id);
  }
  const serialized = JSON.stringify(result);
  assert.doesNotMatch(serialized, /"(points|questPoints)":/);
});

// --- Graph document preflight -------------------------------------------------

test("broken candidate graphs fail closed before any case runs", () => {
  const dupGraph = baseGraph();
  dupGraph.nodes.push({ node_id: "n:001" });
  const dupResult = evaluateFixture(contract, makeFixture({ graph: dupGraph }, []));
  assert.equal(dupResult.ok, false);
  assert.equal(dupResult.results[0].case_id, "__graph_document__");
  assert.deepEqual(codesOf(dupResult), ["DUPLICATE_NODE"]);

  const wrongVersion = baseGraph();
  wrongVersion.graph_version = 3;
  const versionResult = evaluateFixture(contract, makeFixture({ graph: wrongVersion }, []));
  assert.deepEqual(codesOf(versionResult), ["UNKNOWN_GRAPH_VERSION"]);
});

// --- Determinism --------------------------------------------------------------

test("validation is deterministic across repeated runs and case order", () => {
  const first = evaluateFixture(contract, validFixture);
  const second = evaluateFixture(contract, validFixture);
  assert.deepEqual(first, second);

  const reordered = { ...ccFixture, cases: [...ccFixture.cases].reverse() };
  const forward = evaluateFixture(contract, ccFixture);
  const backward = evaluateFixture(contract, reordered);
  const byId = (r) =>
    Object.fromEntries(r.results.map((x) => [x.case_id, x.errors.map((e) => [e.code, e.element])]));
  assert.deepEqual(byId(backward), byId(forward));

  const cliA = runCli("--contract", contractPath, "--fixture", validFixturePath, "--json");
  const cliB = runCli("--contract", contractPath, "--fixture", validFixturePath, "--json");
  assert.equal(cliA.stdout, cliB.stdout);
});

// --- CLI surface ----------------------------------------------------------------

test("CLI round trip: programmatic ordering negative exits 1 through the real CLI", () => {
  const fixturePath = join(here, "fixtures", "negative-tmp.json");
  try {
    writeFileSync(fixturePath, JSON.stringify(makeFixture({}, [{
      case_id: "ORDER",
      mode: "allocation",
      allocation: {
        graph_version: 2,
        allocated_nodes: ["n:000", "n:001", "n:001", "n:003", "n:404"],
        edge_choices: [{ edge_id: "e:001", variant: "v:0" }, { edge_id: "e:999", variant: "v:0" }],
      },
      budget: { persistent_commission_points: 0, live_tree_points: 1, earned: 1, spent: 5, unspent: 0 },
    }])));
    const { status, json } = runCliJson("--contract", contractPath, "--fixture", fixturePath);
    assert.equal(status, 1);
    assert.deepEqual(
      json.errors.map((e) => e.code),
      ["UNKNOWN_NODE", "DUPLICATE_NODE", "MALFORMED_EDGE", "DISCONNECTED_ALLOCATION", "OVERSPENT"],
    );
  } finally {
    rmSync(fixturePath, { force: true });
  }
});

test("CLI usage and IO errors exit 2 without throwing", () => {
  assert.equal(runCli().status, 2);
  assert.equal(runCli("--contract", contractPath).status, 2);
  assert.equal(runCli("--contract", contractPath, "--fixture", validFixturePath, "--bogus").status, 2);
  assert.equal(runCli("--contract", join(here, "fixtures/does-not-exist.json"), "--fixture", validFixturePath).status, 2);
  assert.equal(runCli("--contract", contractPath, "--fixture", join(here, "fixtures/does-not-exist.json")).status, 2);
  assert.throws(() => parseArgs(["--frobnicate"]));
  assert.throws(() => parseArgs([]));
});

test("invalid JSON inputs exit 2 with a diagnostic", () => {
  const badPath = join(here, "fixtures", "bad-tmp.json");
  try {
    writeFileSync(badPath, "{not json");
    assert.equal(runCli("--contract", badPath, "--fixture", validFixturePath).status, 2);
    assert.equal(runCli("--contract", contractPath, "--fixture", badPath).status, 2);
  } finally {
    rmSync(badPath, { force: true });
  }
});

test("human-readable mode prints FAIL lines and a RESULT summary", () => {
  const r = runCli("--contract", contractPath, "--fixture", ccFixturePath);
  assert.equal(r.status, 1);
  assert.match(r.stdout, /CONTRACT OK verdigris\.passive-tree-authority@1\.0\.0/);
  assert.match(r.stdout, /CASE FAIL CC-001-merged-ledger-blob COUNTER_CONFUSION points/);
  assert.match(r.stdout, /RESULT ok=false cases=3 error_count=3/);
});
