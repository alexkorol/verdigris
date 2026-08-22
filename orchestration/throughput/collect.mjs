#!/usr/bin/env node
// TASK-0128 accepted-throughput normalization collector.
// Dependency-free Node.js 22. Deterministic: outputs depend only on committed
// repository evidence (task folders, RUN_STATUS.md, generated D-128 summary,
// Git history). Unknown values are JSON null, never 0 or inferred facts.
import { spawnSync } from 'node:child_process';
import fsp from 'node:fs/promises';
import path from 'node:path';
import { fileURLToPath } from 'node:url';

export const OBSERVATIONS_SCHEMA = 'throughput-observations-v1';
export const SNAPSHOT_SCHEMA = 'runway-snapshot-v1';

export class CollectError extends Error {
  constructor(message, code) { super(message); this.name = 'CollectError'; this.exitCode = code; }
}
const EXIT_OK = 0;
const EXIT_STALE = 2;
const EXIT_VALIDATION = 3;
const EXIT_ESCAPE = 4;
const EXIT_MALFORMED = 5;

// ---------- canonical JSON (sorted keys, stable arrays) ----------

export function canonicalJson(value) { return stableStringify(value) + '\n'; }
function stableStringify(value) {
  if (value === null || typeof value !== 'object') return JSON.stringify(value);
  if (Array.isArray(value)) return '[' + value.map(stableStringify).join(',') + ']';
  const keys = Object.keys(value).sort();
  return '{' + keys.map((k) => JSON.stringify(k) + ':' + stableStringify(value[k])).join(',') + '}';
}
const round3 = (x) => Math.round(x * 1000) / 1000;

// ---------- path safety ----------

export function resolveInside(repoRoot, target) {
  const root = path.resolve(repoRoot);
  const abs = path.resolve(root, target);
  const rel = path.relative(root, abs);
  if (rel === '' || rel.startsWith('..') || path.isAbsolute(rel)) {
    throw new CollectError(`path escape rejected: "${target}" is outside repo root`, EXIT_ESCAPE);
  }
  return abs;
}

// ---------- markdown parsing ----------

export function parseFrontmatter(text) {
  const lines = text.split(/\r?\n/);
  if (lines[0] !== '---') return { data: null, closed: false, hasFrontmatter: false };
  let end = -1;
  for (let i = 1; i < lines.length; i++) if (lines[i] === '---') { end = i; break; }
  if (end === -1) return { data: null, closed: false, hasFrontmatter: true };
  const data = {};
  for (const line of lines.slice(1, end)) {
    const m = /^([A-Za-z0-9_-]+):\s*(.*)$/.exec(line);
    if (m) data[m[1]] = m[2].trim();
  }
  return { data, closed: true, hasFrontmatter: true };
}

// "- key: value" bullets; continuation lines join until blank/next bullet.
export function parseStatusBullets(text) {
  const lines = text.split(/\r?\n/);
  const bullets = new Map();
  for (let i = 0; i < lines.length; i++) {
    const m = /^-\s+([^:]+):\s*(.*)$/.exec(lines[i]);
    if (!m) continue;
    const key = m[1].replace(/\s*\([^)]*\)\s*$/, '').trim().toLowerCase();
    if (!bullets.has(key)) {
      let value = m[2].trim();
      let j = i + 1;
      while (j < lines.length && lines[j] !== '' && !/^-\s+/.test(lines[j])) { value += ' ' + lines[j].trim(); j++; }
      bullets.set(key, value);
      i = j - 1;
    }
  }
  return bullets;
}

