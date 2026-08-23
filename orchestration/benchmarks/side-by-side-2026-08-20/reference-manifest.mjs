#!/usr/bin/env node
import { createHash } from 'node:crypto';
import { readdirSync, readFileSync, statSync, writeFileSync } from 'node:fs';
import { dirname, extname, join, resolve } from 'node:path';
import { fileURLToPath } from 'node:url';

const TOOL_DIR = dirname(fileURLToPath(import.meta.url));
const MANIFEST_NAME = 'reference-manifest.json';
const MANIFEST_PATH = join(TOOL_DIR, MANIFEST_NAME);
const GENERATOR = 'orchestration/benchmarks/side-by-side-2026-08-20/reference-manifest.mjs';
const BENCHMARK = 'side-by-side-2026-08-20 (D-124 native vs browser)';
const SCHEMA_VERSION = 1;

const REVISIONS = {
  'ec7d88cc': 'ec7d88ccf429a1b9d21572ab84f048af40501f95',
  '77b21764': '77b21764c043585f5da55d3c9008361f55e30d2b',
  '6a3a46c3': '6a3a46c33c218aaca38048c297f3806c66567447',
  '0a014420': '0a0144207cccfdf6024ebc467c7d81b2462b2c52',
};

const MATRIX = [
  { scene: '01-route-entrance', native: '01-route-entrance', browser: '01-route-entrance', composite: '01-route-entrance' },
  { scene: '02-pack-combat', native: '02-pack-combat', browser: '02-pack-combat', composite: '02-pack-combat' },
  { scene: '03-elite-telegraph', native: '03-elite-telegraph', browser: '03-combat-aftermath', composite: '03-combat' },
  { scene: '04-named-drop-gear', native: '04-named-drop-gear', browser: '04-inventory-open', composite: '04-inventory' },
  { scene: '05-critical-health', native: '05-critical-health', browser: '05-hud-state', composite: '05-hud' },
];

const COMPOSITE_GENERATIONS = [
  { prefix: 'sxs', revision: REVISIONS.ec7d88cc },
  { prefix: 'sxs2', revision: REVISIONS['77b21764'] },
  { prefix: 'sxs3', revision: REVISIONS['6a3a46c3'] },
];

const RENDER_LIST_DIR = '../../tasks/TASK-0070-reference-scenes/captures';

function expectedSlots() {
  const slots = [];
  for (const row of MATRIX) {
    slots.push({
      path: `native-after/${row.native}-1920x1080.png`,
      kind: 'image',
      side: 'native',
      scene: row.scene,
      width: 1920,
      height: 1080,
      sourceRevision: REVISIONS['77b21764'],
    });
    slots.push({
      path: `browser-${row.browser}-1920x1080.png`,
      kind: 'image',
      side: 'browser',
      scene: row.scene,
      width: 1920,
      height: 1080,
      sourceRevision: REVISIONS.ec7d88cc,
    });
    for (const gen of COMPOSITE_GENERATIONS) {
      slots.push({
        path: `${gen.prefix}-${row.composite}.jpg`,
        kind: 'image',
        side: 'composite',
        scene: row.scene,
        width: 1920,
        height: 568,
        sourceRevision: gen.revision,
      });
    }
    slots.push({
      path: `${RENDER_LIST_DIR}/${row.native}.json`,
      kind: 'render-list-json',
      side: 'native',
      scene: row.scene,
      sourceRevision: REVISIONS['0a014420'],
    });
  }
  return slots.sort((a, b) => (a.path < b.path ? -1 : a.path > b.path ? 1 : 0));
}

function pngDimensions(bytes) {
  const signature = Buffer.from([137, 80, 78, 71, 13, 10, 26, 10]);
  if (bytes.length < 33 || !bytes.subarray(0, 8).equals(signature)) {
    throw new EvidenceError('malformed PNG (bad signature or truncated IHDR)');
  }
  if (bytes.readUInt32BE(8) !== 13 || bytes.subarray(12, 16).toString('latin1') !== 'IHDR') {
    throw new EvidenceError('malformed PNG (IHDR is not the first chunk)');
  }
  const width = bytes.readUInt32BE(16);
  const height = bytes.readUInt32BE(20);
  if (!width || !height) throw new EvidenceError('malformed PNG (zero dimension in IHDR)');
  return { width, height };
}

