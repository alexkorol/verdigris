#!/usr/bin/env node
import fs from "node:fs";
import path from "node:path";
import crypto from "node:crypto";
import process from "node:process";

const VALIDATOR_ID = "verdigris.validate-soak-evidence";
const SUPPORTED_POLICY_ID = "verdigris.soak-integration-policy";
const SUPPORTED_POLICY_MAJOR = 1;
const REQUIRED_POLICY_KEYS = [
  "schema_version",
  "contexts",
  "source_head",
  "platform",
  "port_capsule",
  "repetition",
  "timeout",
  "artifacts",
  "retry",
  "quarantine",
  "escalation",
  "verdict",
];
const KNOWN_CONTEXTS = ["local", "pre_merge", "nightly", "release_proof"];
const ATTEMPT_OUTCOMES = ["PASS", "FAIL", "QUARANTINED"];
const ATTEMPT_CLASSIFICATIONS = ["DETERMINISTIC", "ENVIRONMENTAL", "NOT_APPLICABLE"];
const CONCLUSIONS = ["PASS", "FAIL", "BLOCKED_ENVIRONMENTAL"];
const LOOPBACK_HOSTS = new Set(["127.0.0.1", "localhost", "::1", "::ffff:127.0.0.1"]);
const SHA40 = /^[0-9a-f]{40}$/;
const SHA64 = /^[0-9a-f]{64}$/;
const ISO8601 = /^\d{4}-\d{2}-\d{2}T\d{2}:\d{2}:\d{2}(\.\d+)?(Z|[+-]\d{2}:\d{2})$/;

const USAGE = `USAGE
  node validate-soak-evidence.mjs --policy <policy.json> --bundle <bundle.json>
      [--tip-under-judgment <40-hex sha>]

Evaluates a submitted server-lifecycle soak evidence bundle against the
accepted TASK-0135 soak integration policy. Deterministic and side-effect
free: it never runs a soak, binds or probes ports, mutates files, contacts
external systems, or decides OWNER_PENDING schedule/hosting questions.

STDOUT  one stable JSON result document
STDERR  human-readable diagnostics
EXIT    0  bundle accepted as a policy-valid PASS
        1  deterministic rejection, or a consistent FAIL /
           BLOCKED_ENVIRONMENTAL submission (only PASS exits zero)
        2  usage, parse, schema, or unsupported-policy/context error

Error codes judged (TASK-0135 canonical):
  STALE_SOURCE_HEAD, MISSING_PLATFORM_EVIDENCE, PORT_CAPSULE_COLLISION,
  FORBIDDEN_PORT_6500, TIMEOUT, NONZERO_SOAK, MISSING_ARTIFACT,
  RETRY_MASKED_FAILURE
Fail-closed extensions:
  UNSUPPORTED_PLATFORM, UNCLASSIFIED_FAILURE, INCOMPLETE_RUN,
  ARTIFACT_HASH_MISMATCH, PORT_CAPSULE_OUT_OF_RANGE, NON_LOOPBACK_BIND,
  ARTIFACT_INTERVAL_INVALID, INSUFFICIENT_INDEPENDENT_RUNS,
  RUN_INTERVAL_OVERLAP, FRESH_PROCESS_UNDISCLOSED, CONCLUSION_UNSUPPORTED
`;

