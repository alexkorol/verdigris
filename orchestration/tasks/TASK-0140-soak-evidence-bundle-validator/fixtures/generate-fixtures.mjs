import fs from "node:fs";
import path from "node:path";
import crypto from "node:crypto";
import { fileURLToPath } from "node:url";

const taskDir = path.dirname(path.dirname(fileURLToPath(import.meta.url)));
const fixturesDir = path.join(taskDir, "fixtures");
const artifactsDir = path.join(fixturesDir, "artifacts");

fs.mkdirSync(artifactsDir, { recursive: true });

const cleanArtifact = {
  tool: "server_lifecycle_soak",
  task: "TASK-0129-server-lifecycle-soak",
  host: "127.0.0.1",
  portCapsule: { host: "127.0.0.1", lane: "ox-pc-d", ports: [6681] },
  startedAt: "2026-08-21T10:00:00Z",
  finishedAt: "2026-08-21T10:02:41Z",
  cyclesRequested: 100,
  cyclesCompleted: 100,
  upgradesSucceeded: 100,
  loginsSucceeded: 800,
  cleanCloses: 900,
  burstPassed: true,
  passed: true,
  failures: [],
};

const cleanName = "lifecycle-soak-clean-pass-a.json";
fs.writeFileSync(path.join(artifactsDir, cleanName), JSON.stringify(cleanArtifact));
const cleanSha = crypto
  .createHash("sha256")
  .update(fs.readFileSync(path.join(artifactsDir, cleanName)))
  .digest("hex");

const repoRoot = path.dirname(path.dirname(path.dirname(taskDir)));
const cleanRef = {
  artifact_path: path
    .relative(repoRoot, path.join(artifactsDir, cleanName))
    .split(path.sep)
    .join("/"),
  sha256: cleanSha,
  started_at: cleanArtifact.startedAt,
  finished_at: cleanArtifact.finishedAt,
  outcome: "PASS",
  classification: "NOT_APPLICABLE",
};

writeJson(path.join(fixturesDir, "valid-pass.json"), {
  source_head: "9aa43b7aa715b1b12efd3f33ab434acfc834de14",
  platform: {
    os_name: "windows",
    os_version: "10.0.19045",
    cpu_architecture: "x64",
    toolchain: "MSVC cl.exe driven by native/build.ps1",
  },
  context: "local",
  attempts: [cleanRef],
  conclusion: "PASS",
});

writeJson(path.join(fixturesDir, "retry-masked-failure.json"), {
  source_head: "9aa43b7aa715b1b12efd3f33ab434acfc834de14",
  platform: {
    os_name: "windows",
    os_version: "10.0.19045",
    cpu_architecture: "x64",
    toolchain: "MSVC cl.exe driven by native/build.ps1",
  },
  context: "local",
  attempts: [cleanRef],
  undisclosed_failed_attempts: 1,
  conclusion: "PASS",
});

const collisionStarts = ["2026-08-21T11:00:00Z", "2026-08-21T11:03:00Z", "2026-08-21T11:06:00Z"];
writeJson(path.join(fixturesDir, "valid-blocked-environmental.json"), {
  source_head: "9aa43b7aa715b1b12efd3f33ab434acfc834de14",
  platform: {
    os_name: "windows",
    os_version: "10.0.19045",
    cpu_architecture: "x64",
    toolchain: "MSVC cl.exe driven by native/build.ps1",
  },
  context: "local",
  attempts: collisionStarts.map((started_at, i) => ({
    outcome: "QUARANTINED",
    classification: "ENVIRONMENTAL",
    phase: "server-start",
    binder_error: "no free port in capsule 6680-6699",
    started_at,
    finished_at: started_at,
  })),
  conclusion: "BLOCKED_ENVIRONMENTAL",
});

function writeJson(filePath, value) {
  fs.writeFileSync(filePath, `${JSON.stringify(value, null, 2)}\n`);
}

console.log(`generated fixtures under ${fixturesDir}`);
console.log(`${cleanName} sha256=${cleanSha}`);
