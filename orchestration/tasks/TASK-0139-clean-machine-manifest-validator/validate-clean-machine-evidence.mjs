#!/usr/bin/env node
import { readFileSync } from "node:fs";
import { pathToFileURL } from "node:url";

const FORBIDDEN_PORT = 6500;
const LOOPBACK = "127.0.0.1";
const REQUIRED_LABEL = "clean_machine_verified";
const CACHE_OUTCOMES = new Set(["cold_miss", "hit"]);
const HEX40 = /^[0-9a-f]{40}$/;

function fail(errors, code, message, path, severity) {
  errors.push({ code, severity: severity || "P1", path, message });
}

function isMissing(v) {
  return v === undefined || v === null || (typeof v === "string" && v.trim() === "");
}

function versionAtLeast(observed, floor) {
  const o = String(observed).split(".").map((p) => Number(p));
  const f = String(floor).split(".").map((p) => Number(p));
  if (o.some((n) => !Number.isFinite(n)) || f.some((n) => !Number.isFinite(n))) return false;
  const len = Math.max(o.length, f.length);
  for (let i = 0; i < len; i += 1) {
    const a = o[i] || 0;
    const b = f[i] || 0;
    if (a > b) return true;
    if (a < b) return false;
  }
  return true;
}

function scanForbiddenPort(value, path, errors) {
  if (Array.isArray(value)) {
    value.forEach((v, i) => scanForbiddenPort(v, `${path}[${i}]`, errors));
    return;
  }
  if (value && typeof value === "object") {
    for (const [k, v] of Object.entries(value)) scanForbiddenPort(v, path ? `${path}.${k}` : k, errors);
    return;
  }
  const key = path.split(".").pop().toLowerCase();
  if (typeof value === "number") {
    if (value === FORBIDDEN_PORT && (key.includes("port") || key === "low" || key === "high")) {
      fail(errors, "FORBIDDEN_PORT_6500", `forbidden port ${FORBIDDEN_PORT} at ${path}`, path, "P0");
    }
  } else if (typeof value === "string") {
    if (/:6500(?:\D|$)/.test(value) || /\bport[ =:]*6500\b/i.test(value)) {
      fail(errors, "FORBIDDEN_PORT_6500", `forbidden port ${FORBIDDEN_PORT} referenced at ${path}: ${value}`, path, "P0");
    }
  }
}