function main(argv) {
  const parsed = parseArgs(argv);
  if (parsed.kind === "help") {
    process.stdout.write(USAGE);
    return 0;
  }
  if (parsed.kind === "invalid") {
    return emitInvalid(parsed.errorClass, parsed.message);
  }

  let policyText;
  try {
    policyText = fs.readFileSync(parsed.policyPath, "utf8");
  } catch (error) {
    return emitInvalid("parse", `policy unreadable: ${parsed.policyPath} (${error.message})`);
  }
  let policy;
  try {
    policy = JSON.parse(policyText);
  } catch (error) {
    return emitInvalid("parse", `policy is not valid JSON (${error.message})`);
  }
  const policyProblem = validatePolicy(policy);
  if (policyProblem) {
    return emitInvalid("unsupported_policy", policyProblem);
  }

  let bundleText;
  try {
    bundleText = fs.readFileSync(parsed.bundlePath, "utf8");
  } catch (error) {
    return emitInvalid("parse", `bundle unreadable: ${parsed.bundlePath} (${error.message})`);
  }
  let bundle;
  try {
    bundle = JSON.parse(bundleText);
  } catch (error) {
    return emitInvalid("parse", `bundle is not valid JSON (${error.message})`);
  }
  const schemaProblem = validateBundleSchema(bundle);
  if (schemaProblem) {
    return emitInvalid("schema", schemaProblem);
  }
  const contextProblem = validateContextEnabled(policy, bundle.context);
  if (contextProblem) {
    return emitInvalid("unsupported_context", contextProblem);
  }

  const report = evaluate(policy, bundle, parsed);
  process.stdout.write(`${JSON.stringify(report.document, null, 2)}\n`);
  for (const problem of report.document.errors) {
    process.stderr.write(`${VALIDATOR_ID}: error ${problem.code}: ${problem.detail}\n`);
  }
  for (const finding of report.document.findings) {
    process.stderr.write(`${VALIDATOR_ID}: finding ${finding.code}: ${finding.detail}\n`);
  }
  process.stderr.write(
    `${VALIDATOR_ID}: evaluated=${report.document.evaluated_conclusion} accepted=${report.document.accepted} exit=${report.exitCode}\n`,
  );
  return report.exitCode;
}

function parseArgs(argv) {
  const values = { policy: undefined, bundle: undefined, tip: undefined };
  const seen = new Set();
  for (let i = 0; i < argv.length; i += 1) {
    const arg = argv[i];
    if (arg === "--help" || arg === "-h") {
      return { kind: "help" };
    }
    const flagTargets = {
      "--policy": "policy",
      "--bundle": "bundle",
      "--tip-under-judgment": "tip",
    };
    const target = flagTargets[arg];
    if (!target) {
      return invalidInvocation("usage", `unknown argument: ${arg}`);
    }
    if (seen.has(target)) {
      return invalidInvocation("usage", `duplicate argument: ${arg}`);
    }
    seen.add(target);
    const value = argv[i + 1];
    if (value === undefined || value.startsWith("--")) {
      return invalidInvocation("usage", `missing value for ${arg}`);
    }
    values[target] = value;
    i += 1;
  }
  if (!seen.has("policy")) {
    return invalidInvocation("usage", "missing required argument --policy");
  }
  if (!seen.has("bundle")) {
    return invalidInvocation("usage", "missing required argument --bundle");
  }
  if (values.tip !== undefined && !SHA40.test(values.tip)) {
    return invalidInvocation("usage", "--tip-under-judgment must be a 40-hex lowercase sha");
  }
  return { kind: "ok", policyPath: values.policy, bundlePath: values.bundle, tip: values.tip };
}

function invalidInvocation(errorClass, message) {
  return { kind: "invalid", errorClass, message };
}

function emitInvalid(errorClass, message) {
  const document = {
    validator: VALIDATOR_ID,
    result: "ERROR",
    error_class: errorClass,
    message,
  };
  process.stdout.write(`${JSON.stringify(document, null, 2)}\n`);
  process.stderr.write(`${VALIDATOR_ID}: ${errorClass}: ${message}\n`);
  return 2;
}

