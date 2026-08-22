#!/usr/bin/env node
import { createHash } from "node:crypto";
import { existsSync, readFileSync, statSync } from "node:fs";
import { isAbsolute, resolve } from "node:path";
import { argv, exit, stdout, stderr } from "node:process";
import { pathToFileURL } from "node:url";

const REQUIRED_KEYS = [
  "schema_version",
  "source_head",
  "commands",
  "environment",
  "artifacts",
  "platform_coverage",
  "rollback",
  "owner_actions",
  "verdict",
];

const OWNER_ACTION_KINDS = new Set([
  "release build",
  "deployment",
  "installer packaging",
  "code signing",
  "notarization",
  "distribution account action",
]);

function sha256File(path) {
  return createHash("sha256").update(readFileSync(path)).digest("hex");
}

function err(code, message, detail = {}) {
  return { kind: "integrity_error", code, message, ...detail };
}

function gap(code, message, detail = {}) {
  return { kind: "evidence_gap", code, message, ...detail };
}

function checkHead(manifest, expectedHead) {
  const findings = [];
  const claimed = manifest?.source_head?.commit;
  if (typeof claimed !== "string" || !/^[0-9a-f]{40}$/i.test(claimed)) {
    findings.push(err("HEAD_BINDING_INVALID", "source_head.commit is missing or not a full 40-hex commit SHA", { manifest_head: claimed ?? null }));
    return findings;
  }
  if (claimed.toLowerCase() !== expectedHead.toLowerCase()) {
    findings.push(
      err(
        "STALE_HEAD",
        `manifest binds head ${claimed} but the expected head is ${expectedHead}; every binding in a stale manifest is untrusted`,
        { manifest_head: claimed, expected_head: expectedHead },
      ),
    );
  }
  return findings;
}

function checkCommands(manifest) {
  const findings = [];
  const commands = Array.isArray(manifest.commands) ? manifest.commands : [];
  commands.forEach((cmd, i) => {
    const label = cmd?.name ?? `commands[${i}]`;
    if (typeof cmd?.command !== "string" || cmd.command.trim() === "") {
      findings.push(err("COMMAND_TEXT_MISSING", `command "${label}" records no verbatim command string`));
    }
    if (!Number.isInteger(cmd?.exit_code) || cmd.exit_code < 0) {
      findings.push(err("EXIT_CODE_INVALID", `command "${label}" has no integer exit_code binding`, { recorded: cmd?.exit_code ?? null }));
      return;
    }
    const claimedPass = cmd?.status === "pass";
    if (claimedPass !== (cmd.exit_code === 0)) {
      findings.push(
        err(
          "EXIT_STATUS_CONTRADICTION",
          `command "${label}" claims status "${cmd.status}" with exit_code ${cmd.exit_code}; pass must correspond to exit 0`,
          { status: cmd.status ?? null, exit_code: cmd.exit_code },
        ),
      );
    }
    const captured =
      [cmd.stdout, cmd.stderr, cmd.output].some((v) => typeof v === "string") ||
      (typeof cmd.stdout_summary === "string" && cmd.stdout_summary.length > 0) ||
      (Array.isArray(cmd.stdout_at_preliminary_run) && cmd.stdout_at_preliminary_run.length > 0);
    if (claimedPass && !captured) {
      findings.push(
        err(
          "PASS_WITHOUT_OUTPUT_EVIDENCE",
          `command "${label}" claims pass with no captured stdout/stderr; prose and CI labels cannot stand in for artifacts`,
        ),
      );
    }
  });
  return findings;
}

function checkEnvironment(manifest) {
  const findings = [];
  const env = manifest.environment;
  if (env === null || typeof env !== "object") {
    findings.push(err("ENVIRONMENT_BLOCK_INVALID", "environment block is missing or not an object"));
    return findings;
  }
  if (env.verified !== true) {
    findings.push(err("ENVIRONMENT_UNVERIFIED", "environment.verified is not true; toolchain facts are unproven", { verified: env.verified ?? null }));
  }
  const outputs = Array.isArray(env.verification_outputs) ? env.verification_outputs : [];
  if (outputs.length === 0) {
    findings.push(err("ENVIRONMENT_UNVERIFIED", "environment.verification_outputs records no verbatim command outputs; assertions without captured outputs are prose"));
  } else if (
    outputs.some((o) => !o || typeof o.command !== "string" || typeof o.output !== "string")
  ) {
    findings.push(err("ENVIRONMENT_EVIDENCE_MALFORMED", "environment.verification_outputs entries must each bind { command, output } strings"));
  }
  return findings;
}

