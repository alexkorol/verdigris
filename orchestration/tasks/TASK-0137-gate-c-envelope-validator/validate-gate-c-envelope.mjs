#!/usr/bin/env node
// Gate C decision-envelope validator (TASK-0137).
// Dependency-free Node CLI implementing the 13-check contract in
// orchestration/tasks/TASK-0130-gate-c-decision-envelope/VALIDATION.md.
// Content-neutral: it validates shapes, states, and citations only. Honest
// MISSING / OWNER_PENDING values are preserved and reported, never filled.

import { readFileSync } from "node:fs";
import { argv, cwd, exit } from "node:process";
import { isAbsolute, resolve } from "node:path";
import { pathToFileURL } from "node:url";

const DECISION_FIELDS = [
  "concrete_goal",
  "boss_or_danger",
  "expected_item_family",
  "depth",
  "branch_consequence",
  "extraction_or_return",
];

const FIELD_ERROR_CODES = {
  concrete_goal: "MISSING_CONCRETE_GOAL",
  boss_or_danger: "MISSING_BOSS_OR_DANGER",
  expected_item_family: "MISSING_EXPECTED_ITEM_FAMILY",
  depth: "MISSING_DEPTH",
  branch_consequence: "MISSING_BRANCH_CONSEQUENCE",
  extraction_or_return: "MISSING_EXTRACTION_OR_RETURN",
};

const FIELD_STATES = new Set([
  "AVAILABLE",
  "DERIVABLE-WITHOUT-GAMEPLAY-RULES",
  "MISSING",
]);

const SHA_RE = /^[0-9a-f]{7,40}$/i;
const TASK_ID_RE = /TASK-\d{3,}/;

export function parseArgs(argvSlice) {
  const args = { json: false };
  for (let i = 0; i < argvSlice.length; i++) {
    const a = argvSlice[i];
    if (a === "--json") {
      args.json = true;
    } else if (a === "--schema") {
      args.schema = argvSlice[++i];
    } else if (a === "--fixture") {
      args.fixture = argvSlice[++i];
    } else {
      throw usageError(`unknown argument: ${a}`);
    }
  }
  if (!args.schema) throw usageError("--schema <path> is required");
  if (!args.fixture) throw usageError("--fixture <path> is required");
  return args;
}

function usageError(message) {
  const err = new Error(message);
  err.usage = true;
  return err;
}

export function loadJsonFile(path) {
  const absolute = isAbsolute(path) ? path : resolve(cwd(), path);
  let text;
  try {
    text = readFileSync(absolute, "utf8");
  } catch (err) {
    const wrapped = new Error(`cannot read ${absolute}: ${err.message}`);
    wrapped.io = true;
    throw wrapped;
  }
  return { absolute, text };
}

function isPlainObject(value) {
  return typeof value === "object" && value !== null && !Array.isArray(value);
}

function isNonEmptyString(value) {
  return typeof value === "string" && value.trim().length > 0;
}

function collectNumbers(value, out = []) {
  if (typeof value === "number" && Number.isFinite(value)) {
    out.push(value);
  } else if (Array.isArray(value)) {
    for (const item of value) collectNumbers(item, out);
  } else if (isPlainObject(value)) {
    for (const key of Object.keys(value)) collectNumbers(value[key], out);
  }
  return out;
}

function hasIdentitySubShape(identity) {
  return (
    isPlainObject(identity.road) &&
    isNonEmptyString(identity.road.roadId) &&
    isNonEmptyString(identity.road.roadName) &&
    isPlainObject(identity.node) &&
    isNonEmptyString(identity.node.nodeId) &&
    isNonEmptyString(identity.node.name) &&
    Number.isFinite(identity.tier)
  );
}