function jpegDimensions(bytes) {
  if (bytes.length < 4 || bytes[0] !== 0xff || bytes[1] !== 0xd8) {
    throw new EvidenceError('malformed JPEG (missing SOI marker)');
  }
  let off = 2;
  while (off + 4 <= bytes.length) {
    if (bytes[off] !== 0xff) throw new EvidenceError(`malformed JPEG (expected marker byte at offset ${off})`);
    let marker = bytes[off + 1];
    while (marker === 0xff && off + 2 < bytes.length) {
      off += 1;
      marker = bytes[off + 1];
    }
    if (marker === 0x00 || marker === 0x01 || (marker >= 0xd0 && marker <= 0xd9)) {
      off += 2;
      continue;
    }
    if (off + 4 > bytes.length) break;
    const segLength = bytes.readUInt16BE(off + 2);
    if (segLength < 2) throw new EvidenceError('malformed JPEG (segment length < 2)');
    if (marker >= 0xc0 && marker <= 0xcf && marker !== 0xc4 && marker !== 0xc8 && marker !== 0xcc) {
      if (off + 9 > bytes.length) throw new EvidenceError('malformed JPEG (truncated SOF segment)');
      return { height: bytes.readUInt16BE(off + 5), width: bytes.readUInt16BE(off + 7) };
    }
    if (marker === 0xda) throw new EvidenceError('malformed JPEG (start of scan before SOF)');
    off += 2 + segLength;
  }
  throw new EvidenceError('malformed JPEG (no SOF marker found)');
}

function imageDimensions(path, bytes) {
  switch (extname(path).toLowerCase()) {
    case '.png':
      return pngDimensions(bytes);
    case '.jpg':
    case '.jpeg':
      return jpegDimensions(bytes);
    default:
      throw new EvidenceError(`unsupported image extension: ${path}`);
  }
}

function validateRenderListJson(absPath, expectedScene) {
  let parsed;
  try {
    parsed = JSON.parse(readFileSync(absPath, 'utf8'));
  } catch (error) {
    throw new EvidenceError(`malformed render-list JSON (${error.message})`);
  }
  if (parsed === null || typeof parsed !== 'object' || Array.isArray(parsed)) {
    throw new EvidenceError('malformed render-list JSON (root must be an object)');
  }
  if (typeof parsed.scene !== 'string' || parsed.scene.length === 0) {
    throw new EvidenceError('malformed render-list JSON (missing string "scene")');
  }
  if (parsed.scene !== expectedScene) {
    throw new EvidenceError(`render-list JSON scene "${parsed.scene}" does not match expected scene "${expectedScene}"`);
  }
  if (parsed.width !== 1920 || parsed.height !== 1080) {
    throw new EvidenceError(`wrong-resolution render-list JSON (${parsed.width}x${parsed.height}, expected 1920x1080)`);
  }
  if (!Array.isArray(parsed.ops) || parsed.ops.length === 0) {
    throw new EvidenceError('malformed render-list JSON ("ops" must be a non-empty array)');
  }
  for (let i = 0; i < parsed.ops.length; i++) {
    const op = parsed.ops[i];
    if (op === null || typeof op !== 'object' || Array.isArray(op) || typeof op.op !== 'string' || op.op.length === 0) {
      throw new EvidenceError(`malformed render-list JSON (ops[${i}] lacks a string "op")`);
    }
  }
}

class EvidenceError extends Error {}

