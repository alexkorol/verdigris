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
    polygon([[24, 26], [20.5, 31], [18.5, 39], [21, 47], [25, 45], [27.5, 33], [27, 26]], '#1d3560'),
    polygon([[24, 29], [22, 35], [23, 43], [25.5, 41], [26, 32]], '#24427c'),
    polygon([[26, 42], [31, 42], [30.5, 50], [25.5, 50]], '#37475e'),
    polygon([[25.5, 50], [30.5, 50], [30.5, 55], [25.5, 55]], '#2b3849'),
    polygon([[25, 52.5], [31, 52.5], [31, 54.5], [25, 54.5]], '#93a7bd'),
    polygon([[24, 54], [31, 54], [31, 58], [24, 58]], '#231a10'),
    polygon([[33, 42], [38.5, 42], [38.5, 50], [33.5, 50]], '#41546e'),
    polygon([[33.5, 50], [38.5, 50], [38.5, 55], [33.5, 55]], '#37475e'),
    polygon([[33, 52.5], [39, 52.5], [39, 54.5], [33, 54.5]], '#a6b8cc'),
    polygon([[33, 54], [40, 54], [40, 58], [33, 58]], '#231a10'),
    polygon([[23, 27], [41, 27], [43, 45], [36, 43], [32, 46], [28, 43], [21, 45]], '#24427c'),
    polygon([[23, 27], [28.5, 27], [27.5, 43.5], [21, 45]], '#1b3159'),
    polygon([[21, 42.5], [43, 42.5], [43, 45], [36, 43], [32, 46], [28, 43], [21, 45]], '#3aa898'),
    polygon([[25, 43], [28.5, 43], [28, 47.5], [25.5, 47.5]], '#41546e'),
    polygon([[30.2, 43], [33.8, 43], [33.8, 48], [30.2, 48]], '#37475e'),
    polygon([[35.5, 43], [39, 43], [38.5, 47.5], [36, 47.5]], '#41546e'),
    polygon([[25, 28], [39, 28], [40.5, 38], [32, 40], [23.5, 38]], '#93a7bd'),
    polygon([[25, 28], [28.5, 28], [27.5, 39.5], [23.5, 38]], '#7d92a8'),
    polygon([[36, 28], [39, 28], [40.5, 38], [37, 38.5]], '#b7c9db'),
    polygon([[24, 38.5], [40, 38.5], [40, 41.5], [24, 41.5]], '#5b3a1e'),
    circle(32, 40, 1.8, '#d9a441'),
    circle(32, 40, 0.7, '#8a6420'),
    polygon([[22, 29], [25.5, 30], [24.5, 36], [21.5, 35]], '#7d92a8'),
    circle(19.5, 38, 6.8, '#8a6420'),
    circle(19.5, 38, 5.6, '#3f6fd8'),
    circle(19.5, 38, 1.9, '#d9a441'),
    circle(41.5, 29, 4.6, '#aabdd2'),
    circle(42.3, 28.2, 3.1, '#cfdde9'),
    polygon([[40, 31], [45, 29], [47, 33], [42.5, 35]], '#93a7bd'),
    polygon([[43, 32], [48, 30], [49, 33.5], [44.5, 35.5]], '#5b3a1e'),
    polygon([[47, 29], [56, 15], [59, 17], [49, 31]], '#dfe8ef'),
    polyline([[49.5, 28.5], [55.5, 19.5]], '#aabdd2', 1.2),
    polygon([[44.5, 26.5], [51, 24], [52.5, 27], [46, 29.5]], '#d9a441'),
    polygon([[43, 29], [46.5, 27.5], [47.5, 29.5], [44, 31]], '#5b3a1e'),
    circle(42.6, 31.2, 1.7, '#d9a441'),
    circle(32, 19, 7, '#7f93a9'),
    polygon([[26, 15.5], [38, 15.5], [38, 17], [26, 17]], '#aabdd2'),
    polygon([[26, 18], [38, 18], [38, 23], [26, 23]], '#101720'),
    polygon([[31, 14], [33, 14], [33, 23], [31, 23]], '#67798d'),
    polygon([[25.5, 19], [28.5, 19], [28, 23.5], [25, 22.5]], '#67798d'),
    polygon([[35.5, 19], [38.5, 19], [39, 22.5], [36, 23.5]], '#67798d'),
    polygon([[28.5, 12.5], [33.5, 11.5], [36.5, 5.5], [31, 7.5]], '#3f6fd8'),
    polygon([[34, 6.5], [38.5, 3.5], [40, 5.5], [36.5, 8.5]], '#2f57ad'),
    circle(39.6, 4.6, 1.6, '#3f6fd8'),
  ];
}

