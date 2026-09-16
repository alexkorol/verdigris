import { mkdirSync, readFileSync, writeFileSync, existsSync } from 'node:fs';
import path from 'node:path';
import process from 'node:process';
import { fileURLToPath, pathToFileURL } from 'node:url';

export const GENERATOR_VERSION = 'task0147-gen-1';

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
    // Layered cape behind the figure for depth.
    polygon([[20, 26], [44, 26], [47, 50], [38, 47], [32, 52], [26, 47], [17, 50]], '#1d355f'),
    // Torso with a shaded right half.
    polygon([[22, 27], [42, 27], [44, 51], [37, 48], [32, 53], [27, 48], [20, 51]], '#24427c'),
    polygon([[32, 27], [42, 27], [44, 51], [37, 48], [32, 53]], '#1b3159'),
    // Legs with lit leading edges.
    polygon([[26, 42], [31, 42], [30, 55], [25, 55]], '#37475e'),
    polygon([[33, 42], [38, 42], [39, 55], [34, 55]], '#37475e'),
    polygon([[26, 42], [28, 42], [27.4, 55], [25, 55]], '#46597a'),
    polygon([[33, 42], [35, 42], [34.4, 55], [33, 55]], '#46597a'),
    // Boots and leather cuffs.
    polygon([[23, 53], [30, 53], [30, 58], [23, 58]], '#231a10'),
    polygon([[34, 53], [41, 53], [41, 58], [34, 58]], '#231a10'),
    polygon([[24.2, 52], [30.2, 52], [30, 54], [24, 54]], '#5b3a1e'),
    polygon([[33.8, 52], [39.8, 52], [40, 54], [34, 54]], '#5b3a1e'),
    // Belt with a gold buckle.
    polygon([[25, 41], [39, 41], [39, 45], [25, 45]], '#5b3a1e'),
    circle(32, 43, 1.7, '#d9a441'),
    // Cuirass: steel plate, lit panel, verdigris tabard stripe and emblem.
    polygon([[24, 29], [40, 29], [38.5, 40], [25.5, 40]], '#93a7bd'),
    polygon([[24, 29], [31, 29], [30, 40], [25.5, 40]], '#b7c9da'),
    polygon([[30, 29], [34, 29], [33.6, 40], [30.4, 40]], '#2c5f57'),
    polygon([[32, 32.5], [33.6, 35], [32, 37.5], [30.4, 35]], '#59d6c9'),
    polygon([[24, 29], [40, 29], [39.6, 31.2], [24.4, 31.2]], '#aabdd2'),
    // Pauldrons with rivet sparks.
    circle(22, 29, 4.5, '#93a7bd'),
    circle(42, 29, 4.5, '#93a7bd'),
    circle(20.8, 27.8, 1, '#dfe8ef'),
    circle(43.2, 27.8, 1, '#dfe8ef'),
    // Sword arm, gauntlet, blade with fuller line, guard and pommel.
    polygon([[40.5, 30], [46, 27], [47.4, 30], [42, 33.4]], '#93a7bd'),
    polygon([[43.6, 29.6], [48.4, 31.6], [46.8, 34.8], [42, 32.6]], '#5b3a1e'),
    polygon([[46.5, 31], [56.5, 18.5], [59.5, 20.5], [49.5, 33]], '#dfe8ef'),
    polyline([[48.4, 30.4], [56, 20.6]], '#9fb4c6', 1.1),
    polygon([[43.8, 28.2], [49.8, 30.4], [48.8, 33.2], [42.8, 31]], '#8a6a2c'),
    circle(43.2, 33.4, 1.6, '#d9a441'),
    // Helm with visor, nose guard, sheen and jaw guard.
    circle(32, 19, 7, '#7f93a9'),
    polygon([[26, 18], [38, 18], [38, 22], [26, 22]], '#101720'),
    polygon([[31, 14.5], [33, 14.5], [33, 23], [31, 23]], '#67798d'),
    polygon([[26.6, 15.6], [30, 13.4], [30.6, 15.4], [27.6, 17.4]], '#aabdd2'),
    polygon([[27, 22], [37, 22], [36, 24.6], [28, 24.6]], '#5f7186'),
    // Flowing verdigris plume with its gilded base ring.
    polygon([[29.4, 12.4], [33, 11.2], [35.4, 5.6], [39.6, 3.4], [37, 9.6], [33.6, 13.2]], '#3e8f83'),
    polygon([[30.6, 12.2], [33, 11.4], [34.8, 7.6], [37, 6.2], [34.8, 10.8]], '#59d6c9'),
    circle(31.4, 12.8, 1.8, '#d9a441'),
  ];
}

