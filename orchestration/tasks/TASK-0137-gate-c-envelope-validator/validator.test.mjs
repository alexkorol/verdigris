// TASK-0137 validator tests. Runs the real CLI as a subprocess for the SPEC
// acceptance fixtures and exercises the 13-check contract programmatically
// against synthetic content-neutral negatives. No dependencies beyond node:*.

import test from "node:test";
import assert from "node:assert/strict";
import { spawnSync } from "node:child_process";
import { dirname, join, resolve } from "node:path";
import { fileURLToPath } from "node:url";
import { readFileSync, writeFileSync, rmSync } from "node:fs";
import { validateEnvelope, validateRaw } from "./validate-gate-c-envelope.mjs";

const here = dirname(fileURLToPath(import.meta.url));
const taskDir = here;
const schemaPath = resolve(
  taskDir,
  "../TASK-0130-gate-c-decision-envelope/gate-c-decision-envelope.json",
);
const schema = JSON.parse(readFileSync(schemaPath, "utf8"));
const cliPath = join(taskDir, "validate-gate-c-envelope.mjs");
const negatives = JSON.parse(
  readFileSync(join(taskDir, "fixtures/negatives.json"), "utf8"),
);

function runCli(...args) {
  return spawnSync(process.execPath, [cliPath, ...args], {
    encoding: "utf8",
    cwd: resolve(taskDir, "../../.."),
  });
}

function runCliJson(...args) {
  const r = runCli(...args, "--json");
  assert.equal(r.stderr, "", `unexpected stderr: ${r.stderr}`);
  return { status: r.status, stdout: r.stdout, json: JSON.parse(r.stdout) };
}

const completeField = (value) => ({ state: "AVAILABLE", value });

function readyEnvelope(overrides = {}) {
  return {
    schema_version: "1.0.0",
    route_identity: {
      road: { roadId: "s-road", roadName: "s-road", direction: "north", blurb: null },
      node: { nodeId: "s-node", name: "s-node", template: "t", layout: "l", index: 0, status: "open" },
      tier: 1,
    },
    concrete_goal: completeField("owner-authored goal string"),
    boss_or_danger: completeField({ boss_name: "wardenName string", difficulty_band: "levelHint string" }),
    expected_item_family: completeField("owner-owned family label"),
    depth: completeField({ charted_tier: 1 }),
    branch_consequence: {
      state: "DERIVABLE-WITHOUT-GAMEPLAY-RULES",
      value: null,
      scope: "immediate next stage only",
      derivation: "computed from cited statuses/unlock rule/links",
    },
    extraction_or_return: completeField("stairsUp returns"),
    evidence_provenance: {
      authority_source: "docs/authority.md:1-2",
      audit_reference: "TASK-0086 ACCEPTED at 8ddfb06e16f85c150e9a79ccc9d8bd4932664369",
      base_commit: "be6d555688619819084b352660fc0336a90d0ec3",
    },
    completeness: { ready: true, missing_fields: [] },
    ...overrides,
  };
}

test("SPEC gate: valid-but-incomplete fixture exits 0 with ready:false", () => {
  const { status, json } = runCliJson(
    "--schema",
    schemaPath,
    "--fixture",
    join(taskDir, "fixtures/valid-incomplete.json"),
  );
  assert.equal(status, 0);
  assert.equal(json.valid, true);
  assert.equal(json.ready, false);
  assert.deepEqual(json.missing_field_codes, [
    "MISSING_CONCRETE_GOAL",
    "MISSING_EXPECTED_ITEM_FAMILY",
  ]);
  assert.deepEqual(json.owner_pending_fields.sort(), [
    "concrete_goal",
    "expected_item_family",
  ]);
});

test("SPEC gate: route-name-only fixture exits nonzero with ROUTE_NAME_ONLY", () => {
  const { status, json } = runCliJson(
    "--schema",
    schemaPath,
    "--fixture",
    join(taskDir, "fixtures/route-name-only.json"),
  );
  assert.equal(status, 1);
  assert.equal(json.valid, false);
  assert.equal(json.error, "ROUTE_NAME_ONLY");
  assert.equal(json.error_index, 4);
});

