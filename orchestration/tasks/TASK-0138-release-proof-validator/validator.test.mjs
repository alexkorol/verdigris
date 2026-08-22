import assert from "node:assert/strict";
import { createHash } from "node:crypto";
import { readFileSync } from "node:fs";
import { fileURLToPath } from "node:url";
import { describe, it } from "node:test";
import { spawnSync } from "node:child_process";

import { main, validateManifest } from "./validate-release-proof.mjs";

const REPO_ROOT = fileURLToPath(new URL("../../../", import.meta.url));
const TASK_DIR = "orchestration/tasks/TASK-0138-release-proof-validator";
const EXPECTED_BASE = "be6d555688619819084b352660fc0336a90d0ec3";

const REAL_ARTIFACT_REL = `${TASK_DIR}/fixtures/false-green-artifact.txt`;
const REAL_ARTIFACT_SHA256 = createHash("sha256")
  .update(readFileSync(`${REPO_ROOT}/${REAL_ARTIFACT_REL}`))
  .digest("hex");

function provenManifest(overrides = {}) {
  return {
    schema_version: "1.0.0",
    task: "TASK-0138-TEST",
    source_head: {
      commit: EXPECTED_BASE,
      branch: "synthetic/test",
      immutable_task_base: EXPECTED_BASE,
    },
    commands: [
      {
        name: "test.passing",
        command: "node -e \"console.log('ok')\"",
        exit_code: 0,
        status: "pass",
        stdout: "ok",
      },
    ],
    environment: {
      verified: true,
      verification_outputs: [{ command: "node --version", output: "v22.11.0" }],
    },
    artifacts: [
      {
        path: REAL_ARTIFACT_REL,
        role: "release bundle stand-in",
        bytes: null,
        sha256: REAL_ARTIFACT_SHA256,
      },
    ],
    platform_coverage: [
      {
        platform: "windows-x64",
        build_artifact: "bundle.zip",
        installer_artifact: "installer.exe",
        runtime_verification: "runtime.log",
        status: "proven",
      },
    ],
    rollback: {
      status: "proven",
      procedure_documented: true,
      procedure_artifact: "rollback.md",
      rehearsal_evidence: "rehearsal.log",
    },
    owner_actions: [
      {
        action: "release build",
        authorized: true,
        performed: true,
        evidence: "build.log",
      },
    ],
    verdict: { release_ready: true, state: "PROVEN", evidence_gaps: [] },
    ...overrides,
  };
}

function validate(obj, overrides = {}) {
  return validateManifest(obj, {
    expectedHead: EXPECTED_BASE,
    rootDir: REPO_ROOT,
    manifestPath: "<memory>",
    ...overrides,
  });
}

function runCli(args) {
  const res = spawnSync(process.execPath, [`${TASK_DIR}/validate-release-proof.mjs`, ...args], {
    cwd: REPO_ROOT,
    encoding: "utf8",
  });
  return { status: res.status, out: res.stdout ?? "", errText: res.stderr ?? "" };
}

describe("head binding", () => {
  it("accepts a manifest bound to the expected head", () => {
    const r = validate(provenManifest());
    assert.equal(r.release_ready, true);
    assert.deepEqual(r.integrity_errors, []);
    assert.deepEqual(r.evidence_gaps, []);
  });

  it("rejects a stale head precisely", () => {
    const m = provenManifest();
    m.source_head.commit = "0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f";
    const r = validate(m);
    assert.equal(r.release_ready, false);
    const hit = r.integrity_errors.find((e) => e.code === "STALE_HEAD");
    assert.ok(hit);
    assert.equal(hit.expected_head, EXPECTED_BASE);
    assert.equal(hit.manifest_head, "0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f");
  });

  it("rejects a malformed source_head.commit", () => {
    const m = provenManifest();
    m.source_head.commit = "abc123";
    const r = validate(m);
    assert.ok(r.integrity_errors.some((e) => e.code === "HEAD_BINDING_INVALID"));
  });
});

describe("command evidence", () => {
  it("flags pass/fail contradictions with exit codes", () => {
    const m = provenManifest({
      commands: [{ name: "x", command: "x", exit_code: 3, status: "pass", stdout: "out" }],
    });
    assert.ok(validate(m).integrity_errors.some((e) => e.code === "EXIT_STATUS_CONTRADICTION"));
  });

  it("refuses prose/CI labels in place of captured output", () => {
    const m = provenManifest({
      commands: [{ name: "ci-only", command: "ci job", exit_code: 0, status: "pass", ci_label: "green" }],
    });
    assert.ok(validate(m).integrity_errors.some((e) => e.code === "PASS_WITHOUT_OUTPUT_EVIDENCE"));
  });

  it("requires an integer exit_code binding", () => {
    const m = provenManifest({
      commands: [{ name: "x", command: "x", status: "fail" }],
    });
    assert.ok(validate(m).integrity_errors.some((e) => e.code === "EXIT_CODE_INVALID"));
  });

  it("accepts an explicitly bound empty stdout as verbatim evidence", () => {
    const m = provenManifest({
      commands: [{ name: "silent", command: "git diff --check", exit_code: 0, status: "pass", stdout: "" }],
    });
    assert.ok(!validate(m).integrity_errors.some((e) => e.code === "PASS_WITHOUT_OUTPUT_EVIDENCE"));
  });

  it("accepts a recorded listing array as captured evidence", () => {
    const m = provenManifest({
      commands: [
        {
          name: "inventory",
          command: "git diff --name-only base..HEAD",
          exit_code: 0,
          status: "pass",
          stdout_at_preliminary_run: ["a.txt"],
        },
      ],
    });
    assert.ok(!validate(m).integrity_errors.some((e) => e.code === "PASS_WITHOUT_OUTPUT_EVIDENCE"));
  });
});

