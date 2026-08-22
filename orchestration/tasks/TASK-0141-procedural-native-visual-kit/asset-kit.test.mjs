import test from 'node:test';
import assert from 'node:assert/strict';
import { readFileSync } from 'node:fs';
import path from 'node:path';
import { fileURLToPath } from 'node:url';
import { buildKit, GENERATOR_VERSION } from './generate-assets.mjs';

const TASK_DIR = path.dirname(fileURLToPath(import.meta.url));
const REPO_ROOT = path.resolve(TASK_DIR, '..', '..', '..');
const CANONICAL_ROLES = [
  'player',
  'raider',
  'elite',
  'tree',
  'ruin',
  'dwelling',
  'shrine',
  'terrain',
];

function kitFilesFromDisk() {
  const { files } = buildKit();
  return files.map((file) => ({
    path: file.path,
    text: readFileSync(path.join(REPO_ROOT, file.path), 'utf8'),
  }));
}

function manifestFromDisk() {
  return JSON.parse(
    readFileSync(path.join(REPO_ROOT, 'native/client/assets/manifest.json'), 'utf8'),
  );
}

function headerFromDisk() {
  return readFileSync(
    path.join(REPO_ROOT, 'native/client/assets/generated/visual_kit.h'),
    'utf8',
  );
}

test('manifest declares exactly the eight canonical roles', () => {
  const { manifest } = buildKit();
  assert.deepEqual(
    manifest.roles.map((entry) => entry.role),
    CANONICAL_ROLES,
  );
  assert.equal(manifest.generatorVersion, GENERATOR_VERSION);
});

test('terrain role carries exactly two motifs and others exactly one', () => {
  const { manifest } = buildKit();
  for (const entry of manifest.roles) {
    if (entry.role === 'terrain') {
      assert.equal(entry.motifs.length, 2);
      assert.deepEqual(
        entry.motifs.map((motif) => motif.symbol),
        ['terrain_a', 'terrain_b'],
      );
    } else {
      assert.equal(entry.motifs.length, 1);
      assert.equal(entry.motifs[0].symbol, entry.role);
    }
  }
});