function checkArtifacts(manifest, rootDir) {
  const findings = [];
  const artifacts = Array.isArray(manifest.artifacts) ? manifest.artifacts : [];
  artifacts.forEach((art, i) => {
    const path = art?.path;
    if (typeof path !== "string" || path.trim() === "") {
      findings.push(err("ARTIFACT_PATH_MISSING", `artifacts[${i}] records no path`));
      return;
    }
    const abs = isAbsolute(path) ? path : resolve(rootDir, path);
    if (!existsSync(abs)) {
      findings.push(
        err("MISSING_ARTIFACT", `artifact "${path}" does not exist on disk at validation time`, { path }),
      );
      return;
    }
    const actualBytes = statSync(abs).size;
    if (Number.isInteger(art.bytes) && art.bytes >= 0 && art.bytes !== actualBytes) {
      findings.push(
        err("SIZE_MISMATCH", `artifact "${path}" binds ${art.bytes} bytes but disk holds ${actualBytes}`, { bound_bytes: art.bytes, actual_bytes: actualBytes }),
      );
    }
    if (typeof art.sha256 === "string" && art.sha256.length > 0) {
      const actual = sha256File(abs);
      if (actual !== art.sha256.toLowerCase()) {
        findings.push(
          err(
            "HASH_MISMATCH",
            `artifact "${path}" binds sha256 ${art.sha256} but the file on disk hashes to ${actual}`,
            { path, bound_sha256: art.sha256, actual_sha256: actual },
          ),
        );
      }
    } else if (typeof art.note !== "string" || art.note.trim() === "") {
      findings.push(
        err(
          "DIGESTLESS_WITHOUT_NOTE",
          `artifact "${path}" binds no sha256 digest and carries no note explaining the omission; an undocumented digestless artifact proves nothing`,
          { path },
        ),
      );
    }
  });
  return findings;
}

function checkPlatforms(manifest) {
  const findings = [];
  const platforms = Array.isArray(manifest.platform_coverage) ? manifest.platform_coverage : [];
  if (platforms.length === 0) {
    findings.push(gap("PLATFORM_COVERAGE_EMPTY", "platform_coverage lists no platforms; no distribution target is even claimed"));
    return findings;
  }
  platforms.forEach((p, i) => {
    const name = p?.platform ?? `platform_coverage[${i}]`;
    if (p?.status !== "proven") {
      findings.push(gap("PLATFORM_NOT_PROVEN", `platform "${name}" is not proven: no release build/installer/runtime verification artifacts exist`, { platform: name, status: p?.status ?? null }));
      return;
    }
    const proofs = [p.build_artifact, p.installer_artifact, p.runtime_verification];
    if (proofs.some((v) => v === null || v === undefined || v === "")) {
      findings.push(err("PLATFORM_CONTRADICTION", `platform "${name}" claims proven with null build/installer/runtime evidence bindings`, { platform: name }));
    }
  });
  return findings;
}

function checkRollback(manifest) {
  const findings = [];
  const rb = manifest.rollback;
  if (rb === null || typeof rb !== "object") {
    findings.push(gap("ROLLBACK_MISSING", "rollback block is missing entirely; an unreleasable-without-rollback plan has none"));
    return findings;
  }
  if (rb.status !== "proven") {
    findings.push(gap("ROLLBACK_MISSING", `rollback.status is "${rb.status ?? "unset"}"; rollback procedure and rehearsal remain unproven`, { status: rb.status ?? null }));
    return findings;
  }
  if (rb.procedure_documented !== true || rb.procedure_artifact === null || rb.rehearsal_evidence === null) {
    findings.push(err("ROLLBACK_CONTRADICTION", "rollback claims proven while procedure_documented/procedure_artifact/rehearsal_evidence are unbound"));
  }
  return findings;
}

function checkOwnerActions(manifest) {
  const findings = [];
  const actions = Array.isArray(manifest.owner_actions) ? manifest.owner_actions : [];
  if (actions.length === 0) {
    findings.push(gap("OWNER_ACTION_UNPROVEN", "owner_actions lists nothing; required owner-side release actions are all unproven"));
    return findings;
  }
  actions.forEach((a, i) => {
    const action = a?.action ?? `owner_actions[${i}]`;
    if (a?.performed === true && a?.authorized !== true) {
      findings.push(err("PERFORMED_WITHOUT_AUTHORIZATION", `owner action "${action}" is marked performed without authorization`, { action }));
    }
    if (a?.performed === true && (a.evidence === null || a.evidence === undefined || a.evidence === "")) {
      findings.push(err("OWNER_ACTION_WITHOUT_EVIDENCE", `owner action "${action}" is marked performed with no evidence artifact bound`, { action }));
    }
    if (!a?.performed) {
      findings.push(gap("OWNER_ACTION_UNPROVEN", `owner action "${action}" has not been performed; its proof does not exist`, { action }));
    }
    if (typeof action === "string" && !OWNER_ACTION_KINDS.has(action) && a?.performed === true) {
      findings.push(err("UNKNOWN_OWNER_ACTION", `owner action "${action}" is outside the known release-action vocabulary`, { action }));
    }
  });
  return findings;
}

