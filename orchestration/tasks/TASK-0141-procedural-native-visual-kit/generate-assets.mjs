import { mkdirSync, readFileSync, writeFileSync, existsSync } from 'node:fs';
import path from 'node:path';
import process from 'node:process';
import { fileURLToPath, pathToFileURL } from 'node:url';

export const GENERATOR_VERSION = 'task0141-gen-1';

const VIEW_WIDTH = 64;
const VIEW_HEIGHT = 64;

function mulberry32(seed) {
  let state = seed >>> 0;
  return function next() {
    state = (state + 0x6d2b79f5) >>> 0;
    let t = state;
    t = Math.imul(t ^ (t >>> 15), t | 1);
    t ^= t + Math.imul(t ^ (t >>> 7), t | 61);
    return ((t ^ (t >>> 14)) >>> 0) / 4294967296;
  };
}

function fmt(value) {
  if (!Number.isFinite(value)) throw new Error(`non-finite number: ${value}`);
  let text = value.toFixed(2);
  if (text.includes('.')) text = text.replace(/0+$/, '').replace(/\.$/, '');
  if (text === '-0') text = '0';
  return text;
}

function cppFloat(value) {
  // Standards-conforming C++ float literals: bare digit-suffix tokens such as
  // "22f" reserve "f" as a literal-operator suffix (MSVC C4455). Emitting an
  // explicit decimal point ("22.f") keeps every literal conforming.
  let text = fmt(value);
  if (!text.includes('.')) text = `${text}.`;
  return `${text}f`;
}

function polygon(points, fill, options = {}) {
  return { kind: 'polygon', points, fill, ...options };
}

function polyline(points, stroke, strokeWidth, options = {}) {
  return { kind: 'polyline', points, fill: 'none', stroke, sw: strokeWidth, ...options };
}

function circle(cx, cy, r, fill, options = {}) {
  return { kind: 'circle', cx, cy, r, fill, ...options };
}

function ellipse(cx, cy, rx, ry, fill, options = {}) {
  return { kind: 'ellipse', cx, cy, rx, ry, fill, ...options };
}

function shadow(cx, rx) {
  return ellipse(cx, 57.5, rx, 3.5, '#00000040');
}

function buildPlayer(rng) {
  void rng;
  return [
    shadow(32, 13),
    polygon([[22, 26], [42, 26], [44, 52], [36, 49], [32, 53], [28, 49], [20, 52]], '#24427c'),
    polygon([[26, 42], [31, 42], [30, 55], [25, 55]], '#37475e'),
    polygon([[33, 42], [38, 42], [39, 55], [34, 55]], '#37475e'),
    polygon([[24, 53], [30, 53], [30, 58], [24, 58]], '#231a10'),
    polygon([[34, 53], [40, 53], [40, 58], [34, 58]], '#231a10'),
    polygon([[23, 28], [41, 28], [39, 44], [25, 44]], '#93a7bd'),
    polygon([[25, 42], [39, 42], [39, 45], [25, 45]], '#5b3a1e'),
    circle(22, 29, 4.5, '#aabdd2'),
    circle(42, 29, 4.5, '#aabdd2'),
    polygon([[40, 30], [46, 27], [47, 30], [41, 33]], '#93a7bd'),
    polygon([[45, 26], [49, 31], [46, 33], [42, 29]], '#5b3a1e'),
    polygon([[47, 28], [57, 17], [60, 19], [49, 31]], '#dfe8ef'),
    circle(44.5, 32, 1.6, '#d9a441'),
    circle(32, 19, 7, '#7f93a9'),
    polygon([[26, 18], [38, 18], [38, 23], [26, 23]], '#101720'),
    polygon([[31, 14], [33, 14], [33, 23], [31, 23]], '#67798d'),
    polygon([[29, 12], [34, 12], [37, 6], [31, 8]], '#3f6fd8'),
    circle(34.5, 6.5, 1.8, '#3f6fd8'),
  ];
}