test("every documented error code has a triggering negative fixture", () => {
  const expected = new Set([
    "INVALID_JSON",
    "UNSUPPORTED_VERSION",
    "MISSING_ROUTE_IDENTITY",
    "ROUTE_NAME_ONLY",
    "MISSING_PROVENANCE",
    "MISSING_CONCRETE_GOAL",
    "MISSING_BOSS_OR_DANGER",
    "MISSING_EXPECTED_ITEM_FAMILY",
    "MISSING_DEPTH",
    "MISSING_BRANCH_CONSEQUENCE",
    "MISSING_EXTRACTION_OR_RETURN",
    "CONTRADICTORY_DEPTH",
    "OWNER_PENDING_CONTENT",
  ]);
  const seen = new Set(negatives.cases.map((c) => c.expected_error));
  for (const code of expected) {
    assert.ok(seen.has(code), `no negative fixture for ${code}`);
  }
});

test("negative fixtures produce exactly their expected deterministic outcome", () => {
  for (const caseDef of negatives.cases) {
    const raw = caseDef.envelope_text ?? JSON.stringify(caseDef.envelope);
    const result = validateRaw(schema, raw);
    if (caseDef.expect_exit_zero_blocker) {
      assert.ok(
        result.missing_field_codes.includes(caseDef.expected_error),
        `${caseDef.id}: expected ${caseDef.expected_error} among blockers`,
      );
      assert.equal(result.valid, true, caseDef.id);
      assert.equal(result.ready, false, caseDef.id);
    } else {
      assert.equal(result.error, caseDef.expected_error, caseDef.id);
      assert.equal(result.valid, false, caseDef.id);
      assert.equal(result.ready, false, caseDef.id);
    }
  }
});

test("fatal negative fixtures exit nonzero through the real CLI", () => {
  const fixturePath = join(taskDir, "fixtures", "negative-tmp.json");
  try {
    for (const caseDef of negatives.cases.filter((c) => !c.expect_exit_zero_blocker)) {
      writeFileSync(
        fixturePath,
        caseDef.envelope_text ?? JSON.stringify(caseDef.envelope),
      );
      const r = runCli("--schema", schemaPath, "--fixture", fixturePath, "--json");
      assert.equal(r.status, 1, `${caseDef.id} must exit 1`);
      const parsed = JSON.parse(r.stdout);
      assert.equal(parsed.error, caseDef.expected_error, caseDef.id);
    }
  } finally {
    rmSync(fixturePath, { force: true });
  }
});

test("blocker negative fixtures exit 0 with ready:false through the real CLI", () => {
  const fixturePath = join(taskDir, "fixtures", "negative-tmp.json");
  try {
    for (const caseDef of negatives.cases.filter((c) => c.expect_exit_zero_blocker)) {
      writeFileSync(fixturePath, JSON.stringify(caseDef.envelope));
      const r = runCli("--schema", schemaPath, "--fixture", fixturePath, "--json");
      assert.equal(r.status, 0, `${caseDef.id} must exit 0`);
      const parsed = JSON.parse(r.stdout);
      assert.equal(parsed.valid, true, caseDef.id);
      assert.equal(parsed.ready, false, caseDef.id);
    }
  } finally {
    rmSync(fixturePath, { force: true });
  }
});

test("validation is deterministic across repeated runs", () => {
  const raw = readFileSync(join(taskDir, "fixtures/valid-incomplete.json"), "utf8");
  const first = validateRaw(schema, raw);
  const second = validateRaw(schema, raw);
  assert.deepEqual(first, second);
  const cliA = runCli("--schema", schemaPath, "--fixture", join(taskDir, "fixtures/valid-incomplete.json"), "--json");
  const cliB = runCli("--schema", schemaPath, "--fixture", join(taskDir, "fixtures/valid-incomplete.json"), "--json");
  assert.equal(cliA.stdout, cliB.stdout);
});

test("honest MISSING and OWNER_PENDING values are preserved verbatim, never filled", () => {
  const raw = readFileSync(join(taskDir, "fixtures/valid-incomplete.json"), "utf8");
  const result = validateRaw(schema, raw);
  assert.equal(result.field_states.concrete_goal.state, "MISSING");
  assert.equal(result.field_states.concrete_goal.owner_pending, true);
  assert.equal(result.field_states.expected_item_family.state, "MISSING");
  assert.equal(result.field_states.branch_consequence.state, "DERIVABLE-WITHOUT-GAMEPLAY-RULES");
  const serialized = JSON.stringify(result);
  assert.doesNotMatch(serialized, /"ready":true/);
});

