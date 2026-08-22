import { mkdirSync, readFileSync, writeFileSync, existsSync } from 'node:fs';
import path from 'node:path';
import process from 'node:process';
import { fileURLToPath, pathToFileURL } from 'node:url';

export const GENERATOR_VERSION = 'task0147-gen-2';

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
    shadow(32, 14),
    polygon([[26, 25], [19.5, 33], [18.5, 44], [22.5, 51.5], [27, 44.5], [25.5, 32.5]], '#1d4a40'),
    polygon([[25, 28], [21.5, 35], [21, 44], [23.5, 48]], '#153430'),
    polygon([[27, 42], [31.5, 42], [31, 50], [26.5, 50]], '#2c4a44'),
    polygon([[26.8, 49], [31, 49], [30.6, 54.5], [26.4, 54.5]], '#23403a'),
    polygon([[25.4, 53.5], [30.6, 53.5], [31.2, 58], [24.8, 58]], '#15110e'),
    polygon([[33, 42], [38.2, 42], [38.6, 50], [33.4, 50]], '#35594f'),
    polygon([[33.4, 49], [38.6, 49], [39.2, 54.5], [33.6, 54.5]], '#2a4a42'),
    polygon([[33, 53.5], [39.6, 53.5], [40.4, 58], [32.4, 58]], '#191410'),
    circle(28.9, 46.5, 1.9, '#4a7a6a'),
    circle(36, 46.5, 1.9, '#4a7a6a'),
    polygon([[25.8, 40], [38.6, 40], [39.4, 45.5], [25, 45.5]], '#3a5c50'),
    polygon([[26.6, 44], [29, 44], [28.4, 49.5], [26.2, 48.5]], '#2e6b5d'),
    polygon([[30.8, 44], [33.4, 44], [33.4, 50], [30.8, 50]], '#35786a'),
    polygon([[35.2, 44], [37.6, 44], [38.4, 48.5], [36, 49.5]], '#2e6b5d'),
    polygon([[25.4, 37.2], [38.8, 37.2], [38.8, 40.2], [25.4, 40.2]], '#6e4a26'),
    polygon([[30.8, 37.2], [33.4, 37.2], [33.4, 40.2], [30.8, 40.2]], '#d9a441'),
    polygon([[25, 25.5], [39.4, 25.5], [38.8, 37.6], [25.4, 37.6]], '#3f8f7b'),
    polygon([[26, 26.5], [38.4, 26.5], [37.8, 31], [26.6, 31]], '#59b39a'),
    polygon([[31.6, 26.5], [33, 26.5], [33.4, 37.6], [31.2, 37.6]], '#2e6b5d'),
    polygon([[25.7, 33.5], [38.5, 33.5], [38.8, 35.2], [25.4, 35.2]], '#2e6b5d'),
    polygon([[19.5, 24.5], [27.5, 23.5], [28.4, 29.5], [20.4, 30.5]], '#59b39a'),
    polygon([[20, 28.8], [28.2, 27.8], [28.4, 29.5], [20.4, 30.5]], '#2e6b5d'),
    polygon([[29, 23.5], [35.4, 23.5], [35, 26.5], [29.4, 26.5]], '#2a4a42'),
    polygon([[26.4, 17], [38.4, 17], [38, 23.5], [26.8, 23.5]], '#59b39a'),
    circle(32.4, 17.8, 6.2, '#6fae9c'),
    polygon([[28, 18.6], [36.6, 18.6], [36.6, 20.4], [28, 20.4]], '#0e241f'),
    circle(30.2, 19.5, 0.9, '#ffd98a'),
    circle(34.6, 19.5, 0.9, '#ffd98a'),
    polygon([[29.4, 10.5], [32.4, 8.8], [35.4, 10.5], [34, 12.5], [30.6, 12.5]], '#c96b4a'),
    polygon([[31.4, 7.5], [33.4, 7], [36.4, 12], [33.8, 11.5]], '#d9a441'),
    polygon([[37.4, 23.5], [45.4, 24.5], [44.4, 30.5], [36.4, 29.5]], '#59b39a'),
    polygon([[36.6, 27.7], [44.6, 28.7], [44.4, 30.5], [36.4, 29.5]], '#2e6b5d'),
    polygon([[40, 27.5], [46.5, 24.5], [48, 27.5], [41.5, 30.5]], '#4a7a6a'),
    circle(48, 26, 2.4, '#e6d8b8'),
    polygon([[46.2, 24.6], [48.6, 23.4], [49.4, 25], [47, 26.2]], '#5b3a1e'),
    circle(45.4, 25.4, 1.7, '#d9a441'),
    polygon([[45, 21.5], [52, 18], [53.4, 20.2], [46.4, 23.7]], '#8c5a2b'),
    polygon([[50.5, 21], [59.5, 7.5], [62, 9.5], [53.5, 24]], '#cfe3da'),
    polyline([[52.5, 19.5], [59, 10.5]], '#8fb8ab', 1.3),
  ];
}

