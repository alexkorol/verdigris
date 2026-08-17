/** @vitest-environment node */

import fs from 'node:fs';
import path from 'node:path';
import { afterEach, describe, expect, it, vi } from 'vitest';
import { buildDeathSummary } from '#server/player/handlers/socket-events/index.js';
import playerEvents from '../../src/core/player/events/player.js';
import bus from '../../src/core/utilities/bus.js';

const makePlayer = ({ mortal = false, state = 'permadead' } = {}) => ({
  uuid: 'player-death-test',
  socket_id: 'socket-death-test',
  username: mortal ? 'Oathed Sable' : 'Wayfarer Sable',
  scionId: 'scion-death-test',
  houseId: 'house-death-test',
  chronicles: { mortal, scionId: 'scion-death-test', houseId: 'house-death-test' },
  wear: {
    right_hand: { uuid: 'item-worn', id: 'bronze-spear', name: 'Bronze Spear' },
  },
  inventory: {
    slots: [{ uuid: 'item-pack', id: 'linen-sash', name: 'Linen Sash' }],
  },
  stats: {
    lifecycle: {
      mode: mortal ? 'hard' : 'soft',
      state,
      respawn: {
        pending: state === 'awaiting-respawn',
        at: state === 'awaiting-respawn' ? 123456 : null,
        location: 'old-barrow:entrance',
      },
      lastEvent: { type: state === 'permadead' ? 'permadeath' : 'death', occurredAt: 123456 },
    },
  },
});

afterEach(() => {
  bus.$off('player:death-summary');
});

describe('TASK-0041 death decision moment', () => {
  it('exposes all D-106 carried values as a mortal loss and recovery summary', () => {
    const summary = buildDeathSummary(makePlayer({ mortal: true }));

    expect(summary).toMatchObject({
      state: 'permadead',
      mode: 'hard',
      mortalOath: true,
      permanent: true,
      succession: true,
      respawnDestination: 'The Chronicles — choose a successor',
      scion: { id: 'scion-death-test', houseId: 'house-death-test' },
    });
    expect(summary.losses.map(entry => entry.id)).toEqual(['item-worn', 'item-pack']);
    expect(summary.recoveredToPool).toEqual(summary.losses);
    expect(summary.protected).toEqual([]);
  });

  it('keeps an unoathed death in the expedition and protects carried values', () => {
    const summary = buildDeathSummary(makePlayer({ mortal: false, state: 'awaiting-respawn' }));

    expect(summary).toMatchObject({
      state: 'awaiting-respawn',
      mode: 'soft',
      mortalOath: false,
      permanent: false,
      succession: false,
      respawnDestination: 'old-barrow:entrance',
      recoveredToPool: [],
      losses: [],
    });
    expect(summary.protected.map(entry => entry.id)).toEqual(['item-worn', 'item-pack']);
  });

  it('turns a server death-summary frame into an overlay bus event', () => {
    const listener = vi.fn();
    bus.$on('player:death-summary', listener);
    const context = { game: { player: { uuid: 'player-death-test' } } };
    const summary = { state: 'permadead', mortalOath: true, permanent: true };

    playerEvents['player:death-summary']({
      data: { playerId: 'player-death-test', summary },
    }, context);

    expect(context.game.player.deathSummary).toBe(summary);
    expect(listener).toHaveBeenCalledWith(summary);
  });

  it('keeps a final-death stats frame on the decision overlay when summary is present', () => {
    const context = {
      game: {
        player: { uuid: 'player-death-test' },
        map: { player: null, players: [], monsters: [] },
      },
      handlePermadeath: vi.fn(),
    };
    const listener = vi.fn();
    bus.$on('player:death-summary', listener);
    const summary = { state: 'permadead', mortalOath: true, permanent: true };
    const lifecycle = { state: 'permadead', mode: 'hard' };

    playerEvents['player:stats:update']({
      data: {
        playerId: 'player-death-test',
        lifecycle,
        stats: { lifecycle },
        resources: { health: { current: 0, max: 100 } },
        deathSummary: summary,
      },
    }, context);

    expect(context.handlePermadeath).not.toHaveBeenCalled();
    expect(context.game.player.deathSummary).toBe(summary);
    expect(listener).toHaveBeenCalledWith(summary);
  });

  it('contains the readable overlay, protected input layer, and one primary action', () => {
    const source = fs.readFileSync(
      path.resolve('src/components/ui/world/DeathOverlay.vue'),
      'utf8',
    );
    expect(source).toContain('role="dialog"');
    expect(source).toContain('aria-modal="true"');
    expect(source).toContain('What leaves this run');
    expect(source).toContain('Recovered to the House pool');
    expect(source).toContain('Protected on return');
    expect(source).toContain('player:chronicles:return');
    expect((source.match(/class="death-overlay__continue"/g) || []).length).toBe(1);
  });
});