test("fully populated envelope is decision-ready; DERIVABLE null value does not block", () => {
  const result = validateEnvelope(schema, readyEnvelope());
  assert.equal(result.valid, true);
  assert.equal(result.ready, true);
  assert.deepEqual(result.missing_field_codes, []);
  assert.deepEqual(result.owner_pending_fields, []);
});

test("AVAILABLE field with null value blocks decision readiness", () => {
  const result = validateEnvelope(
    schema,
    readyEnvelope({
      depth: { state: "AVAILABLE", value: null },
      completeness: { ready: false, missing_fields: ["depth"] },
    }),
  );
  assert.equal(result.valid, true);
  assert.equal(result.ready, false);
  assert.deepEqual(result.missing_field_codes, ["MISSING_DEPTH"]);
});

test("unknown field state blocks decision readiness", () => {
  const result = validateEnvelope(
    schema,
    readyEnvelope({
      boss_or_danger: { state: "SOMEDAY", value: "placeholder" },
      completeness: { ready: false, missing_fields: ["boss_or_danger"] },
    }),
  );
  assert.equal(result.valid, true);
  assert.equal(result.ready, false);
  assert.deepEqual(result.missing_field_codes, ["MISSING_BOSS_OR_DANGER"]);
});

test("TASK-0130 narrative provenance shape satisfies check 5", () => {
  const result = validateEnvelope(
    schema,
    readyEnvelope({
      evidence_provenance: {
        state: "AVAILABLE",
        provenance_of_this_contract: [
          "Audit: orchestration/tasks/TASK-0086-gate-c-contract-audit/ (verdict ACCEPTED at reviewed head 8ddfb06e16f85c150e9a79ccc9d8bd4932664369)",
          "Authority: docs/rebuild/NATIVE_PRODUCT_CONVERGENCE.md:68-72 (Gate C - campaign decision)",
          "Generated at base be6d555688619819084b352660fc0336a90d0ec3 on a worker branch",
        ],
      },
    }),
  );
  assert.equal(result.valid, true);
  assert.equal(result.ready, true);
});

test("unsupported versions rejected: absent, wrong, and future", () => {
  for (const version of ["0.9.0", "2.0.0"]) {
    const env = readyEnvelope();
    env.schema_version = version;
    const result = validateEnvelope(schema, env);
    assert.equal(result.error, "UNSUPPORTED_VERSION", version);
  }
  const absent = readyEnvelope();
  delete absent.schema_version;
  assert.equal(validateEnvelope(schema, absent).error, "UNSUPPORTED_VERSION");
});

test("route identity sub-shape gaps hit check 3 before check 4", () => {
  const noTier = readyEnvelope();
  delete noTier.route_identity.tier;
  noTier.concrete_goal = undefined;
  const result = validateRaw(schema, JSON.stringify(stripUndefined(noTier)));
  assert.equal(result.error, "MISSING_ROUTE_IDENTITY");
});

test("CLI usage errors exit 2 without throwing", () => {
  const missingArgs = runCli();
  assert.equal(missingArgs.status, 2);
  const badFixture = runCli("--schema", schemaPath, "--fixture", join(taskDir, "fixtures/does-not-exist.json"));
  assert.equal(badFixture.status, 2);
  const badSchema = runCli("--schema", join(taskDir, "fixtures/does-not-exist.json"), "--fixture", join(taskDir, "fixtures/route-name-only.json"));
  assert.equal(badSchema.status, 2);
});

test("human-readable mode reports FAIL line for the negative control", () => {
  const r = runCli(
    "--schema",
    schemaPath,
    "--fixture",
    join(taskDir, "fixtures/route-name-only.json"),
  );
  assert.equal(r.status, 1);
  assert.match(r.stdout, /FAIL 4 ROUTE_NAME_ONLY/);
});

function stripUndefined(obj) {
  return JSON.parse(JSON.stringify(obj));
}