function provenanceSatisfied(prov) {
  if (!isPlainObject(prov)) return false;
  const flatShape =
    isNonEmptyString(prov.authority_source) &&
    isNonEmptyString(prov.audit_reference) &&
    isNonEmptyString(prov.base_commit) &&
    SHA_RE.test(prov.base_commit.trim());
  if (flatShape) return true;
  const narrative = prov.provenance_of_this_contract;
  if (!Array.isArray(narrative) || narrative.length < 3) return false;
  if (!narrative.every((entry) => isNonEmptyString(entry))) return false;
  const joined = narrative.join("\n");
  return (
    TASK_ID_RE.test(joined) &&
    /\b[0-9a-f]{40}\b/i.test(joined) &&
    /\.md|\.\.(\/)|docs\//i.test(joined)
  );
}

function fieldBlocksDecision(field) {
  if (!isPlainObject(field)) return true;
  const { state } = field;
  if (!FIELD_STATES.has(state)) return true;
  if (state === "MISSING") return true;
  if (state === "AVAILABLE" && field.value === undefined) return true;
  if (state === "AVAILABLE" && field.value === null) return true;
  return false;
}

// Core validation. `raw` is the fixture file text; returns the result object.
// Fatal checks (nonzero exit): 1-5, 12, 13. Readiness blockers (exit 0,
// ready:false): honest MISSING states under checks 6-11.
export function validateRaw(schema, raw) {
  let envelope;
  try {
    envelope = JSON.parse(raw);
  } catch {
    return fatal(1, "INVALID_JSON", null);
  }
  if (!isPlainObject(envelope)) return fatal(1, "INVALID_JSON", null);
  return validateEnvelope(schema, envelope);
}

export function validateEnvelope(schema, envelope) {
  // Check 2: UNSUPPORTED_VERSION
  const supported = Array.isArray(schema?.supported_versions)
    ? schema.supported_versions
    : [];
  const version = envelope.schema_version;
  if (typeof version !== "string" || !supported.includes(version)) {
    return fatal(2, "UNSUPPORTED_VERSION", version ?? null);
  }

  // Check 3: MISSING_ROUTE_IDENTITY
  const identity = envelope.route_identity;
  if (!isPlainObject(identity) || !hasIdentitySubShape(identity)) {
    return fatal(3, "MISSING_ROUTE_IDENTITY", null);
  }

  // Check 4: ROUTE_NAME_ONLY
  const carriesStateOrValue = DECISION_FIELDS.some((name) => {
    const field = envelope[name];
    return (
      isPlainObject(field) &&
      (field.state !== undefined || field.value !== undefined)
    );
  });
  if (!carriesStateOrValue) {
    return fatal(4, "ROUTE_NAME_ONLY", null);
  }

  // Check 5: MISSING_PROVENANCE
  if (!provenanceSatisfied(envelope.evidence_provenance)) {
    return fatal(5, "MISSING_PROVENANCE", null);
  }

  // Checks 6-11: per-field MISSING blockers in fixed order.
  const missingFieldCodes = [];
  const ownerPendingFields = [];
  const fieldStates = {};
  for (const name of DECISION_FIELDS) {
    const field = envelope[name];
    const state = isPlainObject(field) ? field.state : null;
    fieldStates[name] = {
      state: FIELD_STATES.has(state) ? state : null,
      owner_pending: isPlainObject(field) && field.owner_pending === true,
      present: isPlainObject(field),
    };
    if (fieldBlocksDecision(field)) missingFieldCodes.push(FIELD_ERROR_CODES[name]);
    if (isPlainObject(field) && field.owner_pending === true) {
      ownerPendingFields.push(name);
    }
  }

  // Check 12: CONTRADICTORY_DEPTH
  const depthField = envelope.depth;
  const depthUsable =
    isPlainObject(depthField) &&
    (depthField.state === "AVAILABLE" ||
      depthField.state === "DERIVABLE-WITHOUT-GAMEPLAY-RULES");
  if (depthUsable) {
    const numbers = collectNumbers(depthField.value);
    const unique = [...new Set(numbers)];
    if (unique.length > 1 || (unique.length === 1 && unique[0] !== identity.tier)) {
      return fatal(12, "CONTRADICTORY_DEPTH", { depth_numbers: numbers, identity_tier: identity.tier });
    }
  }

  // Check 13: OWNER_PENDING_CONTENT (readiness claimed falsely)
  const completeness = isPlainObject(envelope.completeness)
    ? envelope.completeness
    : {};
  const claimedReady = completeness.ready === true;
  const declaredMissingEmpty =
    Array.isArray(completeness.missing_fields) &&
    completeness.missing_fields.length === 0;
  if (
    claimedReady &&
    (missingFieldCodes.length > 0 ||
      ownerPendingFields.length > 0 ||
      !declaredMissingEmpty)
  ) {
    return fatal(13, "OWNER_PENDING_CONTENT", {
      missing_field_codes: missingFieldCodes,
      owner_pending_fields: ownerPendingFields,
      declared_missing_fields: Array.isArray(completeness.missing_fields)
        ? completeness.missing_fields
        : null,
    });
  }

  const ready =
    missingFieldCodes.length === 0 &&
    ownerPendingFields.length === 0 &&
    declaredMissingEmpty &&
    !fieldStates.concrete_goal.owner_pending;

  return {
    valid: true,
    ready,
    error: null,
    error_index: null,
    detail: null,
    schema_version: version,
    missing_field_codes: missingFieldCodes,
    owner_pending_fields: ownerPendingFields,
    field_states: fieldStates,
    completeness_ready_claimed: completeness.ready === true,
  };
}