function cleanValue(v) { return v.replace(/`/g, '').trim(); }
function firstSegment(v) { return cleanValue(v.split(';')[0]); }
function inlineToken(v, label) {
  const m = new RegExp(label + ':\\s*`?([A-Za-z0-9._/-]+)').exec(v);
  return m ? m[1] : null;
}

export function extractUnit(bullets) {
  const pick = (name) => (bullets.has(name) ? firstSegment(bullets.get(name)) || null : null);
  const modelId = pick('model id');
  let variant = pick('variant');
  if (!variant && bullets.has('model id')) variant = inlineToken(bullets.get('model id'), 'variant');
  const modelAlias = pick('model alias') || pick('agent alias');
  const machineRaw = pick('machine');
  // alias disagreement: alias shaped like a model id but differing from model id
  const aliasConfirmed = !(modelAlias && modelId && modelAlias.includes('/') && modelAlias !== modelId);
  return {
    endpoint: pick('endpoint'),
    provider: pick('provider'),
    model_id: modelId,
    model_alias: modelAlias,
    variant,
    harness: pick('harness'),
    harness_version: pick('harness version'),
    configuration_provenance: pick('configuration provenance'),
    machine: machineRaw ? cleanValue(machineRaw.split('(')[0]) : null,
    alias_confirmed: aliasConfirmed,
  };
}

const VERDICT_RE = /(?:^#{1,6} .*[-\u2014]\s*(REVISE|ACCEPTED|BLOCKED|SUPERSEDED)\s*$)|(?:^Verdict:\s*\*\*(REVISE|ACCEPTED|BLOCKED|SUPERSEDED)\*\*)/gm;

export function parseVerdicts(text) {
  const verdicts = [];
  let m;
  while ((m = VERDICT_RE.exec(text)) !== null) verdicts.push(m[1] || m[2]);
  const final = verdicts.length ? verdicts[verdicts.length - 1] : null;
  const revise_cycles = final === 'ACCEPTED'
    ? verdicts.slice(0, -1).filter((v) => v === 'REVISE').length
    : null;
  return { verdicts, final, revise_cycles };
}

const ISO_EXACT = /^\d{4}-\d{2}-\d{2}[T ]\d{2}:\d{2}:\d{2}(?:\.\d+)?(?:Z|[+-]\d{2}:?\d{2})$/;

// ---------- observation assembly ----------

const UNIT_DIMENSIONS = ['endpoint', 'provider', 'model_id', 'harness', 'harness_version', 'configuration_provenance', 'machine'];
export const AGGREGATION_KEY_FIELDS = [...UNIT_DIMENSIONS, 'task_family', 'packet_type'];

function unitComplete(u) { return UNIT_DIMENSIONS.every((k) => u[k] !== null) && u.alias_confirmed === true; }

function tsField(ms, source) {
  if (ms == null) return { iso: null, source: null, confidence: 'missing' };
  return { iso: new Date(ms).toISOString(), source, confidence: source === 'git_authored' ? 'high' : 'medium' };
}

export async function buildObservation({ folderName, specText, statusText, reviewText, git }) {
  const fm = parseFrontmatter(specText);
  if (fm.hasFrontmatter && !fm.closed) {
    throw new CollectError(`malformed task evidence in orchestration/tasks/${folderName}/SPEC.md`, EXIT_MALFORMED);
  }
  if (!fm.hasFrontmatter || !fm.data || !fm.data.task) return null;
  const relStatus = `orchestration/tasks/${folderName}/STATUS.md`;
  const relReview = `orchestration/tasks/${folderName}/REVIEW.md`;
  const bullets = statusText ? parseStatusBullets(statusText) : new Map();
  const unit = extractUnit(bullets);
  const verdicts = reviewText ? parseVerdicts(reviewText) : { verdicts: [], final: null, revise_cycles: null };

  let claimCommit = null;
  try {
    const commits = git.commitsFor(relStatus).filter((c) => /claim/i.test(c.subject));
    claimCommit = commits.length ? commits[commits.length - 1] : null;
  } catch { claimCommit = null; }
  let acceptCommit = null;
  if (verdicts.final === 'ACCEPTED') {
    try {
      const commits = git.commitsFor(relReview);
      acceptCommit = commits.length ? commits[commits.length - 1] : null;
    } catch { acceptCommit = null; }
  }

  let claimStartMs = null, claimSource = null;
  if (claimCommit?.authoredAtMs != null) { claimStartMs = claimCommit.authoredAtMs; claimSource = 'git_authored'; }
  else {
    const started = bullets.get('started-at');
    if (started && ISO_EXACT.test(started.trim())) { claimStartMs = Date.parse(started.trim()); claimSource = 'task_explicit'; }
  }
  let acceptanceEndMs = null, acceptanceSource = null;
  if (acceptCommit?.authoredAtMs != null) { acceptanceEndMs = acceptCommit.authoredAtMs; acceptanceSource = 'git_authored'; }

  const observation = {
    task_id: fm.data.task,
    title: fm.data.title || null,
    packet_type: fm.data.packet || null,
    job_type: fm.data.job || null,
    topology: fm.data.topology || null,
    coordinator: bullets.has('coordinator') ? cleanValue(bullets.get('coordinator').split('(')[0]) || null : null,
    worker_branch: bullets.has('worker branch') ? cleanValue(bullets.get('worker branch')) : null,
    base_sha: bullets.has('base sha') ? cleanValue(bullets.get('base sha')) : null,
    head_sha: acceptCommit ? acceptCommit.hash : null,
    claim_start: tsField(claimStartMs, claimSource),
    acceptance_end: tsField(acceptanceEndMs, acceptanceSource),
    wall_clock_duration_ms: null,
    experimental_unit: unit,
    task_family: bullets.has('task family') ? firstSegment(bullets.get('task family')) || null : null,
    review_cycles: verdicts.revise_cycles,
    first_pass: verdicts.final === 'ACCEPTED' ? verdicts.revise_cycles === 0 : null,
    false_green_state: null,
    changed_test_disclosure: null,
    accepted_result: verdicts.final === 'ACCEPTED' ? 'accepted owner-visible result recorded in REPORT.md' : null,
    final_verdict: verdicts.final,
    provenance: {
      spec: `orchestration/tasks/${folderName}/SPEC.md`,
      status: statusText ? relStatus : null,
      review: reviewText ? relReview : null,
      claim_commit: claimCommit ? `${claimCommit.hash} ${claimCommit.subject}` : null,
      acceptance_commit: acceptCommit ? `${acceptCommit.hash} ${acceptCommit.subject}` : null,
    },
  };
  validateObservation(observation);
  return observation;
}

export function validateObservation(o) {
  if (!o.task_id || !/^TASK-\d+/.test(o.task_id)) {
    throw new CollectError(`observation missing valid task id: ${o.task_id}`, EXIT_VALIDATION);
  }
  if (o.claim_start.iso && o.acceptance_end.iso) {
    const start = Date.parse(o.claim_start.iso);
    const end = Date.parse(o.acceptance_end.iso);
    if (!(end > start)) {
      throw new CollectError(
        `zero/negative duration rejected for ${o.task_id}: acceptance must strictly follow claim`,
        EXIT_VALIDATION);
    }
    o.wall_clock_duration_ms = end - start;
  }
}

// internal numeric mirrors used while aggregating; stripped before serialization
const attachMs = (o) => {
  o.claim_start.ms = o.claim_start.iso ? Date.parse(o.claim_start.iso) : null;
  o.acceptance_end.ms = o.acceptance_end.iso ? Date.parse(o.acceptance_end.iso) : null;
};
const stripMs = (o) => { delete o.claim_start.ms; delete o.acceptance_end.ms; };

// ---------- aggregation ----------

export function aggregateUnits(observations) {
  const groups = new Map();
  for (const o of observations) {
    if (o.final_verdict !== 'ACCEPTED') continue;
    const u = o.experimental_unit;
    const dims = {};
    for (const k of UNIT_DIMENSIONS) dims[k] = u[k];
    dims.task_family = o.task_family;
    dims.packet_type = o.packet_type;
    const complete = unitComplete(u) && dims.task_family !== null && dims.packet_type !== null;
    const key = stableStringify(dims);
    if (!groups.has(key)) groups.set(key, { dims, complete, members: [] });
    if (!groups.get(key).members.includes(o.task_id)) groups.get(key).members.push(o.task_id);
  }
  const units = [];
  for (const [, g] of [...groups.entries()].sort()) {
    const memberObs = observations.filter((o) => g.members.includes(o.task_id));
    const durations = memberObs.filter((o) => o.wall_clock_duration_ms != null).map((o) => o.wall_clock_duration_ms);
    let windowStart = Infinity, windowEnd = -Infinity;
    for (const o of memberObs) {
      if (o.claim_start.ms != null && o.claim_start.ms < windowStart) windowStart = o.claim_start.ms;
      if (o.acceptance_end.ms != null && o.acceptance_end.ms > windowEnd) windowEnd = o.acceptance_end.ms;
    }
    const totalHours = durations.reduce((a, b) => a + b, 0) / 3600000;
    units.push({
      dimensions: g.dims,
      complete_unit: g.complete,
      sample_count: g.members.length,
      duration_sample_count: durations.length,
      window: windowStart === Infinity ? null : { start: new Date(windowStart).toISOString(), end: new Date(windowEnd).toISOString() },
      total_duration_hours: durations.length ? round3(totalHours) : null,
      tasks_per_hour: durations.length && totalHours > 0 ? round3(g.members.length / totalHours) : null,
      members: [...g.members].sort(),
      coordinator: memberObs[0]?.coordinator ?? null,
      provenance: 'derived exclusively from cited per-task evidence files and git clocks',
    });
  }
  units.sort((a, b) => stableStringify(a.dimensions) < stableStringify(b.dimensions) ? -1 : 1);
  return { units, pooled_headline: null, note: 'no cross-unit pooled headline is allowed (D-128); incomplete units remain individual observations without a rate' };
}

// ---------- board parsing ----------

export function parseLanes(runStatusText) {
  const lanes = [];
  let inTable = false;
  for (const line of runStatusText.split(/\r?\n/)) {
    if (/^\|\s*Lane\s*\|/.test(line)) { inTable = true; continue; }
    if (!inTable) continue;
    if (!line.startsWith('|')) { inTable = false; continue; }
    const cols = line.split('|').map((c) => c.trim());
    const cells = cols.slice(1, -1);
    if (!cells.length || cells.every((c) => /^:?-{3,}:?$/.test(c))) continue;
    if (cells[0]) lanes.push({ lane: cells[0], ports: cells[1] ?? null });
  }
  return lanes;
}

export function parseEffectiveReady(runStatusText) {
  const rows = [];
  let inSection = false;
  for (const line of runStatusText.split(/\r?\n/)) {
    if (/^## Effective READY/.test(line)) { inSection = true; continue; }
    if (!inSection) continue;
    if (/^\|[\s:-]+\|$/.test(line.replace(/\|$/, '|'))) continue;
    if (!line.startsWith('|')) { if (rows.length) break; continue; }
    const cols = line.split('|').map((c) => c.trim());
    const taskMatch = /(TASK-\d+)/.exec(cols[2] || '');
    if (!taskMatch) continue;
    rows.push({
      priority: cols[1] || null,
      task_id: taskMatch[1],
      title: (cols[2] || '').replace(taskMatch[1], '').trim(),
      topology_job: cols[3] || null,
      preferred_route: cols[4] || null,
    });
  }
  return rows;
}

async function specPacketType(tasksRoot, taskId) {
  let entries;
  try { entries = await fsp.readdir(tasksRoot, { withFileTypes: true }); } catch { return null; }
  const match = entries.filter((e) => e.isDirectory() && e.name.startsWith(taskId)).map((e) => e.name).sort()[0];
  if (!match) return null;
  try {
    const text = await fsp.readFile(path.join(tasksRoot, match.name, 'SPEC.md'), 'utf8');
    const fm = parseFrontmatter(text);
    return fm.data ? (fm.data.packet || null) : null;
  } catch { return null; }
}

// ---------- runway snapshot ----------

export function buildRunwaySnapshot({ repoRevision, lanes, readyTasks, reserveSummary, units }) {
  const thresholds = { target_hours: 72, warning_below_hours: 48, critical_below_hours: 24 };
  const readyCountsByPacket = new Map();
  for (const t of readyTasks) readyCountsByPacket.set(t.packet_type, (readyCountsByPacket.get(t.packet_type) || 0) + 1);

  const laneSnapshots = [];
  for (const lane of lanes) {
    const laneUnits = units.filter((u) => u.complete_unit && u.tasks_per_hour != null && u.coordinator === lane.lane);
    let hours = null, confidence = 'UNKNOWN', missing_dimensions = null, observed_rate = null;
    if (laneUnits.length) {
      const options = laneUnits.map((u) => ({
        packet_type: u.dimensions.packet_type,
        compatible_ready_count: readyCountsByPacket.get(u.dimensions.packet_type) ?? 0,
        tasks_per_hour: u.tasks_per_hour,
        hours: round3((readyCountsByPacket.get(u.dimensions.packet_type) ?? 0) / u.tasks_per_hour),
      }));
      options.sort((a, b) => a.hours - b.hours || a.packet_type.localeCompare(b.packet_type));
      observed_rate = options;
      hours = options[0].hours;
      confidence = 'MEASURED';
    } else {
      const probe = units.find((u) => u.coordinator === lane.lane);
      missing_dimensions = probe
        ? [...UNIT_DIMENSIONS.filter((k) => probe.dimensions[k] === null),
           ...(probe.complete_unit ? [] : ['aggregation_incomplete'])]
        : ['no accepted observations for lane'];
    }
    laneSnapshots.push({ lane: lane.lane, ports: lane.ports, observed_rate, hours, confidence, missing_dimensions });
  }
  laneSnapshots.sort((a, b) => a.lane < b.lane ? -1 : 1);

  let overall_state = 'UNKNOWN';
  if (laneSnapshots.length && laneSnapshots.every((l) => l.confidence === 'MEASURED')) {
    const min = Math.min(...laneSnapshots.map((l) => l.hours));
    overall_state = min >= thresholds.target_hours ? 'ADEQUATE' : min >= thresholds.warning_below_hours ? 'WARNING' : 'CRITICAL';
  }
  const primary = laneSnapshots[0] || null;
  return {
    schema_version: SNAPSHOT_SCHEMA,
    repo_revision: repoRevision,
    thresholds,
    board: {
      effective_ready: readyTasks.length,
      effective_ready_by_priority: countBy(readyTasks, (t) => t.priority),
      auto_release_reserve: reserveSummary?.packet_states?.AUTO_RELEASE ?? null,
      reserve_schema: reserveSummary?.schema_version ?? null,
    },
    lanes: laneSnapshots,
    hours: primary ? primary.hours : null,
    confidence: primary ? primary.confidence : null,
    overall_state,
    unknown_is_non_green: true,
    note: 'hours:null means no comparable accepted sample exists in this exact experimental unit; historical fleet averages and illustrative numbers are excluded',
  };
}

function countBy(items, fn) {
  const out = {};
  for (const item of items) { const k = fn(item) ?? 'null'; out[k] = (out[k] || 0) + 1; }
  return out;
}

// ---------- default git implementation ----------

export function realGit(repoRoot) {
  return {
    revHead() {
      const r = spawnSync('git', ['-C', repoRoot, 'rev-parse', 'HEAD'], { encoding: 'utf8' });
      if (r.status !== 0) throw new CollectError(`git rev-parse HEAD failed: ${r.stderr}`, EXIT_MALFORMED);
      return r.stdout.trim();
    },
    commitsFor(relativePath) {
      const r = spawnSync('git', ['-C', repoRoot, 'log', '--format=%H%x09%aI%x09%s', '--', relativePath], { encoding: 'utf8' });
      if (r.status !== 0) throw new CollectError(`git log failed for ${relativePath}: ${r.stderr}`, EXIT_MALFORMED);
      return r.stdout.split(/\r?\n/).filter(Boolean).map((line) => {
        const [hash, iso, subject] = line.split('\t');
        return { hash, authoredAtMs: Date.parse(iso), subject };
      });
    },
  };
}

// ---------- filesystem adapter (injectable in tests) ----------

const defaultIo = {
  readdir: (p, opts) => fsp.readdir(p, opts),
  readFile: (p) => fsp.readFile(p, 'utf8'),
  stat: (p) => fsp.stat(p),
  mkdir: (p, opts) => fsp.mkdir(p, opts),
  writeFile: (p, data) => fsp.writeFile(p, data, 'utf8'),
};

// ---------- pipeline ----------

export async function runCollector({ repoRoot, outDir, check = false, io = defaultIo, git = realGit(path.resolve(repoRoot)) }) {
  const absRepo = path.resolve(repoRoot);
  const absOut = resolveInside(absRepo, outDir);
  const repoRevision = git.revHead();
  const tasksRoot = path.join(absRepo, 'orchestration', 'tasks');

  const entries = (await io.readdir(tasksRoot, { withFileTypes: true }))
    .filter((e) => e.isDirectory() && /^TASK-/.test(e.name)).map((e) => e.name).sort();

  const observations = [];
  const skipped = [];
  for (const name of entries) {
    const dir = path.join(tasksRoot, name);
    let specText;
    try { specText = await io.readFile(path.join(dir, 'SPEC.md')); }
    catch { skipped.push({ folder: name, reason: 'no SPEC.md' }); continue; }
    let statusText = null, reviewText = null;
    try { statusText = await io.readFile(path.join(dir, 'STATUS.md')); }
    catch { statusText = null; }
    try { reviewText = await io.readFile(path.join(dir, 'REVIEW.md')); }
    catch { reviewText = null; }
    void dir;
    const observation = await buildObservation({ folderName: name, specText, statusText, reviewText, git });
    if (observation === null) skipped.push({ folder: name, reason: 'SPEC.md without parsable task frontmatter (legacy)' });
    else observations.push(observation);
  }
  observations.sort(byTaskId);
  observations.forEach(attachMs);

  let runStatusText = null;
  try { runStatusText = await io.readFile(path.join(absRepo, 'orchestration', 'RUN_STATUS.md')); }
  catch { runStatusText = null; }
  const lanes = runStatusText ? parseLanes(runStatusText) : [];
  const readyRows = runStatusText ? parseEffectiveReady(runStatusText) : [];
  const readyTasks = [];
  for (const row of readyRows) readyTasks.push({ ...row, packet_type: await specPacketType(tasksRoot, row.task_id) });

  let reserveSummary = null;
  try { reserveSummary = JSON.parse(await io.readFile(path.join(absRepo, 'orchestration', 'backlog-factory', 'generated', 'summary.json'))); }
  catch { reserveSummary = null; }

  const aggregation = aggregateUnits(observations);
  observations.forEach(stripMs);

  const snapshot = buildRunwaySnapshot({ repoRevision, lanes, readyTasks, reserveSummary, units: aggregation.units });

  const outputs = [
    ['throughput-observations.json', canonicalJson({
      schema_version: OBSERVATIONS_SCHEMA,
      repo_revision: repoRevision,
      deterministic: true,
      skipped_folders: skipped.sort((a, b) => a.folder < b.folder ? -1 : 1),
      observations,
      aggregation,
    })],
    ['runway-snapshot.json', canonicalJson(snapshot)],
  ];

  if (check) {
    for (const [name, expected] of outputs) {
      const target = path.join(absOut, name);
      let actual = null;
      try { actual = await io.readFile(target); } catch { actual = null; }
      if (actual == null) throw new CollectError(`--check failed: missing output ${target}`, EXIT_STALE);
      if (actual !== expected) {
        throw new CollectError(`--check failed: stale output ${target} does not match recomputed evidence at ${repoRevision}`, EXIT_STALE);
      }
    }
    return { ok: true, mode: 'check', files: outputs.map(([n]) => n), repo_revision: repoRevision };
  }

  await io.mkdir(absOut, { recursive: true });
  for (const [name, json] of outputs) await io.writeFile(path.join(absOut, name), json);
  return { ok: true, mode: 'write', wrote: outputs.map(([n]) => path.join(absOut, n)), repo_revision: repoRevision };
}

function byTaskId(a, b) { return a.task_id < b.task_id ? -1 : a.task_id > b.task_id ? 1 : 0; }

// ---------- CLI ----------

export function usage() {
  return 'usage: node collect.mjs --repo <root> --out <captures-dir> [--check]';
}

export async function main(argv, io = defaultIo) {
  const args = {};
  for (let i = 0; i < argv.length; i++) {
    const a = argv[i];
    if (a === '--repo' || a === '--out') args[a.slice(2)] = argv[++i];
    else if (a === '--check') args.check = true;
  }
  if (!args.repo || !args.out) {
    process.stderr.write(usage() + '\n');
    return 1;
  }
  try {
    const result = await runCollector({ repoRoot: args.repo, outDir: args.out, check: !!args.check, io });
    if (result.mode === 'check') process.stdout.write(`collect --check OK at ${result.repo_revision}: ${result.files.join(', ')}\n`);
    else process.stdout.write(`collect wrote ${result.wrote.length} file(s) at ${result.repo_revision}\n`);
    return EXIT_OK;
  } catch (err) {
    process.stderr.write(`collect error (${err.exitCode ?? 1}): ${err.message}\n`);
    return err.exitCode ?? 1;
  }
}

if (process.argv[1] && path.resolve(process.argv[1]) === fileURLToPath(import.meta.url)) {
  main(process.argv.slice(2)).then((code) => process.exit(code));
}