function buildRaider(rng) {
  void rng;
  return [
    shadow(33, 13),
    polygon([[20, 28], [16, 36], [18, 46], [22, 52], [26, 47], [24, 38], [26, 30]], '#4a2018'),
    polygon([[22, 32], [20, 40], [22, 47], [24, 44], [23.5, 35]], '#5c2a1e'),
    polygon([[24, 26], [40, 23], [44, 36], [40, 44], [28, 45], [23, 36]], '#6e2a22'),
    polygon([[24, 27], [28.5, 25.5], [26.5, 44], [23, 36]], '#57201a'),
    polygon([[27, 26.5], [30, 25.5], [40, 40], [37.5, 41.5]], '#3c2f26'),
    polygon([[24, 25], [38, 22], [40, 26], [36, 28], [26, 29]], '#3c2f26'),
    polygon([[26, 24.5], [28, 21.5], [29.5, 24]], '#57432f'),
    polygon([[31, 23.5], [33, 20.5], [34.5, 23.5]], '#57432f'),
    polygon([[26, 20], [38, 17], [40, 24], [28, 27]], '#3c2f26'),
    polygon([[25, 20], [18, 12], [27, 18]], '#d9c9a3'),
    polygon([[39, 17], [46, 9], [42, 20.5]], '#d9c9a3'),
    polygon([[28.5, 19.5], [38.5, 17.5], [39, 19.5], [29, 21.5]], '#241a12'),
    circle(31, 22, 1.4, '#ffcf5e'),
    circle(36, 21.2, 1.4, '#ffcf5e'),
    polygon([[30, 25.5], [37, 24.5], [37.5, 27], [30.5, 28]], '#d9c9a3'),
    polyline([[32, 25.3], [32.2, 27.6]], '#a89a78', 0.8),
    polyline([[34, 25], [34.2, 27.3]], '#a89a78', 0.8),
    polyline([[36, 24.7], [36.2, 27]], '#a89a78', 0.8),
    polygon([[36, 30], [44, 32], [43.5, 36], [36.5, 34]], '#8a4a2f'),
    polygon([[42, 33], [49, 34.5], [48.5, 38], [42, 37]], '#3c2f26'),
    circle(49.5, 36.8, 2.2, '#a5794f'),
    polygon([[50, 30], [62, 28], [63, 33], [58, 36], [54.5, 33.5], [52, 35.5], [50, 34]], '#b7bcc2'),
    polygon([[57, 30], [59.5, 29.5], [58.5, 33]], '#8f959c'),
    polygon([[48.5, 32.5], [51.5, 32], [51.8, 35.5], [48.8, 36]], '#3a2a1a'),
    polygon([[26, 44], [32, 44], [31.5, 53], [26, 53]], '#33241b'),
    polyline([[26.5, 47], [31.3, 47]], '#57432f', 1),
    polyline([[26.5, 50], [31.2, 50]], '#57432f', 1),
    polygon([[36, 44], [42, 44], [42.5, 53], [36.5, 53]], '#2b1c15'),
    polygon([[25, 53.5], [32, 53.5], [32, 57.5], [25, 57.5]], '#231a10'),
    polygon([[36, 53.5], [43.5, 53.5], [43.5, 57.5], [36.5, 57.5]], '#231a10'),
    circle(23.5, 45, 3.4, '#6b5a3e'),
    polygon([[21.5, 42.5], [25, 42], [25.5, 44], [22, 44.5]], '#57432f'),
  ];
}