describe("environment evidence", () => {
  it("rejects unverified environment blocks", () => {
    const m = provenManifest({ environment: { verified: false, verification_outputs: [] } });
    const codes = validate(m).integrity_errors.map((e) => e.code);
    assert.ok(codes.includes("ENVIRONMENT_UNVERIFIED"));
  });

  it("rejects verification_outputs without verbatim pairs", () => {
    const m = provenManifest({
      environment: { verified: true, verification_outputs: ["v22.11.0"] },
    });
    assert.ok(validate(m).integrity_errors.some((e) => e.code === "ENVIRONMENT_EVIDENCE_MALFORMED"));
  });
});

describe("artifact/hash integrity", () => {
  it("detects a missing artifact by exact path", () => {
    const m = provenManifest();
    m.artifacts[0].path = `${TASK_DIR}/fixtures/does-not-exist.bin`;
    const hit = validate(m).integrity_errors.find((e) => e.code === "MISSING_ARTIFACT");
    assert.ok(hit);
    assert.ok(hit.path.endsWith("does-not-exist.bin"));
  });

  it("detects a hash mismatch with both digests", () => {
    const m = provenManifest();
    m.artifacts[0].sha256 = "e".repeat(64);
    const hit = validate(m).integrity_errors.find((e) => e.code === "HASH_MISMATCH");
    assert.ok(hit);
    assert.equal(hit.bound_sha256, "e".repeat(64));
    assert.equal(hit.actual_sha256, REAL_ARTIFACT_SHA256);
  });

  it("detects a byte-size mismatch", () => {
    const m = provenManifest();
    m.artifacts[0].bytes = 999999;
    assert.ok(validate(m).integrity_errors.some((e) => e.code === "SIZE_MISMATCH"));
  });

  it("rejects digestless artifacts lacking an explanatory note", () => {
    const m = provenManifest();
    delete m.artifacts[0].sha256;
    delete m.artifacts[0].note;
    assert.ok(validate(m).integrity_errors.some((e) => e.code === "DIGESTLESS_WITHOUT_NOTE"));
  });

  it("tolerates digestless artifacts that document why", () => {
    const m = provenManifest();
    delete m.artifacts[0].sha256;
    m.artifacts[0].note = "self-referential; digest computed externally";
    const r = validate(m);
    assert.ok(!r.integrity_errors.some((e) => e.code === "DIGESTLESS_WITHOUT_NOTE"));
  });
});

describe("platform coverage, rollback, owner actions", () => {
  it("reports unproven platforms as evidence gaps", () => {
    const m = provenManifest();
    m.platform_coverage[0].status = "unproven";
    const r = validate(m);
    assert.ok(r.evidence_gaps.some((g) => g.code === "PLATFORM_NOT_PROVEN"));
    assert.equal(r.release_ready, false);
  });

  it("reports empty platform coverage as a gap", () => {
    const r = validate(provenManifest({ platform_coverage: [] }));
    assert.ok(r.evidence_gaps.some((g) => g.code === "PLATFORM_COVERAGE_EMPTY"));
  });

  it("contradicts claimed-proven platforms with null bindings", () => {
    const m = provenManifest();
    m.platform_coverage[0].installer_artifact = null;
    assert.ok(validate(m).integrity_errors.some((e) => e.code === "PLATFORM_CONTRADICTION"));
  });

  it("reports missing rollback as a gap", () => {
    const m = provenManifest({ rollback: { status: "missing", procedure_artifact: null } });
    const r = validate(m);
    assert.ok(r.evidence_gaps.some((g) => g.code === "ROLLBACK_MISSING"));
    assert.equal(r.release_ready, false);
  });

  it("contradicts rollback proven with unrehearsed bindings", () => {
    const m = provenManifest();
    m.rollback.rehearsal_evidence = null;
    assert.ok(validate(m).integrity_errors.some((e) => e.code === "ROLLBACK_CONTRADICTION"));
  });

  it("reports unperformed owner actions as gaps", () => {
    const m = provenManifest();
    m.owner_actions.push({ action: "notarization", authorized: false, performed: false, evidence: null });
    assert.ok(validate(m).evidence_gaps.some((g) => g.code === "OWNER_ACTION_UNPROVEN"));
  });

  it("flags performance without authorization or evidence", () => {
    const m = provenManifest();
    m.owner_actions.push({ action: "code signing", authorized: false, performed: true, evidence: null });
    const codes = validate(m).integrity_errors.map((e) => e.code);
    assert.ok(codes.includes("PERFORMED_WITHOUT_AUTHORIZATION"));
    assert.ok(codes.includes("OWNER_ACTION_WITHOUT_EVIDENCE"));
  });
});