function buildRaider(rng) {
  void rng;
  return [
    shadow(33, 13),
    // Lashing tail behind the stance.
    polyline([[44, 44], [50, 47], [53, 53]], '#5c231b', 2),
    // Hunched back silhouette then lit body mass with shaded right half.
    polygon([[19, 30], [44, 26], [48, 48], [40, 46], [34, 52], [27, 47], [21, 52], [16, 48]], '#4a1f18'),
    polygon([[20, 30], [44, 28], [46, 48], [41, 46], [35, 51], [28, 47], [23, 51], [18, 49]], '#6e2a22'),
    polygon([[31, 29], [44, 28], [46, 48], [41, 46], [35, 51]], '#5c231b'),
    // Chest wrap with strap lines.
    polygon([[24, 30], [42, 27], [44, 38], [26, 41]], '#8a4a2f'),
    polyline([[25, 33], [43, 30.4]], '#6b3520', 1.2),
    polyline([[25.4, 37.4], [43.4, 34.6]], '#6b3520', 1.2),
    // Bone trophy necklace across the wrap.
    circle(28, 34.4, 1, '#d9c9a3'),
    circle(32.4, 33.8, 1, '#d9c9a3'),
    circle(36.8, 33.2, 1, '#d9c9a3'),
    // Jagged fur collar around the shoulders.
    polygon([[22.6, 29.4], [26, 25.6], [29.6, 29], [33.6, 25], [37.6, 28.6], [41, 26], [42.6, 30], [23, 31.6]], '#3c2f26'),
    // Loincloth with a bone talisman button.
    polygon([[24, 44], [34, 44], [33, 52], [23, 52]], '#5c2d1c'),
    polygon([[24, 44], [29, 44], [28.6, 52], [23, 52]], '#6b3a24'),
    circle(28.6, 48, 1.3, '#d9c9a3'),
    // Digitigrade legs, shaded far leg, heavy feet on the ground line.
    polygon([[23, 47], [29.6, 47], [28.6, 54.6], [22.6, 54.6]], '#33241b'),
    polygon([[35, 46], [41.6, 45], [42.6, 52], [36, 52.6]], '#2a1c14'),
    polygon([[21.6, 54], [27.4, 54], [27.8, 58], [20.6, 58]], '#241710'),
    polygon([[36, 51.4], [42.4, 51], [43.6, 58], [37.2, 57.4]], '#1f130c'),
    // Off-hand forearm with a dark bracer.
    polygon([[18, 32], [23, 31], [23.6, 36], [18.6, 37]], '#7c3e28'),
    polygon([[16.6, 36], [21.6, 35.4], [22, 39.6], [17, 40]], '#3a2a1a'),
    // Weapon arm, wrapped fist, wood haft and a broad steel axe head.
    polygon([[38, 31], [47, 31.6], [47, 36.6], [38, 36.4]], '#7c3e28'),
    polygon([[44, 33], [50.6, 33.4], [50.4, 37.8], [43.8, 37.4]], '#3a2a1a'),
    polyline([[48.6, 35.2], [59.4, 31.6]], '#6b4a26', 2.2),
    polygon([[47.4, 23.4], [61.4, 27.4], [60.4, 34.4], [51.6, 33.6], [48.8, 30.4], [46.6, 31.4]], '#b7bcc2'),
    polygon([[48.6, 24.6], [60.2, 28], [59.8, 30.2], [49.2, 27]], '#d7dde2'),
    polyline([[49.4, 32.4], [59, 29.4]], '#8f979e', 1),
    // Hyena profile head: muzzle, teeth flash, ears and brow over amber eyes.
    polygon([[25, 21.4], [31, 17.4], [38, 16], [42.4, 19.4], [41.4, 24], [35, 27.4], [27, 26.4]], '#7a4a30'),
    polygon([[38, 19.4], [44.4, 21], [42, 25], [38.4, 23.8]], '#8f5a3a'),
    polygon([[41, 21.6], [44, 22], [43.6, 23.4], [40.8, 23]], '#e8e0d0'),
    polygon([[27.4, 16.4], [30.4, 13.4], [31.6, 17]], '#4a2c1c'),
    polygon([[31.8, 15.6], [34.4, 13], [35.4, 16.8]], '#4a2c1c'),
    polyline([[31.6, 19.4], [35, 18.8], [38.6, 19.2]], '#3c2216', 1.2),
    circle(33.4, 21.4, 1.5, '#ffcf5e'),
    circle(37.6, 20.8, 1.5, '#ffcf5e'),
  ];
}