function buildElite(rng) {
  void rng;
  return [
    shadow(32, 16),
    polygon([[18, 22], [14, 34], [16, 50], [22, 52], [26, 46], [24, 30], [26, 22]], '#3f0d14'),
    polygon([[18, 24], [16.5, 34], [18, 46], [21, 47], [22.5, 36], [22, 26]], '#571420'),
    polygon([[24, 42], [30, 42], [29.5, 50], [24, 50]], '#5d1620'),
    polygon([[24, 50], [29.5, 50], [29.5, 55], [24, 55]], '#4a1016'),
    polygon([[22.5, 54], [30, 54], [30, 58], [22.5, 58]], '#1c0a0d'),
    circle(27, 49.5, 1.9, '#d9a441'),
    polygon([[34, 42], [40, 42], [40, 50], [34.5, 50]], '#6b1a26'),
    polygon([[34.5, 50], [40, 50], [40, 55], [34.5, 55]], '#5d1620'),
    polygon([[34, 54], [41.5, 54], [41.5, 58], [34, 58]], '#1c0a0d'),
    circle(37, 49.5, 1.9, '#d9a441'),
    polygon([[21, 24], [43, 24], [41, 46], [32, 44], [23, 46]], '#7a2230'),
    polygon([[21, 24], [26, 24], [24.5, 45], [23, 46]], '#5d1620'),
    polygon([[36, 25], [41, 25], [40, 42], [37, 42]], '#94303e'),
    polygon([[30, 24], [34, 24], [34, 44], [32, 45], [30, 44]], '#4a1016'),
    polygon([[32, 29], [35.5, 33], [32, 37], [28.5, 33]], '#d9a441'),
    polygon([[32, 31], [34, 33], [32, 35], [30, 33]], '#8a6420'),
    polygon([[15.5, 24], [23, 22.5], [24.5, 28], [17, 30]], '#8c2a3a'),
    polyline([[15.5, 24], [23, 22.5], [24.5, 28]], '#d9a441', 1.4),
    polygon([[41, 22.5], [48.5, 24], [47, 30], [39.5, 28]], '#8c2a3a'),
    polyline([[48.5, 24], [41, 22.5], [39.5, 28]], '#d9a441', 1.4),
    polygon([[15, 29], [19.5, 30.5], [18.5, 36], [14.5, 34.5]], '#5d1620'),
    polygon([[44, 29], [48, 30.5], [47, 35], [43.5, 34]], '#6b1a26'),
    polygon([[46, 32], [51, 33.5], [50, 38], [45.5, 36.5]], '#3a2a1a'),
    circle(32, 17, 8, '#8c2a3a'),
    polygon([[24.5, 13.5], [39.5, 13.5], [39.5, 15], [24.5, 15]], '#d9a441'),
    polygon([[25.5, 16], [38.5, 16], [38.5, 19], [25.5, 19]], '#140508'),
    circle(30.5, 21, 0.5, '#140508'),
    circle(32, 21.5, 0.5, '#140508'),
    circle(33.5, 21, 0.5, '#140508'),
    circle(29.5, 17.5, 1.5, '#ff5d3a'),
    circle(34.5, 17.5, 1.5, '#ff5d3a'),
    polygon([[25, 12], [13, 3], [24.5, 17]], '#d9a441'),
    polygon([[24, 12.5], [17.5, 7], [23.5, 14.5]], '#b9852f'),
    polygon([[39, 12], [51, 3], [39.5, 17]], '#d9a441'),
    polygon([[40, 12.5], [46.5, 7], [40.5, 14.5]], '#b9852f'),
    polygon([[27, 10], [28.5, 6], [30, 10.5]], '#d9a441'),
    polygon([[31, 9.5], [32, 5], [33, 9.5]], '#d9a441'),
    polygon([[34, 10.5], [35.5, 6], [37, 10]], '#d9a441'),
    polygon([[53.5, 6], [57, 8], [58.5, 33], [54.5, 32]], '#cfd6dd'),
    polyline([[55.5, 11], [56.5, 31]], '#9aa1a8', 1.2),
    polygon([[50, 33.5], [61, 32], [61.8, 35.5], [50.8, 37]], '#d9a441'),
    polygon([[52, 36], [56.5, 35], [57, 38.5], [52.5, 39.5]], '#3a2a1a'),
    circle(53.5, 40.5, 2.2, '#d9a441'),
  ];
}