export function validateManifest(manifest, options) {
  const errors = [];
  const gaps = [];
  const push = (findings) => {
    for (const f of findings) (f.kind === "evidence_gap" ? gaps : errors).push(f);
  };

  if (manifest === null || typeof manifest !== "object" || Array.isArray(manifest)) {
    errors.push(err("MANIFEST_UNREADABLE", "manifest is not a JSON object"));
    return report(null, options, errors, gaps);
  }
  for (const key of REQUIRED_KEYS) {
    if (!(key in manifest)) {
      errors.push(err("SCHEMA_KEY_MISSING", `required manifest key "${key}" is absent`));
    }
  }
  push(checkHead(manifest, options.expectedHead));
  push(checkCommands(manifest));
  push(checkEnvironment(manifest));
  push(checkArtifacts(manifest, options.rootDir));
  push(checkPlatforms(manifest));
  push(checkRollback(manifest));
  push(checkOwnerActions(manifest));

  const ready = errors.length === 0 && gaps.length === 0;
  const verdict = manifest.verdict;
  if (verdict !== null && typeof verdict === "object") {
    if (verdict.release_ready === true && !ready) {
      errors.push(err("VERDICT_CONTRADICTION", "verdict.release_ready is true while validation found unresolved integrity errors or evidence gaps", { integrity_errors: errors.length, evidence_gaps: gaps.length }));
    }
    if (verdict.release_ready === false && ready) {
      errors.push(err("VERDICT_CONTRADICTION", "verdict.release_ready is false while every proof binding validated; manifest understates its own evidence"));
    }
  }
  return report(manifest, options, errors, gaps);
}

function report(manifest, options, errors, gaps) {
  const ready = errors.length === 0 && gaps.length === 0;
  return {
    validator: "validate-release-proof.mjs",
    schema_version: manifest?.schema_version ?? null,
    task: manifest?.task ?? null,
    manifest_path: options.manifestPath,
    expected_head: options.expectedHead,
    manifest_head: manifest?.source_head?.commit ?? null,
    release_ready: ready,
    state: ready ? "RELEASE_READY" : "NOT_PROVEN",
    integrity_errors: errors,
    evidence_gaps: gaps,
    summary: {
      integrity_error_count: errors.length,
      evidence_gap_count: gaps.length,
      commands_checked: Array.isArray(manifest?.commands) ? manifest.commands.length : 0,
      artifacts_checked: Array.isArray(manifest?.artifacts) ? manifest.artifacts.length : 0,
      platforms_checked: Array.isArray(manifest?.platform_coverage) ? manifest.platform_coverage.length : 0,
      owner_actions_checked: Array.isArray(manifest?.owner_actions) ? manifest.owner_actions.length : 0,
    },
  };
}

function parseArgs(args) {
  const parsed = { json: false, manifest: null, expectedHead: null };
  for (let i = 0; i < args.length; i++) {
    const a = args[i];
    if (a === "--json") parsed.json = true;
    else if (a === "--manifest") parsed.manifest = args[++i] ?? null;
    else if (a === "--expected-head") parsed.expectedHead = args[++i] ?? null;
    else {
      stderr.write(`unknown argument: ${a}\n`);
      return null;
    }
  }
  return parsed;
}

function printHuman(r) {
  const lines = [];
  lines.push(`release-proof validation: ${r.manifest_path}`);
  lines.push(`head binding: manifest=${r.manifest_head} expected=${r.expected_head}`);
  for (const e of r.integrity_errors) lines.push(`INTEGRITY ERROR [${e.code}] ${e.message}`);
  for (const g of r.evidence_gaps) lines.push(`EVIDENCE GAP [${g.code}] ${g.message}`);
  lines.push(`verdict: ${r.state} (${r.summary.integrity_error_count} integrity errors, ${r.summary.evidence_gap_count} evidence gaps)`);
  stdout.write(lines.join("\n") + "\n");
}

export function main(args) {
  const parsed = parseArgs(args);
  if (!parsed) return 2;
  if (!parsed.manifest) {
    stderr.write("usage: validate-release-proof.mjs --manifest <path> --expected-head <sha> [--json]\n");
    return 2;
  }
  if (!parsed.expectedHead || !/^[0-9a-f]{40}$/i.test(parsed.expectedHead)) {
    stderr.write("--expected-head requires a full 40-hex commit SHA\n");
    return 2;
  }
  const manifestPath = resolve(parsed.manifest);
  let manifest;
  try {
    manifest = JSON.parse(readFileSync(manifestPath, "utf8"));
  } catch (e) {
    const r = report(null, { manifestPath: parsed.manifest, expectedHead: parsed.expectedHead }, [
      err("MANIFEST_UNREADABLE", `manifest could not be read/parsed: ${e.message}`),
    ], []);
    if (parsed.json) stdout.write(JSON.stringify(r, null, 2) + "\n");
    else printHuman(r);
    return 1;
  }
  const r = validateManifest(manifest, {
    expectedHead: parsed.expectedHead.toLowerCase(),
    rootDir: process.cwd(),
    manifestPath: parsed.manifest,
  });
  if (parsed.json) stdout.write(JSON.stringify(r, null, 2) + "\n");
  else printHuman(r);
  return r.release_ready ? 0 : 1;
}

const invokedDirectly =
  process.argv[1] && import.meta.url === pathToFileURL(resolve(process.argv[1])).href;
if (invokedDirectly) {
  exit(main(argv.slice(2)));
}
