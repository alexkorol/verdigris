import test from "node:test";
import assert from "node:assert/strict";
import fs from "node:fs";
import os from "node:os";
import path from "node:path";
import crypto from "node:crypto";
import { spawnSync } from "node:child_process";
import { fileURLToPath } from "node:url";

const taskDir = path.dirname(fileURLToPath(import.meta.url));
const cliPath = path.join(taskDir, "validate-soak-evidence.mjs");
const fixturesDir = path.join(taskDir, "fixtures");
const policyPath = path.join(
  taskDir,
  "..",
  "TASK-0135-server-lifecycle-soak-integration-contract",
  "soak-integration-policy.json",
);
const repoRoot = path.resolve(taskDir, "..", "..", "..");

const validPassBundle = JSON.parse(fs.readFileSync(path.join(fixturesDir, "valid-pass.json"), "utf8"));
const cleanArtifactBytes = fs.readFileSync(path.join(fixturesDir, "artifacts", "lifecycle-soak-clean-pass-a.json"));

function runCli(args) {
  return spawnSync(process.execPath, [cliPath, ...args], { cwd: repoRoot, encoding: "utf8" });
}

function runValidator(bundleObject, options = {}) {
  const dir = fs.mkdtempSync(path.join(os.tmpdir(), "soak-evidence-test-"));
  const bundlePath = path.join(dir, "bundle.json");
  fs.writeFileSync(bundlePath, `${JSON.stringify(bundleObject, null, 2)}\n`);
  return runCli(["--policy", options.policyPath ?? policyPath, "--bundle", bundlePath]);
}

function writeArtifact(dir, name, artifactObject) {
  const artifactPath = path.join(dir, name);
  const bytes = Buffer.from(JSON.stringify(artifactObject));
  fs.writeFileSync(artifactPath, bytes);
  return {
    path: artifactPath,
    sha256: crypto.createHash("sha256").update(bytes).digest("hex"),
  };
}

function makeAttempt(artifact, overrides = {}) {
  return {
    artifact_path: artifact.path,
    sha256: artifact.sha256,
    started_at: "2026-08-21T10:00:00Z",
    finished_at: "2026-08-21T10:02:41Z",
    outcome: "PASS",
    classification: "NOT_APPLICABLE",
    ...overrides,
  };
}

function basePlatform() {
  return { ...validPassBundle.platform };
}

function codes(result) {
  const document = JSON.parse(result.stdout);
  return [
    ...document.errors.map((entry) => entry.code),
    ...document.findings.map((entry) => entry.code),
  ];
}

test("valid-pass fixture exits 0 with accepted PASS", () => {
  const result = runCli([
    "--policy",
    policyPath,
    "--bundle",
    path.join(fixturesDir, "valid-pass.json"),
  ]);
  assert.equal(result.status, 0, result.stderr);
  const document = JSON.parse(result.stdout);
  assert.equal(document.evaluated_conclusion, "PASS");
  assert.equal(document.accepted, true);
  assert.deepEqual(document.errors, []);
});

test("retry-masked negative control exits 1 emitting RETRY_MASKED_FAILURE", () => {
  const result = runCli([
    "--policy",
    policyPath,
    "--bundle",
    path.join(fixturesDir, "retry-masked-failure.json"),
  ]);
  assert.equal(result.status, 1);
  const document = JSON.parse(result.stdout);
  assert.equal(document.evaluated_conclusion, "FAIL");
  assert.equal(document.accepted, false);
  assert.ok(document.errors.some((entry) => entry.code === "RETRY_MASKED_FAILURE"));
  assert.match(result.stderr, /RETRY_MASKED_FAILURE/);
});

test("valid-blocked-environmental fixture exits 1 consistently BLOCKED_ENVIRONMENTAL", () => {
  const result = runCli([
    "--policy",
    policyPath,
    "--bundle",
    path.join(fixturesDir, "valid-blocked-environmental.json"),
  ]);
  assert.equal(result.status, 1);
  const document = JSON.parse(result.stdout);
  assert.equal(document.submitted_conclusion, "BLOCKED_ENVIRONMENTAL");
  assert.equal(document.evaluated_conclusion, "BLOCKED_ENVIRONMENTAL");
  assert.equal(document.accepted, true);
  assert.ok(document.findings.some((entry) => entry.code === "PORT_CAPSULE_COLLISION"));
});