function validatePolicy(policy) {
  if (typeof policy !== "object" || policy === null || Array.isArray(policy)) {
    return "policy root must be a JSON object";
  }
  for (const key of REQUIRED_POLICY_KEYS) {
    if (!(key in policy)) {
      return `policy missing required key: ${key}`;
    }
  }
  if (policy.policy_id !== SUPPORTED_POLICY_ID) {
    return `unsupported policy_id: ${String(policy.policy_id)}`;
  }
  if (typeof policy.schema_version !== "string" || !policy.schema_version.startsWith(`${SUPPORTED_POLICY_MAJOR}.`)) {
    return `unsupported policy schema_version: ${String(policy.schema_version)}`;
  }
  const capsule = policy.port_capsule;
  if (!Array.isArray(capsule.forbidden_ports) || !capsule.forbidden_ports.includes(6500)) {
    return "refusing to evaluate: policy does not forbid port 6500";
  }
  const range = capsule.allowed_range;
  if (
    !Array.isArray(range) ||
    range.length !== 2 ||
    !Number.isInteger(range[0]) ||
    !Number.isInteger(range[1]) ||
    range[0] >= range[1]
  ) {
    return "policy port_capsule.allowed_range must be [low, high] integers";
  }
  const requiredFields = policy.artifacts.required_fields;
  if (!Array.isArray(requiredFields) || requiredFields.length === 0 || requiredFields.some((f) => typeof f !== "string")) {
    return "policy artifacts.required_fields must be a non-empty string array";
  }
  if (!Number.isInteger(policy.retry.max_attempts_per_verdict_session) || policy.retry.max_attempts_per_verdict_session < 1) {
    return "policy retry.max_attempts_per_verdict_session must be a positive integer";
  }
  if (typeof policy.timeout.hard_watchdog_ms !== "number" || policy.timeout.hard_watchdog_ms <= 0) {
    return "policy timeout.hard_watchdog_ms must be a positive number";
  }
  const runsByContext = policy.repetition.independent_runs_by_context;
  if (typeof runsByContext !== "object" || runsByContext === null) {
    return "policy repetition.independent_runs_by_context must be an object";
  }
  const verdictValues = policy.verdict.allowed_values;
  if (!Array.isArray(verdictValues) || CONCLUSIONS.some((c) => !verdictValues.includes(c))) {
    return "policy verdict.allowed_values must include PASS, FAIL, BLOCKED_ENVIRONMENTAL";
  }
  for (const contextName of KNOWN_CONTEXTS) {
    if (typeof policy.contexts[contextName] !== "object" || policy.contexts[contextName] === null) {
      return `policy contexts missing entry: ${contextName}`;
    }
  }
  return undefined;
}

function validateBundleSchema(bundle) {
  if (typeof bundle !== "object" || bundle === null || Array.isArray(bundle)) {
    return "bundle root must be a JSON object";
  }
  for (const key of ["source_head", "platform", "context", "attempts", "conclusion"]) {
    if (!(key in bundle)) {
      return `bundle missing required key: ${key}`;
    }
  }
  if (typeof bundle.source_head !== "string" || !SHA40.test(bundle.source_head)) {
    return "bundle source_head must be a 40-hex lowercase git sha";
  }
  if (typeof bundle.platform !== "object" || bundle.platform === null || Array.isArray(bundle.platform)) {
    return "bundle platform must be an object";
  }
  if (!KNOWN_CONTEXTS.includes(bundle.context)) {
    return `bundle context must be one of ${KNOWN_CONTEXTS.join("|")}`;
  }
  if (!Array.isArray(bundle.attempts)) {
    return "bundle attempts must be an array";
  }
  for (const [index, attempt] of bundle.attempts.entries()) {
    if (typeof attempt !== "object" || attempt === null || Array.isArray(attempt)) {
      return `bundle attempts[${index}] must be an object`;
    }
    if (!ATTEMPT_OUTCOMES.includes(attempt.outcome)) {
      return `bundle attempts[${index}].outcome must be one of ${ATTEMPT_OUTCOMES.join("|")}`;
    }
    if (!ATTEMPT_CLASSIFICATIONS.includes(attempt.classification)) {
      return `bundle attempts[${index}].classification must be one of ${ATTEMPT_CLASSIFICATIONS.join("|")}`;
    }
  }
  if (!CONCLUSIONS.includes(bundle.conclusion)) {
    return `bundle conclusion must be one of ${CONCLUSIONS.join("|")}`;
  }
  return undefined;
}

function validateContextEnabled(policy, contextName) {
  const context = policy.contexts[contextName];
  if (context.enabled !== true) {
    const state = context.state ? ` (state ${context.state})` : "";
    return `context ${contextName} is not enabled in this policy${state}; judging it would decide an OWNER_PENDING question`;
  }
  return undefined;
}