function buildRaider(rng) {
  void rng;
  return [
    shadow(33, 14),
    polygon([[20, 26], [30, 16], [40, 18], [46, 28], [44, 40], [22, 42]], '#7c3a26'),
    polygon([[21, 27], [25, 20], [27, 26]], '#4a2f22'),
    polygon([[26, 22], [31, 15], [33, 22]], '#4a2f22'),
    polygon([[32, 17], [38, 14], [39, 21]], '#4a2f22'),
    polygon([[38, 16], [44, 18], [43, 24]], '#4a2f22'),
    polygon([[20.5, 25], [27, 21.5], [29, 26], [22.5, 29.5]], '#9aa1a8'),
    circle(24, 26, 1.6, '#7c4a2f'),
    polygon([[33.5, 19], [28, 12], [34, 16.5]], '#d9c9a3'),
    polygon([[41, 17], [46, 10], [43.5, 18.5]], '#d9c9a3'),
    circle(37.5, 21.5, 4.6, '#a86a44'),
    circle(36, 20.8, 1, '#ffcf5e'),
    circle(39.6, 20.4, 1, '#ffcf5e'),
    polygon([[24, 28], [34, 24], [36, 28], [26, 32]], '#4a2f1e'),
    circle(31, 30, 2, '#d9c9a3'),
    circle(30.3, 29.6, 0.5, '#1c1210'),
    circle(31.9, 29.6, 0.5, '#1c1210'),
    circle(28, 34.5, 1.1, '#d9c9a3'),
    circle(31.5, 35.5, 1.1, '#d9c9a3'),
    circle(35, 35, 1.1, '#d9c9a3'),
    polygon([[24, 38], [42, 36], [44, 46], [40, 44], [36, 48], [32, 44], [28, 48], [24, 44]], '#8a5a34'),
    polygon([[26, 46], [31, 46], [30, 54], [26, 54]], '#5a3a26'),
    polygon([[25, 53], [30, 53], [30, 58], [24.5, 58]], '#2a1d14'),
    polygon([[34, 45], [39, 45], [40, 53], [35, 53]], '#6b452c'),
    polygon([[34, 52], [40, 52], [40.6, 58], [33.4, 58]], '#2e2016'),
    polygon([[22, 30], [18, 38], [21, 40], [25, 33]], '#6e2a22'),
    polygon([[38, 26], [45, 22], [47, 26], [40, 30]], '#7c3a26'),
    circle(46.8, 24.2, 2.3, '#c9a97a'),
    polygon([[44, 25.5], [58, 14], [59.5, 16], [45.5, 27.5]], '#5b3a1e'),
    polygon([[54, 10], [63, 17], [59, 23], [52, 16]], '#9aa1a8'),
    polyline([[55, 11], [60, 17.5]], '#c9d2d8', 1.4),
    polygon([[46, 24], [48, 22.8], [48.8, 24.2], [46.8, 25.4]], '#3a2a1a'),
  ];
}

