import { test } from "node:test";
import assert from "node:assert/strict";
import { spawnSync } from "node:child_process";
import { dirname, join } from "node:path";
import { fileURLToPath } from "node:url";
import { readFileSync } from "node:fs";
import { validateManifest, versionAtLeast, scanForbiddenPort } from "./validate-clean-machine-evidence.mjs";

const here = dirname(fileURLToPath(import.meta.url));
const contractPath = join(here, "..", "TASK-0132-clean-machine-harness-contract", "clean-machine-contract.json");
const validPath = join(here, "fixtures", "valid-synthetic.json");
const forbiddenPath = join(here, "fixtures", "forbidden-port.json");
const cliPath = join(here, "validate-clean-machine-evidence.mjs");

const contract = JSON.parse(readFileSync(contractPath, "utf8"));
const validManifest = JSON.parse(readFileSync(validPath, "utf8"));
const forbiddenManifest = JSON.parse(readFileSync(forbiddenPath, "utf8"));

function codes(result) {
  return new Set(result.errors.map((e) => e.code));
}

function mutated(mutate) {
  const copy = structuredClone(validManifest);
  mutate(copy);
  return validateManifest(copy, contract);
}

test("valid synthetic fixture validates with zero errors", () => {
  const result = validateManifest(validManifest, contract);
  assert.equal(result.verdict, "VALID");
  assert.deepEqual(result.errors, []);
  assert.equal(result.summary.stages_validated, contract.platform_matrix[0].gates.length);
});

test("forbidden-port fixture fails with FORBIDDEN_PORT_6500", () => {
  const result = validateManifest(forbiddenManifest, contract);
  assert.equal(result.verdict, "INVALID");
  assert.ok(codes(result).has("FORBIDDEN_PORT_6500"));
});

test("dirty base is rejected", () => {
  const result = mutated((m) => { m.resolved_head = "0".repeat(40); });
  assert.ok(codes(result).has("DIRTY_BASE"));
});

test("unclean checkout status is rejected as DIRTY_BASE", () => {
  const result = mutated((m) => { m.checkout.git_status_clean = false; });
  assert.ok(codes(result).has("DIRTY_BASE"));
});

test("non-disposable host is rejected", () => {
  const result = mutated((m) => { m.host.disposable = false; });
  assert.ok(codes(result).has("NON_DISPOSABLE_HOST"));
});

test("toolchain version below floor is rejected", () => {
  const result = mutated((m) => { m.toolchain.find((t) => t.tool === "npm").observed_version = "9.9.9"; });
  assert.ok(codes(result).has("MISSING_TOOLCHAIN"));
});

test("absent required tool is rejected", () => {
  const result = mutated((m) => { m.toolchain = m.toolchain.filter((t) => t.tool !== "node"); });
  assert.ok(codes(result).has("MISSING_TOOLCHAIN"));
});

test("dependency fingerprint mismatch is rejected", () => {
  const result = mutated((m) => { m.dependencies.fingerprint_after = "a".repeat(64); });
  assert.ok(codes(result).has("DEPENDENCY_DRIFT"));
});

test("non-lockfile install command is rejected", () => {
  const result = mutated((m) => { m.dependencies.install_command = "npm install"; });
  assert.ok(codes(result).has("NON_LOCKFILE_INSTALL"));
});

test("cache hit without provenance is a CACHE_LEAK", () => {
  const result = mutated((m) => { delete m.caches[1].producer_run_id; });
  assert.ok(codes(result).has("CACHE_LEAK"));
});

test("nonzero stage exit code is rejected", () => {
  const result = mutated((m) => { m.stages.find((s) => s.stage === "tests").exit_code = 1; });
  assert.ok(codes(result).has("NONZERO_STAGE"));
});

test("missing stage evidence record is rejected", () => {
  const result = mutated((m) => { m.stages = m.stages.filter((s) => s.stage !== "smoke"); });
  assert.ok(result.errors.some((e) => e.code === "NONZERO_STAGE" || e.code === "STAGE_ORDER_VIOLATION"));
});