function evaluate(policy, bundle, parsedArgs) {
  const errors = [];
  const findings = [];
  const notes = [];
  const attempts = bundle.attempts;
  const submitted = bundle.conclusion;
  const capsule = policy.port_capsule;
  const [rangeLow, rangeHigh] = capsule.allowed_range;

  const pushError = (code, detail, attemptIndex) => {
    const entry = { code, detail };
    if (attemptIndex !== undefined) {
      entry.attempt_index = attemptIndex;
    }
    errors.push(entry);
  };
  const pushFinding = (code, detail, attemptIndex) => {
    const entry = { code, detail };
    if (attemptIndex !== undefined) {
      entry.attempt_index = attemptIndex;
    }
    findings.push(entry);
  };

  const masking = collectMasking(bundle, attempts);
  for (const detail of masking) {
    pushError("RETRY_MASKED_FAILURE", detail);
  }
  if (errors.length > 0) {
    return finish("FAIL", false, submitted);
  }

  for (const [index, attempt] of attempts.entries()) {
    for (const problem of scanDisclosedPorts(attempt, rangeLow, rangeHigh)) {
      pushError(problem.code, problem.detail, index);
    }
  }
  if (errors.length > 0) {
    return finish("FAIL", false, submitted);
  }

  if (!platformComplete(policy, bundle.platform)) {
    pushError("MISSING_PLATFORM_EVIDENCE", "platform evidence is missing one of os_name, os_version, cpu_architecture, toolchain");
    return finish("FAIL", false, submitted);
  }

  const tip = parsedArgs.tip !== undefined ? parsedArgs.tip : bundle.tip_under_judgment;
  if (tip !== undefined && tip !== bundle.source_head) {
    pushError("STALE_SOURCE_HEAD", `source_head ${bundle.source_head} does not match tip under judgment ${tip}`);
    return finish("FAIL", false, submitted);
  }

  if (submitted !== "PASS" && !platformSupported(policy, bundle.platform)) {
    pushError(
      "UNSUPPORTED_PLATFORM",
      `platform ${bundle.platform.os_name}/${bundle.platform.cpu_architecture} is outside policy supported_now`,
    );
    return finish("FAIL", false, submitted);
  }

  const timeoutFacts = collectTimeoutFacts(policy, bundle, attempts);
  const deterministicFacts = [];
  const nonzeroFacts = [];
  const missingArtifactExitFacts = [];
  const collidedIndexes = [];
  const cleanIndexes = [];
  for (const [index, attempt] of attempts.entries()) {
    const exitCode = attempt.process_exit_code;
    if (exitCode === 3) {
      timeoutFacts.push(`attempt ${index} hit the hard watchdog (process_exit_code 3)`);
    } else if (exitCode === 2) {
      missingArtifactExitFacts.push(`attempt ${index} exited 2: evidence file could not be written`);
    } else if (exitCode === 1) {
      nonzeroFacts.push(`attempt ${index} exited 1 with soak phase failures`);
    }
    if (attempt.timed_out === true) {
      timeoutFacts.push(`attempt ${index} disclosed timed_out=true`);
    }
    if (attempt.outcome === "FAIL") {
      if (attempt.classification === "DETERMINISTIC") {
        deterministicFacts.push(`attempt ${index} failed with classification DETERMINISTIC`);
      } else if (attempt.classification === "NOT_APPLICABLE") {
        deterministicFacts.push(`attempt ${index} failed without an environmental classification`);
      } else {
        collidedIndexes.push(index);
      }
    } else if (attempt.outcome === "QUARANTINED") {
      collidedIndexes.push(index);
    } else {
      cleanIndexes.push(index);
    }
  }

  if (submitted === "PASS") {
    if (errors.length > 0) {
      return finish("FAIL", false, submitted);
    }
    if (!platformSupported(policy, bundle.platform)) {
      pushError("UNSUPPORTED_PLATFORM", `platform ${bundle.platform.os_name}/${bundle.platform.cpu_architecture} is outside policy supported_now; greens are never extrapolated`);
      return finish("FAIL", false, submitted);
    }
    if (timeoutFacts.length > 0) {
      pushError("TIMEOUT", timeoutFacts[0]);
      return finish("FAIL", false, submitted);
    }
    if (deterministicFacts.length > 0 || nonzeroFacts.length > 0) {
      const first = deterministicFacts[0] ?? nonzeroFacts[0];
      pushError("NONZERO_SOAK", first);
      return finish("FAIL", false, submitted);
    }
    if (missingArtifactExitFacts.length > 0) {
      pushError("MISSING_ARTIFACT", missingArtifactExitFacts[0]);
      return finish("FAIL", false, submitted);
    }
    const maxAttempts = policy.retry.max_attempts_per_verdict_session;
    if (cleanIndexes.length === 0 && collidedIndexes.length > 0) {
      pushError("PORT_CAPSULE_COLLISION", "every attempt in the session collided; no clean full re-run completed");
      return finish("BLOCKED_ENVIRONMENTAL", submitted === "BLOCKED_ENVIRONMENTAL", submitted);
    }
    if (attempts.length > maxAttempts) {
      pushError("PORT_CAPSULE_COLLISION", `session discloses ${attempts.length} attempts, above the limit of ${maxAttempts}; persistent collision`);
      return finish("BLOCKED_ENVIRONMENTAL", false, submitted);
    }
    const artifactProblems = verifyCleanArtifacts(policy, bundle, parsedArgs, cleanIndexes, pushFinding);
    if (artifactProblems.length > 0) {
      const first = artifactProblems[0];
      pushError(first.code, first.detail, first.attempt_index);
      return finish("FAIL", false, submitted);
    }
    const countingProblem = checkIndependence(policy, bundle, cleanIndexes);
    if (countingProblem) {
      pushError(countingProblem.code, countingProblem.detail);
      return finish("FAIL", false, submitted);
    }
    return finish("PASS", true, submitted);
  }

  const substantiating = [];
  const addCause = (code, detail) => substantiating.push({ code, detail });
  for (const fact of timeoutFacts) {
    addCause("TIMEOUT", fact);
  }
  for (const fact of deterministicFacts) {
    addCause("NONZERO_SOAK", fact);
  }
  for (const fact of nonzeroFacts) {
    addCause("NONZERO_SOAK", fact);
  }
  for (const fact of missingArtifactExitFacts) {
    addCause("MISSING_ARTIFACT", fact);
  }
  if (collidedIndexes.length > 0) {
    addCause("PORT_CAPSULE_COLLISION", `${collidedIndexes.length} attempt(s) collided in the port capsule (classification ENVIRONMENTAL)`);
  }
  for (const index of attempts.keys()) {
    if (attempts[index].outcome !== "PASS" && typeof attempts[index].artifact_path !== "string") {
      addCause("MISSING_ARTIFACT", `attempt ${index} discloses no artifact reference`);
    }
  }

  if (submitted === "FAIL") {
    if (substantiating.length === 0) {
      pushError("CONCLUSION_UNSUPPORTED", "FAIL submitted but every disclosed attempt is a clean pass");
      return finish("FAIL", false, submitted);
    }
    for (const cause of substantiating) {
      pushFinding(cause.code, cause.detail);
    }
    return finish("FAIL", true, submitted);
  }

  if (deterministicFacts.length > 0 || nonzeroFacts.length > 0) {
    pushError("CONCLUSION_UNSUPPORTED", "BLOCKED_ENVIRONMENTAL submitted but the session contains deterministic failures");
    return finish("FAIL", false, submitted);
  }
  if (cleanIndexes.length > 0 && attempts.length <= policy.retry.max_attempts_per_verdict_session) {
    pushError("CONCLUSION_UNSUPPORTED", "BLOCKED_ENVIRONMENTAL submitted but clean full re-runs completed within quarantine limits");
    return finish("FAIL", false, submitted);
  }
  if (collidedIndexes.length === 0) {
    pushError("CONCLUSION_UNSUPPORTED", "BLOCKED_ENVIRONMENTAL submitted but no port-capsule collision is disclosed");
    return finish("FAIL", false, submitted);
  }
  for (const cause of substantiating) {
    pushFinding(cause.code, cause.detail);
  }
  return finish("BLOCKED_ENVIRONMENTAL", true, submitted);

  function finish(evaluated, accepted, submittedConclusion) {
    const document = {
      validator: VALIDATOR_ID,
      policy: {
        id: policy.policy_id,
        revision: policy.revision,
        schema_version: policy.schema_version,
      },
      bundle: parsedArgs.bundlePath,
      context: bundle.context,
      source_head: bundle.source_head,
      submitted_conclusion: submittedConclusion,
      evaluated_conclusion: evaluated,
      accepted,
      errors,
      findings,
      notes,
    };
    const passAccepted = evaluated === "PASS" && accepted;
    return { document, exitCode: passAccepted ? 0 : 1 };
  }
}