function buildElite(rng) {
  void rng;
  return [
    shadow(32, 16),
    // Sweeping cape with a shaded trailing half.
    polygon([[16, 22], [48, 22], [52, 54], [41, 50], [32, 56], [23, 50], [12, 54]], '#4a1016'),
    polygon([[32, 22], [48, 22], [52, 54], [41, 50], [32, 56]], '#3a0c11'),
    // Greaves with lit leading edges.
    polygon([[25, 44], [31, 44], [30, 56], [24, 56]], '#3c1218'),
    polygon([[33, 44], [39, 44], [40, 56], [34, 56]], '#3c1218'),
    polygon([[25, 44], [27.6, 44], [27, 56], [24, 56]], '#4d1822'),
    polygon([[33, 44], [35.6, 44], [34.6, 56], [34, 56]], '#4d1822'),
    // Sabatons with gilded trim on the ground line.
    polygon([[23, 54], [30, 54], [30, 58], [23, 58]], '#1c0a0d'),
    polygon([[34, 54], [41, 54], [41, 58], [34, 58]], '#1c0a0d'),
    polygon([[23, 54], [30, 54], [30, 55.2], [23, 55.2]], '#d9a441'),
    polygon([[34, 54], [41, 54], [41, 55.2], [34, 55.2]], '#d9a441'),
    // Cuirass: lit left plate, gilt center ridge, modeled pecs and belt line.
    polygon([[21, 25], [43, 25], [41, 46], [23, 46]], '#7a2230'),
    polygon([[21, 25], [32, 25], [31, 46], [23, 46]], '#8c2a3a'),
    polygon([[31, 25], [33, 25], [33, 46], [31, 46]], '#d9a441'),
    polygon([[23.4, 27], [31, 27], [30.6, 33], [24, 33]], '#9c3343'),
    polygon([[33, 27], [40.6, 27], [40, 33], [33.4, 33]], '#6b1a26'),
    polyline([[24.6, 36], [39.4, 36]], '#5d1420', 1.1),
    circle(27, 38.4, 1.1, '#d9a441'),
    circle(37, 38.4, 1.1, '#d9a441'),
    // Gilded-rim pauldrons with molten rivets.
    circle(20, 26, 5.4, '#d9a441'),
    circle(44, 26, 5.4, '#d9a441'),
    circle(20, 26, 4.5, '#8c2a3a'),
    circle(44, 26, 4.5, '#8c2a3a'),
    circle(18.8, 24.8, 1.2, '#ffb84d'),
    circle(45.2, 24.8, 1.2, '#ffb84d'),
    // Horned helm: crown band over a dark face recess.
    polygon([[25, 10], [39, 10], [41, 21], [23, 21]], '#5d1620'),
    polygon([[26.4, 13], [37.6, 13], [37, 20], [27, 20]], '#200a0e'),
    polygon([[25, 10], [39, 10], [40, 13], [24, 13]], '#7a2230'),
    // Curved horns with inner shading.
    polygon([[24.6, 12], [12, 4], [25.6, 18]], '#d9a441'),
    polygon([[23.4, 12.6], [15.4, 7.2], [24, 15.8]], '#b3862f'),
    polygon([[39.4, 12], [52, 4], [38.4, 18]], '#d9a441'),
    polygon([[40.6, 12.6], [48.6, 7.2], [40, 15.8]], '#b3862f'),
    // Burning eyes with soft halos.
    circle(29, 16.4, 2.6, '#ff5d3a30'),
    circle(35, 16.4, 2.6, '#ff5d3a30'),
    circle(29, 16.4, 1.5, '#ff5d3a'),
    circle(35, 16.4, 1.5, '#ff5d3a'),
    // Greatsword: lit edge, dark fuller, gilt guard, wrapped grip, ember pommel.
    polygon([[44, 33], [58, 22], [61, 25], [47, 36]], '#9aa1a8'),
    polygon([[46, 32.2], [57.6, 23], [59, 24.4], [47.4, 33.6]], '#cfd6dc'),
    polyline([[46.6, 33.8], [57.4, 25.2]], '#5f666d', 1),
    polygon([[42, 30], [47.4, 34.6], [45.4, 37], [40, 32.4]], '#d9a441'),
    polygon([[38.6, 33.8], [42.4, 37], [40.4, 39], [36.6, 35.8]], '#3a2a1a'),
    circle(37.4, 37.4, 1.5, '#d9a441'),
    circle(37.4, 37.4, 0.8, '#ff5d3a'),
    // Belt trophy skulls.
    circle(24.6, 40.2, 1.2, '#d9c9a3'),
    circle(39, 40.2, 1.2, '#d9c9a3'),
  ];
}