function buildElite(rng) {
  void rng;
  return [
    shadow(32, 16),
    polygon([[16, 22], [10, 40], [14, 54], [22, 48], [24, 30]], '#4a1016'),
    polygon([[48, 22], [54, 40], [50, 54], [42, 48], [40, 30]], '#4a1016'),
    polygon([[17, 24], [12.5, 40], [15.5, 51], [21, 46], [23, 30]], '#5d1620'),
    polygon([[47, 24], [51.5, 40], [48.5, 51], [43, 46], [41, 30]], '#5d1620'),
    polygon([[25, 44], [30.5, 44], [30, 53], [24.5, 53]], '#5d1620'),
    polygon([[24.5, 52], [30, 52], [30.4, 57], [24.2, 57]], '#43101a'),
    polygon([[23.4, 55.5], [30.6, 55.5], [31, 58], [23, 58]], '#12070a'),
    polygon([[33.5, 44], [39, 44], [39.5, 53], [34, 53]], '#6b1a28'),
    polygon([[34, 52], [39.6, 52], [39.8, 57], [33.6, 57]], '#43101a'),
    polygon([[33.4, 55.5], [40.6, 55.5], [41, 58], [33, 58]], '#12070a'),
    polygon([[23, 38], [41, 38], [43, 46], [38, 44], [32, 47], [26, 44], [21, 46]], '#4a1016'),
    polygon([[22.6, 38.5], [25, 38.5], [24, 45], [22, 44.6]], '#d9a441'),
    polygon([[39, 38.5], [41.4, 38.5], [42, 44.6], [40, 45]], '#d9a441'),
    polygon([[22, 24], [42, 24], [41, 38.5], [23, 38.5]], '#7a2230'),
    polygon([[23, 25], [41, 25], [40.2, 30], [23.8, 30]], '#8c2a3a'),
    polygon([[32, 25.5], [35.5, 29], [32, 32.5], [28.5, 29]], '#d9a441'),
    polygon([[32, 27], [34.2, 29], [32, 31], [29.8, 29]], '#59d6c9'),
    polygon([[22, 24], [42, 24], [41.8, 25.8], [22.2, 25.8]], '#d9a441'),
    polygon([[23, 36.5], [41, 36.5], [41, 38.5], [23, 38.5]], '#3a2a1a'),
    circle(32, 37.5, 1.6, '#d9a441'),
    polygon([[16.5, 22], [25, 20.5], [26, 27], [18, 28.5]], '#8c2a3a'),
    polygon([[17.3, 26.5], [25.8, 25], [26, 27], [18, 28.5]], '#d9a441'),
    polygon([[39, 20.5], [47.5, 22], [46, 28.5], [38, 27]], '#8c2a3a'),
    polygon([[38.2, 25], [46.7, 26.5], [46, 28.5], [38, 27]], '#d9a441'),
    circle(32, 15.5, 6, '#5d1620'),
    polygon([[26.4, 14.5], [37.6, 14.5], [37.2, 21.5], [26.8, 21.5]], '#7a2230'),
    polygon([[30.8, 15], [33.2, 15], [33.2, 20.5], [30.8, 20.5]], '#0d0508'),
    polygon([[27.5, 16], [36.5, 16], [36.5, 17.6], [27.5, 17.6]], '#0d0508'),
    circle(29.8, 16.8, 1.1, '#ff5d3a'),
    circle(34.2, 16.8, 1.1, '#ff5d3a'),
    polygon([[26.5, 13], [18, 4.5], [24, 16]], '#d9a441'),
    polygon([[37.5, 13], [46, 4.5], [40, 16]], '#d9a441'),
    polyline([[25.6, 12.4], [20.8, 7]], '#b8862f', 1),
    polyline([[38.4, 12.4], [43.2, 7]], '#b8862f', 1),
    polygon([[30.8, 35], [33.2, 35], [33.2, 40], [30.8, 40]], '#3a2a1a'),
    circle(32, 34.2, 1.8, '#d9a441'),
    circle(30, 38.5, 1.9, '#e6d8b8'),
    circle(34, 38.5, 1.9, '#e6d8b8'),
    polygon([[27.5, 40], [36.5, 40], [36.5, 42.2], [27.5, 42.2]], '#d9a441'),
    polygon([[30.6, 42.2], [33.4, 42.2], [32.7, 58], [31.3, 58]], '#c9d2d8'),
    polyline([[32, 43.4], [32, 56]], '#8fa0aa', 1.2),
  ];
}