function collectMasking(bundle, attempts) {
  const details = [];
  const undisclosed = bundle.undisclosed_failed_attempts;
  if (Number.isInteger(undisclosed) && undisclosed > 0) {
    details.push(`bundle discloses ${undisclosed} undisclosed failed attempt(s)`);
  }
  const sessionCount = bundle.session_attempt_count;
  if (Number.isInteger(sessionCount) && sessionCount !== attempts.length) {
    details.push(`bundle records ${sessionCount} session attempts but lists ${attempts.length}`);
  }
  const seenPaths = new Map();
  for (const [index, attempt] of attempts.entries()) {
    if (typeof attempt.artifact_path !== "string") {
      continue;
    }
    if (seenPaths.has(attempt.artifact_path)) {
      details.push(`attempts ${seenPaths.get(attempt.artifact_path)} and ${index} cite the same artifact ${attempt.artifact_path}`);
    } else {
      seenPaths.set(attempt.artifact_path, index);
    }
  }
  return details;
}

function scanDisclosedPorts(attempt, rangeLow, rangeHigh) {
  const problems = [];
  const portFields = ["listener_port", "port", "client_port", "server_port"];
  for (const field of portFields) {
    const value = attempt[field];
    if (typeof value !== "number" || !Number.isInteger(value)) {
      continue;
    }
    if (value === 6500) {
      problems.push({ code: "FORBIDDEN_PORT_6500", detail: `attempt discloses ${field}=6500, the owner-reserved forbidden port` });
    } else if (value < rangeLow || value > rangeHigh) {
      problems.push({ code: "PORT_CAPSULE_OUT_OF_RANGE", detail: `attempt discloses ${field}=${value} outside allowed range ${rangeLow}-${rangeHigh}` });
    }
  }
  if (typeof attempt.host === "string" && attempt.host.length > 0 && !LOOPBACK_HOSTS.has(attempt.host.toLowerCase())) {
    problems.push({ code: "NON_LOOPBACK_BIND", detail: `attempt discloses non-loopback host ${attempt.host}` });
  }
  return problems;
}

