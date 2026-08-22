// TASK-0128 collector test battery (node --test).
// Covers the ten fixture cases mandated by the SPEC.
import test from 'node:test';
import assert from 'node:assert/strict';
import fs from 'node:fs';
import os from 'node:os';
import path from 'node:path';
import { fileURLToPath } from 'node:url';

import {
  canonicalJson, parseFrontmatter, parseStatusBullets, extractUnit, parseVerdicts,
  buildObservation, validateObservation, aggregateUnits, buildRunwaySnapshot,
  runCollector, main, CollectError,
} from './collect.mjs';

const FIXTURES = path.join(path.dirname(fileURLToPath(import.meta.url)), 'fixtures');
const T0 = 1755822109000;
const H1 = 3600000;

function stubGit(fixtureRoot) {
  const spec = JSON.parse(fs.readFileSync(path.join(fixtureRoot, '_git.json'), 'utf8'));
  return {
    revHead: () => spec.head,
    commitsFor(rel) { return spec[rel] ?? []; },
  };
}
const io = {
  readdir: (p, opts) => fs.promises.readdir(p, opts),
  readFile: (p) => fs.promises.readFile(p, 'utf8'),
  stat: (p) => fs.promises.stat(p),
  mkdir: (p, opts) => fs.promises.mkdir(p, opts),
  writeFile: (p, data) => fs.promises.writeFile(p, data, 'utf8'),
};

async function collectFixture(caseName) {
  const root = path.join(FIXTURES, caseName);
  return runCollector({
    repoRoot: root,
    outDir: path.join(root, '_out'),
    io,
    git: stubGit(root),
  });
}

function readOut(root) {
  return {
    obs: JSON.parse(fs.readFileSync(path.join(root, '_out', 'throughput-observations.json'), 'utf8')),
    snap: JSON.parse(fs.readFileSync(path.join(root, '_out', 'runway-snapshot.json'), 'utf8')),
  };
}

test('case 1: complete comparable accepted observations aggregate correctly', async () => {
  await collectFixture('case1-complete');
  const { obs } = readOut(path.join(FIXTURES, 'case1-complete'));
  assert.equal(obs.schema_version, 'throughput-observations-v1');
  assert.equal(obs.observations.length, 2);
  const units = obs.aggregation.units;
  assert.equal(units.length, 1);
  assert.equal(units[0].complete_unit, true);
  assert.equal(units[0].sample_count, 2);
  assert.equal(units[0].duration_sample_count, 2);
  assert.equal(units[0].total_duration_hours, 2);
  assert.equal(units[0].tasks_per_hour, 1);
  assert.ok(units[0].window.start < units[0].window.end);
});

test('case 2: one changed dimension prevents aggregation', async () => {
  await collectFixture('case2-changed-dimension');
  const { obs } = readOut(path.join(FIXTURES, 'case2-changed-dimension'));
  assert.equal(obs.aggregation.units.length, 2);
  for (const u of obs.aggregation.units) {
    assert.equal(u.sample_count, 1);
    assert.equal(u.tasks_per_hour, 1);
  }
  const machines = new Set(obs.aggregation.units.map((u) => u.dimensions.machine));
  assert.deepEqual([...machines].sort(), ['OTHERBOX', 'TESTBOX']);
});

test('case 3: missing unit dimensions remain null and produce UNKNOWN runway', async () => {
  await collectFixture('case3-missing-dims');
  const { obs, snap } = readOut(path.join(FIXTURES, 'case3-missing-dims'));
  const o = obs.observations[0];
  assert.equal(o.experimental_unit.provider, null);
  assert.equal(o.experimental_unit.model_id, null);
  assert.equal(o.experimental_unit.harness, null);
  assert.equal(o.experimental_unit.harness_version, null);
  assert.equal(o.experimental_unit.configuration_provenance, null);
  assert.equal(o.experimental_unit.machine, null);
  assert.equal(o.final_verdict, 'ACCEPTED');
  assert.equal(o.wall_clock_duration_ms, null); // missing duration tolerated, never invented
  assert.equal(obs.aggregation.units[0].complete_unit, false);
  assert.equal(obs.aggregation.units[0].tasks_per_hour, null);

  // lane-level snapshot over this incomplete unit
  const lanes = [{ lane: 'ox-pc-a', ports: '6620-6639' }];
  const readyTasks = [{ task_id: 'TASK-0105', packet_type: 'IMPLEMENTATION', priority: 'P0' }];
  const snapshot = buildRunwaySnapshot({ repoRevision: 'x', lanes, readyTasks, reserveSummary: null, units: obs.aggregation.units });
  const lane = snapshot.lanes.find((l) => l.lane === 'ox-pc-a');
  assert.equal(lane.hours, null);
  assert.equal(lane.confidence, 'UNKNOWN');
  assert.ok(lane.missing_dimensions.includes('provider'));
  assert.equal(snapshot.overall_state, 'UNKNOWN');
  assert.equal(snapshot.hours, null);
  assert.equal(snapshot.confidence, 'UNKNOWN');
});

test('case 4: zero duration is rejected, not converted to infinite/zero throughput', async () => {
  await assert.rejects(
    () => collectFixture('case4-zero-duration'),
    (err) => err instanceof CollectError && err.exitCode === 3 && /zero\/negative duration rejected/.test(err.message),
  );
});