test("STALE_SOURCE_HEAD rejects a green bound to another head", () => {
  const bundle = structuredClone(validPassBundle);
  bundle.tip_under_judgment = "2222222222222222222222222222222222222222";
  const result = runValidator(bundle);
  assert.equal(result.status, 1);
  assert.ok(codes(result).includes("STALE_SOURCE_HEAD"));
});

test("--tip-under-judgment mismatch triggers STALE_SOURCE_HEAD", () => {
  const result = runCli([
    "--policy",
    policyPath,
    "--bundle",
    path.join(fixturesDir, "valid-pass.json"),
    "--tip-under-judgment",
    "3333333333333333333333333333333333333333",
  ]);
  assert.equal(result.status, 1);
  assert.ok(codes(result).includes("STALE_SOURCE_HEAD"));
});

test("MISSING_PLATFORM_EVIDENCE rejects incomplete platform identity", () => {
  const bundle = structuredClone(validPassBundle);
  delete bundle.platform.toolchain;
  const result = runValidator(bundle);
  assert.equal(result.status, 1);
  assert.ok(codes(result).includes("MISSING_PLATFORM_EVIDENCE"));
});

test("UNSUPPORTED_PLATFORM never reports an extrapolated green", () => {
  const bundle = structuredClone(validPassBundle);
  bundle.platform.os_name = "linux";
  const result = runValidator(bundle);
  assert.equal(result.status, 1);
  assert.ok(codes(result).includes("UNSUPPORTED_PLATFORM"));
});

test("PORT_CAPSULE_COLLISION reclassifies a submitted all-collided PASS as BLOCKED_ENVIRONMENTAL", () => {
  const bundle = structuredClone(validPassBundle);
  bundle.attempts = [
    { outcome: "FAIL", classification: "ENVIRONMENTAL", phase: "server-start" },
    { outcome: "FAIL", classification: "ENVIRONMENTAL", phase: "server-start" },
    { outcome: "QUARANTINED", classification: "ENVIRONMENTAL", phase: "server-start" },
  ];
  const result = runValidator(bundle);
  assert.equal(result.status, 1);
  const document = JSON.parse(result.stdout);
  assert.equal(document.evaluated_conclusion, "BLOCKED_ENVIRONMENTAL");
  assert.ok(document.errors.some((entry) => entry.code === "PORT_CAPSULE_COLLISION"));
});

test("FORBIDDEN_PORT_6500 fires on attempt disclosure", () => {
  const bundle = structuredClone(validPassBundle);
  bundle.attempts[0].listener_port = 6500;
  const result = runValidator(bundle);
  assert.equal(result.status, 1);
  assert.ok(codes(result).includes("FORBIDDEN_PORT_6500"));
});

test("FORBIDDEN_PORT_6500 fires inside a cited artifact capsule", () => {
  const dir = fs.mkdtempSync(path.join(os.tmpdir(), "soak-evidence-test-"));
  const artifact = JSON.parse(cleanArtifactBytes.toString("utf8"));
  artifact.portCapsule.ports = [6500];
  const written = writeArtifact(dir, "forbidden-port-artifact.json", artifact);
  const bundle = structuredClone(validPassBundle);
  bundle.attempts = [makeAttempt(written)];
  const result = runValidator(bundle);
  assert.equal(result.status, 1);
  assert.ok(codes(result).includes("FORBIDDEN_PORT_6500"));
});

test("PORT_CAPSULE_OUT_OF_RANGE rejects listeners outside the lane range", () => {
  const bundle = structuredClone(validPassBundle);
  bundle.attempts[0].listener_port = 7000;
  const result = runValidator(bundle);
  assert.equal(result.status, 1);
  assert.ok(codes(result).includes("PORT_CAPSULE_OUT_OF_RANGE"));
});