function buildTree(rng) {
  const shapes = [
    shadow(32, 12),
    polygon([[29, 40], [35, 40], [37.5, 58], [26.5, 58]], '#5a4030'),
    polygon([[33, 40], [35, 40], [37.5, 58], [34.5, 58]], '#4a3526'),
    polygon([[24, 55], [29, 52], [30, 58], [23, 58]], '#4a3526'),
    polygon([[34, 52], [40, 55], [41, 58], [33, 58]], '#4a3526'),
    polyline([[30.5, 44], [30, 56]], '#3e2c1f', 1.2),
    polyline([[34, 43], [34.8, 56]], '#3e2c1f', 1.2),
    polygon([
      [32, 4], [47, 22], [43, 20], [52, 34], [45, 32], [54, 46],
      [10, 46], [18, 32], [12, 34], [20, 20], [17, 22],
    ], '#245c31'),
    polygon([
      [32, 8], [43, 24], [39, 22], [47, 35], [40, 33], [47, 43],
      [17, 43], [24, 33], [18, 35], [25, 22], [21, 24],
    ], '#2f7a3d'),
    polygon([[26, 14], [34, 10], [38, 16], [33, 20], [27, 19]], '#3d9a4c'),
    polygon([[18, 26], [26, 20], [31, 26], [25, 32], [19, 31]], '#3d9a4c'),
    polygon([[36, 22], [44, 18], [48, 25], [42, 31], [36, 29]], '#3d9a4c'),
  ];
  for (let i = 0; i < 12; i += 1) {
    const angle = rng() * Math.PI * 2;
    const radius = 3 + rng() * 11;
    const x = 32 + Math.cos(angle) * radius * 1.25;
    const y = 26 + Math.sin(angle) * radius * 0.85;
    if (y < 9 || y > 41 || x < 15 || x > 49) continue;
    shapes.push(circle(
      Math.round(x * 2) / 2,
      Math.round(y * 2) / 2,
      Math.round((0.8 + rng() * 1) * 2) / 2,
      rng() < 0.5 ? '#57a75f' : '#6fbe72',
    ));
  }
  return shapes;
}

function buildRuin(rng) {
  void rng;
  return [
    shadow(32, 21),
    polygon([[8, 52], [20, 48], [44, 48], [56, 52], [57, 58], [7, 58]], '#6a705f'),
    polygon([[14, 46], [24, 46], [25, 52], [13, 52]], '#8d8f94'),
    polygon([[15, 24], [23, 24], [24, 46], [14, 46]], '#9aa1a8'),
    polyline([[17, 26], [16.4, 45]], '#7f8186', 1.1),
    polyline([[20, 26], [20.4, 45]], '#7f8186', 1.1),
    polygon([[21.6, 24], [23, 24], [24, 46], [22.6, 46]], '#84868b'),
    polygon([[15, 24], [16.5, 19.5], [19, 22.5], [21, 18.5], [23, 24]], '#a3a5aa'),
    polygon([[24.5, 20], [28, 18.5], [29.5, 22], [26, 23.5]], '#989a9f'),
    polygon([[44, 38], [51, 38], [52, 52], [43, 52]], '#76787d'),
    polygon([[44, 38], [46, 34.5], [48.5, 37], [50.5, 34], [51, 38]], '#84868b'),
    polygon([[42.5, 50], [53, 50], [54, 54], [41.5, 54]], '#6f7176'),
    polygon([[26, 44], [44, 40], [46, 44], [28, 48]], '#84868b'),
    polygon([[44, 40], [46, 40.8], [48, 44.6], [46, 44]], '#6f7176'),
    polyline([[32, 44.5], [35, 42.5], [37, 43.5]], '#5f6165', 1.2),
    polygon([[30, 52], [34, 50], [36, 53], [32, 55]], '#7a7c81'),
    circle(39, 52, 2.2, '#66686d'),
    circle(12, 54, 1.8, '#6f7176'),
    polygon([[47, 54], [51, 53], [52, 56], [48, 57]], '#66686d'),
    circle(18, 30, 2.2, '#4e9a8a33'),
    circle(20, 38, 1.8, '#4e9a8a2b'),
    circle(47, 42, 1.9, '#4e9a8a30'),
    polyline([[14.8, 52], [14.2, 49.2]], '#557d4a', 1.2),
    polyline([[16.4, 52.5], [16.8, 49.8]], '#557d4a', 1.1),
    polyline([[44.8, 52.5], [45.4, 49.9]], '#4e7a44', 1.1),
    circle(25, 51, 1.2, '#557d4a'),
    circle(45, 49, 1, '#4e7a44'),
  ];
}