test('every motif palette names concrete colors', () => {
  const { manifest } = buildKit();
  for (const entry of manifest.roles) {
    for (const motif of entry.motifs) {
      assert.ok(Array.isArray(motif.palette), `${motif.symbol} palette`);
      assert.ok(motif.palette.length >= 3, `${motif.symbol} palette size`);
      for (const color of motif.palette) {
        assert.match(color, /^#[0-9a-f]{6}([0-9a-f]{2})?$/);
      }
    }
  }
});

test('committed SVG sources exist with valid roots matching the generator', () => {
  const files = kitFilesFromDisk();
  const generated = new Map(buildKit().files.map((file) => [file.path, file.text]));
  const svgs = files.filter((file) => file.path.endsWith('.svg'));
  assert.equal(svgs.length, 9);
  for (const file of svgs) {
    assert.equal(file.text, generated.get(file.path), `${file.path} regenerates byte-for-byte`);
    assert.ok(file.text.startsWith('<svg xmlns="http://www.w3.org/2000/svg"'), file.path);
    assert.ok(file.text.endsWith('</svg>\n'), file.path);
    assert.ok(file.text.split('\n')[0].endsWith('viewBox="0 0 64 64">'), file.path);
    assert.ok(!file.text.includes('<script'), file.path);
    assert.ok(!file.text.includes('href'), file.path);
    assert.ok(!file.text.includes('url('), file.path);
    const openTags = (file.text.match(/</g) || []).length;
    const closeTags = (file.text.match(/\/>/g) || []).length;
    assert.equal(openTags, closeTags + 2, `${file.path} balanced tags`);
  }
});

test('generation is deterministic across repeated runs', () => {
  const first = buildKit();
  const second = buildKit();
  assert.deepEqual(first, second);
  const firstJson = JSON.stringify(first.files);
  const secondJson = JSON.stringify(second.files);
  assert.equal(firstJson, secondJson);
});

test('committed manifest and header match regeneration byte-for-byte', () => {
  const generated = new Map(buildKit().files.map((file) => [file.path, file.text]));
  for (const file of kitFilesFromDisk()) {
    assert.equal(file.text, generated.get(file.path), `${file.path} up to date`);
  }
});

test('generated header covers every declared symbol', () => {
  const header = headerFromDisk();
  const manifest = manifestFromDisk();
  assert.match(header, /inline constexpr char kKitVersion\[\] = "[^"]+"/);
  const symbolRows = [...header.matchAll(/\{"([^"]+)", "([^"]+)", "([^"]+)", [\d.-]+f, [\d.-]+f, (\d+), (\d+)\}/g)];
  const expected = [];
  for (const entry of manifest.roles) {
    for (const motif of entry.motifs) {
      expected.push([entry.role, motif.motif, motif.source]);
    }
  }
  assert.equal(symbolRows.length, expected.length);
  let previousEnd = 0;
  expected.forEach((tuple, index) => {
    const row = symbolRows[index].slice(1);
    assert.deepEqual(row.slice(0, 3), tuple);
    const begin = Number(row[3]);
    const end = Number(row[4]);
    assert.equal(begin, previousEnd);
    assert.ok(end > begin);
    assert.ok(header.includes(`"${row[2]}"`));
    previousEnd = end;
  });
  const shapeRowCount = (header.match(/\{ShapeKind::/g) || []).length;
  assert.equal(previousEnd, shapeRowCount);
});

test('generated header is data only with no platform or behavior surface', () => {
  const header = headerFromDisk();
  const includes = [...header.matchAll(/^#include (.+)$/gm)].map((match) => match[1]);
  assert.deepEqual(includes, ['<cstdint>']);
  for (const banned of ['#include <windows', 'HDC', 'HWND', 'WINAPI', 'void ', '->', 'new ']) {
    assert.ok(!header.includes(banned), `header must not contain ${banned}`);
  }
  assert.ok(header.includes('inline constexpr'));
  assert.ok(header.includes(`kKitVersion[] = "${GENERATOR_VERSION}"`));
});

test('kit artifacts carry no forbidden port or external references', () => {
  const forbiddenPort = ['65', '00'].join('');
  const forbiddenHost = ['local', 'host'].join('');
  const scanned = [
    ...kitFilesFromDisk().map((file) => ({ name: file.path, text: file.text })),
    { name: 'generate-assets.mjs', text: readFileSync(path.join(TASK_DIR, 'generate-assets.mjs'), 'utf8') },
    { name: 'asset-kit.test.mjs', text: readFileSync(fileURLToPath(import.meta.url), 'utf8') },
  ];
  for (const file of scanned) {
    assert.ok(!file.text.includes(forbiddenPort), `${file.name}: forbidden port literal`);
    const schemes = [...file.text.matchAll(/\b(https?|wss?|ftp):\/\/([\w.-]+)/g)];
    for (const scheme of schemes) {
      const host = scheme[2];
      const allowed = scheme[1] === 'http' && host === 'www.w3.org';
      assert.ok(allowed, `${file.name}: external reference ${scheme[0]}`);
    }
    assert.ok(!file.text.includes(forbiddenHost), `${file.name}: loopback host reference`);
    assert.ok(!/\bfetch\s*\(/.test(file.text), `${file.name}: fetch call`);
    const imports = [...file.text.matchAll(/(?:require\s*\(\s*|from\s+|import\s+)["']([^"']+)["']/g)];
    for (const entry of imports) {
      const specifier = entry[1];
      const allowed = specifier.startsWith('node:') || specifier.startsWith('./') || specifier.startsWith('../');
      assert.ok(allowed, `${file.name}: non-builtin import ${specifier}`);
    }
  }
});