describe("schema and verdict coherence", () => {
  it("names every missing required key", () => {
    const m = provenManifest();
    delete m.rollback;
    delete m.verdict;
    const codes = validate(m).integrity_errors.filter((e) => e.code === "SCHEMA_KEY_MISSING").map((e) => e.message);
    assert.ok(codes.some((msg) => msg.includes('"rollback"')));
    assert.ok(codes.some((msg) => msg.includes('"verdict"')));
  });

  it("contradicts a green verdict over unresolved findings", () => {
    const m = provenManifest();
    m.platform_coverage[0].status = "unproven";
    const hit = validate(m).integrity_errors.find((e) => e.code === "VERDICT_CONTRADICTION");
    assert.ok(hit);
  });

  it("contradicts a false verdict over complete proof", () => {
    const m = provenManifest();
    m.verdict.release_ready = false;
    assert.ok(validate(m).integrity_errors.some((e) => e.code === "VERDICT_CONTRADICTION"));
  });

  it("treats a non-object manifest as unreadable", () => {
    const r = validate("nope");
    assert.equal(r.state, "NOT_PROVEN");
    assert.ok(r.integrity_errors.some((e) => e.code === "MANIFEST_UNREADABLE"));
  });
});

describe("CLI end-to-end", () => {
  it("exits 0 with RELEASE_READY on a fully proven manifest", () => {
    const res = runCli([
      "--manifest", `${TASK_DIR}/fixtures/ready-minimal.json`,
      "--expected-head", EXPECTED_BASE,
      "--json",
    ]);
    assert.equal(res.status, 0, res.out + res.errText);
    const report = JSON.parse(res.out);
    assert.equal(report.release_ready, true);
    assert.equal(report.state, "RELEASE_READY");
  });

  it("exits nonzero on the false-green fixture with precise integrity errors", () => {
    const res = runCli([
      "--manifest", `${TASK_DIR}/fixtures/false-green.json`,
      "--expected-head", EXPECTED_BASE,
      "--json",
    ]);
    assert.notEqual(res.status, 0);
    const report = JSON.parse(res.out);
    assert.equal(report.release_ready, false);
    const codes = report.integrity_errors.map((e) => e.code);
    for (const code of [
      "STALE_HEAD",
      "EXIT_STATUS_CONTRADICTION",
      "PASS_WITHOUT_OUTPUT_EVIDENCE",
      "SIZE_MISMATCH",
      "HASH_MISMATCH",
      "MISSING_ARTIFACT",
      "DIGESTLESS_WITHOUT_NOTE",
      "PLATFORM_CONTRADICTION",
      "ROLLBACK_CONTRADICTION",
      "PERFORMED_WITHOUT_AUTHORIZATION",
      "OWNER_ACTION_WITHOUT_EVIDENCE",
      "VERDICT_CONTRADICTION",
    ]) {
      assert.ok(codes.includes(code), `expected ${code}; got ${codes.join(",")}`);
    }
    const hashMiss = report.integrity_errors.find((e) => e.code === "HASH_MISMATCH");
    assert.match(hashMiss.message, /false-green-artifact\.txt/);
  });

  it("exits nonzero with gaps only (no integrity errors) on the accepted TASK-0131 manifest", () => {
    const res = runCli([
      "--manifest", "orchestration/tasks/TASK-0131-release-proof-manifest/release-proof-manifest.json",
      "--expected-head", "b3599c80122d09cd0685ae96830990cc5bada5cf",
      "--json",
    ]);
    assert.equal(res.status, 1);
    const report = JSON.parse(res.out);
    assert.equal(report.release_ready, false);
    assert.deepEqual(report.integrity_errors, []);
    assert.ok(report.evidence_gaps.length >= 4);
    assert.ok(report.evidence_gaps.every((g) => g.code !== undefined));
  });

  it("exits nonzero on an unreadable manifest", () => {
    const res = runCli(["--manifest", `${TASK_DIR}/fixtures/nope.json`, "--expected-head", EXPECTED_BASE]);
    assert.equal(res.status, 1);
    assert.match(res.out, /MANIFEST_UNREADABLE|NOT_PROVEN/);
  });

  it("exits 2 on usage errors", () => {
    assert.equal(main([]), 2);
    assert.equal(main(["--manifest", "x"]), 2);
    assert.equal(runCli(["--manifest", `${TASK_DIR}/fixtures/ready-minimal.json`, "--bogus"]).status, 2);
  });

  it("prints a human-readable verdict without --json", () => {
    const res = runCli([
      "--manifest", `${TASK_DIR}/fixtures/false-green.json`,
      "--expected-head", EXPECTED_BASE,
    ]);
    assert.notEqual(res.status, 0);
    assert.match(res.out, /INTEGRITY ERROR \[STALE_HEAD\]/);
    assert.match(res.out, /NOT_PROVEN/);
  });
});
