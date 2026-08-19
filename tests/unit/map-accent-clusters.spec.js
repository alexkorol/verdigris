/** @vitest-environment node */

import { describe, expect, it } from 'vitest';

import GameMap from '#server/core/map.js';
import { dungeonGroupGids } from '#shared/dungeon-tiles.js';

// TASK-0057: clustered floor accents. These tests pin the two properties the
// spec cares about — (1) accents form coherent blobs rather than one-cell
// checkerboard noise, and (2) the whole pass is deterministic per seed.

const WIDTH = 60;
const HEIGHT = 60;
const wallPool = dungeonGroupGids('wall', 'stone');
const wallFill = wallPool[0];
const floorPool = dungeonGroupGids('floor', 'stone');
const accentPool = dungeonGroupGids('floor', 'grey');

const carveOpenRoom = (background) => {
  for (let y = 8; y < 52; y += 1) {
    for (let x = 8; x < 52; x += 1) {
      background[(y * WIDTH) + x] = floorPool[0];
    }
  }
};

const makeBackground = () => {
  const background = new Array(WIDTH * HEIGHT).fill(wallFill);
  carveOpenRoom(background);
  return background;
};

const accentCells = (background) => {
  const accentSet = new Set(accentPool);
  const cells = [];
  for (let i = 0; i < background.length; i += 1) {
    if (accentSet.has(background[i])) {
      cells.push(i);
    }
  }
  return cells;
};

const adjacencyRatio = (cells) => {
  if (!cells.length) {
    return 1;
  }
  const accentSet = new Set(cells);
  let withNeighbour = 0;
  for (const i of cells) {
    const x = i % WIDTH;
    const y = Math.floor(i / WIDTH);
    const touches = [
      [x + 1, y], [x - 1, y], [x, y + 1], [x, y - 1],
    ].some(([nx, ny]) => (
      nx >= 0 && ny >= 0 && nx < WIDTH && ny < HEIGHT && accentSet.has((ny * WIDTH) + nx)
    ));
    if (touches) {
      withNeighbour += 1;
    }
  }
  return withNeighbour / cells.length;
};

describe('paintAccentClusters (0057)', () => {
  it('paints accents as contiguous blobs at the density budget', () => {
    const background = makeBackground();
    const floorCount = background.filter(tile => tile !== wallFill).length;
    GameMap.paintAccentClusters({
      background,
      width: WIDTH,
      height: HEIGHT,
      rng: GameMap.createSeededGenerator(1234),
      accentPool,
      wallFill,
    });

    const cells = accentCells(background);
    expect(floorCount).toBe(44 * 44); // 8..51 inclusive on both axes
    // Density stays near the 12% budget (spec allows ±10% → 10.8%–13.2%).
    expect(cells.length / floorCount).toBeGreaterThan(0.10);
    expect(cells.length / floorCount).toBeLessThan(0.14);
    // Clustered: the overwhelming majority of accent cells touch another.
    expect(adjacencyRatio(cells)).toBeGreaterThan(0.7);
  });

  it('is deterministic for a fixed seed', () => {
    const a = makeBackground();
    const b = makeBackground();
    GameMap.paintAccentClusters({
      background: a,
      width: WIDTH,
      height: HEIGHT,
      rng: GameMap.createSeededGenerator(99),
      accentPool,
      wallFill,
    });
    GameMap.paintAccentClusters({
      background: b,
      width: WIDTH,
      height: HEIGHT,
      rng: GameMap.createSeededGenerator(99),
      accentPool,
      wallFill,
    });
    expect(a).toEqual(b);
  });

  it('clusters contrast sharply with per-cell checkerboard noise', () => {
    // Reference: the old per-cell 12% pick leaves accents mostly isolated.
    const checker = makeBackground();
    const floorCells = [];
    for (let i = 0; i < checker.length; i += 1) {
      if (checker[i] !== wallFill) {
        floorCells.push(i);
      }
    }
    const checkerRng = GameMap.createSeededGenerator(555);
    for (const i of floorCells) {
      if (checkerRng() < 0.12) {
        checker[i] = accentPool[Math.floor(checkerRng() * accentPool.length)];
      }
    }
    const checkerRatio = adjacencyRatio(accentCells(checker));
    expect(checkerRatio).toBeLessThan(0.5);

    const clustered = makeBackground();
    GameMap.paintAccentClusters({
      background: clustered,
      width: WIDTH,
      height: HEIGHT,
      rng: GameMap.createSeededGenerator(1234),
      accentPool,
      wallFill,
    });
    const clusterRatio = adjacencyRatio(accentCells(clustered));
    expect(clusterRatio).toBeGreaterThan(checkerRatio + 0.25);
  });
});

describe('generateInstance accent determinism (0057)', () => {
  it('produces identical maps across repeated runs of the same seed', async () => {
    const input = { seed: 20240818, template: 'marsh', layout: 'clearings', depth: 1 };
    const first = await GameMap.generateInstance(input);
    const second = await GameMap.generateInstance(input);
    expect(first.map.background).toEqual(second.map.background);
    expect(first.map.foreground).toEqual(second.map.foreground);
  });
});