function buildTree(rng) {
  const shapes = [
    shadow(32, 15),
    // Tapered trunk with bark ridges and flared roots.
    polygon([[29, 42], [35, 42], [38, 58], [26, 58]], '#5a4030'),
    polyline([[31, 44], [30.4, 57]], '#4a3526', 1.2),
    polyline([[33.4, 44], [34, 57]], '#4a3526', 1.2),
    polygon([[24, 55], [29, 52.6], [30, 58], [24, 58]], '#4a3526'),
    polygon([[34, 52.6], [40, 55], [40, 58], [34, 58]], '#4a3526'),
    polygon([[29.6, 55], [34.4, 55], [33.6, 58], [30.4, 58]], '#4a3526'),
    // Three stacked canopy layers, darkest to lightest.
    polygon([
      [32, 6], [46, 26], [41, 24.6], [50, 39], [42.6, 36.6], [52, 50],
      [12, 50], [21.4, 36.6], [14, 39], [23, 24.6], [18, 26],
    ], '#245830'),
    polygon([
      [32, 11], [42.6, 27.6], [38.6, 26.6], [44.6, 37.6], [38.6, 35.6], [45.4, 46],
      [18.6, 46], [25.4, 35.6], [19.4, 37.6], [25.4, 26.6], [21.4, 27.6],
    ], '#2f6b3a'),
    polygon([
      [32, 16], [39.4, 30], [36.4, 29], [41, 38], [36, 36.4], [40.4, 44],
      [23.6, 44], [28, 36.4], [23, 38], [27.6, 29], [24.6, 30],
    ], '#3d8a49'),
    // Verdigris lichen strands hanging from the lower boughs.
    polyline([[26, 44], [25.4, 47.6]], '#4e9e8f', 1),
    polyline([[38, 43], [38.8, 46.6]], '#4e9e8f', 1),
  ];
  for (let i = 0; i < 9; i += 1) {
    const angle = rng() * Math.PI * 2;
    const radius = 4 + rng() * 10;
    const x = 32 + Math.cos(angle) * radius * 1.4;
    const y = 30 + Math.sin(angle) * radius * 0.7;
    if (y < 14 || y > 47 || x < 15 || x > 49) continue;
    shapes.push(circle(Math.round(x * 2) / 2, Math.round(y * 2) / 2, 1 + rng() * 1.4, '#57a75f'));
  }
  for (let i = 0; i < 4; i += 1) {
    const x = 19 + rng() * 14;
    const y = 17 + rng() * 12;
    if (y < 15 || y > 31 || x < 18 || x > 34) continue;
    shapes.push(circle(Math.round(x * 2) / 2, Math.round(y * 2) / 2, 0.8 + rng() * 0.8, '#79c46f'));
  }
  for (let i = 0; i < 3; i += 1) {
    const x = 34 + rng() * 11;
    const y = 20 + rng() * 13;
    if (y < 19 || y > 34 || x < 33 || x > 46) continue;
    shapes.push(circle(Math.round(x * 2) / 2, Math.round(y * 2) / 2, 0.9 + rng() * 0.4, '#bff3ec40'));
  }
  return shapes;
}

