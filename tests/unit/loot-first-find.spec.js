/** @vitest-environment node */

import {
  afterEach,
  describe,
  expect,
  it,
  vi,
} from 'vitest';

vi.mock('#server/core/world.js', () => {
  const scenes = new Map();
  return {
    default: {
      players: [],
      npcs: [],
      items: [],
      map: { background: [], foreground: [] },
      scenes,
      defaultTownId: 'town-1',
      getScene: id => scenes.get(id) || null,
      getScenePlayers: () => [],
    },
  };
});

vi.mock('#server/socket.js', () => ({
  default: {
    emit: vi.fn(),
    broadcast: vi.fn(),
    sendMessageToPlayer: vi.fn(),
  },
}));

const { dropMonsterLoot, FIRST_FIND } = await import('#server/core/combat/loot.js');
const { default: world } = await import('#server/core/world.js');
const { default: Socket } = await import('#server/socket.js');
const { default: ItemFactory } = await import('#server/core/items/factory.js');
const {
  announceFirstFindDrop,
  buildFirstFindComparison,
  newFirstFindDrop,
  newFirstFindPickup,
  presentFirstFindPickup,
} = await import('../../src/core/player/events/loot-moment.js');
const { default: worldEvents } = await import('../../src/core/player/events/world.js');
const { default: bus } = await import('../../src/core/utilities/bus.js');

const makeRngQueue = (values) => {
  const queue = [...values];
  return () => (queue.length ? queue.shift() : 0.99);
};

const makeSlainMonster = (sceneId, overrides = {}) => ({
  uuid: `monster-${sceneId}`,
  name: 'Dread Vanguard',
  x: 12,
  y: 14,
  sceneId,
  rarityId: 'common',
  rewards: { experience: 30, coins: 45 },
  ...overrides,
});

const makeFirstDelveScene = (sceneId) => ({
  id: sceneId,
  items: [],
  players: [],
  metadata: { encounter: { id: 'first-delve' } },
});

describe('first-find drop rule', () => {
  afterEach(() => {
    vi.restoreAllMocks();
  });

  it('guarantees one curated, tagged drop on the first delve kill', () => {
    const sceneId = 'scene-first-find-1';
    const scene = makeFirstDelveScene(sceneId);
    world.scenes.set(sceneId, scene);
    const player = { socket_id: 'socket-first-find-1' };

    const drops = dropMonsterLoot(makeSlainMonster(sceneId), {
      player,
      rng: makeRngQueue([0.5]),
    });

    expect(drops).toHaveLength(2);
    expect(drops[0].id).toBe('coins');
    const find = drops[1];
    expect(FIRST_FIND.pool).toContain(find.id);
    expect(find.firstFind).toBe(FIRST_FIND.id);
    expect(find.displayName).toBeTruthy();
    expect(Socket.emit).toHaveBeenCalledWith('game:send:message', expect.objectContaining({
      player: { socket_id: player.socket_id },
      text: expect.stringContaining(find.displayName),
    }));
  });

  it('grants the find once per player session, not on later kills', () => {
    const sceneId = 'scene-first-find-2';
    const scene = makeFirstDelveScene(sceneId);
    world.scenes.set(sceneId, scene);
    const player = { socket_id: 'socket-first-find-2' };

    const first = dropMonsterLoot(makeSlainMonster(sceneId), {
      player,
      rng: makeRngQueue([0.5]),
    });
    const second = dropMonsterLoot(makeSlainMonster(sceneId), {
      player,
      rng: makeRngQueue([0.5]),
    });

    expect(first.some(drop => drop.firstFind)).toBe(true);
    expect(second.some(drop => drop.firstFind)).toBe(false);
    expect(second).toHaveLength(1);
    expect(second[0].id).toBe('coins');
  });

  it('never fires outside the first delve', () => {
    const sceneId = 'scene-first-find-3';
    world.scenes.set(sceneId, { id: sceneId, items: [], players: [] });
    const player = { socket_id: 'socket-first-find-3' };

    const drops = dropMonsterLoot(makeSlainMonster(sceneId), {
      player,
      rng: makeRngQueue([0.5]),
    });

    expect(drops).toHaveLength(1);
    expect(drops[0].id).toBe('coins');
    expect(drops.some(drop => drop.firstFind)).toBe(false);
  });

  it('retries within the kill window when creation itself fails', () => {
    const sceneId = 'scene-first-find-4';
    const scene = makeFirstDelveScene(sceneId);
    world.scenes.set(sceneId, scene);
    const player = { socket_id: 'socket-first-find-4' };

    const original = ItemFactory.createById;
    let poolAttempts = 0;
    vi.spyOn(ItemFactory, 'createById').mockImplementation((id, options) => {
      if (FIRST_FIND.pool.includes(id)) {
        poolAttempts += 1;
        if (poolAttempts === 1) {
          return null;
        }
      }
      return original(id, options);
    });

    const first = dropMonsterLoot(makeSlainMonster(sceneId), {
      player,
      rng: makeRngQueue([0.5]),
    });
    const second = dropMonsterLoot(makeSlainMonster(sceneId), {
      player,
      rng: makeRngQueue([0.5]),
    });

    expect(first.some(drop => drop.firstFind)).toBe(false);
    expect(second.some(drop => drop.firstFind)).toBe(true);
  });
});