function platformComplete(policy, platform) {
  return policy.platform.evidence_required.every((field) => {
    const value = platform[field];
    return typeof value === "string" && value.trim().length > 0;
  });
}

function platformSupported(policy, platform) {
  return (policy.platform.supported_now ?? []).some((entry) => {
    const osMatches = entry.os_name.toLowerCase() === platform.os_name.toLowerCase();
    const archMatches = entry.cpu_architecture.toLowerCase() === platform.cpu_architecture.toLowerCase();
    const toolchainToken = String(entry.toolchain).split(/\s+/)[0] ?? "";
    const toolchainMatches =
      toolchainToken.length === 0 || platform.toolchain.toLowerCase().includes(toolchainToken.toLowerCase());
    return osMatches && archMatches && toolchainMatches;
  });
}

function collectTimeoutFacts(policy, bundle, attempts) {
  const facts = [];
  const contextBudget = resolveContextBudget(policy, bundle.context);
  for (const [index, attempt] of attempts.entries()) {
    const interval = parseInterval(attempt.started_at, attempt.finished_at);
    if (interval === undefined) {
      continue;
    }
    if (interval.error !== undefined) {
      continue;
    }
    if (interval.durationMs > policy.timeout.hard_watchdog_ms) {
      facts.push(`attempt ${index} ran ${interval.durationMs}ms, beyond the ${policy.timeout.hard_watchdog_ms}ms hard watchdog`);
    } else if (contextBudget !== undefined && interval.durationMs > contextBudget) {
      facts.push(`attempt ${index} ran ${interval.durationMs}ms, beyond the ${bundle.context} budget of ${contextBudget}ms`);
    }
  }
  return facts;
}

function resolveContextBudget(policy, contextName) {
  const context = policy.contexts[contextName] ?? {};
  const onceEnabled = context.bindings_once_enabled ?? {};
  return (
    onceEnabled.total_wall_clock_budget_ms ??
    onceEnabled.wall_clock_budget_ms ??
    context.wall_clock_budget_ms ??
    policy.timeout.context_wall_clock_budget_ms?.[contextName]
  );
}