function buildDwelling(rng) {
  void rng;
  return [
    shadow(32, 20),
    polygon([[10, 32], [32, 10], [54, 32]], '#8a6a2c'),
    polygon([[12, 32], [32, 12], [52, 32]], '#b98f3e'),
    polyline([[24, 22], [30, 31]], '#9a7734', 1.2),
    polyline([[40, 22], [34, 31]], '#9a7734', 1.2),
    polygon([[29.5, 11.5], [32, 9], [34.5, 11.5], [32, 13.5]], '#caa04a'),
    polygon([[40, 16], [44, 16], [44.5, 26], [39.5, 26]], '#7d7a70'),
    polygon([[39, 15], [45, 15], [45, 17], [39, 17]], '#6b6960'),
    circle(43.5, 11, 2.2, '#e8e3d82e'),
    circle(45.5, 7.5, 2.8, '#e8e3d824'),
    circle(48, 4, 3.2, '#e8e3d81b'),
    polygon([[12, 32], [52, 32], [52, 34.5], [12, 34.5]], '#6b5120'),
    polygon([[16, 34], [48, 34], [48, 58], [16, 58]], '#c9b08a'),
    polygon([[19, 36], [26, 43], [24, 45], [17.5, 38]], '#4a3623'),
    polygon([[45, 36], [38, 43], [40, 45], [46.5, 38]], '#4a3623'),
    polygon([[16, 34], [18.5, 34], [18.5, 58], [16, 58]], '#4a3623'),
    polygon([[45.5, 34], [48, 34], [48, 58], [45.5, 58]], '#4a3623'),
    polygon([[28, 44], [36, 44], [36, 58], [28, 58]], '#3c2a18'),
    polyline([[31, 44], [31, 58]], '#2e2012', 1),
    circle(34.4, 51.5, 0.9, '#b98f3e'),
    polygon([[21, 39], [26, 39], [26, 45], [21, 45]], '#4a3623'),
    polygon([[21.8, 39.8], [25.2, 39.8], [25.2, 44.2], [21.8, 44.2]], '#ffd98a'),
    polyline([[23.5, 39.8], [23.5, 44.2]], '#b98f3e', 0.9),
    polygon([[38, 39], [43, 39], [43, 45], [38, 45]], '#4a3623'),
    polygon([[38.8, 39.8], [42.2, 39.8], [42.2, 44.2], [38.8, 44.2]], '#ffd98a'),
    polyline([[40.5, 39.8], [40.5, 44.2]], '#b98f3e', 0.9),
    polygon([[15.5, 52], [48.5, 52], [48.5, 58], [15.5, 58]], '#7d7a70'),
    polyline([[22, 52], [22, 58]], '#6b6960', 1),
    polyline([[30, 52], [30, 58]], '#6b6960', 1),
    polyline([[38, 52], [38, 58]], '#6b6960', 1),
    polyline([[13, 57], [12.5, 54]], '#557d4a', 1.1),
  ];
}

function buildShrine(rng) {
  void rng;
  return [
    shadow(32, 18),
    polygon([[12, 54], [52, 54], [54, 58], [10, 58]], '#9aa1a8'),
    polygon([[16, 50], [48, 50], [50, 54], [14, 54]], '#a8aeb6'),
    polygon([[20, 46], [44, 46], [46, 50], [18, 50]], '#b4bac2'),
    polygon([[22, 24], [26.5, 24], [26.5, 46], [22, 46]], '#b9bec6'),
    polygon([[25, 24], [26.5, 24], [26.5, 46], [25, 46]], '#a2a8b0'),
    polygon([[37.5, 24], [42, 24], [42, 46], [37.5, 46]], '#b9bec6'),
    polygon([[40.5, 24], [42, 24], [42, 46], [40.5, 46]], '#a2a8b0'),
    polygon([[18, 20], [46, 20], [47, 25], [17, 25]], '#c6cbd2'),
    polygon([[18, 20], [46, 20], [45.8, 21.6], [18.2, 21.6]], '#d2d7de'),
    polygon([[30, 21.8], [31.5, 23.3], [30, 24.8], [28.5, 23.3]], '#8f96a0'),
    polygon([[34.5, 21.8], [36, 23.3], [34.5, 24.8], [33, 23.3]], '#8f96a0'),
    ellipse(32, 40, 9, 7, '#59d6c91f'),
    circle(32, 31, 5, 'none', { stroke: '#4e9a8a', sw: 2.2 }),
    circle(32, 31, 3, 'none', { stroke: '#59d6c9', sw: 1.4 }),
    circle(32, 31, 1.8, '#bff3ec'),
    circle(32, 24.6, 0.9, '#59d6c9'),
    circle(37.2, 34, 0.8, '#59d6c9'),
    circle(26.8, 34, 0.8, '#59d6c9'),
    polygon([[27.5, 44.5], [36.5, 44.5], [35, 47.5], [29, 47.5]], '#6f7680'),
    polygon([[29.5, 46], [32, 36.5], [34.5, 46]], '#ff9a3c'),
    polygon([[30.6, 46], [32, 39.5], [33.4, 46]], '#ffd166'),
    circle(32, 44.6, 1.1, '#fff3c4'),
    circle(23, 45.5, 1.3, '#557d4a'),
    circle(41, 45.5, 1.2, '#557d4a'),
  ];
}