function buildTree(rng) {
  const shapes = [
    polygon([[28, 42], [36, 42], [38.5, 58], [25.5, 58]], '#5a4030'),
    polygon([[33, 42], [36, 42], [38.5, 58], [34.5, 58]], '#4a3526'),
    polygon([[22, 54], [28, 52], [29, 58], [21, 58]], '#4a3526'),
    polygon([[36, 52], [42, 54], [43, 58], [35, 58]], '#4a3526'),
    polyline([[30, 45], [29.5, 56]], '#3d2c1f', 1),
    polyline([[34, 44], [35, 56]], '#3d2c1f', 1),
    polygon([
      [32, 4], [40, 14], [36, 13], [42, 22], [35, 20], [38, 27],
      [26, 27], [29, 20], [23, 21], [28, 13], [24, 13],
    ], '#2f6b3a'),
    polygon([[24, 26], [32, 28], [40, 26], [38, 30], [26, 30]], '#275c31'),
    polygon([
      [32, 12], [44, 26], [38, 24], [46, 34], [38, 32], [44, 40],
      [20, 40], [26, 32], [18, 34], [25, 25], [20, 26],
    ], '#357840'),
    polygon([
      [32, 22], [48, 38], [42, 36], [50, 46], [42, 44], [46, 50],
      [18, 50], [22, 44], [14, 46], [22, 36], [16, 38],
    ], '#3d8a49'),
    polygon([[20, 38], [32, 40], [44, 38], [42, 43], [22, 43]], '#275c31'),
    circle(24, 56.5, 1.6, '#557d4a'),
    circle(40.5, 56.5, 1.4, '#557d4a'),
    polygon([[27.5, 55.5], [28.5, 55.5], [28.4, 57.5], [27.6, 57.5]], '#d9c9a3'),
    polygon([[26.8, 55.5], [29.2, 55.5], [28, 53.8]], '#a34a3a'),
  ];
  for (let i = 0; i < 10; i += 1) {
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
    polyline([[15.5, 30], [15, 57]], '#76787d', 0.9),
    polyline([[18.5, 29], [18, 57]], '#76787d', 0.9),
    polyline([[21.5, 24], [21, 57]], '#76787d', 0.9),
    polygon([[16.5, 24], [19, 26.5], [16, 28]], '#b9bec6'),
    polygon([[19, 28], [22, 24], [22.5, 26.5], [19.5, 29.5]], '#76787d'),
    polygon([[14.5, 40], [15.5, 45], [14.8, 50]], '#5f6165'),
    polygon([[41, 38], [44.5, 33], [47, 37], [50, 32], [51, 58], [41, 58]], '#76787d'),
    polygon([[41, 38], [44.5, 33], [47, 37], [50, 32], [51, 36], [47, 41], [44, 39]], '#8d8f94'),
    polyline([[43.5, 39], [43, 57]], '#5f6165', 0.9),
    polyline([[46.5, 38], [46, 57]], '#5f6165', 0.9),
    polyline([[49.5, 34], [49, 57]], '#5f6165', 0.9),
    polygon([[24, 44], [44, 38], [46, 44], [26, 50]], '#84868b'),
    polygon([[24, 44], [44, 38], [45, 40], [25, 46]], '#a3a5aa'),
    polygon([[44, 38], [46, 44], [44.5, 45.5], [43, 40.5]], '#6f7176'),
    polyline([[24, 44], [44, 38], [46, 44], [26, 50], [24, 44]], '#5f6165', 1.5),
    polygon([[47.5, 44], [51, 43], [52, 48], [48.5, 49]], '#909297'),
    polyline([[48.5, 45], [51.5, 44]], '#76787d', 0.8),
    polygon([[28, 54], [32, 52.5], [33.5, 56], [29, 57]], '#7a7c81'),
    polygon([[34, 52], [39, 51], [40, 56], [35, 57]], '#7a7c81'),
    polygon([[34, 52], [39, 51], [39.5, 53], [34.5, 54]], '#8d8f94'),
    circle(28, 55, 2.4, '#6f7176'),
    circle(46, 54, 1.8, '#66686d'),
    circle(17, 52, 1.5, '#557d4a'),
    circle(43, 41, 1.3, '#557d4a'),
    ellipse(30, 57.5, 3.2, 1.1, '#4c7243'),
    polygon([[24.5, 57.5], [25.5, 54.5], [26.5, 57.5]], '#557d4a'),
    polygon([[36.5, 57.5], [37.5, 55], [38.5, 57.5]], '#4c7243'),
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
    polyline([[19.5, 37], [25, 45]], '#4a3623', 2),
    polyline([[44.5, 37], [39, 45]], '#4a3623', 2),
    polygon([[16, 53], [48, 53], [48, 58], [16, 58]], '#8d8f94'),
    polyline([[22, 53.5], [22, 57.5]], '#76787d', 0.8),
    polyline([[30, 53.5], [30, 57.5]], '#76787d', 0.8),
    polyline([[38, 53.5], [38, 57.5]], '#76787d', 0.8),
    polyline([[16, 55.5], [48, 55.5]], '#76787d', 0.8),
    polygon([[11, 33], [32, 11], [53, 33]], '#b98f3e'),
    polyline([[22, 26], [18.5, 31.5]], '#8a6a2c', 0.9),
    polyline([[26, 20], [21, 28]], '#8a6a2c', 0.9),
    polyline([[38, 20], [43, 28]], '#8a6a2c', 0.9),
    polyline([[42, 26], [46, 31.5]], '#8a6a2c', 0.9),
    polygon([[9, 32], [55, 32], [55, 36], [9, 36]], '#8a6a2c'),
    polygon([[30.5, 11.5], [32, 10], [33.5, 11.5], [32, 13]], '#caa04a'),
    polygon([[40, 15], [44, 15], [44, 24], [40, 24]], '#8d8f94'),
    polyline([[40, 18], [44, 18]], '#76787d', 0.8),
    polyline([[40, 21], [44, 21]], '#76787d', 0.8),
    polygon([[39, 14], [45, 14], [45, 15.5], [39, 15.5]], '#a3a5aa'),
    circle(45.5, 10.5, 2.2, '#ffffff59'),
    circle(47.5, 7, 1.7, '#ffffff40'),
    circle(49.3, 4.2, 1.2, '#ffffff2e'),
    polygon([[28, 43], [36, 43], [36, 58], [28, 58]], '#3c2a18'),
    polyline([[30.7, 43.5], [30.7, 57.5]], '#2b1d10', 0.7),
    polyline([[33.4, 43.5], [33.4, 57.5]], '#2b1d10', 0.7),
    polyline([[28.4, 46], [30.4, 46]], '#5b5b60', 1.2),
    polyline([[28.4, 52], [30.4, 52]], '#5b5b60', 1.2),
    circle(34.6, 51, 0.9, '#b98f3e'),
    polygon([[17.5, 39], [19.5, 39], [19.5, 45], [17.5, 45]], '#5b4326'),
    polygon([[20, 39], [26, 39], [26, 45], [20, 45]], '#4a3623'),
    polygon([[21, 40], [25, 40], [25, 44], [21, 44]], '#ffd98a'),
    polyline([[23, 40], [23, 44]], '#4a3623', 0.6),
    polyline([[21, 42], [25, 42]], '#4a3623', 0.6),
    polygon([[38, 39], [44, 39], [44, 45], [38, 45]], '#4a3623'),
    polygon([[39, 40], [43, 40], [43, 44], [39, 44]], '#ffd98a'),
    polyline([[41, 40], [41, 44]], '#4a3623', 0.6),
    polyline([[39, 42], [43, 42]], '#4a3623', 0.6),
    polygon([[44.5, 39], [46.5, 39], [46.5, 45], [44.5, 45]], '#5b4326'),
  ];
}