function buildRuin(rng) {
  void rng;
  return [
    shadow(32, 20),
    // Tall broken wall with jagged top and a lit left strip.
    polygon([[10, 26], [15, 23], [18, 26.6], [21, 22.6], [24, 25], [25, 58], [11, 58]], '#84868b'),
    polygon([[10, 26], [12.6, 24.8], [13.4, 58], [10, 58]], '#9a9ca1'),
    // Masonry course joints and a weathering crack.
    polyline([[11.6, 34], [24.4, 33.4]], '#6f7176', 1),
    polyline([[11.4, 42], [24.6, 41.4]], '#6f7176', 1),
    polyline([[11.2, 50], [24.8, 49.4]], '#6f7176', 1),
    polyline([[20.6, 30], [19, 38], [20.8, 45]], '#5f6165', 1),
    // Shorter stub wall with a shaded fracture face.
    polygon([[41, 37], [45.4, 33.6], [47.6, 36.6], [51, 32.6], [52, 58], [41, 58]], '#76787d'),
    polygon([[48.6, 33.4], [51, 32.6], [52, 58], [48.8, 58]], '#66686d'),
    // Fallen lintel bridging the walls, top-lit with a fractured end.
    polygon([[21.6, 45.6], [43.6, 37.6], [46.6, 44.6], [24.6, 52.6]], '#8d8f94'),
    polygon([[21.6, 45.6], [43.6, 37.6], [44.6, 40], [22.6, 48]], '#a3a5aa'),
    polyline([[21.6, 45.6], [43.6, 37.6], [46.6, 44.6], [24.6, 52.6], [21.6, 45.6]], '#5f6165', 1.4),
    polygon([[43.6, 37.6], [46.6, 44.6], [44, 45.6], [41.8, 39.4]], '#6f7176'),
    // Standing column drum with cap and flutes.
    polygon([[14.6, 44], [21.4, 44], [22.4, 56], [13.6, 56]], '#9a9ca1'),
    ellipse(18.4, 44, 3.9, 1.5, '#aeb1b6'),
    polyline([[16.4, 46], [16, 55]], '#7f8186', 0.9),
    polyline([[18.6, 46], [18.4, 55]], '#7f8186', 0.9),
    polyline([[20.6, 46], [20.8, 55]], '#7f8186', 0.9),
    // Rubble chunks and pebbles.
    polygon([[27, 53.6], [31, 52.4], [32.4, 56], [28, 57.4]], '#7a7c81'),
    polygon([[35, 50.6], [38.6, 49.8], [39.6, 53.4], [35.8, 54.2]], '#6f7176'),
    polygon([[44, 52.6], [47.6, 52], [48.4, 55.4], [44.6, 56]], '#66686d'),
    circle(25.4, 56.4, 1.1, '#5f6165'),
    circle(33.4, 57, 0.9, '#66686d'),
    circle(40.4, 56.8, 1, '#5f6165'),
    circle(47, 57.2, 0.8, '#66686d'),
    // Moss colonizing the stone and grass tufts in the cracks.
    circle(13.4, 56.2, 1.8, '#557d4a'),
    circle(15.4, 57.4, 1.4, '#6b9a5a'),
    circle(22, 24.4, 1.5, '#557d4a'),
    circle(45.6, 34.4, 1.3, '#557d4a'),
    circle(46.6, 36, 1, '#6b9a5a'),
    circle(19.6, 44.2, 1.2, '#557d4a'),
    polygon([[30.4, 58], [31.4, 54.6], [32.4, 58]], '#5d8a5a'),
    polygon([[42.4, 58], [43.4, 55.4], [44.4, 58]], '#5d8a5a'),
    // Weathered verdigris rune plaque set into the old wall.
    polygon([[16.4, 33.4], [20.4, 33.4], [20.4, 37.4], [16.4, 37.4]], '#3e8f83'),
    polygon([[17.6, 34.6], [19.2, 34.6], [19.2, 36.2], [17.6, 36.2]], '#2c5f57'),
  ];
}

function buildDwelling(rng) {
  void rng;
  return [
    shadow(32, 19),
    // Stone footing with masonry joints.
    polygon([[15, 52], [49, 52], [49, 58], [15, 58]], '#84868b'),
    polyline([[15, 54.8], [49, 54.8]], '#6f7176', 1),
    polyline([[24, 53.2], [24, 58]], '#6f7176', 1),
    polyline([[33, 53.2], [33, 58]], '#6f7176', 1),
    polyline([[42, 53.2], [42, 58]], '#6f7176', 1),
    // Daub walls with a shaded right side.
    polygon([[16, 34], [48, 34], [48, 53], [16, 53]], '#cbb089'),
    polygon([[40, 34], [48, 34], [48, 53], [40, 53]], '#b89e77'),
    // Timber frame: posts, beams, rail and diagonal braces.
    polygon([[16, 34], [19, 34], [19, 53], [16, 53]], '#4a3623'),
    polygon([[45, 34], [48, 34], [48, 53], [45, 53]], '#4a3623'),
    polygon([[16, 34], [48, 34], [48, 37], [16, 37]], '#4a3623'),
    polygon([[16, 43], [48, 43], [48, 45.4], [16, 45.4]], '#4a3623'),
    polygon([[19.4, 37.4], [22, 37.4], [19.4, 42.6], [19, 42]], '#4a3623'),
    polygon([[44.6, 37.4], [42, 37.4], [44.6, 42.6], [45, 42]], '#4a3623'),
    // Thatched roof: dark under-eave, lit face, texture rows and rolled ridge.
    polygon([[12, 34], [52, 34], [32, 12]], '#8a6a2c'),
    polygon([[14, 33.4], [50, 33.4], [32, 13]], '#b98f3e'),
    polyline([[18.6, 29.4], [45.4, 29.4]], '#8a6a2c', 1),
    polyline([[23.4, 24], [40.6, 24]], '#8a6a2c', 1),
    polyline([[27.6, 19], [36.4, 19]], '#8a6a2c', 1),
    polygon([[27.4, 15], [36.6, 15], [33.4, 11.4], [30.6, 11.4]], '#caa04a'),
    // Stone chimney with a cap and a wisp of smoke.
    polygon([[41, 25.4], [45.4, 23.6], [45.4, 15.4], [41.4, 17.2]], '#76787d'),
    polygon([[40.8, 16.4], [45.8, 14.4], [45.8, 12.6], [40.8, 14.6]], '#8d8f94'),
    circle(44.6, 11.4, 1.6, '#d9c9a340'),
    circle(46, 8.8, 1.2, '#d9c9a330'),
    // Plank door with iron hinges and a round handle.
    polygon([[28, 42.6], [36, 42.6], [36, 58], [28, 58]], '#3c2a18'),
    polyline([[30.6, 43.6], [30.6, 57]], '#2a1d0f', 0.9),
    polyline([[33.4, 43.6], [33.4, 57]], '#2a1d0f', 0.9),
    polyline([[28, 46], [30.2, 46]], '#6f7176', 1.1),
    polyline([[28, 50], [30.2, 50]], '#6f7176', 1.1),
    circle(34.6, 51, 0.9, '#d9a441'),
    // Cross-mullioned windows glowing warm.
    polygon([[20, 38.4], [26.6, 38.4], [26.6, 45], [20, 45]], '#4a3623'),
    polygon([[21, 39.4], [25.6, 39.4], [25.6, 44], [21, 44]], '#ffd98a'),
    polyline([[23.3, 39.4], [23.3, 44]], '#4a3623', 0.8),
    polyline([[21, 41.7], [25.6, 41.7]], '#4a3623', 0.8),
    polygon([[37.4, 38.4], [44, 38.4], [44, 45], [37.4, 45]], '#4a3623'),
    polygon([[38.4, 39.4], [43, 39.4], [43, 44], [38.4, 44]], '#ffd98a'),
    polyline([[40.7, 39.4], [40.7, 44]], '#4a3623', 0.8),
    polyline([[38.4, 41.7], [43, 41.7]], '#4a3623', 0.8),
    // Window planter with foliage and blossoms.
    polygon([[19.4, 47.8], [25, 47.8], [25, 49.4], [19.4, 49.4]], '#5a4030'),
    circle(20.4, 47, 0.8, '#6fae67'),
    circle(22.2, 47.2, 0.8, '#6fae67'),
    circle(24, 47, 0.8, '#6fae67'),
    circle(21.3, 46.2, 0.7, '#c96a6a'),
    circle(23.1, 46.4, 0.7, '#e8e4c8'),
  ];
}