function buildTerrainA(rng) {
  const shapes = [
    polygon([[0, 0], [VIEW_WIDTH, 0], [VIEW_WIDTH, VIEW_HEIGHT], [0, VIEW_HEIGHT]], '#3f6b4a'),
  ];
  const mottles = [
    [[6, 8], [22, 5], [26, 18], [10, 20]],
    [[36, 4], [58, 8], [54, 22], [38, 18]],
    [[4, 34], [20, 30], [24, 46], [6, 48]],
    [[40, 32], [60, 38], [56, 52], [42, 48]],
    [[16, 52], [44, 50], [50, 62], [12, 62]],
  ];
  for (const quad of mottles) {
    const jittered = quad.map(([x, y]) => [
      Math.round(Math.min(61.5, Math.max(2.5, x + (rng() - 0.5) * 3)) * 2) / 2,
      Math.round(Math.min(61.5, Math.max(2.5, y + (rng() - 0.5) * 3)) * 2) / 2,
    ]);
    shapes.push(polygon(jittered, rng() < 0.5 ? '#44754f' : '#497d54'));
  }
  shapes.push(polyline([
    [0, 40], [10, 37], [22, 41], [34, 36], [47, 40], [58, 35], [64, 38],
  ], '#52724a', 2));
  for (let i = 0; i < 16; i += 1) {
    const x = Math.round((6 + rng() * 52) * 2) / 2;
    const y = Math.round((6 + rng() * 52) * 2) / 2;
    shapes.push(polyline(
      [[x - 1.5, y], [x - 0.7, y - 2.4], [x, y - 0.6], [x + 0.8, y - 2.6], [x + 1.6, y]],
      rng() < 0.5 ? '#5d9963' : '#6fae74',
      1,
    ));
  }
  for (let i = 0; i < 26; i += 1) {
    const x = 6 + rng() * 52;
    const y = 6 + rng() * 52;
    const roll = rng();
    const fill = roll < 0.4 ? '#4f7047' : roll < 0.75 ? '#365a41' : '#57794d';
    shapes.push(circle(Math.round(x), Math.round(y), Math.round((0.8 + rng() * 1.4) * 2) / 2, fill));
  }
  for (let i = 0; i < 8; i += 1) {
    const x = 10 + rng() * 42;
    const y = 10 + rng() * 42;
    shapes.push(circle(Math.round(x), Math.round(y), Math.round((0.9 + rng() * 0.7) * 2) / 2,
      i % 2 === 0 ? '#7fbf85' : '#c9d98a'));
  }
  for (let i = 0; i < 5; i += 1) {
    const x = 10 + rng() * 42;
    const y = 10 + rng() * 42;
    const w = 3 + rng() * 2;
    const h = 2.2 + rng() * 1.8;
    const base = i % 2 === 0 ? '#6b7060' : '#5c6154';
    shapes.push(polygon(
      [[x, y], [x + w, y + rng()], [x + w + 0.5, y + h], [x - 0.5, y + h + rng()]], base,
    ));
    shapes.push(polyline(
      [[x + 0.4, y + 0.4], [x + w * 0.6, y + 0.5]], '#82876f', 0.8,
    ));
  }
  return shapes;
}