test("NON_LOOPBACK_BIND rejects a non-loopback host disclosure", () => {
  const bundle = structuredClone(validPassBundle);
  bundle.attempts[0].host = "0.0.0.0";
  const result = runValidator(bundle);
  assert.equal(result.status, 1);
  assert.ok(codes(result).includes("NON_LOOPBACK_BIND"));
});

test("TIMEOUT fires on watchdog exit code 3", () => {
  const bundle = structuredClone(validPassBundle);
  bundle.attempts.push({
    outcome: "FAIL",
    classification: "DETERMINISTIC",
    process_exit_code: 3,
    cyclesRequested: 100,
    cyclesCompleted: 61,
  });
  const result = runValidator(bundle);
  assert.equal(result.status, 1);
  assert.ok(codes(result).includes("TIMEOUT"));
});

test("TIMEOUT fires when an attempt exceeds its context wall-clock budget", () => {
  const bundle = structuredClone(validPassBundle);
  bundle.attempts[0].started_at = "2026-08-21T10:00:00Z";
  bundle.attempts[0].finished_at = "2026-08-21T10:25:00Z";
  const result = runValidator(bundle);
  assert.equal(result.status, 1);
  assert.ok(codes(result).includes("TIMEOUT"));
});

test("NONZERO_SOAK blocks same-head deterministic failure masked by a later pass", () => {
  const artifact = writeArtifact(
    os.tmpdir(),
    `soak-evidence-clean-${crypto.randomBytes(4).toString("hex")}.json`,
    JSON.parse(cleanArtifactBytes.toString("utf8")),
  );
  try {
    const bundle = structuredClone(validPassBundle);
    bundle.attempts = [
      { outcome: "FAIL", classification: "DETERMINISTIC", process_exit_code: 1 },
      makeAttempt(artifact),
    ];
    const result = runValidator(bundle);
    assert.equal(result.status, 1);
    assert.ok(codes(result).includes("NONZERO_SOAK"));
  } finally {
    fs.rmSync(artifact.path, { force: true });
  }
});

test("NONZERO_SOAK fires on a bare exit code 1 attempt", () => {
  const bundle = structuredClone(validPassBundle);
  bundle.attempts.push({
    outcome: "FAIL",
    classification: "ENVIRONMENTAL",
    phase: "client-login",
    process_exit_code: 1,
  });
  const result = runValidator(bundle);
  assert.equal(result.status, 1);
  assert.ok(codes(result).includes("NONZERO_SOAK"));
});

test("MISSING_ARTIFACT fires when the cited artifact file does not exist", () => {
  const bundle = structuredClone(validPassBundle);
  bundle.attempts[0].artifact_path = path.join(os.tmpdir(), "lifecycle-soak-does-not-exist.json");
  const result = runValidator(bundle);
  assert.equal(result.status, 1);
  assert.ok(codes(result).includes("MISSING_ARTIFACT"));
});

test("MISSING_ARTIFACT fires when the artifact lacks required fields", () => {
  const dir = fs.mkdtempSync(path.join(os.tmpdir(), "soak-evidence-test-"));
  const artifact = JSON.parse(cleanArtifactBytes.toString("utf8"));
  delete artifact.burstPassed;
  const written = writeArtifact(dir, "field-incomplete.json", artifact);
  const bundle = structuredClone(validPassBundle);
  bundle.attempts = [makeAttempt(written)];
  const result = runValidator(bundle);
  assert.equal(result.status, 1);
  assert.ok(codes(result).includes("MISSING_ARTIFACT"));
});

test("MISSING_ARTIFACT fires when the artifact is unparsable", () => {
  const dir = fs.mkdtempSync(path.join(os.tmpdir(), "soak-evidence-test-"));
  const artifactPath = path.join(dir, "unparsable.json");
  fs.writeFileSync(artifactPath, "{not json");
  const bundle = structuredClone(validPassBundle);
  bundle.attempts = [
    makeAttempt({ path: artifactPath, sha256: crypto.createHash("sha256").update("{not json").digest("hex") }),
  ];
  const result = runValidator(bundle);
  assert.equal(result.status, 1);
  assert.ok(codes(result).includes("MISSING_ARTIFACT"));
});