function fatal(index, code, detail) {
  return {
    valid: false,
    ready: false,
    error: code,
    error_index: index,
    detail,
    schema_version: null,
    missing_field_codes: [],
    owner_pending_fields: [],
    field_states: {},
    completeness_ready_claimed: false,
  };
}

export function runValidation(schemaPath, fixturePath) {
  const schemaLoaded = loadJsonFile(schemaPath);
  let schema;
  try {
    schema = JSON.parse(schemaLoaded.text);
  } catch (err) {
    const wrapped = new Error(`schema is not valid JSON: ${err.message}`);
    wrapped.io = true;
    throw wrapped;
  }
  if (!isPlainObject(schema) || !Array.isArray(schema.supported_versions)) {
    const err = new Error("schema lacks supported_versions[]");
    err.io = true;
    throw err;
  }
  const fixtureLoaded = loadJsonFile(fixturePath);
  const result = validateRaw(schema, fixtureLoaded.text);
  return {
    contract_id: schema.contract_id ?? "gate-c-decision-envelope",
    schema_path: schemaLoaded.absolute,
    fixture_path: fixtureLoaded.absolute,
    ...result,
  };
}

function main() {
  let args;
  try {
    args = parseArgs(argv.slice(2));
  } catch (err) {
    console.error(`usage error: ${err.message}`);
    console.error(
      "usage: node validate-gate-c-envelope.mjs --schema <schema.json> --fixture <envelope.json> [--json]",
    );
    return exit(2);
  }
  let result;
  try {
    result = runValidation(args.schema, args.fixture);
  } catch (err) {
    console.error(err.message);
    return exit(2);
  }
  if (args.json) {
    console.log(JSON.stringify(result, null, 2));
  } else if (!result.valid) {
    console.log(`FAIL ${result.error_index} ${result.error}`);
  } else {
    console.log(`OK valid=${result.valid} ready=${result.ready}`);
    for (const code of result.missing_field_codes) console.log(`BLOCKED ${code}`);
    for (const field of result.owner_pending_fields) {
      console.log(`OWNER_PENDING ${field}`);
    }
  }
  return exit(result.valid ? 0 : 1);
}

const invokedDirectly =
  argv[1] !== undefined &&
  import.meta.url === pathToFileURL(resolve(argv[1])).href;

if (invokedDirectly) main();