describe('first-find comparison content', () => {
  const flintSpear = {
    id: 'flint-spear',
    name: 'Flint Spear',
    displayName: 'Flint Spear',
    equipSlot: 'right_hand',
    stats: {
      attack: { stab: 8, slash: 2, crush: -1, range: 0 },
      defense: { stab: 1, slash: 0, crush: 0, range: 0 },
    },
  };
  const bronzeDagger = {
    id: 'bronze-dagger',
    name: 'Bronze Dagger',
    stats: {
      attack: { stab: 4, slash: 2, crush: 4, range: 0 },
      defense: { stab: 0, slash: 1, crush: 0, range: 1 },
    },
  };
  const hideWrap = {
    id: 'hide-wrap',
    name: 'Hide Wrap',
    displayName: 'Hide Wrap',
    equipSlot: 'armor',
    stats: {
      attack: { stab: 0, slash: 0, crush: 0, range: 0 },
      defense: { stab: 5, slash: 7, crush: 8, range: 4 },
    },
  };

  it('compares a weapon against what the scion holds', () => {
    const line = buildFirstFindComparison(flintSpear, bronzeDagger);

    expect(line).toContain('vs Bronze Dagger');
    expect(line).toContain('+4 stab');
    expect(line).toContain('-5 crush');
    expect(line).toContain('attack');
  });

  it('reads as pure gain when the slot is empty', () => {
    const line = buildFirstFindComparison(hideWrap, null);

    expect(line).toContain('defense');
    expect(line).toContain('nothing held in that slot');
    expect(line).toContain('+8 crush');
  });

  it('says so plainly when the trade is even', () => {
    expect(buildFirstFindComparison(flintSpear, flintSpear)).toBe('Even trade vs Flint Spear.');
  });
});

describe('first-find client seams', () => {
  afterEach(() => {
    vi.restoreAllMocks();
  });

  it('spots only the newly-dropped tagged item', () => {
    const settled = { uuid: 'old-find', firstFind: 'first-find', name: 'Hide Wrap' };
    const fresh = { uuid: 'new-find', firstFind: 'first-find', name: 'Flint Spear' };

    expect(newFirstFindDrop([settled], [settled, fresh])).toBe(fresh);
    expect(newFirstFindDrop([settled], [settled])).toBeNull();
    expect(newFirstFindDrop(null, [{ uuid: 'plain', name: 'Coins' }])).toBeNull();
  });

  it('spots only the newly-picked-up tagged item', () => {
    const carried = { uuid: 'carried-find', firstFind: 'first-find', name: 'Hide Wrap' };

    expect(newFirstFindPickup([], [carried])).toBe(carried);
    expect(newFirstFindPickup([carried], [carried])).toBeNull();
  });

  it('announces the drop with the loot chime and a Take prompt', () => {
    const emit = vi.spyOn(bus, '$emit');

    announceFirstFindDrop({ uuid: 'drop-1', displayName: 'Flint Spear' });

    expect(emit).toHaveBeenCalledWith('sound:loot');
    const prompt = emit.mock.calls.find(([event]) => event === 'game:context-menu:first-only');
    expect(prompt).toBeTruthy();
    const { firstItem, count } = prompt[1].data.data;
    expect(count).toBe(0);
    expect(firstItem.label).toContain('Flint Spear');
    expect(firstItem.label).toContain('Z');
    expect(firstItem.action).toBeUndefined();
  });

  it('presents the pickup toast once per find through the open:screen seam', () => {
    const emit = vi.spyOn(bus, '$emit');
    const item = {
      uuid: 'pickup-1',
      firstFind: 'first-find',
      displayName: 'Flint Spear',
      examine: 'A long ash shaft tipped with carefully flaked flint.',
      equipSlot: 'right_hand',
      stats: {
        attack: { stab: 8, slash: 2, crush: -1, range: 0 },
        defense: { stab: 1, slash: 0, crush: 0, range: 0 },
      },
    };
    const dagger = {
      displayName: 'Bronze Dagger',
      stats: {
        attack: { stab: 4, slash: 2, crush: 4, range: 0 },
        defense: { stab: 0, slash: 1, crush: 0, range: 1 },
      },
    };
    const player = { wear: { right_hand: dagger } };

    const comparison = presentFirstFindPickup({ item, player });

    expect(comparison).toContain('vs Bronze Dagger');
    const toast = emit.mock.calls.find(([event]) => event === 'open:screen');
    expect(toast).toBeTruthy();
    expect(toast[1].data.payload).toEqual({
      name: 'Flint Spear',
      examine: item.examine,
      comparison,
    });
    expect(toast[1].data.screen).toBeTruthy();

    // The same find never re-toasts (e.g. on a later login refresh).
    expect(presentFirstFindPickup({ item, player })).toBeNull();
  });

  it('wires world:itemDropped to the drop announcement', () => {
    const emit = vi.spyOn(bus, '$emit');
    const context = { game: { map: { droppedItems: [] } } };
    const find = { uuid: 'wired-find', firstFind: 'first-find', displayName: 'Hide Wrap' };

    worldEvents['world:itemDropped']({ data: [find] }, context);

    expect(context.game.map.droppedItems).toEqual([find]);
    expect(emit).toHaveBeenCalledWith('sound:loot');
  });
});