test("ARTIFACT_HASH_MISMATCH detects tampered or misbound artifacts", () => {
  const dir = fs.mkdtempSync(path.join(os.tmpdir(), "soak-evidence-test-"));
  const artifact = JSON.parse(cleanArtifactBytes.toString("utf8"));
  artifact.loginsSucceeded = 801;
  const written = writeArtifact(dir, "tampered.json", artifact);
  const bundle = structuredClone(validPassBundle);
  bundle.attempts = [makeAttempt(written)];
  bundle.attempts[0].sha256 = crypto
    .createHash("sha256")
    .update(cleanArtifactBytes)
    .digest("hex");
  const result = runValidator(bundle);
  assert.equal(result.status, 1);
  assert.ok(codes(result).includes("ARTIFACT_HASH_MISMATCH"));
});

test("INCOMPLETE_RUN rejects partial cycle counts and failed bursts", () => {
  const dir = fs.mkdtempSync(path.join(os.tmpdir(), "soak-evidence-test-"));
  const artifact = JSON.parse(cleanArtifactBytes.toString("utf8"));
  artifact.cyclesCompleted = 99;
  const written = writeArtifact(dir, "partial.json", artifact);
  const bundle = structuredClone(validPassBundle);
  bundle.attempts = [makeAttempt(written)];
  let result = runValidator(bundle);
  assert.equal(result.status, 1);
  assert.ok(codes(result).includes("INCOMPLETE_RUN"));

  const burstArtifact = JSON.parse(cleanArtifactBytes.toString("utf8"));
  burstArtifact.burstPassed = false;
  const burstWritten = writeArtifact(dir, "burst-failed.json", burstArtifact);
  bundle.attempts = [makeAttempt(burstWritten)];
  result = runValidator(bundle);
  assert.equal(result.status, 1);
  assert.ok(codes(result).includes("INCOMPLETE_RUN"));
});

test("INSUFFICIENT_INDEPENDENT_RUNS rejects a green with no verified runs", () => {
  const bundle = structuredClone(validPassBundle);
  bundle.context = "pre_merge";
  bundle.attempts = [];
  const result = runValidator(bundle);
  assert.equal(result.status, 1);
  assert.ok(codes(result).includes("INSUFFICIENT_INDEPENDENT_RUNS"));
});

test("disabled nightly context refuses judgment with exit 2", () => {
  const bundle = structuredClone(validPassBundle);
  bundle.context = "nightly";
  const result = runValidator(bundle);
  assert.equal(result.status, 2);
  const document = JSON.parse(result.stdout);
  assert.equal(document.result, "ERROR");
  assert.equal(document.error_class, "unsupported_context");
});