function measureSlot(slot) {
  const absPath = resolve(TOOL_DIR, ...slot.path.split('/'));
  let stats;
  try {
    stats = statSync(absPath);
  } catch {
    throw new EvidenceError(`missing evidence file: ${slot.path}`);
  }
  if (!stats.isFile()) throw new EvidenceError(`missing evidence file (not a regular file): ${slot.path}`);
  if (stats.size === 0) throw new EvidenceError(`zero-byte evidence file: ${slot.path}`);
  const bytes = readFileSync(absPath);
  const entry = {
    path: slot.path,
    kind: slot.kind,
    side: slot.side,
    scene: slot.scene,
    byteLength: stats.size,
  };
  if (slot.kind === 'image') {
    const dims = imageDimensions(slot.path, bytes);
    if (dims.width !== slot.width || dims.height !== slot.height) {
      throw new EvidenceError(
        `wrong-resolution evidence: ${slot.path} is ${dims.width}x${dims.height}, expected ${slot.width}x${slot.height}`,
      );
    }
    entry.width = dims.width;
    entry.height = dims.height;
  } else if (slot.kind === 'render-list-json') {
    validateRenderListJson(absPath, slot.scene);
  } else {
    throw new EvidenceError(`unknown evidence kind: ${slot.kind}`);
  }
  entry.sha256 = createHash('sha256').update(bytes).digest('hex');
  entry.sourceRevision = slot.sourceRevision;
  return entry;
}

function listImageFiles(dir, prefix = '') {
  const found = [];
  for (const name of readdirSync(dir, { withFileTypes: true })) {
    const relPath = prefix ? `${prefix}/${name.name}` : name.name;
    if (name.isDirectory()) found.push(...listImageFiles(join(dir, name.name), relPath));
    else if (['.png', '.jpg', '.jpeg'].includes(extname(name.name).toLowerCase())) found.push(relPath);
  }
  return found.sort();
}

function buildManifestEntries() {
  const entries = [];
  for (const slot of expectedSlots()) entries.push(measureSlot(slot));
  const expectedPaths = new Set(expectedSlots().map((slot) => slot.path));
  const stray = listImageFiles(TOOL_DIR).filter((path) => !expectedPaths.has(path));
  if (stray.length > 0) {
    throw new EvidenceError(`unmanifested evidence present: ${stray.join(', ')}`);
  }
  return entries;
}

function readManifest(path) {
  let parsed;
  try {
    parsed = JSON.parse(readFileSync(path, 'utf8'));
  } catch (error) {
    throw new EvidenceError(`malformed manifest (${error.message})`);
  }
  if (parsed === null || typeof parsed !== 'object' || Array.isArray(parsed)) {
    throw new EvidenceError('malformed manifest (root must be an object)');
  }
  if (parsed.schemaVersion !== SCHEMA_VERSION) {
    throw new EvidenceError(`unsupported manifest schemaVersion ${JSON.stringify(parsed.schemaVersion)}, expected ${SCHEMA_VERSION}`);
  }
  if (!Array.isArray(parsed.entries)) throw new EvidenceError('malformed manifest ("entries" must be an array)');
  return parsed;
}