function validateManifest(manifest, contract) {
  const errors = [];
  if (!contract || typeof contract !== "object" || !Array.isArray(contract.platform_matrix) || !contract.platform_matrix[0] || !Array.isArray(contract.platform_matrix[0].gates)) {
    fail(errors, "CONTRACT_INVALID", "contract is missing platform_matrix[0].gates", "$.contract", "P0");
    return { verdict: "INVALID", label: null, errors, summary: { checks: 0, stages: 0 } };
  }
  const gates = contract.platform_matrix[0].gates;
  const schema = contract.evidence_record_schema
    || (contract.contract && contract.contract.evidence_record_schema);
  const defaultRecordFields = ["stage", "host_id", "host_image", "base_commit", "command", "exit_code", "started_at_utc", "finished_at_utc", "log_ref"];
  const requiredRecordFields = (schema && Array.isArray(schema.required)) ? schema.required : defaultRecordFields;

  if (!manifest || typeof manifest !== "object" || Array.isArray(manifest)) {
    fail(errors, "MANIFEST_INVALID", "manifest must be a JSON object", "$", "P0");
    return { verdict: "INVALID", label: null, errors, summary: { checks: 1, stages: 0 } };
  }

  scanForbiddenPort(manifest, "$", errors);

  for (const field of ["schema_version", "run_id", "label", "host", "base_commit", "resolved_head", "checkout", "toolchain", "dependencies", "caches", "capsule", "stages", "processes", "listeners"]) {
    if (isMissing(manifest[field])) fail(errors, "MISSING_FIELD", `missing required field ${field}`, `$.${field}`, "P0");
  }
  if (errors.some((e) => e.code === "MISSING_FIELD")) {
    return { verdict: "INVALID", label: manifest.label || null, errors, summary: { checks: 2, stages: 0 } };
  }

  if (manifest.label !== REQUIRED_LABEL) {
    fail(errors, "NOT_CLEAN_MACHINE_EVIDENCE", `label must be "${REQUIRED_LABEL}", observed "${manifest.label}"`, "$.label", "P0");
  }

  if (manifest.host.disposable !== true || isMissing(manifest.host.id) || isMissing(manifest.host.image)) {
    fail(errors, "NON_DISPOSABLE_HOST", "host must be disposable with declared id and image (D-1)", "$.host", "P0");
  }

  if (!HEX40.test(manifest.base_commit)) {
    fail(errors, "DIRTY_BASE", "base_commit must be a full 40-hex SHA", "$.base_commit", "P0");
  }
  if (manifest.resolved_head !== manifest.base_commit) {
    fail(errors, "DIRTY_BASE", "resolved HEAD does not equal pinned base_commit", "$.resolved_head", "P0");
  }
  if (manifest.checkout.fresh_clone !== true || manifest.checkout.git_status_clean !== true) {
    fail(errors, "DIRTY_BASE", "workspace must be a fresh clone with empty git status (D-2)", "$.checkout", "P0");
  }

  const pins = (contract.contract && contract.contract.toolchain && contract.contract.toolchain.pins)
    || (contract.toolchain && contract.toolchain.pins)
    || {};
  const observedTools = new Map();
  if (!Array.isArray(manifest.toolchain)) {
    fail(errors, "MISSING_TOOLCHAIN", "toolchain must be an array", "$.toolchain", "P0");
  } else {
    for (const t of manifest.toolchain) {
      if (t && t.tool) observedTools.set(t.tool, t);
    }
    for (const [tool, pin] of Object.entries(pins)) {
      const entry = observedTools.get(tool);
      if (!entry || isMissing(entry.observed_version)) {
        fail(errors, "MISSING_TOOLCHAIN", `tool "${tool}" has no observed version (pin: ${JSON.stringify(pin)})`, `$.toolchain`, "P0");
        continue;
      }
      if (pin.floor && !versionAtLeast(entry.observed_version, pin.floor)) {
        fail(errors, "MISSING_TOOLCHAIN", `tool "${tool}" version ${entry.observed_version} violates floor ${pin.floor}`, "$.toolchain", "P0");
      }
    }
  }

  const deps = manifest.dependencies;
  if (typeof deps.install_command !== "string" || !deps.install_command.startsWith("npm ci")) {
    fail(errors, "NON_LOCKFILE_INSTALL", `install_command must be lockfile-driven "npm ci ...", observed "${deps.install_command}"`, "$.dependencies.install_command", "P0");
  }
  if (isMissing(deps.fingerprint_before) || isMissing(deps.fingerprint_after)) {
    fail(errors, "DEPENDENCY_DRIFT", "dependency fingerprints before/after install are required", "$.dependencies", "P0");
  } else if (deps.fingerprint_before !== deps.fingerprint_after) {
    fail(errors, "DEPENDENCY_DRIFT", "dependency fingerprints before and after install differ (DEPENDENCY_DRIFT)", "$.dependencies", "P0");
  }

  if (!Array.isArray(manifest.caches)) {
    fail(errors, "CACHE_LEAK", "caches must be an array", "$.caches", "P0");
  } else {
    for (const [i, cache] of manifest.caches.entries()) {
      const path = `$.caches[${i}]`;
      if (isMissing(cache.name)) fail(errors, "CACHE_LEAK", "cache record requires a name", `${path}.name`, "P0");
      if (!CACHE_OUTCOMES.has(cache.outcome)) {
        fail(errors, "CACHE_LEAK", `cache "${cache.name}" outcome must be cold_miss or hit, observed "${cache.outcome}"`, `${path}.outcome`, "P0");
        continue;
      }
      if (cache.outcome === "hit") {
        for (const field of ["key", "content_hash", "producer_run_id"]) {
          if (isMissing(cache[field])) {
            fail(errors, "CACHE_LEAK", `cache hit "${cache.name}" is missing provenance field ${field} (CACHE-3)`, `${path}.${field}`, "P0");
          }
        }
      }
    }
  }

  const capsule = manifest.capsule;
  if (!Number.isInteger(capsule.low) || !Number.isInteger(capsule.high) || capsule.low > capsule.high || capsule.low < 0 || capsule.high > 65535) {
    fail(errors, "CAPSULE_INVALID", "capsule must declare integer bounds low <= high within 0-65535", "$.capsule", "P0");
  } else {
    if (capsule.low <= FORBIDDEN_PORT && FORBIDDEN_PORT <= capsule.high) {
      fail(errors, "FORBIDDEN_PORT_6500", `capsule range ${capsule.low}-${capsule.high} contains forbidden port ${FORBIDDEN_PORT}`, "$.capsule", "P0");
    }
    if (!Array.isArray(manifest.listeners)) {
      fail(errors, "MISSING_FIELD", "listeners must be an array", "$.listeners", "P0");
    } else {
      for (const [i, l] of manifest.listeners.entries()) {
        const path = `$.listeners[${i}]`;
        if (!Number.isInteger(l.port)) {
          fail(errors, "LISTENER_INVALID", "listener port must be an integer", `${path}.port`, "P0");
          continue;
        }
        if (l.bind_address !== LOOPBACK) {
          fail(errors, "NON_LOOPBACK_BIND", `listener on port ${l.port} binds ${l.bind_address}, not ${LOOPBACK} (PORT-1)`, `${path}.bind_address`, "P0");
        }
        if (l.port < capsule.low || l.port > capsule.high) {
          fail(errors, "OUTSIDE_CAPSULE", `listener port ${l.port} outside capsule ${capsule.low}-${capsule.high} (PORT-2)`, `${path}.port`, "P0");
        }
      }
    }
  }

  const procs = manifest.processes;
  if (!Array.isArray(procs.leaked_pids)) {
    fail(errors, "LEAKED_PROCESS", "processes.leaked_pids must be an array", "$.processes.leaked_pids", "P0");
  } else if (procs.leaked_pids.length > 0) {
    fail(errors, "LEAKED_PROCESS", `leaked tracked processes survived cleanup: ${procs.leaked_pids.join(", ")} (PROC-2)`, "$.processes.leaked_pids", "P0");
  }

  let stagesValidated = 0;
  if (!Array.isArray(manifest.stages)) {
    fail(errors, "NONZERO_STAGE", "stages must be an array", "$.stages", "P0");
  } else {
    if (manifest.stages.length !== gates.length) {
      fail(errors, "NONZERO_STAGE", `expected ${gates.length} stage evidence records (one per contract gate), observed ${manifest.stages.length}`, "$.stages", "P0");
    }
    for (const [i, stage] of manifest.stages.entries()) {
      const path = `$.stages[${i}]`;
      const expectedGate = gates[i];
      if (!stage || stage.stage !== expectedGate) {
        fail(errors, "STAGE_ORDER_VIOLATION", `stage at position ${i} must be "${expectedGate}" (INV-1), observed "${stage && stage.stage}"`, `${path}.stage`, "P0");
        continue;
      }
      stagesValidated += 1;
      for (const field of requiredRecordFields) {
        if (isMissing(stage[field])) fail(errors, "NONZERO_STAGE", `stage "${expectedGate}" evidence record missing field ${field} (INV-2)`, `${path}.${field}`, "P0");
      }
      if (stage.exit_code !== 0) {
        fail(errors, "NONZERO_STAGE", `stage "${expectedGate}" exited ${stage.exit_code} (D-4)`, `${path}.exit_code`, "P0");
      }
      if (stage.base_commit !== manifest.base_commit) {
        fail(errors, "NONZERO_STAGE", `stage "${expectedGate}" base_commit does not match manifest base_commit`, `${path}.base_commit`, "P0");
      }
      if (!isMissing(stage.host_id) && stage.host_id !== manifest.host.id) {
        fail(errors, "STAGE_HOST_MISMATCH", `stage "${expectedGate}" host_id does not match manifest host.id`, `${path}.host_id`, "P1");
      }
      if (!isMissing(stage.host_image) && stage.host_image !== manifest.host.image) {
        fail(errors, "STAGE_HOST_MISMATCH", `stage "${expectedGate}" host_image does not match manifest host.image`, `${path}.host_image`, "P1");
      }
      const start = Date.parse(stage.started_at_utc);
      const finish = Date.parse(stage.finished_at_utc);
      if (Number.isNaN(start) || Number.isNaN(finish) || finish < start) {
        fail(errors, "STAGE_TIMESTAMP_INVALID", `stage "${expectedGate}" timestamps must be UTC date-times with finished_at_utc >= started_at_utc`, `${path}.started_at_utc`, "P1");
      }
    }
  }

  const byCode = {};
  for (const e of errors) byCode[e.code] = (byCode[e.code] || 0) + 1;
  return {
    verdict: errors.length === 0 ? "VALID" : "INVALID",
    label: manifest.label,
    errors,
    summary: { checks: gates.length + 12, stages_validated: stagesValidated, error_counts: byCode },
  };
}