function parseInterval(startedAt, finishedAt) {
  if (typeof startedAt !== "string" || typeof finishedAt !== "string") {
    return undefined;
  }
  if (!ISO8601.test(startedAt) || !ISO8601.test(finishedAt)) {
    return { error: "not ISO-8601" };
  }
  const start = Date.parse(startedAt);
  const end = Date.parse(finishedAt);
  if (!Number.isFinite(start) || !Number.isFinite(end)) {
    return { error: "unparseable timestamp" };
  }
  if (end < start) {
    return { error: "finished before started" };
  }
  return { startMs: start, endMs: end, durationMs: end - start };
}

function verifyCleanArtifacts(policy, bundle, parsedArgs, cleanIndexes, pushFinding) {
  const problems = [];
  const requiredFields = policy.artifacts.required_fields;
  const contextBudget = resolveContextBudget(policy, bundle.context);
  const [rangeLow, rangeHigh] = policy.port_capsule.allowed_range;
  for (const index of cleanIndexes) {
    const attempt = bundle.attempts[index];
    const fail = (code, detail) => problems.push({ code, detail, attempt_index: index });
    if (typeof attempt.artifact_path !== "string") {
      fail("MISSING_ARTIFACT", `passing attempt ${index} cites no artifact_path`);
      continue;
    }
    if (typeof attempt.sha256 !== "string" || !SHA64.test(attempt.sha256)) {
      fail("MISSING_ARTIFACT", `passing attempt ${index} lacks a 64-hex sha256 artifact binding`);
      continue;
    }
    const interval = parseInterval(attempt.started_at, attempt.finished_at);
    if (interval === undefined || interval.error !== undefined) {
      fail("MISSING_ARTIFACT", `passing attempt ${index} lacks valid ISO-8601 started_at/finished_at bindings`);
      continue;
    }
    const artifactPath = resolveArtifactPath(parsedArgs.bundlePath, attempt.artifact_path);
    let artifactBytes;
    try {
      artifactBytes = fs.readFileSync(artifactPath);
    } catch {
      fail("MISSING_ARTIFACT", `artifact not readable at ${attempt.artifact_path}`);
      continue;
    }
    let artifact;
    try {
      artifact = JSON.parse(artifactBytes.toString("utf8"));
    } catch {
      fail("MISSING_ARTIFACT", `artifact at ${attempt.artifact_path} is not valid JSON`);
      continue;
    }
    if (typeof artifact !== "object" || artifact === null || Array.isArray(artifact)) {
      fail("MISSING_ARTIFACT", `artifact at ${attempt.artifact_path} is not a JSON object`);
      continue;
    }
    const missingFields = requiredFields.filter((field) => !(field in artifact));
    if (missingFields.length > 0) {
      fail("MISSING_ARTIFACT", `artifact at ${attempt.artifact_path} missing required fields: ${missingFields.join(", ")}`);
      continue;
    }
    const actualSha = crypto.createHash("sha256").update(artifactBytes).digest("hex");
    if (actualSha !== attempt.sha256) {
      fail("ARTIFACT_HASH_MISMATCH", `artifact ${attempt.artifact_path} hashes to ${actualSha}, bundle claims ${attempt.sha256}`);
      continue;
    }
    if (artifact.passed !== true || !Array.isArray(artifact.failures) || artifact.failures.length !== 0) {
      fail("INCOMPLETE_RUN", `artifact ${attempt.artifact_path} is not a clean pass (passed=${JSON.stringify(artifact.passed)}, failures=${JSON.stringify(artifact.failures)})`);
      continue;
    }
    if (
      artifact.cyclesCompleted !== artifact.cyclesRequested ||
      artifact.cyclesRequested !== policy.repetition.cycles_per_run ||
      artifact.burstPassed !== true
    ) {
      fail("INCOMPLETE_RUN", `artifact ${attempt.artifact_path} did not complete ${policy.repetition.cycles_per_run} cycles plus burst`);
      continue;
    }
    const countFields = ["upgradesSucceeded", "loginsSucceeded", "cleanCloses"];
    const badCounts = countFields.filter((field) => typeof artifact[field] !== "number" || artifact[field] < 0);
    if (badCounts.length > 0) {
      fail("INCOMPLETE_RUN", `artifact ${attempt.artifact_path} has invalid counters: ${badCounts.join(", ")}`);
      continue;
    }
    const artifactInterval = parseInterval(artifact.startedAt, artifact.finishedAt);
    if (artifactInterval === undefined || artifactInterval.error !== undefined) {
      fail("ARTIFACT_INTERVAL_INVALID", `artifact ${attempt.artifact_path} startedAt/finishedAt are not a valid ordered ISO-8601 interval`);
      continue;
    }
    if (artifactInterval.durationMs > policy.timeout.hard_watchdog_ms) {
      fail("TIMEOUT", `artifact ${attempt.artifact_path} ran ${artifactInterval.durationMs}ms, beyond the hard watchdog`);
      continue;
    }
    if (contextBudget !== undefined && artifactInterval.durationMs > contextBudget) {
      fail("TIMEOUT", `artifact ${attempt.artifact_path} ran ${artifactInterval.durationMs}ms, beyond the ${bundle.context} budget of ${contextBudget}ms`);
      continue;
    }
    const capsuleProblem = scanArtifactPortCapsule(artifact, rangeLow, rangeHigh);
    if (capsuleProblem) {
      fail(capsuleProblem.code, capsuleProblem.detail);
      continue;
    }
    pushFinding("CLEAN_PASS_VERIFIED", `artifact ${attempt.artifact_path} verified clean (sha256 bound, ${artifact.cyclesCompleted}/${artifact.cyclesRequested} cycles, burst green)`, index);
  }
  return problems;
}