function verify(manifestPath) {
  const errors = [];
  const slotsByPath = new Map(expectedSlots().map((slot) => [slot.path, slot]));
  const manifest = readManifest(manifestPath);
  const seenPaths = new Set();
  for (const entry of manifest.entries) {
    const path = entry && typeof entry === 'object' ? entry.path : undefined;
    if (typeof path !== 'string' || path.length === 0) {
      errors.push('malformed manifest entry (missing "path")');
      continue;
    }
    if (seenPaths.has(path)) errors.push(`duplicate manifest entry: ${path}`);
    seenPaths.add(path);
    const slot = slotsByPath.get(path);
    if (!slot) {
      errors.push(`unexpected manifest entry (not part of the five-scene matrix): ${path}`);
      continue;
    }
    try {
      const measured = measureSlot(slot);
      for (const key of ['kind', 'side', 'scene']) {
        if (entry[key] !== measured[key]) errors.push(`${key} mismatch for ${path}: recorded ${JSON.stringify(entry[key])}, actual ${JSON.stringify(measured[key])}`);
      }
      if (entry.byteLength !== measured.byteLength) {
        errors.push(`byteLength mismatch for ${path}: recorded ${entry.byteLength}, actual ${measured.byteLength}`);
      }
      if (measured.kind === 'image' && (entry.width !== measured.width || entry.height !== measured.height)) {
        errors.push(`dimensions mismatch for ${path}: recorded ${entry.width}x${entry.height}, actual ${measured.width}x${measured.height}`);
      }
      if (entry.sha256 !== measured.sha256) {
        errors.push(`sha256 mismatch for ${path}: recorded ${entry.sha256}, actual ${measured.sha256}`);
      }
      if (entry.sourceRevision !== measured.sourceRevision) {
        errors.push(`sourceRevision mismatch for ${path}: recorded ${entry.sourceRevision}, actual ${measured.sourceRevision}`);
      }
    } catch (error) {
      errors.push(error instanceof EvidenceError ? error.message : `internal error verifying ${path}: ${error.message}`);
    }
  }
  for (const path of slotsByPath.keys()) {
    if (!seenPaths.has(path)) errors.push(`missing-from-manifest: no entry for ${path}`);
  }
  const manifestedImagePaths = new Set(
    manifest.entries.filter((entry) => entry && typeof entry === 'object' && entry.kind === 'image').map((entry) => entry.path),
  );
  const stray = listImageFiles(TOOL_DIR).filter((path) => !manifestedImagePaths.has(path));
  for (const path of stray) errors.push(`unmanifested evidence on disk: ${path}`);
  return { manifest, errors };
}

function write() {
  const entries = buildManifestEntries();
  const manifest = {
    schemaVersion: SCHEMA_VERSION,
    generator: GENERATOR,
    benchmark: BENCHMARK,
    entryCount: entries.length,
    entries,
  };
  writeFileSync(MANIFEST_PATH, `${JSON.stringify(manifest, null, 2)}\n`);
  printSummary(`wrote ${MANIFEST_PATH}`, entries);
}

function printSummary(label, entries) {
  console.log(`${label}: ${entries.length} entries (${entries.filter((entry) => entry.side === 'native').length} native, ${entries.filter((entry) => entry.side === 'browser').length} browser, ${entries.filter((entry) => entry.side === 'composite').length} composite)`);
}

function usage() {
  console.error(`usage: node reference-manifest.mjs [--verify | --write] [--manifest <path>]

  (default)      verify the frozen reference capture against reference-manifest.json
  --verify       same as default, explicitly read-only
  --write        regenerate ONLY reference-manifest.json from the evidence on disk
  --manifest <p> verify against an alternate manifest copy (e.g. negative tests)`);
}

function main() {
  const args = process.argv.slice(2);
  let mode = 'verify';
  let manifestPath = MANIFEST_PATH;
  for (let i = 0; i < args.length; i++) {
    if (args[i] === '--verify') mode = 'verify';
    else if (args[i] === '--write') mode = 'write';
    else if (args[i] === '--manifest') {
      if (i + 1 >= args.length) {
        usage();
        process.exitCode = 2;
        return;
      }
      manifestPath = resolve(process.cwd(), args[++i]);
    } else {
      console.error(`unknown argument: ${args[i]}`);
      usage();
      process.exitCode = 2;
      return;
    }
  }
  if (mode === 'write') {
    try {
      write();
    } catch (error) {
      console.error(`FAIL: ${error.message}`);
      process.exitCode = 1;
    }
    return;
  }
  const { errors } = verify(manifestPath);
  if (errors.length > 0) {
    for (const message of errors) console.error(`FAIL: ${message}`);
    console.error(`verification FAILED with ${errors.length} error(s) against ${manifestPath}`);
    process.exitCode = 1;
    return;
  }
  const manifest = readManifest(manifestPath);
  printSummary(`verification OK (${manifestPath})`, manifest.entries);
}

main();