function buildShrine(rng) {
  void rng;
  return [
    shadow(32, 18),
    polygon([[14, 54], [50, 54], [52, 58], [12, 58]], '#9aa1a8'),
    polygon([[14, 54], [50, 54], [51, 55.5], [13, 55.5]], '#a8aeb6'),
    polygon([[18, 50], [46, 50], [48, 54], [16, 54]], '#a8aeb6'),
    polygon([[18, 50], [46, 50], [47, 51.5], [17, 51.5]], '#b9bec6'),
    polygon([[27.5, 26], [30, 26], [30, 48], [27.5, 48]], '#9aa1a8'),
    polygon([[34, 26], [36.5, 26], [36.5, 48], [34, 48]], '#9aa1a8'),
    polygon([[22, 28], [26, 28], [26, 50], [22, 50]], '#b9bec6'),
    polygon([[38, 28], [42, 28], [42, 50], [38, 50]], '#b9bec6'),
    polyline([[23.3, 29], [23.3, 49]], '#9aa1a8', 0.7),
    polyline([[24.7, 29], [24.7, 49]], '#9aa1a8', 0.7),
    polyline([[39.3, 29], [39.3, 49]], '#9aa1a8', 0.7),
    polyline([[40.7, 29], [40.7, 49]], '#9aa1a8', 0.7),
    polygon([[21, 26], [27, 26], [27, 28], [21, 28]], '#c6cbd2'),
    polygon([[37, 26], [43, 26], [43, 28], [37, 28]], '#c6cbd2'),
    polygon([[18, 22], [46, 22], [46, 26], [18, 26]], '#c6cbd2'),
    circle(23, 24, 0.6, '#9aa1a8'),
    circle(29, 24, 0.6, '#9aa1a8'),
    circle(35, 24, 0.6, '#9aa1a8'),
    circle(41, 24, 0.6, '#9aa1a8'),
    polygon([[16, 22], [32, 14], [48, 22]], '#d2d7de'),
    circle(32, 19.5, 2.2, '#59d6c9'),
    circle(32, 19.5, 1.2, '#2b7a70'),
    polyline([[41.5, 30], [42.5, 36], [41.8, 42]], '#557d4a', 1),
    circle(42.5, 34, 0.8, '#557d4a'),
    circle(41.6, 38.5, 0.8, '#557d4a'),
    circle(32, 36.5, 7.5, '#59d6c926'),
    polyline([[30, 44], [27.5, 50]], '#6f7680', 1.4),
    polyline([[34, 44], [36.5, 50]], '#6f7680', 1.4),
    polygon([[27.5, 42.5], [36.5, 42.5], [38, 46], [26, 46]], '#6f7680'),
    polygon([[27, 42], [37, 42], [37.5, 43.5], [26.5, 43.5]], '#878e98'),
    polygon([[32, 30], [36.5, 36], [35, 41], [32, 42.5], [29, 41], [27.5, 36]], '#3fae9e'),
    polygon([[32, 33], [35, 37], [32, 42], [29, 37]], '#59d6c9'),
    polygon([[32, 35.5], [33.8, 38], [32, 41], [30.2, 38]], '#bff3ec'),
    circle(27, 33, 0.9, '#bff3ec'),
    circle(37.5, 34.5, 0.8, '#59d6c9'),
    circle(30, 28.5, 0.7, '#bff3ec'),
  ];
}