function buildRaider(rng) {
  void rng;
  return [
    shadow(33, 13),
    polygon([[20, 30], [44, 28], [47, 50], [42, 47], [38, 52], [33, 48], [28, 53], [23, 48], [18, 51]], '#6e2a22'),
    polygon([[24, 30], [42, 26], [44, 38], [26, 42]], '#8a4a2f'),
    polygon([[38, 32], [50, 34], [50, 38], [38, 40]], '#7c3e28'),
    polygon([[44, 34], [50, 35], [50, 38], [44, 37]], '#3a2a1a'),
    polygon([[48, 26], [62, 30], [61, 35], [52, 36], [50, 33], [48, 34]], '#b7bcc2'),
    polygon([[26, 20], [38, 17], [40, 25], [28, 27]], '#3c2f26'),
    polygon([[25, 20], [18, 12], [27, 18]], '#d9c9a3'),
    polygon([[39, 17], [46, 9], [42, 21]], '#d9c9a3'),
    circle(31, 22, 1.4, '#ffcf5e'),
    circle(36, 21, 1.4, '#ffcf5e'),
    polygon([[24, 48], [30, 48], [30, 55], [24, 55]], '#33241b'),
    polygon([[36, 47], [42, 47], [42, 54], [36, 54]], '#33241b'),
  ];
}

function buildElite(rng) {
  void rng;
  return [
    shadow(32, 16),
    polygon([[18, 24], [46, 24], [50, 54], [40, 50], [32, 55], [24, 50], [14, 54]], '#4a1016'),
    polygon([[25, 44], [31, 44], [30, 56], [24, 56]], '#3c1218'),
    polygon([[33, 44], [39, 44], [40, 56], [34, 56]], '#3c1218'),
    polygon([[23, 54], [30, 54], [30, 58], [23, 58]], '#1c0a0d'),
    polygon([[34, 54], [41, 54], [41, 58], [34, 58]], '#1c0a0d'),
    polygon([[21, 24], [43, 24], [41, 46], [23, 46]], '#7a2230'),
    polygon([[30, 24], [34, 24], [34, 46], [30, 46]], '#d9a441'),
    circle(20, 26, 5, '#8c2a3a'),
    circle(44, 26, 5, '#8c2a3a'),
    polygon([[16, 23], [20, 12], [24, 23]], '#d9a441'),
    polygon([[40, 23], [44, 12], [48, 23]], '#d9a441'),
    polygon([[25, 10], [39, 10], [41, 21], [23, 21]], '#5d1620'),
    polygon([[24, 12], [12, 4], [26, 18]], '#d9a441'),
    polygon([[40, 12], [52, 4], [38, 18]], '#d9a441'),
    circle(29, 16, 1.6, '#ff5d3a'),
    circle(35, 16, 1.6, '#ff5d3a'),
    polygon([[44, 32], [57, 24], [59, 27], [46, 35]], '#3a2a1a'),
    polygon([[54, 16], [63, 21], [60, 28], [51, 24]], '#9aa1a8'),
  ];
}

function buildTree(rng) {
  const shapes = [
    polygon([[29, 44], [35, 44], [37, 58], [27, 58]], '#5a4030'),
    polygon([[24, 55], [29, 53], [30, 58], [24, 58]], '#4a3526'),
    polygon([[35, 53], [40, 55], [40, 58], [34, 58]], '#4a3526'),
    polygon([
      [32, 6], [46, 27], [41, 25], [50, 40], [42, 37], [52, 50],
      [12, 50], [22, 37], [14, 40], [23, 25], [18, 27],
    ], '#2f6b3a'),
    polygon([
      [32, 12], [42, 28], [38, 27], [44, 38], [38, 36], [45, 46],
      [19, 46], [26, 36], [20, 38], [26, 27], [22, 28],
    ], '#3d8a49'),
  ];
  for (let i = 0; i < 7; i += 1) {
    const angle = rng() * Math.PI * 2;
    const radius = 4 + rng() * 10;
    const x = 32 + Math.cos(angle) * radius * 1.4;
    const y = 30 + Math.sin(angle) * radius * 0.7;
    if (y < 14 || y > 47 || x < 15 || x > 49) continue;
    shapes.push(circle(Math.round(x * 2) / 2, Math.round(y * 2) / 2, 1 + rng() * 1.4, '#57a75f'));
  }
  return shapes;
}

