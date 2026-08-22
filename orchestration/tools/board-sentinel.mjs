#!/usr/bin/env node
import fs from "node:fs";
import path from "node:path";
import { spawnSync } from "node:child_process";
import { fileURLToPath } from "node:url";

const SCHEMA = "board-sentinel/1";
const KNOWN_STATUS_STATES = new Set([
  "CLAIMED",
  "IMPLEMENTED",
  "REVIEW_REQUESTED",
  "BLOCKED",
  "INTEGRATED",
]);
const KNOWN_SPEC_STATES = new Set([
  "DRAFT",
  "AUTO_RELEASE",
  "READY",
  "SUPERSEDED",
]);
const LIVE_CLAIM_STATES = new Set(["CLAIMED", "IMPLEMENTED", "REVIEW_REQUESTED"]);

function parseArgs(argv) {
  const opts = { repo: ".", minReady: 8, json: false };
  for (let i = 0; i < argv.length; i++) {
    const a = argv[i];
    if (a === "--repo") {
      opts.repo = argv[++i];
      if (opts.repo === undefined) return { error: "--repo requires a value" };
    } else if (a === "--min-ready") {
      const v = Number(argv[++i]);
      if (!Number.isInteger(v) || v < 0) {
        return { error: "--min-ready requires a non-negative integer" };
      }
      opts.minReady = v;
    } else if (a === "--json") {
      opts.json = true;
    } else {
      return { error: `unknown argument: ${a}` };
    }
  }
  return { opts };
}

function readIfExists(p) {
  try {
    return fs.readFileSync(p, "utf8");
  } catch {
    return null;
  }
}

