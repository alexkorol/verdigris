/** @vitest-environment node */

import { describe, expect, it } from 'vitest';

import GameMap from '#server/core/map.js';
import {
  D114_FIRST_DELVE_PRESSURE,
  FIRST_DELVE_ENCOUNTER,
  FIRST_DELVE_PRESSURE_CURVE,
  firstDelvePackCap,
  isFirstDelve,
  recordEncounterKill,
  separateSceneActors,
  unlockRangedInScene,
} from '#server/core/combat/encounter.js';

describe('first-delve encounter readability', () => {
  it('identifies only the authored stone warren floor as the first delve', () => {
    expect(isFirstDelve({ depth: 1, theme: 'stone', layout: 'warren' })).toBe(true);
    expect(isFirstDelve({ depth: 2, theme: 'stone', layout: 'warren' })).toBe(false);
    expect(isFirstDelve({ depth: 1, theme: 'crypt', layout: 'warren' })).toBe(false);
    expect(isFirstDelve({ depth: 1, theme: 'stone', layout: 'gauntlet' })).toBe(false);
  });

  it('starts physically with one melee actor and gates the ranged roster across seeds', async () => {
    for (const seed of [3, 5, 9, 12, 21, 37, 90140]) {
      const generation = await GameMap.generateInstance({
        seed,
        template: 'dungeon',
        depth: 1,
      });
      const entry = generation.metadata.stairsUp;
      const entryDistance = monster => Math.max(
        Math.abs(monster.spawn.x - entry.x),
        Math.abs(monster.spawn.y - entry.y),
      );
      const byEntryDistance = [...generation.monsters].sort(
        (a, b) => entryDistance(a) - entryDistance(b),
      );
      const ranged = generation.monsters.filter(monster => monster.name === 'Ashen Marksman');

      expect(generation.metadata.encounter).toMatchObject({
        id: 'first-delve',
        openingMeleeCount: 1,
        rangedUnlockKills: FIRST_DELVE_ENCOUNTER.rangedUnlockKills,
      });
      expect(byEntryDistance[0]).toBe(generation.monsters[0]);
      expect(byEntryDistance[0].behaviour.type).toBe('melee');
      expect(byEntryDistance[0].name).toBe('Dread Vanguard');
      expect(entryDistance(byEntryDistance[0])).toBe(FIRST_DELVE_ENCOUNTER.openingSpawnRadius);
      expect(entryDistance(byEntryDistance[1]))
        .toBeGreaterThanOrEqual(FIRST_DELVE_ENCOUNTER.laterMonsterEntryRadius);
      const treasureRoomIndex = generation.metadata.roomCentres.findIndex(room => (
        room.x === generation.metadata.treasureRoom.x
        && room.y === generation.metadata.treasureRoom.y
      ));
      expect(treasureRoomIndex).toBeGreaterThan(FIRST_DELVE_ENCOUNTER.earlyRoomCount);
      expect(generation.monsters.filter(monster => (
        entryDistance(monster) < FIRST_DELVE_ENCOUNTER.laterMonsterEntryRadius
      ))).toEqual([generation.monsters[0]]);
      expect(ranged.length).toBeGreaterThan(0);
      ranged.forEach((monster) => {
        expect(monster.behaviour).toMatchObject({
          type: 'melee',
          encounterRole: 'ranged',
          encounterLocked: true,
        });
        expect(monster.behaviour.attack.range).toBe(1);
      });
    }
  });

  it('encodes a rising learn-win-pressure-reward pack curve', () => {
    expect(FIRST_DELVE_PRESSURE_CURVE.map(stage => stage.id))
      .toEqual(['learn', 'win', 'pressure', 'reward']);
    expect(FIRST_DELVE_PRESSURE_CURVE.map(stage => stage.maxPackSize))
      .toEqual([1, 2, 3, 4]);
    expect(firstDelvePackCap(1)).toBe(1);
    expect(firstDelvePackCap(2)).toBe(2);
    expect(firstDelvePackCap(3)).toBe(3);
    expect(firstDelvePackCap(4)).toBe(4);
    expect(D114_FIRST_DELVE_PRESSURE).toMatchObject({
      meleeSecondsToContact: 5.76,
      rangedSecondsToContact: 3.3,
      meleeWindupMs: 320,
      rangedWindupMs: 480,
    });
  });

  it('unlocks ranged actors only after the named kill threshold', () => {
    const ranged = {
      behaviour: {
        type: 'melee',
        encounterRole: 'ranged',
        encounterLocked: true,
        attack: { range: 1, minimumRange: 1 },
        encounterUnlock: { range: 5, minimumRange: 2, aggressionRange: 8, pursuitRange: 11 },
      },
      ai: { setBehaviour: () => { ranged.aiCalls += 1; } },
      aiCalls: 0,
    };
    const scene = {
      id: 'instance:first-delve',
      metadata: {
        encounter: {
          rangedUnlockKills: FIRST_DELVE_ENCOUNTER.rangedUnlockKills,
          rangedUnlocked: false,
        },
      },
      monsters: [ranged],
    };
    const player = { sceneId: scene.id, combat: {} };
    const partyMember = { sceneId: scene.id, combat: {} };

    expect(recordEncounterKill(player, scene).unlocked).toBe(false);
    expect(ranged.behaviour.encounterLocked).toBe(true);
    expect(recordEncounterKill(partyMember, scene).newlyUnlocked).toBe(1);
    expect(scene.metadata.encounter.kills).toBe(2);
    expect(ranged.behaviour).toMatchObject({
      type: 'ranged',
      encounterLocked: false,
      aggressionRange: 8,
      pursuitRange: 11,
    });
    expect(ranged.behaviour.attack).toMatchObject({ range: 5, minimumRange: 2 });
    expect(ranged.aiCalls).toBe(1);
    expect(unlockRangedInScene(scene, 3)).toBe(0);
  });

  it('repairs player/monster and monster/monster overlap without moving the player', () => {
    const scene = {
      map: {
        width: 4,
        height: 4,
        background: new Array(16).fill(1),
        foreground: new Array(16).fill(0),
      },
      players: [{ x: 2, y: 2, stats: { resources: { health: { current: 10 } } } }],
      monsters: new Array(3).fill(null).map(() => ({
        x: 2,
        y: 2,
        state: {},
        isAlive: true,
        stats: { resources: { health: { current: 10 } } },
      })),
    };
    const playerPosition = { ...scene.players[0] };

    expect(separateSceneActors(scene)).toBe(3);
    expect(scene.players[0]).toEqual(playerPosition);
    const actorTiles = [scene.players[0], ...scene.monsters]
      .map(actor => `${Math.round(actor.x)}:${Math.round(actor.y)}`);
    expect(new Set(actorTiles).size).toBe(actorTiles.length);
  });
});