function buildRuin(rng) {
  void rng;
  return [
    shadow(32, 20),
    polygon([[13, 30], [16.5, 24], [19, 28], [22, 22], [23, 58], [13, 58]], '#8d8f94'),
    polygon([[13, 30], [16.5, 24], [19, 28], [22, 22], [23, 26], [19, 31], [16, 29]], '#a3a5aa'),
    polygon([[41, 38], [44.5, 33], [47, 37], [50, 32], [51, 58], [41, 58]], '#76787d'),
    polygon([[24, 44], [44, 38], [46, 44], [26, 50]], '#84868b'),
    polyline([[24, 44], [44, 38], [46, 44], [26, 50], [24, 44]], '#5f6165', 1.5),
    circle(28, 55, 2.4, '#6f7176'),
    polygon([[34, 52], [39, 51], [40, 56], [35, 57]], '#7a7c81'),
    circle(46, 54, 1.8, '#66686d'),
    circle(17, 52, 1.5, '#557d4a'),
    circle(43, 41, 1.3, '#557d4a'),
    polygon([[20, 40], [23, 39], [24, 44], [21, 45]], '#7f8186'),
  ];
}

function buildDwelling(rng) {
  void rng;
  return [
    shadow(32, 19),
    polygon([[16, 33], [48, 33], [48, 58], [16, 58]], '#a3805a'),
    polygon([[16, 33], [19, 33], [19, 58], [16, 58]], '#4a3623'),
    polygon([[45, 33], [48, 33], [48, 58], [45, 58]], '#4a3623'),
    polygon([[16, 33], [48, 33], [48, 36], [16, 36]], '#4a3623'),
    polygon([[11, 33], [32, 11], [53, 33]], '#b98f3e'),
    polygon([[9, 32], [55, 32], [55, 36], [9, 36]], '#8a6a2c'),
    polygon([[32, 11], [35, 14.5], [32, 14.5], [29, 14.5]], '#caa04a'),
    polygon([[28, 43], [36, 43], [36, 58], [28, 58]], '#3c2a18'),
    circle(34.5, 51, 0.9, '#b98f3e'),
    polygon([[20, 39], [26, 39], [26, 45], [20, 45]], '#4a3623'),
    polygon([[21, 40], [25, 40], [25, 44], [21, 44]], '#ffd98a'),
    polygon([[38, 39], [44, 39], [44, 45], [38, 45]], '#4a3623'),
    polygon([[39, 40], [43, 40], [43, 44], [39, 44]], '#ffd98a'),
  ];
}

function buildShrine(rng) {
  void rng;
  return [
    shadow(32, 18),
    polygon([[14, 54], [50, 54], [52, 58], [12, 58]], '#9aa1a8'),
    polygon([[18, 50], [46, 50], [48, 54], [16, 54]], '#a8aeb6'),
    polygon([[22, 28], [26, 28], [26, 50], [22, 50]], '#b9bec6'),
    polygon([[38, 28], [42, 28], [42, 50], [38, 50]], '#b9bec6'),
    polygon([[18, 22], [46, 22], [46, 28], [18, 28]], '#c6cbd2'),
    polygon([[16, 22], [32, 14], [48, 22]], '#d2d7de'),
    circle(32, 38, 7.5, '#59d6c926'),
    polygon([[28, 44], [36, 44], [38, 50], [26, 50]], '#6f7680'),
    polygon([[30, 44], [34, 44], [35, 37], [32, 31], [29, 37]], '#59d6c9'),
    polygon([[31, 44], [33, 44], [33.5, 39], [32, 35.5], [30.5, 39]], '#bff3ec'),
  ];
}

function buildTerrainA(rng) {
  const shapes = [
    polygon([[0, 0], [VIEW_WIDTH, 0], [VIEW_WIDTH, VIEW_HEIGHT], [0, VIEW_HEIGHT]], '#46653f'),
  ];
  for (let i = 0; i < 26; i += 1) {
    const x = 6 + rng() * 52;
    const y = 6 + rng() * 52;
    const roll = rng();
    const fill = roll < 0.4 ? '#4f7047' : roll < 0.75 ? '#3d5938' : '#57794d';
    shapes.push(circle(Math.round(x), Math.round(y), 1 + rng() * 1.6, fill));
  }
  for (let i = 0; i < 4; i += 1) {
    const x = 10 + rng() * 42;
    const y = 10 + rng() * 42;
    const w = 3 + rng() * 2.5;
    const h = 2.5 + rng() * 2;
    shapes.push(polygon(
      [[x, y], [x + w, y + rng()], [x + w + 0.5, y + h], [x - 0.5, y + h + rng()]],
      i % 2 === 0 ? '#6b7060' : '#5c6154',
    ));
  }
  return shapes;
}