function buildShrine(rng) {
  void rng;
  return [
    shadow(32, 18),
    // Three stepped slabs with beveled top light on the upper courses.
    polygon([[12, 54], [52, 54], [54, 58], [10, 58]], '#8f959d'),
    polygon([[16, 50], [48, 50], [50, 54], [14, 54]], '#9aa1a8'),
    polygon([[16, 50], [48, 50], [48.8, 51.4], [15.2, 51.4]], '#a8aeb6'),
    polygon([[20, 46.4], [44, 46.4], [46, 50], [18, 50]], '#a5abb2'),
    polygon([[20, 46.4], [44, 46.4], [45, 47.8], [19, 47.8]], '#b9bec6'),
    // Soft aura behind the centerpiece, over the dark backdrop only.
    circle(32, 38, 7.5, '#59d6c926'),
    // Fluted pillars with shaded strips and cap stones.
    polygon([[22, 28], [26.4, 28], [26.4, 46.4], [22, 46.4]], '#b9bec6'),
    polygon([[24.8, 28], [26.4, 28], [26.4, 46.4], [24.8, 46.4]], '#9ba1a9'),
    polyline([[23.2, 29.4], [23.2, 43.6]], '#9ba1a9', 0.8),
    polygon([[21.2, 26.6], [27.2, 26.6], [27.2, 28.4], [21.2, 28.4]], '#c6cbd2'),
    polygon([[21.2, 44.6], [27.2, 44.6], [27.2, 46.4], [21.2, 46.4]], '#c6cbd2'),
    polygon([[37.6, 28], [42, 28], [42, 46.4], [37.6, 46.4]], '#b9bec6'),
    polygon([[37.6, 28], [39.2, 28], [39.2, 46.4], [37.6, 46.4]], '#9ba1a9'),
    polyline([[40.8, 29.4], [40.8, 43.6]], '#9ba1a9', 0.8),
    polygon([[36.8, 26.6], [42.8, 26.6], [42.8, 28.4], [36.8, 28.4]], '#c6cbd2'),
    polygon([[36.8, 44.6], [42.8, 44.6], [42.8, 46.4], [36.8, 46.4]], '#c6cbd2'),
    // Entablature with dentil blocks under the pediment.
    polygon([[18, 22.4], [46, 22.4], [46, 26.6], [18, 26.6]], '#c6cbd2'),
    polygon([[20.6, 25], [22.2, 25], [22.2, 26.6], [20.6, 26.6]], '#9ba1a9'),
    polygon([[26.2, 25], [27.8, 25], [27.8, 26.6], [26.2, 26.6]], '#9ba1a9'),
    polygon([[31.8, 25], [33.4, 25], [33.4, 26.6], [31.8, 26.6]], '#9ba1a9'),
    polygon([[37.4, 25], [39, 25], [39, 26.6], [37.4, 26.6]], '#9ba1a9'),
    // Pediment with recessed tympanum and a verdigris sun medallion.
    polygon([[16, 22.4], [32, 13.6], [48, 22.4]], '#d2d7de'),
    polygon([[20.6, 21.4], [32, 15.4], [43.4, 21.4]], '#aeb4bc'),
    circle(32, 19.6, 2.4, '#4e9e8f'),
    circle(32, 19.6, 1.6, '#59d6c9'),
    circle(32, 19.6, 0.7, '#bff3ec'),
    // Brazier pedestal, bowl and rim.
    polygon([[30, 44], [34, 44], [34.8, 46.4], [29.2, 46.4]], '#6f7680'),
    polygon([[27.4, 42.4], [36.6, 42.4], [34.6, 46], [29.4, 46]], '#7a8188'),
    ellipse(32, 42.4, 4.6, 1.1, '#8f959d'),
    // Layered teal flame with pale core and drifting sparks.
    polygon([[29.4, 42], [31, 37.4], [30, 35.4], [32, 31.4], [33.6, 34.6], [34.8, 36.4], [34.6, 42]], '#59d6c9'),
    polygon([[30.8, 42], [31.8, 38.6], [32.6, 40], [33.2, 42]], '#bff3ec'),
    circle(34.4, 33.4, 0.8, '#bff3ecb0'),
    circle(29.8, 35.8, 0.7, '#bff3ec90'),
    // Votive candles flanking the brazier.
    polygon([[24.4, 43.4], [25.6, 43.4], [25.6, 46.4], [24.4, 46.4]], '#c6cbd2'),
    circle(25, 42.6, 0.7, '#ffd98a'),
    polygon([[38.4, 43.4], [39.6, 43.4], [39.6, 46.4], [38.4, 46.4]], '#c6cbd2'),
    circle(39, 42.6, 0.7, '#ffd98a'),
    // Inscription dashes on the ground step.
    polyline([[22, 56.4], [26, 56.4]], '#6f7680', 0.9),
    polyline([[30, 56.4], [36, 56.4]], '#6f7680', 0.9),
    polyline([[40, 56.4], [44, 56.4]], '#6f7680', 0.9),
  ];
}