test("release_proof requires fresh processes and non-overlapping intervals once enabled", () => {
  const dir = fs.mkdtempSync(path.join(os.tmpdir(), "soak-evidence-test-"));
  const enabledPolicy = JSON.parse(fs.readFileSync(policyPath, "utf8"));
  enabledPolicy.contexts.release_proof.enabled = true;
  const enabledPolicyPath = path.join(dir, "policy-release-proof-enabled.json");
  fs.writeFileSync(enabledPolicyPath, JSON.stringify(enabledPolicy, null, 2));

  const mkArtifact = (name, startMinutes) =>
    writeArtifact(dir, name, {
      ...JSON.parse(cleanArtifactBytes.toString("utf8")),
      startedAt: `2026-08-21T1${startMinutes}:00:00Z`,
      finishedAt: `2026-08-21T1${startMinutes}:05:00Z`,
    });

  const bundle = structuredClone(validPassBundle);
  bundle.context = "release_proof";
  bundle.attempts = [mkArtifact("a.json", 0), mkArtifact("b.json", 1)].map((artifact) =>
    makeAttempt(artifact, {
      started_at: artifact.path.includes("a.json") ? "2026-08-21T10:00:00Z" : "2026-08-21T11:00:00Z",
      finished_at: artifact.path.includes("a.json") ? "2026-08-21T10:05:00Z" : "2026-08-21T11:05:00Z",
    }),
  );
  let result = runValidator(bundle, { policyPath: enabledPolicyPath });
  assert.equal(result.status, 1);
  assert.ok(codes(result).includes("FRESH_PROCESS_UNDISCLOSED"));

  bundle.attempts = bundle.attempts.map((attempt) => ({ ...attempt, fresh_process: true }));
  result = runValidator(bundle, { policyPath: enabledPolicyPath });
  assert.equal(result.status, 1);
  assert.ok(codes(result).includes("INSUFFICIENT_INDEPENDENT_RUNS"));

  bundle.attempts.push(
    makeAttempt(mkArtifact("c.json", 2), {
      fresh_process: true,
      started_at: "2026-08-21T10:01:00Z",
      finished_at: "2026-08-21T10:06:00Z",
    }),
  );
  result = runValidator(bundle, { policyPath: enabledPolicyPath });
  assert.equal(result.status, 1);
  assert.ok(codes(result).includes("RUN_INTERVAL_OVERLAP"));
});

test("release_proof accepts three distinct fresh-process runs", () => {
  const dir = fs.mkdtempSync(path.join(os.tmpdir(), "soak-evidence-test-"));
  const enabledPolicy = JSON.parse(fs.readFileSync(policyPath, "utf8"));
  enabledPolicy.contexts.release_proof.enabled = true;
  const enabledPolicyPath = path.join(dir, "policy-release-proof-enabled.json");
  fs.writeFileSync(enabledPolicyPath, JSON.stringify(enabledPolicy, null, 2));

  const starts = ["2026-08-21T10:00:00Z", "2026-08-21T11:00:00Z", "2026-08-21T12:00:00Z"];
  const bundle = structuredClone(validPassBundle);
  bundle.context = "release_proof";
  bundle.attempts = starts.map((start, index) => {
    const endHour = 10 + index;
    const artifact = writeArtifact(dir, `run-${index}.json`, {
      ...JSON.parse(cleanArtifactBytes.toString("utf8")),
      startedAt: start,
      finishedAt: `2026-08-21T${endHour}:03:00Z`,
    });
    return makeAttempt(artifact, {
      fresh_process: true,
      started_at: start,
      finished_at: `2026-08-21T${endHour}:03:00Z`,
    });
  });
  const result = runValidator(bundle, { policyPath: enabledPolicyPath });
  assert.equal(result.status, 0, result.stderr);
});

test("honest FAIL submission with disclosed deterministic failure is consistent", () => {
  const bundle = structuredClone(validPassBundle);
  bundle.attempts = [{ outcome: "FAIL", classification: "DETERMINISTIC", process_exit_code: 1 }];
  bundle.conclusion = "FAIL";
  const result = runValidator(bundle);
  assert.equal(result.status, 1);
  const document = JSON.parse(result.stdout);
  assert.equal(document.evaluated_conclusion, "FAIL");
  assert.equal(document.accepted, true);
  assert.deepEqual(document.errors, []);
  assert.ok(document.findings.some((entry) => entry.code === "NONZERO_SOAK"));
});

test("CONCLUSION_UNSUPPORTED rejects unsubstantiated FAIL submissions", () => {
  const bundle = structuredClone(validPassBundle);
  bundle.conclusion = "FAIL";
  const result = runValidator(bundle);
  assert.equal(result.status, 1);
  assert.ok(codes(result).includes("CONCLUSION_UNSUPPORTED"));
});

test("usage errors exit 2", () => {
  assert.equal(runCli([]).status, 2);
  assert.equal(runCli(["--policy", policyPath]).status, 2);
  assert.equal(runCli(["--unknown"]).status, 2);
  assert.equal(runCli(["--policy", policyPath, "--bundle", policyPath, "--bundle", policyPath]).status, 2);
  assert.equal(runCli(["--tip-under-judgment", "nothex"]).status, 2);
});