function buildTerrainB(rng) {
  const shapes = [
    polygon([[0, 0], [VIEW_WIDTH, 0], [VIEW_WIDTH, VIEW_HEIGHT], [0, VIEW_HEIGHT]], '#33504a'),
  ];
  const slabs = [
    [[6, 6], [26, 5], [28, 22], [8, 24]],
    [[32, 7], [56, 6], [57, 20], [34, 23]],
    [[5, 30], [24, 28], [26, 44], [7, 46]],
    [[31, 29], [55, 27], [58, 42], [33, 45]],
    [[12, 50], [34, 49], [36, 60], [10, 61]],
    [[40, 50], [58, 49], [59, 60], [41, 61]],
  ];
  for (const slab of slabs) {
    const jitter = slab.map(([x, y]) => [
      Math.round((x + (rng() - 0.5) * 2) * 2) / 2,
      Math.round((y + (rng() - 0.5) * 2) * 2) / 2,
    ]);
    shapes.push(polygon(jitter, '#48695f'));
  }
  for (let i = 0; i < 12; i += 1) {
    const x = 6 + rng() * 52;
    const y = 6 + rng() * 52;
    const fill = rng() < 0.6 ? '#5d8a5a' : '#596058';
    shapes.push(circle(Math.round(x), Math.round(y), 0.8 + rng() * 1.2, fill));
  }
  return shapes;
}

const ROLE_BUILDERS = [
  { role: 'player', symbol: 'player', source: 'svg/player.svg', seed: 20260822, build: buildPlayer },
  { role: 'raider', symbol: 'raider', source: 'svg/raider.svg', seed: 20260823, build: buildRaider },
  { role: 'elite', symbol: 'elite', source: 'svg/elite.svg', seed: 20260824, build: buildElite },
  { role: 'tree', symbol: 'tree', source: 'svg/tree.svg', seed: 20260825, build: buildTree },
  { role: 'ruin', symbol: 'ruin', source: 'svg/ruin.svg', seed: 20260826, build: buildRuin },
  { role: 'dwelling', symbol: 'dwelling', source: 'svg/dwelling.svg', seed: 20260827, build: buildDwelling },
  { role: 'shrine', symbol: 'shrine', source: 'svg/shrine.svg', seed: 20260828, build: buildShrine },
];

const TERRAIN_MOTIFS = [
  { role: 'terrain', motif: 'grass-court', symbol: 'terrain_a', source: 'svg/terrain-a.svg', seed: 20260829, build: buildTerrainA },
  { role: 'terrain', motif: 'mossy-stone', symbol: 'terrain_b', source: 'svg/terrain-b.svg', seed: 20260830, build: buildTerrainB },
];

function variantPalette(shapes) {
  const seen = [];
  for (const shape of shapes) {
    for (const key of ['fill', 'stroke']) {
      const value = shape[key];
      if (value && value !== 'none' && !seen.includes(value)) seen.push(value);
    }
  }
  return seen;
}

function strokeAttributes(shape) {
  if (!shape.stroke || shape.stroke === 'none') return '';
  return ` stroke="${shape.stroke}" stroke-width="${fmt(shape.sw)}" stroke-linejoin="round"`;
}

function svgText(shapes) {
  const lines = [];
  lines.push(`<svg xmlns="http://www.w3.org/2000/svg" width="${VIEW_WIDTH}" height="${VIEW_HEIGHT}" viewBox="0 0 ${VIEW_WIDTH} ${VIEW_HEIGHT}">`);
  for (const shape of shapes) {
    if (shape.kind === 'circle') {
      lines.push(`  <circle cx="${fmt(shape.cx)}" cy="${fmt(shape.cy)}" r="${fmt(shape.r)}" fill="${shape.fill}"${strokeAttributes(shape)}/>`);
    } else if (shape.kind === 'ellipse') {
      lines.push(`  <ellipse cx="${fmt(shape.cx)}" cy="${fmt(shape.cy)}" rx="${fmt(shape.rx)}" ry="${fmt(shape.ry)}" fill="${shape.fill}"${strokeAttributes(shape)}/>`);
    } else {
      const points = shape.points.map(([x, y]) => `${fmt(x)},${fmt(y)}`).join(' ');
      lines.push(`  <${shape.kind} points="${points}" fill="${shape.fill}"${strokeAttributes(shape)}/>`);
    }
  }
  lines.push('</svg>');
  return `${lines.join('\n')}\n`;
}