function buildTerrainA(rng) {
  const shapes = [
    polygon([[0, 0], [VIEW_WIDTH, 0], [VIEW_WIDTH, VIEW_HEIGHT], [0, VIEW_HEIGHT]], '#46653f'),
  ];
  // Broad low-contrast mottling so the court does not read flat.
  const patches = [
    [[8, 10], [26, 7], [30, 20], [12, 24]],
    [[36, 30], [56, 27], [58, 42], [40, 46]],
    [[10, 40], [26, 38], [28, 52], [12, 55]],
    [[42, 6], [58, 9], [56, 22], [44, 20]],
    [[28, 48], [46, 50], [44, 60], [30, 58]],
  ];
  const patchFills = ['#4d7044', '#3f5c39', '#4a6c42', '#52754a', '#3d5936'];
  for (let i = 0; i < patches.length; i += 1) {
    const jittered = patches[i].map(([x, y]) => [
      Math.round((x + (rng() - 0.5) * 2) * 2) / 2,
      Math.round((y + (rng() - 0.5) * 2) * 2) / 2,
    ]);
    shapes.push(polygon(jittered, patchFills[i]));
  }
  // Worn diagonal path with a lit inner swath and scattered grit.
  shapes.push(polygon([[24, VIEW_HEIGHT], [40, VIEW_HEIGHT], [52, 40], [44, 38]], '#5d6b4f'));
  shapes.push(polygon([[28, VIEW_HEIGHT], [36, VIEW_HEIGHT], [47, 41], [43, 40]], '#6b7562'));
  for (let i = 0; i < 5; i += 1) {
    const t = 0.15 + rng() * 0.7;
    const x = Math.round(30 + t * 18 + (rng() - 0.5) * 3);
    const y = Math.round(VIEW_HEIGHT - t * 22 + (rng() - 0.5) * 3);
    shapes.push(circle(x, y, 0.8 + rng() * 0.6, i % 2 === 0 ? '#78806e' : '#596058'));
  }
  // Grass blade clusters in three greens.
  for (let i = 0; i < 24; i += 1) {
    const x = 5 + rng() * 54;
    const y = 6 + rng() * 50;
    if (x < 5 || x > 57 || y < 6 || y > 56) continue;
    const fill = i % 3 === 0 ? '#6fae67' : i % 3 === 1 ? '#5f8a52' : '#547d49';
    shapes.push(polygon(
      [[Math.round(x), Math.round(y)], [Math.round(x) + 0.9, Math.round(y) - 2.2], [Math.round(x) + 1.8, Math.round(y)]],
      fill,
    ));
  }
  // Clover dots.
  for (let i = 0; i < 6; i += 1) {
    const x = 7 + rng() * 50;
    const y = 8 + rng() * 48;
    shapes.push(circle(Math.round(x), Math.round(y), 0.7, '#6fae67'));
  }
  // Two-tone stones: body, top light, under-shadow.
  for (let i = 0; i < 5; i += 1) {
    const x = 9 + rng() * 44;
    const y = 10 + rng() * 42;
    const w = 3 + rng() * 2.2;
    const h = 2.4 + rng() * 1.8;
    const body = i % 2 === 0 ? '#6b7060' : '#5c6154';
    shapes.push(polygon(
      [[x, y], [x + w, y + rng()], [x + w + 0.5, y + h], [x - 0.5, y + h + rng()]],
      body,
    ));
    shapes.push(polygon([[x, y], [x + w, y + rng() * 0.6], [x + w - 0.6, y + 0.9], [x + 0.4, y + 0.9]], '#7d8272'));
    shapes.push(polygon([[x - 0.5, y + h + rng() * 0.5], [x + w + 0.5, y + h], [x + w - 0.4, y + h + 0.7], [x - 0.2, y + h + 0.8]], '#4c5146'));
  }
  // A few meadow flowers.
  for (let i = 0; i < 3; i += 1) {
    const x = 10 + rng() * 42;
    const y = 12 + rng() * 40;
    shapes.push(circle(Math.round(x), Math.round(y), 1, '#e8e4c8'));
    shapes.push(circle(Math.round(x), Math.round(y), 0.45, '#d9a441'));
  }
  return shapes;
}

