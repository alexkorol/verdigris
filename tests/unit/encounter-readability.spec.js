/** @vitest-environment node */

import { describe, expect, it, vi } from 'vitest';

import GameMap from '#server/core/map.js';
import Monster from '#server/core/monster.js';
import {
  D114_FIRST_DELVE_PRESSURE,
  FIRST_DELVE_ENCOUNTER,
  FIRST_DELVE_PRESSURE_CURVE,
  advanceEncounterStage,
  firstDelvePackCap,
  isEncounterActorActive,
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
      expect(byEntryDistance[0].rarity).toBe('common');
      expect(byEntryDistance[0]).toMatchObject({
        encounterStage: 'learn',
        encounterMinKills: 0,
      });
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
          encounterInactive: true,
        });
        expect(monster.encounterMinKills).toBeGreaterThanOrEqual(
          FIRST_DELVE_ENCOUNTER.rangedUnlockKills,
        );
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

  it('enforces runtime stages and keeps Marksmen dormant until two scene kills', async () => {
    const generation = await GameMap.generateInstance({
      seed: 90140,
      template: 'dungeon',
      depth: 1,
    });
    const scene = {
      id: 'instance:first-delve',
      metadata: generation.metadata,
      monsters: generation.monsters.map((definition, index) => new Monster({
        ...definition,
        sceneId: 'instance:first-delve',
        instanceId: `runtime:${index}`,
      })),
    };
    const player = { sceneId: scene.id, combat: {} };
    const partyMember = { sceneId: scene.id, combat: {} };
    const active = () => scene.monsters.filter(isEncounterActorActive);
    const marksmen = () => scene.monsters.filter(monster => monster.name === 'Ashen Marksman');

    const initial = advanceEncounterStage(scene, 0);
    const [opener] = active();
    expect(initial).toMatchObject({ stage: 'learn', activated: 0, rangedUnlocked: 0 });
    expect(active()).toHaveLength(1);
    expect(opener).toMatchObject({ name: 'Dread Vanguard', rarityId: 'common' });
    expect(opener.behaviour).toMatchObject({
      type: 'melee',
      stepIntervalMs: D114_FIRST_DELVE_PRESSURE.meleeStepIntervalMs,
      attack: {
        intervalMs: D114_FIRST_DELVE_PRESSURE.meleeAttackIntervalMs,
        windupMs: D114_FIRST_DELVE_PRESSURE.meleeWindupMs,
        range: 1,
      },
    });

    const dormantMarksman = marksmen()[0];
    const dormantPosition = { x: dormantMarksman.x, y: dormantMarksman.y };
    expect(isEncounterActorActive(dormantMarksman)).toBe(false);
    expect(dormantMarksman.state.mode).toBe('dormant');
    expect(dormantMarksman.update(1000)).toBe(false);
    expect({ x: dormantMarksman.x, y: dormantMarksman.y }).toEqual(dormantPosition);

    expect(recordEncounterKill(player, scene)).toMatchObject({
      stage: 'win',
      activated: 2,
      unlocked: false,
    });
    expect(active()).toHaveLength(3);
    expect(active().some(monster => monster.name === 'Ashen Marksman')).toBe(false);

    const pressure = recordEncounterKill(partyMember, scene);
    expect(pressure).toMatchObject({ stage: 'pressure', activated: 3, unlocked: true });
    expect(scene.metadata.encounter.kills).toBe(2);
    expect(active()).toHaveLength(6);
    const activeMarksman = active().find(monster => monster.name === 'Ashen Marksman');
    expect(activeMarksman.behaviour).toMatchObject({
      type: 'ranged',
      encounterLocked: false,
      encounterInactive: false,
      aggressionRange: 8,
      pursuitRange: 11,
      attack: { range: 5, minimumRange: 2 },
    });

    recordEncounterKill(player, scene);
    expect(active()).toHaveLength(6);
    const reward = recordEncounterKill(partyMember, scene);
    expect(reward).toMatchObject({ stage: 'reward' });
    expect(reward.activated).toBe(generation.monsters.length - 6);
    expect(active()).toHaveLength(generation.monsters.length);
    expect(marksmen().every(monster => (
      monster.behaviour.type === 'ranged'
      && monster.behaviour.encounterInactive === false
      && monster.behaviour.encounterLocked === false
    ))).toBe(true);
    expect(unlockRangedInScene(scene, 4)).toBe(0);
  });

  it('limits development inspection to one actor beside an explicit dev teleport', async () => {
    const previousNodeEnv = process.env.NODE_ENV;
    process.env.NODE_ENV = 'development';
    vi.resetModules();

    try {
      const { advanceEncounterStage: advanceDevelopmentStage } = await import(
        '#server/core/combat/encounter.js'
      );
      const makeDormant = (uuid, x) => ({
        uuid,
        x,
        y: 4,
        behaviour: { encounterInactive: true, encounterMinKills: 4 },
        state: {},
        ai: { update: () => true },
      });
      const inspected = makeDormant('inspected', 5);
      const other = makeDormant('other', 6);
      const player = {
        x: 6,
        y: 4,
        movementStep: {
          startedAt: Date.now(),
          duration: 150,
          walkId: null,
          stepIndex: null,
          steps: null,
          direction: 'left',
          blocked: false,
          interrupted: true,
        },
      };
      const scene = {
        metadata: { encounter: { kills: 0, rangedUnlockKills: 2 } },
        players: [player],
        monsters: [other, inspected],
      };

      advanceDevelopmentStage(scene, 0);
      expect(inspected.behaviour.encounterInactive).toBe(true);
      expect(other.behaviour.encounterInactive).toBe(true);

      player.movementStep = {
        startedAt: Date.now(),
        duration: 0,
        walkId: null,
        stepIndex: null,
        steps: null,
        direction: null,
        blocked: false,
        interrupted: true,
      };
      advanceDevelopmentStage(scene, 0);
      expect(inspected.behaviour.encounterInactive).toBe(false);
      expect(inspected.behaviour.encounterDevActive).toBeUndefined();
      expect(other.behaviour.encounterInactive).toBe(true);

      advanceDevelopmentStage(scene, 0);
      expect(other.behaviour.encounterInactive).toBe(true);
    } finally {
      if (previousNodeEnv === undefined) {
        delete process.env.NODE_ENV;
      } else {
        process.env.NODE_ENV = previousNodeEnv;
      }
      vi.resetModules();
    }
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