function parseArgs(argv) {
  const args = {};
  for (let i = 0; i < argv.length; i += 1) {
    if (argv[i] === "--json") args.json = true;
    else if (argv[i] === "--contract") args.contract = argv[++i];
    else if (argv[i] === "--fixture") args.fixture = argv[++i];
  }
  return args;
}

function readJson(path, label) {
  try {
    return JSON.parse(readFileSync(path, "utf8"));
  } catch (err) {
    throw new Error(`cannot read ${label} at ${path}: ${err.message}`);
  }
}

function main() {
  const args = parseArgs(process.argv.slice(2));
  if (!args.contract || !args.fixture) {
    const message = "usage: validate-clean-machine-evidence.mjs --contract <contract.json> --fixture <manifest.json> [--json]";
    if (args.json) {
      process.stdout.write(`${JSON.stringify({ verdict: "ERROR", errors: [{ code: "USAGE", message }], summary: { checks: 0, stages: 0, error_counts: { USAGE: 1 } } })}\n`);
    } else {
      process.stderr.write(`${message}\n`);
    }
    process.exit(2);
  }
  let contract;
  let manifest;
  try {
    contract = readJson(args.contract, "contract");
    manifest = readJson(args.fixture, "fixture");
  } catch (err) {
    if (args.json) {
      process.stdout.write(`${JSON.stringify({ verdict: "ERROR", errors: [{ code: "IO", message: err.message }], summary: { checks: 0, stages: 0, error_counts: { IO: 1 } } })}\n`);
    } else {
      process.stderr.write(`${err.message}\n`);
    }
    process.exit(2);
  }
  const result = validateManifest(manifest, contract);
  if (args.json) {
    process.stdout.write(`${JSON.stringify(result, null, 2)}\n`);
  } else {
    process.stdout.write(`verdict: ${result.verdict}\n`);
    for (const e of result.errors) process.stdout.write(`${e.severity} ${e.code}: ${e.message}\n`);
    if (result.errors.length === 0) process.stdout.write("all clean-machine evidence checks passed\n");
  }
  process.exit(result.verdict === "VALID" ? 0 : 1);
}

const isMain = process.argv[1] && import.meta.url === pathToFileURL(process.argv[1]).href;
if (isMain) main();

export { validateManifest, versionAtLeast, scanForbiddenPort };
