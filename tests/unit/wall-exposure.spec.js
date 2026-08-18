/** @vitest-environment node */

import { describe, expect, it } from 'vitest';

import {
  isExposedWallCell,
  treeLineGidFor,
} from '../../src/core/rendering/perspective-renderer.js';

const STONE_WALL_GID = 541 + 86; // groups.wall.stone[0]
const STONE_FLOOR_GID = 541 + 0; // groups.floor.stone[0]

// 5x5 map: wall border, floor interior, solid wall mass to the east.
const WIDTH = 5;
const HEIGHT = 5;
const background = [
  STONE_WALL_GID, STONE_WALL_GID, STONE_WALL_GID, STONE_WALL_GID, STONE_WALL_GID,
  STONE_WALL_GID, STONE_FLOOR_GID, STONE_FLOOR_GID, STONE_WALL_GID, STONE_WALL_GID,
  STONE_WALL_GID, STONE_FLOOR_GID, STONE_FLOOR_GID, STONE_WALL_GID, STONE_WALL_GID,
  STONE_WALL_GID, STONE_FLOOR_GID, STONE_FLOOR_GID, STONE_WALL_GID, STONE_WALL_GID,
  STONE_WALL_GID, STONE_WALL_GID, STONE_WALL_GID, STONE_WALL_GID, STONE_WALL_GID,
];

describe('TASK-0053 wall exposure', () => {
  it('exposes wall cells bordering walkable floor', () => {
    expect(isExposedWallCell(background, WIDTH, HEIGHT, 0, 0)).toBe(true); // corner, floor diagonal
    expect(isExposedWallCell(background, WIDTH, HEIGHT, 1, 0)).toBe(true); // north edge over floor
    expect(isExposedWallCell(background, WIDTH, HEIGHT, 3, 2)).toBe(true); // room's east face
  });

  it('keeps interior wall mass unexposed', () => {
    expect(isExposedWallCell(background, WIDTH, HEIGHT, 4, 1)).toBe(false); // solid mass east
    expect(isExposedWallCell(background, WIDTH, HEIGHT, 4, 4)).toBe(false); // far corner mass
  });

  it('treats out-of-bounds neighbours as not walkable', () => {
    const floorless = new Array(WIDTH * HEIGHT).fill(STONE_WALL_GID);
    expect(isExposedWallCell(floorless, WIDTH, HEIGHT, 0, 0)).toBe(false);
  });
});

describe('TASK-0053 tree-line gid', () => {
  it('is deterministic per cell and stable across calls', () => {
    expect(treeLineGidFor(10, 20)).toBe(treeLineGidFor(10, 20));
    expect(treeLineGidFor(10, 21)).not.toBe(0);
  });

  it('only returns living-tree gids from the dungeon tileset', () => {
    const gids = new Set();
    for (let x = 0; x < 40; x += 1) {
      for (let y = 0; y < 40; y += 1) {
        gids.add(treeLineGidFor(x, y));
      }
    }
    expect(gids.size).toBeGreaterThan(1); // real variety, not a constant
    gids.forEach(gid => expect(gid).toBeGreaterThanOrEqual(541));
  });
});