function buildTerrainB(rng) {
  const shapes = [
    polygon([[0, 0], [VIEW_WIDTH, 0], [VIEW_WIDTH, VIEW_HEIGHT], [0, VIEW_HEIGHT]], '#33504a'),
    // Corner vignettes settle the tile into the ground plane.
    polygon([[0, 0], [14, 0], [0, 14]], '#2e4741'),
    polygon([[50, 0], [VIEW_WIDTH, 0], [VIEW_WIDTH, 14]], '#2e4741'),
    polygon([[0, 50], [0, VIEW_HEIGHT], [14, VIEW_HEIGHT]], '#2e4741'),
    polygon([[VIEW_WIDTH, 50], [VIEW_WIDTH, VIEW_HEIGHT], [50, VIEW_HEIGHT]], '#2e4741'),
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
    // Top-left edge catches light; bottom-right edge falls to shade.
    shapes.push(polyline(
      [jitter[3], jitter[0], jitter[1]],
      '#54786d',
      1.2,
    ));
    shapes.push(polyline(
      [jitter[1], jitter[2], jitter[3]],
      '#3a564e',
      1.1,
    ));
  }
  // Settlement cracks across two flagstones.
  for (let i = 0; i < 2; i += 1) {
    const x = 12 + rng() * 36;
    const y = 10 + rng() * 34;
    shapes.push(polyline(
      [
        [Math.round(x * 2) / 2, Math.round(y * 2) / 2],
        [Math.round((x + 2.4 + rng() * 2) * 2) / 2, Math.round((y - 1.6 + rng()) * 2) / 2],
        [Math.round((x + 4.6 + rng() * 2.4) * 2) / 2, Math.round((y + 0.8 + rng()) * 2) / 2],
      ],
      '#2e463f',
      0.9,
    ));
  }
  // Moss clumps colonizing the joints in three greens.
  for (let i = 0; i < 12; i += 1) {
    const x = 6 + rng() * 52;
    const y = 6 + rng() * 52;
    const fill = i % 3 === 0 ? '#6fa06a' : i % 3 === 1 ? '#5d8a5a' : '#7ab06f';
    shapes.push(circle(Math.round(x), Math.round(y), 0.8 + rng() * 1, fill));
  }
  // Grit and a fallen twig for scale.
  for (let i = 0; i < 8; i += 1) {
    const x = 6 + rng() * 52;
    const y = 6 + rng() * 52;
    shapes.push(circle(Math.round(x), Math.round(y), 0.6 + rng() * 0.4, i % 2 === 0 ? '#596058' : '#6a7268'));
  }
  shapes.push(polyline([[40, 52], [46, 49.6]], '#5a4030', 1.1));
  shapes.push(polyline([[43, 50.8], [44.4, 52.4]], '#5a4030', 0.9));
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
      // Point spans are counted in vertices (pairs), matching the GDI
      // consumer which indexes kPoints[(point_begin + p) * 2] for each
      // vertex of the (point_end - point_begin)-vertex span.
      const pointBegin = flatPoints.length / 2;
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
        pointEnd: flatPoints.length / 2,
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