function hexToColor(hex) {
  if (!/^#[0-9a-f]{6}([0-9a-f]{2})?$/.test(hex)) throw new Error(`bad color: ${hex}`);
  const digits = hex.slice(1);
  const channel = (index) => parseInt(digits.slice(index, index + 2), 16) / 255;
  const round = (value) => Math.round(value * 100) / 100;
  const alphaHex = digits.slice(6, 8);
  const alpha = alphaHex ? parseInt(alphaHex, 16) / 255 : 1;
  return [round(channel(0)), round(channel(2)), round(channel(4)), round(alpha)];
}

function buildVariants() {
  const entries = [...ROLE_BUILDERS.map((entry) => ({ ...entry, motif: 'default' })), ...TERRAIN_MOTIFS];
  return entries.map(({ role, motif, symbol, source, seed, build }) => {
    const rng = mulberry32(seed);
    const shapes = build(rng);
    for (const shape of shapes) {
      if (shape.kind === 'polyline') shape.fill = 'none';
    }
    return { role, motif, symbol, source, shapes, palette: variantPalette(shapes) };
  });
}

function headerText(variants) {
  const colors = [];
  const colorIndex = (hex) => {
    if (hex === undefined || hex === 'none') return -1;
    let index = colors.indexOf(hex);
    if (index < 0) {
      index = colors.length;
      colors.push(hex);
    }
    return index;
  };

  const flatPoints = [];
  const flatShapes = [];
  const symbols = [];
  for (const variant of variants) {
    const shapeBegin = flatShapes.length;
    for (const shape of variant.shapes) {
      const pointBegin = flatPoints.length;
      let kind;
      let extra = { cx: 0, cy: 0, rx: 0, ry: 0 };
      if (shape.kind === 'polygon') kind = 'Polygon';
      else if (shape.kind === 'polyline') kind = 'Polyline';
      else if (shape.kind === 'circle') {
        kind = 'Circle';
        extra = { cx: shape.cx, cy: shape.cy, rx: shape.r, ry: 0 };
      } else if (shape.kind === 'ellipse') {
        kind = 'Ellipse';
        extra = { cx: shape.cx, cy: shape.cy, rx: shape.rx, ry: shape.ry };
      } else {
        throw new Error(`unknown kind: ${shape.kind}`);
      }
      if (shape.kind === 'polygon' || shape.kind === 'polyline') {
        for (const [x, y] of shape.points) flatPoints.push(x, y);
      }
      flatShapes.push({
        kind,
        pointBegin,
        pointEnd: flatPoints.length,
        fill: colorIndex(shape.fill),
        stroke: colorIndex(shape.stroke),
        sw: shape.sw || 0,
        ...extra,
      });
    }
    symbols.push({ variant, shapeBegin, shapeEnd: flatShapes.length });
  }

  const lines = [];
  lines.push(`// Generated by orchestration/tasks/TASK-0141-procedural-native-visual-kit/generate-assets.mjs version ${GENERATOR_VERSION}. DO NOT EDIT.`);
  lines.push('#pragma once');
  lines.push('');
  lines.push('#include <cstdint>');
  lines.push('');
  lines.push('namespace verdigris::visual_kit {');
  lines.push('');
  lines.push(`inline constexpr char kKitVersion[] = "${GENERATOR_VERSION}";`);
  lines.push('');
  lines.push('enum class ShapeKind : int32_t {');
  lines.push('  Polygon,');
  lines.push('  Polyline,');
  lines.push('  Circle,');
  lines.push('  Ellipse,');
  lines.push('};');
  lines.push('');
  lines.push('struct Color {');
  lines.push('  float r;');
  lines.push('  float g;');
  lines.push('  float b;');
  lines.push('  float a;');
  lines.push('};');
  lines.push('');
  lines.push('struct Shape {');
  lines.push('  ShapeKind kind;');
  lines.push('  int32_t point_begin;');
  lines.push('  int32_t point_end;');
  lines.push('  int32_t fill;');
  lines.push('  int32_t stroke;');
  lines.push('  float stroke_width;');
  lines.push('  float cx;');
  lines.push('  float cy;');
  lines.push('  float rx;');
  lines.push('  float ry;');
  lines.push('};');
  lines.push('');
  lines.push('struct Symbol {');
  lines.push('  const char* role;');
  lines.push('  const char* motif;');
  lines.push('  const char* source;');
  lines.push('  float width;');
  lines.push('  float height;');
  lines.push('  int32_t shape_begin;');
  lines.push('  int32_t shape_end;');
  lines.push('};');
  lines.push('');
  lines.push('inline constexpr Color kColors[] = {');
  for (const hex of colors) {
    const [r, g, b, a] = hexToColor(hex);
    lines.push(`    {${cppFloat(r)}, ${cppFloat(g)}, ${cppFloat(b)}, ${cppFloat(a)}},`);
  }
  lines.push('};');
  lines.push('');
  lines.push('inline constexpr float kPoints[] = {');
  for (let i = 0; i < flatPoints.length; i += 8) {
    const slice = flatPoints.slice(i, i + 8).map(cppFloat).join(', ');
    lines.push(`    ${slice},`);
  }
  lines.push('};');
  lines.push('');
  lines.push('inline constexpr Shape kShapes[] = {');
  for (const shape of flatShapes) {
    lines.push(
      `    {ShapeKind::${shape.kind}, ${shape.pointBegin}, ${shape.pointEnd}, ` +
      `${shape.fill}, ${shape.stroke}, ${cppFloat(shape.sw)}, ${cppFloat(shape.cx)}, ` +
      `${cppFloat(shape.cy)}, ${cppFloat(shape.rx)}, ${cppFloat(shape.ry)}},`,
    );
  }
  lines.push('};');
  lines.push('');
  lines.push('inline constexpr Symbol kSymbols[] = {');
  for (const { variant, shapeBegin, shapeEnd } of symbols) {
    const sourcePath = `native/client/assets/${variant.source}`;
    lines.push(
      `    {"${variant.role}", "${variant.motif}", "${sourcePath}", ` +
      `${cppFloat(VIEW_WIDTH)}, ${cppFloat(VIEW_HEIGHT)}, ${shapeBegin}, ${shapeEnd}},`,
    );
  }
  lines.push('};');
  lines.push('');
  lines.push(`inline constexpr int32_t kSymbolCount = ${symbols.length};`);
  lines.push('');
  lines.push('}');
  return `${lines.join('\n')}\n`;
}

function manifestText(variants) {
  const roles = [];
  for (const variant of variants) {
    let entry = roles.find((candidate) => candidate.role === variant.role);
    if (!entry) {
      entry = { role: variant.role, motifs: [] };
      roles.push(entry);
    }
    entry.motifs.push({
      motif: variant.motif,
      symbol: variant.symbol,
      source: `native/client/assets/${variant.source}`,
      palette: variant.palette,
    });
  }
  const manifest = {
    generatorVersion: GENERATOR_VERSION,
    viewBox: { width: VIEW_WIDTH, height: VIEW_HEIGHT },
    roles,
  };
  return `${JSON.stringify(manifest, null, 2)}\n`;
}

export function buildKit() {
  const variants = buildVariants();
  const files = [];
  for (const variant of variants) {
    files.push({
      path: path.posix.join('native/client/assets', variant.source),
      text: svgText(variant.shapes),
    });
  }
  files.push({
    path: 'native/client/assets/manifest.json',
    text: manifestText(variants),
  });
  files.push({
    path: 'native/client/assets/generated/visual_kit.h',
    text: headerText(variants),
  });
  return { files, manifest: JSON.parse(manifestText(variants)) };
}

function repoRoot() {
  const taskDir = path.dirname(fileURLToPath(import.meta.url));
  return path.resolve(taskDir, '..', '..', '..');
}

function run(argv) {
  const checkOnly = argv.includes('--check');
  const root = repoRoot();
  const kit = buildKit();
  let failures = 0;
  for (const file of kit.files) {
    const absolute = path.join(root, file.path);
    if (checkOnly) {
      if (!existsSync(absolute)) {
        process.stdout.write(`MISSING ${file.path}\n`);
        failures += 1;
        continue;
      }
      const actual = readFileSync(absolute, 'utf8');
      if (actual !== file.text) {
        process.stdout.write(`STALE ${file.path}\n`);
        failures += 1;
      } else {
        process.stdout.write(`OK ${file.path}\n`);
      }
    } else {
      mkdirSync(path.dirname(absolute), { recursive: true });
      writeFileSync(absolute, file.text, 'utf8');
      process.stdout.write(`WROTE ${file.path}\n`);
    }
  }
  if (failures > 0) {
    process.stdout.write(`generate-assets: ${failures} file(s) out of date\n`);
    return 1;
  }
  process.stdout.write(checkOnly ? 'visual kit up to date\n' : 'visual kit written\n');
  return 0;
}

if (process.argv[1] && import.meta.url === pathToFileURL(process.argv[1]).href) {
  process.exit(run(process.argv.slice(2)));
}