test('case 5: REVISE then ACCEPTED records review cycles and not-first-pass', async () => {
  await collectFixture('case5-revise-cycle');
  const { obs } = readOut(path.join(FIXTURES, 'case5-revise-cycle'));
  const o = obs.observations[0];
  assert.equal(o.final_verdict, 'ACCEPTED');
  assert.equal(o.review_cycles, 1);
  assert.equal(o.first_pass, false);
});

test('case 6: claim without acceptance is excluded from accepted throughput', async () => {
  await collectFixture('case6-unaccepted');
  const { obs } = readOut(path.join(FIXTURES, 'case6-unaccepted'));
  assert.equal(obs.observations[0].final_verdict, null);
  assert.equal(obs.observations[0].acceptance_end.iso, null);
  assert.equal(obs.aggregation.units.length, 0); // nothing accepted -> no rates at all
});

test('case 7: provider/model alias disagreement remains unconfirmed', async () => {
  await collectFixture('case7-alias-disagreement');
  const { obs } = readOut(path.join(FIXTURES, 'case7-alias-disagreement'));
  const o = obs.observations[0];
  assert.equal(o.experimental_unit.alias_confirmed, false);
  assert.equal(obs.aggregation.units.find((u) => u.complete_unit === true), undefined);
});

test('case 8: stale output makes --check fail without rewriting it', async () => {
  const root = path.join(FIXTURES, 'case1-complete');
  await collectFixture('case1-complete');
  const out = path.join(root, '_out', 'throughput-observations.json');
  fs.writeFileSync(out, '{"tampered": true}\n');
  await assert.rejects(
    () => runCollector({ repoRoot: root, outDir: path.join(root, '_out'), check: true, io, git: stubGit(root) }),
    (err) => err instanceof CollectError && err.exitCode === 2 && /stale output/.test(err.message),
  );
  assert.equal(JSON.parse(fs.readFileSync(out, 'utf8')).tampered, true); // check mode never rewrites
});

test('case 9a: path escape fails closed', async () => {
  const root = path.join(fs.mkdtempSync(path.join(os.tmpdir(), 'tp-escape-')), 'repo');
  fs.mkdirSync(path.join(root, 'orchestration', 'tasks'), { recursive: true });
  const outside = fs.mkdtempSync(path.join(os.tmpdir(), 'tp-outside-'));
  const code = await main(['--repo', root, '--out', outside]);
  assert.equal(code, 4);
});

test('case 9b: malformed task evidence fails closed', async () => {
  const code = await main(['--repo', path.join(FIXTURES, 'case9-malformed'), '--out', '_out']);
  assert.equal(code, 5);
});

test('case 10: identical inputs produce byte-identical outputs', async () => {
  const root = path.join(FIXTURES, 'case1-complete');
  await collectFixture('case1-complete');
  const aObs = fs.readFileSync(path.join(root, '_out', 'throughput-observations.json'));
  const aSnap = fs.readFileSync(path.join(root, '_out', 'runway-snapshot.json'));
  await collectFixture('case1-complete');
  const bObs = fs.readFileSync(path.join(root, '_out', 'throughput-observations.json'));
  const bSnap = fs.readFileSync(path.join(root, '_out', 'runway-snapshot.json'));
  assert.deepEqual(aObs, bObs);
  assert.deepEqual(aSnap, bSnap);
});

// ---------- focused parser checks ----------

test('frontmatter parsing distinguishes legacy, malformed, and closed specs', () => {
  assert.equal(parseFrontmatter('# no frontmatter\n').hasFrontmatter, false);
  const broken = parseFrontmatter('---\ntask: TASK-1\n');
  assert.equal(broken.hasFrontmatter, true);
  assert.equal(broken.closed, false);
  const good = parseFrontmatter('---\ntask: TASK-1\npacket: IMPLEMENTATION\n---\nbody');
  assert.equal(good.closed, true);
  assert.equal(good.data.packet, 'IMPLEMENTATION');
});

test('status bullets join continuations and normalize keys', () => {
  const bullets = parseStatusBullets('- configuration provenance: owner project\n  continued here\n\n- state: CLAIMED\n');
  assert.match(bullets.get('configuration provenance'), /owner project continued here/);
  assert.equal(bullets.get('state'), 'CLAIMED');
});

test('extractUnit reads variant inline and machine prefix only', () => {
  const unit = extractUnit(parseStatusBullets('- provider: `opencode`; upstream unknown\n- model id: `m-free`; variant: max (meta)\n- machine: BOX-A (user x)\n'));
  assert.equal(unit.provider, 'opencode');
  assert.equal(unit.model_id, 'm-free');
  assert.equal(unit.variant, 'max');
  assert.equal(unit.machine, 'BOX-A');
});

test('verdict parser counts revise cycles before final acceptance', () => {
  const v = parseVerdicts('# r - REVISE\nmid text ACCEPTED word should not match bare token line rule\nVerdict: **REVISE**.\n\n## rev3 - ACCEPTED\nVerdict: **ACCEPTED**.\n');
  assert.equal(v.final, 'ACCEPTED');
  assert.equal(v.revise_cycles >= 1, true);
});

test('canonical json is sorted and stable', () => {
  const a = canonicalJson({ b: 1, a: [2, { z: 1, y: 2 }] });
  const b = canonicalJson({ a: [2, { y: 2, z: 1 }], b: 1 });
  assert.equal(a, b);
});