test("--help exits 0 printing usage", () => {
  const result = runCli(["--help"]);
  assert.equal(result.status, 0);
  assert.match(result.stdout, /USAGE/);
});

test("parse errors exit 2 for unreadable or malformed documents", () => {
  const missing = path.join(os.tmpdir(), "definitely-absent-policy.json");
  assert.equal(runCli(["--policy", missing, "--bundle", missing]).status, 2);

  const dir = fs.mkdtempSync(path.join(os.tmpdir(), "soak-evidence-test-"));
  const brokenPolicy = path.join(dir, "broken-policy.json");
  fs.writeFileSync(brokenPolicy, "{oops");
  assert.equal(runCli(["--policy", brokenPolicy, "--bundle", path.join(fixturesDir, "valid-pass.json")]).status, 2);

  const brokenBundle = path.join(dir, "broken-bundle.json");
  fs.writeFileSync(brokenBundle, "[1,");
  assert.equal(runCli(["--policy", policyPath, "--bundle", brokenBundle]).status, 2);
});

test("schema violations exit 2", () => {
  const cases = [];
  const mutate = (fn) => {
    const bundle = structuredClone(validPassBundle);
    fn(bundle);
    cases.push(bundle);
  };
  mutate((bundle) => {
    bundle.source_head = "nothex";
  });
  mutate((bundle) => {
    bundle.conclusion = "MAYBE";
  });
  mutate((bundle) => {
    bundle.context = "weekly";
  });
  mutate((bundle) => {
    bundle.attempts = "all-of-them";
  });
  mutate((bundle) => {
    bundle.attempts[0].outcome = "SORT_OF_PASS";
  });
  mutate((bundle) => {
    bundle.attempts[0].classification = "MYSTERY";
  });
  mutate((bundle) => {
    delete bundle.attempts;
  });
  for (const [index, bundle] of cases.entries()) {
    const result = runValidator(bundle);
    assert.equal(result.status, 2, `case ${index}`);
    const document = JSON.parse(result.stdout);
    assert.equal(document.error_class, "schema", `case ${index}`);
  }
});

test("unsupported or unsafe policies are refused with exit 2", () => {
  const dir = fs.mkdtempSync(path.join(os.tmpdir(), "soak-evidence-test-"));
  const buildPolicy = (mutate) => {
    const policy = JSON.parse(fs.readFileSync(policyPath, "utf8"));
    mutate(policy);
    const policyPathTmp = path.join(dir, `policy-${crypto.randomBytes(4).toString("hex")}.json`);
    fs.writeFileSync(policyPathTmp, JSON.stringify(policy, null, 2));
    return policyPathTmp;
  };
  const foreignId = buildPolicy((policy) => {
    policy.policy_id = "someone-elses.policy";
  });
  assert.equal(runCli(["--policy", foreignId, "--bundle", path.join(fixturesDir, "valid-pass.json")]).status, 2);

  const futureVersion = buildPolicy((policy) => {
    policy.schema_version = "2.0.0";
  });
  assert.equal(runCli(["--policy", futureVersion, "--bundle", path.join(fixturesDir, "valid-pass.json")]).status, 2);

  const unsafeCapsule = buildPolicy((policy) => {
    policy.port_capsule.forbidden_ports = [];
  });
  assert.equal(runCli(["--policy", unsafeCapsule, "--bundle", path.join(fixturesDir, "valid-pass.json")]).status, 2);

  const incomplete = buildPolicy((policy) => {
    delete policy.quarantine;
  });
  assert.equal(runCli(["--policy", incomplete, "--bundle", path.join(fixturesDir, "valid-pass.json")]).status, 2);
});

test("evaluation is deterministic across repeated runs", () => {
  const args = ["--policy", policyPath, "--bundle", path.join(fixturesDir, "valid-pass.json")];
  const first = runCli(args);
  const second = runCli(args);
  assert.equal(first.status, second.status);
  assert.equal(first.stdout, second.stdout);
});
