/** @vitest-environment node */

import { afterEach, describe, expect, it } from 'vitest';

import world from '#server/core/world.js';

const town = () => world.getDefaultTown();

afterEach(() => {
  world.players.length = 0;
  town().players = [];
});

describe('WorldManager player identity lifecycle', () => {
  it('does not let an old disconnect remove a replacement session', () => {
    const original = { uuid: 'same-scion', sceneId: town().id };
    const replacement = { uuid: 'same-scion', sceneId: town().id };

    world.addPlayer(original);
    world.addPlayer(replacement);
    world.removePlayer(original);

    expect(world.players).toEqual([replacement]);
    expect(town().players).toEqual([replacement]);
  });

  it('removes the current player object and its scene membership', () => {
    const player = { uuid: 'current-scion', sceneId: town().id };

    world.addPlayer(player);
    world.removePlayer(player);

    expect(world.players).toEqual([]);
    expect(town().players).toEqual([]);
  });
});