function buildTerrainB(rng) {
  const shapes = [
    polygon([[0, 0], [VIEW_WIDTH, 0], [VIEW_WIDTH, VIEW_HEIGHT], [0, VIEW_HEIGHT]], '#31463f'),
  ];
  const slabFills = ['#4d685f', '#54756a', '#5f8073', '#577066'];
  const slabs = [
    { cx: 16, cy: 14, rx: 9, ry: 6.5 },
    { cx: 42, cy: 12, rx: 10, ry: 6 },
    { cx: 13, cy: 34, rx: 8, ry: 6 },
    { cx: 38, cy: 32, rx: 11, ry: 7 },
    { cx: 58, cy: 36, rx: 6, ry: 5 },
    { cx: 20, cy: 54, rx: 9, ry: 5.5 },
    { cx: 46, cy: 53, rx: 8, ry: 5 },
  ];
  for (let s = 0; s < slabs.length; s += 1) {
    const slab = slabs[s];
    const points = [];
    for (let k = 0; k < 6; k += 1) {
      const angle = (Math.PI * 2 * k) / 6 + rng() * 0.5;
      const jx = 0.82 + rng() * 0.33;
      const jy = 0.82 + rng() * 0.33;
      const x = Math.round(Math.min(61, Math.max(3, slab.cx + Math.cos(angle) * slab.rx * jx)) * 2) / 2;
      const y = Math.round(Math.min(61, Math.max(3, slab.cy + Math.sin(angle) * slab.ry * jy)) * 2) / 2;
      points.push([x, y]);
    }
    shapes.push(polygon(points, slabFills[s % slabFills.length]));
    shapes.push(polyline(
      [points[4], points[5], points[0], points[1]].map(([x, y]) => [x, y - 0.8]),
      '#6d8f84', 1,
    ));
    shapes.push(polyline(
      [points[1], points[2], points[3], points[4]].map(([x, y]) => [x, y + 0.8]),
      '#263731', 1,
    ));
  }
  for (let i = 0; i < 3; i += 1) {
    const x = 8 + rng() * 44;
    const y = 6 + rng() * 50;
    shapes.push(polyline(
      [[x, y], [x + 2 + rng() * 3, y + 2 + rng() * 2], [x + 5 + rng() * 4, y + 1 + rng() * 3]],
      '#263731', 0.9,
    ));
  }
  for (let i = 0; i < 14; i += 1) {
    const x = Math.round((6 + rng() * 52) * 2) / 2;
    const y = Math.round((6 + rng() * 52) * 2) / 2;
    shapes.push(circle(x, y, Math.round((0.8 + rng() * 1.3) * 2) / 2,
      rng() < 0.55 ? '#4e8a5a' : '#3f7a4c'));
  }
  for (let i = 0; i < 6; i += 1) {
    const x = Math.round((8 + rng() * 48) * 2) / 2;
    const y = Math.round((8 + rng() * 48) * 2) / 2;
    shapes.push(circle(x, y, Math.round((0.7 + rng() * 0.5) * 2) / 2, '#7fae8f'));
  }
  shapes.push(polygon([[24, 28], [28, 26.5], [29.5, 30], [25.5, 31.5]], '#4e9a8a'));
  shapes.push(polyline([[25, 28.6], [27.6, 27.6]], '#7ed3b8', 0.9));
  shapes.push(polyline([[12, 24], [18, 23], [22, 25]], '#3b544c', 1.6));
  shapes.push(polyline([[44, 46], [50, 47], [56, 45]], '#3b544c', 1.6));
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
  // point_begin/point_end are counted in vertex units, matching the frozen
  // GDI consumer contract: it reads kPoints[(point_begin + p) * 2] for
  // p in [0, point_end - point_begin). Counting floats here instead made every
  // polygon swallow the following shapes' vertices and pushed the final
  // symbols past the end of kPoints.
  let pointCursor = 0;
  for (const variant of variants) {
    const shapeBegin = flatShapes.length;
    for (const shape of variant.shapes) {
      const pointBegin = pointCursor;
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
        for (const [x, y] of shape.points) {
          flatPoints.push(x, y);
          pointCursor += 1;
        }
      }
      flatShapes.push({
        kind,
        pointBegin,
        pointEnd: pointCursor,
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