function clamp(value, low, high) {
  return Math.min(high, Math.max(low, value));
}

function buildTerrainA(rng) {
  const shapes = [
    polygon([[0, 0], [VIEW_WIDTH, 0], [VIEW_WIDTH, VIEW_HEIGHT], [0, VIEW_HEIGHT]], '#46653f'),
    polygon([[6, 6], [24, 5], [28, 18], [10, 20]], '#4a6c43'),
    polygon([[38, 8], [57, 10], [55, 24], [40, 22]], '#426040'),
    polygon([[8, 38], [26, 36], [28, 50], [10, 52]], '#4f7047'),
    polygon([[36, 42], [56, 44], [54, 58], [38, 56]], '#3d5938'),
  ];
  for (let i = 0; i < 30; i += 1) {
    const x = 6 + rng() * 52;
    const y = 6 + rng() * 52;
    const roll = rng();
    const fill = roll < 0.4 ? '#4f7047' : roll < 0.75 ? '#3d5938' : '#57794d';
    shapes.push(circle(Math.round(x), Math.round(y), Math.round((1 + rng() * 1.4) * 2) / 2, fill));
  }
  for (let i = 0; i < 10; i += 1) {
    const x = clamp(7 + rng() * 50, 7, 55);
    const y = clamp(9 + rng() * 46, 9, 55);
    shapes.push(polygon(
      [[Math.round(x * 2) / 2, Math.round(y * 2) / 2],
       [Math.round(x * 2) / 2 + 1, Math.round(y * 2) / 2 - 3],
       [Math.round(x * 2) / 2 + 2, Math.round(y * 2) / 2]],
      i % 2 === 0 ? '#5d8a52' : '#527c49',
    ));
  }
  for (let i = 0; i < 4; i += 1) {
    const x = 10 + rng() * 40;
    const y = 12 + rng() * 38;
    const w = 3 + rng() * 2;
    const h = 2.5 + rng() * 1.5;
    const fill = i % 2 === 0 ? '#6b7060' : '#5c6154';
    shapes.push(polygon(
      [[x, y], [x + w, y + rng()], [x + w + 0.5, y + h], [x - 0.5, y + h + rng()]],
      fill,
    ));
    shapes.push(polygon(
      [[x, y], [x + w, y + rng() * 0.6], [x + w * 0.55, y + h * 0.45]],
      i % 2 === 0 ? '#7c8072' : '#6d7264',
    ));
  }
  for (let i = 0; i < 5; i += 1) {
    const x = clamp(8 + rng() * 48, 8, 56);
    const y = clamp(8 + rng() * 48, 8, 56);
    shapes.push(circle(Math.round(x), Math.round(y), 1.1, '#c9d6a3'));
    shapes.push(circle(Math.round(x), Math.round(y), 0.5, '#d9a441'));
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
    const [p0, p1, p2] = jitter;
    shapes.push(polygon(
      [p0, p1,
       [Math.round(((p1[0] + p2[0]) / 2) * 2) / 2, Math.round(((p1[1] + p2[1]) / 2) * 2) / 2]],
      '#54796e',
    ));
    const [, , q2, q3] = jitter;
    shapes.push(polygon(
      [q2, q3,
       [Math.round(((q3[0] + p0[0]) / 2) * 2) / 2, Math.round(((q3[1] + p0[1]) / 2) * 2) / 2]],
      '#3d5c53',
    ));
  }
  for (let i = 0; i < 5; i += 1) {
    const x = 8 + rng() * 46;
    const y = 8 + rng() * 44;
    shapes.push(polyline(
      [[Math.round(x), Math.round(y)],
       [Math.round(x + 2 + rng() * 3), Math.round(y + 2 + rng() * 2)],
       [Math.round(x + 5 + rng() * 4), Math.round(y + 1 + rng() * 4)]],
      '#2b423d', 0.7,
    ));
  }
  for (let i = 0; i < 8; i += 1) {
    const cx = clamp(6 + rng() * 52, 6, 58);
    const cy = clamp(6 + rng() * 52, 6, 58);
    const r = 1 + rng() * 1.4;
    shapes.push(circle(Math.round(cx * 2) / 2, Math.round(cy * 2) / 2, Math.round(r * 2) / 2, '#5d8a5a'));
    shapes.push(circle(
      Math.round((cx + r) * 2) / 2, Math.round((cy - r * 0.6) * 2) / 2,
      Math.round(r * 0.7 * 2) / 2, i % 2 === 0 ? '#4c7243' : '#54796e',
    ));
  }
  for (let i = 0; i < 8; i += 1) {
    const x = 6 + rng() * 52;
    const y = 6 + rng() * 52;
    const fill = rng() < 0.6 ? '#596058' : '#6a7168';
    shapes.push(circle(Math.round(x), Math.round(y), Math.round((0.8 + rng() * 0.8) * 2) / 2, fill));
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

export function buildVariants() {
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
