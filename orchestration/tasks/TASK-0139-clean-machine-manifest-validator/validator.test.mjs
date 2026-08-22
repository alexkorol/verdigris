import test from "node:test";
import assert from "node:assert/strict";
import { execFileSync } from "node:child_process";
import { readFileSync } from "node:fs";
import path from "node:path";
import { fileURLToPath } from "node:url";
import { validateManifest, versionAtLeast, scanForbiddenPort } from "./validate-clean-machine-evidence.mjs";

const here = path.dirname(fileURLToPath(import.meta.url));
const validatorPath = path.join(here, "validate-clean-machine-evidence.mjs");
const contractPath = path.resolve(here, "../../TASK-0132-clean-machine-harness-contract/clean-machine-contract.json");
const validFixturePath = path.join(here, "fixtures", "valid-synthetic.json");
const forbiddenFixturePath = path.join(here, "fixtures", "forbidden-port.json");

const contract = JSON.parse(readFileSync(contractPath, "utf8"));
const validManifest = JSON.parse(readFileSync(validFixturePath, "utf8"));
const forbiddenManifest = JSON.parse(readFileSync(forbiddenFixturePath, "utf8"));

function codesOf(result) {
  return result.errors.map((e) => e.code);
}

function mutated(fn) {
  const copy = structuredClone(validManifest);
  fn(copy);
  return validateManifest(copy, contract);
}

test("valid synthetic manifest passes all checks", () => {
  const result = validateManifest(structuredClone(validManifest), contract);
  assert.equal(result.verdict, "VALID");
  assert.deepEqual(result.errors, []);
});

test("contract gates are the twelve TASK-0132 stages in order", () => {
  assert.deepEqual(contract.platform_matrix[0].gates, [
    "checkout", "toolchain", "dependencies", "cache", "build", "tests",
    "launch", "smoke", "process", "port", "cleanup", "artifacts",
  ]);
});

test("forbidden-port fixture fails with FORBIDDEN_PORT_6500", () => {
  const result = validateManifest(structuredClone(forbiddenManifest), contract);
  assert.equal(result.verdict, "INVALID");
  assert.ok(codesOf(result).includes("FORBIDDEN_PORT_6500"));
});

test("capsule containing port 6500 is rejected even with no listener on it", () => {
  const result = mutated((m) => { m.capsule = { lane: "ox-pc-g", low: 6480, high: 6760 }; });
  assert.ok(codesOf(result).includes("FORBIDDEN_PORT_6500"));
});

test("wrong label is NOT_CLEAN_MACHINE_EVIDENCE", () => {
  const result = mutated((m) => { m.label = "developer_local"; });
  assert.ok(codesOf(result).includes("NOT_CLEAN_MACHINE_EVIDENCE"));
});

test("non-disposable host fails D-1", () => {
  const result = mutated((m) => { m.host.disposable = false; });
  assert.ok(codesOf(result).includes("NON_DISPOSABLE_HOST"));
});

test("resolved head differing from pinned base fails DIRTY_BASE", () => {
  const result = mutated((m) => { m.resolved_head = "a".repeat(40); });
  assert.ok(codesOf(result).includes("DIRTY_BASE"));
});

test("dirty checkout fails DIRTY_BASE", () => {
  const result = mutated((m) => { m.checkout.git_status_clean = false; });
  assert.ok(codesOf(result).includes("DIRTY_BASE"));
});

test("missing required top-level field fails MISSING_FIELD", () => {
  const result = mutated((m) => { delete m.capsule; });
  assert.ok(codesOf(result).includes("MISSING_FIELD"));
});

test("non-lockfile install command fails NON_LOCKFILE_INSTALL", () => {
  const result = mutated((m) => { m.dependencies.install_command = "npm install"; });
  assert.ok(codesOf(result).includes("NON_LOCKFILE_INSTALL"));
});

test("dependency fingerprint drift fails DEPENDENCY_DRIFT", () => {
  const result = mutated((m) => { m.dependencies.fingerprint_after = "sha256:different"; });
  assert.ok(codesOf(result).includes("DEPENDENCY_DRIFT"));
});

test("toolchain below declared floor fails MISSING_TOOLCHAIN", () => {
  const result = mutated((m) => {
    m.toolchain.find((t) => t.tool === "node").observed_version = "20.11.0";
  });
  assert.ok(codesOf(result).includes("MISSING_TOOLCHAIN"));
});

test("absent toolchain entry for a pinned tool fails MISSING_TOOLCHAIN", () => {
  const result = mutated((m) => {
    m.toolchain = m.toolchain.filter((t) => t.tool !== "npm");
  });
  assert.ok(codesOf(result).includes("MISSING_TOOLCHAIN"));
});

test("cache hit without provenance fails CACHE_LEAK", () => {
  const result = mutated((m) => { delete m.caches[1].producer_run_id; });
  assert.ok(codesOf(result).includes("CACHE_LEAK"));
});

test("cache outcome outside cold_miss|hit fails CACHE_LEAK", () => {
  const result = mutated((m) => { m.caches[0].outcome = "warm"; });
  assert.ok(codesOf(result).includes("CACHE_LEAK"));
});