test("reordered stages are rejected", () => {
  const result = mutated((m) => { [m.stages[4], m.stages[5]] = [m.stages[5], m.stages[4]]; });
  assert.ok(codes(result).has("STAGE_ORDER_VIOLATION"));
});

test("leaked process is rejected", () => {
  const result = mutated((m) => { m.processes.leaked_pids = [48120]; });
  assert.ok(codes(result).has("LEAKED_PROCESS"));
});

test("non-loopback bind is rejected", () => {
  const result = mutated((m) => { m.listeners[0].bind_address = "0.0.0.0"; });
  assert.ok(codes(result).has("NON_LOOPBACK_BIND"));
});

test("listener outside the lane capsule is rejected", () => {
  const result = mutated((m) => { m.listeners[0].port = 6510; });
  assert.ok(codes(result).has("OUTSIDE_CAPSULE"));
});

test("capsule containing port 6500 is rejected", () => {
  const result = mutated((m) => { m.capsule = { low: 6480, high: 6510 }; });
  assert.ok(codes(result).has("FORBIDDEN_PORT_6500"));
});

test("developer_local label is not clean-machine evidence", () => {
  const result = mutated((m) => { m.label = "developer_local"; });
  assert.ok(codes(result).has("NOT_CLEAN_MACHINE_EVIDENCE"));
});

test("missing required top-level field short-circuits with MISSING_FIELD", () => {
  const result = mutated((m) => { delete m.capsule; });
  assert.ok(codes(result).has("MISSING_FIELD"));
});

test("non-object manifest is rejected", () => {
  const result = validateManifest([validManifest], contract);
  assert.equal(result.verdict, "INVALID");
  assert.ok(codes(result).has("MANIFEST_INVALID"));
});

test("invalid contract without platform_matrix gates is rejected", () => {
  const result = validateManifest(validManifest, {});
  assert.equal(result.verdict, "INVALID");
  assert.ok(codes(result).has("CONTRACT_INVALID"));
});

test("versionAtLeast compares semver numerically", () => {
  assert.equal(versionAtLeast("22.11.0", "22.11.0"), true);
  assert.equal(versionAtLeast("23.0.1", "22.11.0"), true);
  assert.equal(versionAtLeast("10.9.1", "10.0.0"), true);
  assert.equal(versionAtLeast("9.9.9", "10.0.0"), false);
  assert.equal(versionAtLeast("not-a-version", "10.0.0"), false);
});

test("scanForbiddenPort flags string and numeric references", () => {
  const errors = [];
  scanForbiddenPort({ a: { url: "ws://127.0.0.1:6500/x" }, b: [{ port: 6500 }], c: "port=6500", d: "ws://127.0.0.1:6745" }, "$", errors);
  assert.equal(errors.length, 3);
  assert.ok(errors.every((e) => e.code === "FORBIDDEN_PORT_6500"));
});

test("CLI exits zero and emits VALID json for the accepted synthetic fixture", () => {
  const r = spawnSync(process.execPath, [cliPath, "--contract", contractPath, "--fixture", validPath, "--json"], { encoding: "utf8" });
  assert.equal(r.status, 0, r.stdout + r.stderr);
  assert.equal(JSON.parse(r.stdout).verdict, "VALID");
});

test("CLI exits nonzero with FORBIDDEN_PORT_6500 for the forbidden-port fixture", () => {
  const r = spawnSync(process.execPath, [cliPath, "--contract", contractPath, "--fixture", forbiddenPath, "--json"], { encoding: "utf8" });
  assert.notEqual(r.status, 0);
  const parsed = JSON.parse(r.stdout);
  assert.equal(parsed.verdict, "INVALID");
  assert.ok(parsed.errors.some((e) => e.code === "FORBIDDEN_PORT_6500" && e.severity === "P0"));
});

test("CLI exits 2 on missing arguments", () => {
  const r = spawnSync(process.execPath, [cliPath, "--json"], { encoding: "utf8" });
  assert.equal(r.status, 2);
  assert.equal(JSON.parse(r.stdout).errors[0].code, "USAGE");
});