function resolveArtifactPath(bundlePath, artifactRef) {
  const fromCwd = path.resolve(process.cwd(), artifactRef);
  if (fs.existsSync(fromCwd)) {
    return fromCwd;
  }
  const fromBundle = path.resolve(path.dirname(path.resolve(bundlePath)), artifactRef);
  if (fs.existsSync(fromBundle)) {
    return fromBundle;
  }
  return fromCwd;
}

function scanArtifactPortCapsule(artifact, rangeLow, rangeHigh) {
  const capsule = artifact.portCapsule;
  if (typeof capsule !== "object" || capsule === null) {
    return undefined;
  }
  if (typeof capsule.host === "string" && capsule.host.length > 0 && !LOOPBACK_HOSTS.has(capsule.host.toLowerCase())) {
    return { code: "NON_LOOPBACK_BIND", detail: `artifact portCapsule.host ${capsule.host} is not loopback` };
  }
  const ports = Array.isArray(capsule.ports) ? capsule.ports : [];
  for (const port of ports) {
    if (typeof port !== "number" || !Number.isInteger(port)) {
      continue;
    }
    if (port === 6500) {
      return { code: "FORBIDDEN_PORT_6500", detail: "artifact portCapsule discloses port 6500, the owner-reserved forbidden port" };
    }
    if (port < rangeLow || port > rangeHigh) {
      return { code: "PORT_CAPSULE_OUT_OF_RANGE", detail: `artifact portCapsule discloses port ${port} outside allowed range ${rangeLow}-${rangeHigh}` };
    }
  }
  return undefined;
}

function checkIndependence(policy, bundle, cleanIndexes) {
  const contextName = bundle.context;
  if (contextName === "release_proof") {
    const intervals = [];
    for (const index of cleanIndexes) {
      const attempt = bundle.attempts[index];
      if (attempt.fresh_process !== true) {
        return {
          code: "FRESH_PROCESS_UNDISCLOSED",
          detail: `release_proof attempt ${index} does not disclose fresh_process=true`,
        };
      }
      const interval = parseInterval(attempt.started_at, attempt.finished_at);
      if (interval !== undefined && interval.error === undefined) {
        for (const prior of intervals) {
          if (interval.startMs < prior.endMs && prior.startMs < interval.endMs) {
            return {
              code: "RUN_INTERVAL_OVERLAP",
              detail: `release_proof attempts ${prior.index} and ${index} have overlapping wall-clock intervals`,
            };
          }
        }
        intervals.push({ ...interval, index });
      }
    }
  }
  const required =
    policy.repetition.independent_runs_by_context[contextName] ??
    policy.contexts[contextName]?.independent_runs;
  if (Number.isInteger(required) && cleanIndexes.length < required) {
    return {
      code: "INSUFFICIENT_INDEPENDENT_RUNS",
      detail: `context ${contextName} requires ${required} verified independent run(s); only ${cleanIndexes.length} verified`,
    };
  }
  return undefined;
}

process.exitCode = main(process.argv.slice(2));