test("non-loopback bind fails NON_LOOPBACK_BIND", () => {
  const result = mutated((m) => { m.listeners[0].bind_address = "0.0.0.0"; });
  assert.ok(codesOf(result).includes("NON_LOOPBACK_BIND"));
});

test("listener outside capsule fails OUTSIDE_CAPSULE", () => {
  const result = mutated((m) => { m.listeners[0].port = 6510; });
  assert.ok(codesOf(result).includes("OUTSIDE_CAPSULE"));
});

test("leaked tracked pid fails LEAKED_PROCESS", () => {
  const result = mutated((m) => { m.processes.leaked_pids = [67401]; });
  assert.ok(codesOf(result).includes("LEAKED_PROCESS"));
});

test("reordered stages fail STAGE_ORDER_VIOLATION", () => {
  const result = mutated((m) => { [m.stages[0], m.stages[1]] = [m.stages[1], m.stages[0]]; });
  assert.ok(codesOf(result).includes("STAGE_ORDER_VIOLATION"));
});

test("stage count differing from gate count fails NONZERO_STAGE", () => {
  const result = mutated((m) => { m.stages.pop(); });
  assert.ok(codesOf(result).includes("NONZERO_STAGE"));
});

test("nonzero stage exit code fails NONZERO_STAGE", () => {
  const result = mutated((m) => { m.stages[5].exit_code = 1; });
  assert.ok(codesOf(result).includes("NONZERO_STAGE"));
});

test("stage evidence record missing a schema field fails NONZERO_STAGE", () => {
  const result = mutated((m) => { delete m.stages[2].command; });
  assert.ok(codesOf(result).includes("NONZERO_STAGE"));
});

test("stage host mismatch fails STAGE_HOST_MISMATCH", () => {
  const result = mutated((m) => { m.stages[3].host_id = "other-host"; });
  assert.ok(codesOf(result).includes("STAGE_HOST_MISMATCH"));
});

test("inverted stage timestamps fail STAGE_TIMESTAMP_INVALID", () => {
  const result = mutated((m) => { m.stages[4].finished_at_utc = "2026-08-21T00:00:00Z"; });
  assert.ok(codesOf(result).includes("STAGE_TIMESTAMP_INVALID"));
});

test("invalid contract shape fails fast with CONTRACT_INVALID", () => {
  const result = validateManifest(structuredClone(validManifest), {});
  assert.equal(result.verdict, "INVALID");
  assert.ok(codesOf(result).includes("CONTRACT_INVALID"));
});

test("non-object manifests are rejected", () => {
  assert.equal(validateManifest(null, contract).verdict, "INVALID");
  assert.equal(validateManifest([1, 2], contract).verdict, "INVALID");
  assert.equal(validateManifest("x", contract).verdict, "INVALID");
});

test("versionAtLeast compares numerically component-wise", () => {
  assert.equal(versionAtLeast("22.14.0", "22.11.0"), true);
  assert.equal(versionAtLeast("22.11.0", "22.11.0"), true);
  assert.equal(versionAtLeast("23.0.0", "22.99.99"), true);
  assert.equal(versionAtLeast("10.9.2", "10.0.0"), true);
  assert.equal(versionAtLeast("9.8", "10.0.0"), false);
  assert.equal(versionAtLeast("22.10.9", "22.11.0"), false);
  assert.equal(versionAtLeast("abc", "1.0.0"), false);
  assert.equal(versionAtLeast("1.0.0", undefined), false);
});

test("scanForbiddenPort flags numeric ports and :6500 strings only", () => {
  const flagged = [];
  scanForbiddenPort({ port: 6500, low: 6740, url: "ws://127.0.0.1:6500/ws" }, "$", flagged);
  assert.deepEqual(flagged.map((e) => e.code), ["FORBIDDEN_PORT_6500", "FORBIDDEN_PORT_6500"]);
  const clean = [];
  scanForbiddenPort({ port: 6740, high: 6759, note: "ports 6740-6759 reserved" }, "$", clean);
  assert.deepEqual(clean, []);
});

test("CLI accepts the valid synthetic fixture with exit code 0", () => {
  const stdout = execFileSync(process.execPath, [
    validatorPath,
    "--contract", contractPath,
    "--fixture", validFixturePath,
    "--json",
  ], { encoding: "utf8" });
  assert.equal(JSON.parse(stdout).verdict, "VALID");
});

test("CLI negative control: forbidden-port fixture exits nonzero citing FORBIDDEN_PORT_6500", () => {
  assert.throws(
    () => execFileSync(process.execPath, [
      validatorPath,
      "--contract", contractPath,
      "--fixture", forbiddenFixturePath,
      "--json",
    ], { encoding: "utf8" }),
    (err) => err.status === 1 && err.stdout.includes("FORBIDDEN_PORT_6500"),
  );
});

test("CLI usage error exits nonzero", () => {
  assert.throws(
    () => execFileSync(process.execPath, [validatorPath, "--json"], { encoding: "utf8" }),
    (err) => err.status === 2 && err.stdout.includes("USAGE"),
  );
});