function parseFrontmatter(text) {
  if (text === null) return null;
  const m = text.match(/^---\r?\n([\s\S]*?)\r?\n---/);
  if (!m) return null;
  const fields = {};
  const lines = m[1].split(/\r?\n/);
  for (let i = 0; i < lines.length; i++) {
    const kv = lines[i].match(/^([A-Za-z0-9_-]+):\s*(.*)$/);
    if (!kv) continue;
    const key = kv[1];
    let value = kv[2].trim();
    if (value === "") {
      const items = [];
      let j = i + 1;
      while (j < lines.length) {
        const item = lines[j].match(/^\s+-\s+(.*)$/);
        if (!item) break;
        items.push(item[1].trim());
        j++;
      }
      fields[key] = items.length ? items : "";
      i = j - 1;
    } else if (value.startsWith("[") && value.endsWith("]")) {
      const inner = value.slice(1, -1).trim();
      fields[key] = inner
        ? inner.split(",").map((s) => s.trim().replace(/^['"]|['"]$/g, ""))
        : [];
    } else {
      fields[key] = value;
    }
  }
  return fields;
}

function extractSections(text) {
  const sections = [];
  let current = null;
  for (const line of text.split(/\r?\n/)) {
    const h2 = line.match(/^##\s+(.*)$/);
    if (h2) {
      current = { title: h2[1].trim(), lines: [] };
      sections.push(current);
    } else if (current) {
      current.lines.push(line);
    }
  }
  return sections;
}

function tableTaskIds(section) {
  const ids = [];
  for (const line of section.lines) {
    if (!line.trim().startsWith("|")) continue;
    const cells = line
      .split("|")
      .slice(1, -1)
      .map((c) => c.trim());
    if (cells.length === 0) continue;
    if (cells.every((c) => /^[-:\s]*$/.test(c))) continue;
    let found = null;
    for (const cell of cells) {
      const m = cell.match(/TASK-\d{4}/);
      if (m) {
        found = m[0];
        break;
      }
    }
    if (found) ids.push(found);
  }
  return ids;
}

function readRunStatus(repoPath, errors) {
  const file = path.join(repoPath, "orchestration", "RUN_STATUS.md");
  const text = readIfExists(file);
  if (text === null) {
    errors.push({
      type: "missing_run_status",
      detail: "orchestration/RUN_STATUS.md not found",
    });
    return { ready: [], hold: [], draft: [] };
  }
  const ready = [];
  const hold = [];
  const draft = [];
  for (const section of extractSections(text)) {
    const t = section.title;
    if (/effective\s+READY/i.test(t)) {
      ready.push(...tableTaskIds(section));
    } else if (/^HOLD\b/i.test(t)) {
      hold.push(...tableTaskIds(section));
    } else if (/sequenced\s+successors/i.test(t)) {
      draft.push(...tableTaskIds(section));
    }
  }
  return { ready, hold, draft };
}

function scanTaskFolders(repoPath, errors, annotations) {
  const tasksRoot = path.join(repoPath, "orchestration", "tasks");
  const tasks = new Map();
  let dirs = [];
  try {
    dirs = fs
      .readdirSync(tasksRoot, { withFileTypes: true })
      .filter((d) => d.isDirectory() && /^TASK-\d{4}/.test(d.name))
      .map((d) => d.name)
      .sort();
  } catch {
    errors.push({
      type: "missing_task_root",
      detail: "orchestration/tasks not found",
    });
    return tasks;
  }
  const seenIds = new Map();
  for (const dir of dirs) {
    const id = dir.match(/^TASK-\d{4}/)[0];
    if (seenIds.has(id)) {
      errors.push({
        type: "duplicate_task_folder",
        detail: `${id} has folders ${seenIds.get(id)} and ${dir}`,
      });
      continue;
    }
    seenIds.set(id, dir);
    const folder = path.join(tasksRoot, dir);
    const specFields = parseFrontmatter(readIfExists(path.join(folder, "SPEC.md")));
    let specState = null;
    if (specFields) {
      const rawState =
        typeof specFields.state === "string" ? specFields.state : "";
      const stateToken = rawState.split(/\s+/)[0] ?? "";
      if (!KNOWN_SPEC_STATES.has(stateToken)) {
        errors.push({
          type: "malformed_spec_state",
          detail: `${dir}: unknown SPEC state ${JSON.stringify(rawState)}`,
        });
        specState = null;
      } else {
        specState = stateToken;
        const annotation = rawState.slice(stateToken.length).trim();
        if (annotation && annotations) {
          annotations[id] = annotation;
        }
      }
    }
    const ownedPaths = Array.isArray(specFields?.owned_paths)
      ? specFields.owned_paths.filter((p) => typeof p === "string" && p)
      : [];
    let status = null;
    const statusText = readIfExists(path.join(folder, "STATUS.md"));
    if (statusText !== null) {
      const stateMatch = statusText.match(/^(?:-\s+)?state:\s*(\S+)/m);
      const state = stateMatch ? stateMatch[1] : "";
      if (!KNOWN_STATUS_STATES.has(state)) {
        errors.push({
          type: "malformed_status_state",
          detail: `${dir}: unknown STATUS state ${JSON.stringify(state)}`,
        });
      } else {
        status = { state };
        if (LIVE_CLAIM_STATES.has(state)) {
          const coordinator = statusText.match(
            /^(?:-\s+)?coordinator:\s*(\S.*)$/m,
          );
          if (!coordinator) {
            errors.push({
              type: "malformed_status_missing_coordinator",
              detail: `${dir}: ${state} claim has no coordinator line`,
            });
          } else {
            status.coordinator = coordinator[1].trim();
          }
          const branch = statusText.match(/^(?:-\s+)?worker_branch:\s*(\S+)/m);
          if (branch) status.worker_branch = branch[1];
          const started = statusText.match(/^(?:-\s+)?started_at:\s*(\S.*)$/m);
          if (started) status.started_at = started[1].trim();
        }
      }
    }
    let reviewVerdict = null;
    const reviewText = readIfExists(path.join(folder, "REVIEW.md"));
    if (reviewText !== null) {
      const verdicts = [...reviewText.matchAll(/^verdict:\s*(\S+)/gm)];
      if (verdicts.length) reviewVerdict = verdicts[verdicts.length - 1][1];
    }
    tasks.set(id, {
      id,
      folder: dir,
      specState,
      ownedPaths,
      status,
      reviewVerdict,
    });
  }
  return tasks;
}

function readIntegrationLog(repoPath) {
  const ids = new Set();
  for (const rel of ["INTEGRATION_LOG.md", path.join("orchestration", "INTEGRATION_LOG.md")]) {
    const text = readIfExists(path.join(repoPath, rel));
    if (text === null) continue;
    for (const m of text.matchAll(/TASK-\d{4}/g)) ids.add(m[0]);
  }
  return ids;
}

function globCovers(pattern, candidate) {
  const pSegs = pattern.split("/");
  const cSegs = candidate.split("/");
  let i = 0;
  for (; i < pSegs.length; i++) {
    const p = pSegs[i];
    if (p === "**") return true;
    if (i >= cSegs.length) return false;
    if (p.includes("*")) {
      continue;
    }
    if (p !== cSegs[i]) return false;
  }
  return i === cSegs.length;
}

function patternsCollide(a, b) {
  return a === b || globCovers(a, b) || globCovers(b, a);
}

function findCollisions(liveTasks, errors) {
  const collisions = [];
  const entries = [];
  for (const task of liveTasks) {
    for (const pattern of task.ownedPaths) {
      entries.push({ id: task.id, pattern });
    }
  }
  for (let i = 0; i < entries.length; i++) {
    for (let j = i + 1; j < entries.length; j++) {
      const a = entries[i];
      const b = entries[j];
      if (a.id === b.id) continue;
      if (patternsCollide(a.pattern, b.pattern)) {
        collisions.push({ a: a.id, b: b.id, pattern_a: a.pattern, pattern_b: b.pattern });
      }
    }
  }
  if (collisions.length) {
    errors.push({
      type: "owned_path_collision",
      detail: collisions
        .map((c) => `${c.a}(${c.pattern_a}) vs ${c.b}(${c.pattern_b})`)
        .join("; "),
    });
  }
  return collisions;
}

function readCoordinatorBranches(repoPath) {
  try {
    const res = spawnSync(
      "git",
      [
        "for-each-ref",
        "--format=%(refname)%09%(objectname:short)%09%(committerdate:iso-strict)",
        "refs/remotes",
      ],
      { cwd: repoPath, encoding: "utf8", timeout: 15000 },
    );
    if (res.status !== 0 || !res.stdout) return [];
    const refs = [];
    for (const line of res.stdout.split(/\r?\n/)) {
      if (!line) continue;
      const [refname, commit, date] = line.split("\t");
      const m = refname.match(/(?:^|\/)(TASK-\d{4})(?:$|[^0-9])/);
      if (!m) continue;
      refs.push({
        ref: refname.replace(/^refs\/remotes\//, ""),
        task_id: m[1],
        commit,
        committed_at: date,
      });
    }
    refs.sort((x, y) => (x.ref < y.ref ? -1 : x.ref > y.ref ? 1 : 0));
    return refs;
  } catch {
    return [];
  }
}

export function runSentinel(repoPath, minReady) {
  const errors = [];
  const annotations = {};
  const board = readRunStatus(repoPath, errors);

  const dupCheck = (list, label) => {
    const seen = new Set();
    for (const id of list) {
      if (seen.has(id)) {
        errors.push({
          type: "duplicate_task_id",
          detail: `${id} listed twice in ${label}`,
        });
      }
      seen.add(id);
    }
  };
  dupCheck(board.ready, "Effective READY table");
  dupCheck(board.hold, "HOLD table");
  dupCheck(board.draft, "successors table");

  const readySet = new Set(board.ready);
  const holdSet = new Set(board.hold);
  for (const id of readySet) {
    if (holdSet.has(id)) {
      errors.push({
        type: "contradictory_state",
        detail: `${id} appears in both Effective READY and HOLD`,
      });
    }
  }

  const tasks = scanTaskFolders(repoPath, errors, annotations);
  const integratedLogIds = readIntegrationLog(repoPath);

  const effectiveReady = [];
  const claimed = [];
  const reviewRequested = [];
  const revise = [];
  const hold = [];
  const draft = [];
  const staleClaims = [];
  const integrated = new Set();
  const supersededOrIntegrated = (task) =>
    task.status?.state === "INTEGRATED" ||
    task.reviewVerdict === "SUPERSEDED" ||
    task.specState === "SUPERSEDED" ||
    (task.reviewVerdict === "ACCEPTED" && integratedLogIds.has(task.id));

  for (const id of board.ready) {
    const task = tasks.get(id);
    if (!task) {
      effectiveReady.push(id);
      continue;
    }
    if (supersededOrIntegrated(task)) {
      integrated.add(id);
      continue;
    }
    if (task.reviewVerdict === "REVISE") {
      revise.push({ id });
      continue;
    }
    if (task.status && LIVE_CLAIM_STATES.has(task.status.state)) {
      if (task.status.state === "REVIEW_REQUESTED") {
        reviewRequested.push({ id, coordinator: task.status.coordinator ?? null });
      } else {
        claimed.push({ id, ...task.status });
      }
      continue;
    }
    effectiveReady.push(id);
  }

  for (const id of board.hold) hold.push(id);
  for (const id of board.draft) draft.push(id);

  for (const task of tasks.values()) {
    const live = task.status && LIVE_CLAIM_STATES.has(task.status.state);
    if (!live) {
      if (task.status?.state === "INTEGRATED" || integratedLogIds.has(task.id)) {
        integrated.add(task.id);
      }
      continue;
    }
    const reasons = [];
    if (task.reviewVerdict === "SUPERSEDED") {
      reasons.push("latest REVIEW verdict SUPERSEDED");
    }
    if (task.specState === "SUPERSEDED") {
      reasons.push("SPEC frontmatter state SUPERSEDED");
    }
    if (integratedLogIds.has(task.id)) {
      reasons.push("implementation recorded in INTEGRATION_LOG.md");
    }
    if (reasons.length) {
      staleClaims.push({ id: task.id, state: task.status.state, reasons });
    }
  }

  const effectiveReadySet = new Set(effectiveReady);
  const liveTasks = [];
  for (const id of effectiveReadySet) {
    const task = tasks.get(id);
    if (task) liveTasks.push(task);
  }
  for (const c of [...claimed, ...reviewRequested, ...revise]) {
    const task = tasks.get(c.id);
    if (task) liveTasks.push(task);
  }
  const collisions = findCollisions(liveTasks, errors);
  const coordinatorBranches = readCoordinatorBranches(repoPath);

  const queueFloor = {
    min_ready: minReady,
    effective_ready_count: effectiveReady.length,
    satisfied: effectiveReady.length >= minReady,
  };
  if (!queueFloor.satisfied) {
    errors.push({
      type: "queue_floor_violation",
      detail: `effective READY ${effectiveReady.length} < min-ready ${minReady}`,
    });
  }

  const healthy = errors.length === 0;
  return {
    schema: SCHEMA,
    repo: repoPath,
    min_ready: minReady,
    counts: {
      effective_ready: effectiveReady.length,
      claimed: claimed.length,
      review_requested: reviewRequested.length,
      revise: revise.length,
      hold: hold.length,
      draft: draft.length,
      stale_claims: staleClaims.length,
      collisions: collisions.length,
      integrated: integrated.size,
      coordinator_branches: coordinatorBranches.length,
    },
    effective_ready: effectiveReady,
    claimed,
    review_requested: reviewRequested,
    revise,
    hold,
    draft,
    stale_claims: staleClaims,
    collisions,
    coordinator_branches: coordinatorBranches,
    spec_state_annotations: annotations,
    integrated: [...integrated].sort(),
    queue_floor: queueFloor,
    errors,
    healthy,
    exit_code: healthy ? 0 : 1,
  };
}

function main() {
  const parsed = parseArgs(process.argv.slice(2));
  if (parsed.error) {
    console.error(`board-sentinel: ${parsed.error}`);
    console.error(
      "usage: node board-sentinel.mjs --repo <dir> --min-ready <n> [--json]",
    );
    process.exit(2);
  }
  const { repo, minReady, json } = parsed.opts;
  if (!fs.existsSync(repo) || !fs.statSync(repo).isDirectory()) {
    console.error(`board-sentinel: repo path not found: ${repo}`);
    process.exit(2);
  }
  const report = runSentinel(path.resolve(repo), minReady);
  if (json) {
    console.log(JSON.stringify(report, null, 2));
  } else {
    console.log(
      `board-sentinel: effective READY ${report.counts.effective_ready}` +
        ` (floor ${minReady}), claimed ${report.counts.claimed},` +
        ` review_requested ${report.counts.review_requested},` +
        ` revise ${report.counts.revise}, hold ${report.counts.hold},` +
        ` draft ${report.counts.draft}, stale ${report.counts.stale_claims},` +
        ` collisions ${report.counts.collisions}`,
    );
    for (const e of report.errors) console.log(`error: ${e.type}: ${e.detail}`);
    console.log(report.healthy ? "healthy" : "unhealthy");
  }
  process.exit(report.exit_code);
}

function isMainModule() {
  try {
    return fileURLToPath(import.meta.url) === path.resolve(process.argv[1]);
  } catch {
    return false;
  }
}

if (isMainModule()) {
  main();
}
