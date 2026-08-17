import UI from '#shared/ui.js';

/**
 * First-delve encounter rules.
 *
 * The browser client already has the pieces needed to show melee telegraphs
 * and combat results.  This module keeps the first lesson server-authoritative:
 * generation describes the pressure curve, while kills unlock the next threat.
 * Values that affect distance or contact are named here so they can be checked
 * together under D-114 instead of being scattered through map generation.
 */

// D-114 pressure table. Distances are tiles and timings are milliseconds.
// Derived contact times use the existing continuous movement rule of one tile
// per step interval:
//   melee:  (8 aggression - 1.6 contact) * 900ms = 5.76s
//   ranged: (8 aggression - 5 attack) * 1100ms = 3.30s
// Keeping the authored composition, spacing, range, cadence, and derived
// seconds-to-contact in one table prevents a later isolated tuning change.
export const D114_FIRST_DELVE_PRESSURE = Object.freeze({
  depth: 1,
  theme: 'stone',
  layout: 'warren',
  openingRoomIndex: 1,
  openingPackCap: 1,
  earlyPackCaps: Object.freeze([1, 2, 3]),
  earlyRoomCount: 3,
  rangedEarliestRoomIndex: 3,
  rangedUnlockKills: 2,
  openingSpawnRadius: 7,
  laterMonsterEntryRadius: 15,
  meleeAggressionRange: 8,
  meleePursuitRange: 9,
  meleeContactRange: 1.6,
  meleeStepIntervalMs: 900,
  meleeAttackIntervalMs: 1500,
  meleeWindupMs: 320,
  meleeSecondsToContact: 5.76,
  rangedAggressionRange: 8,
  rangedPursuitRange: 11,
  rangedPreferredRange: 5,
  rangedMinimumRange: 2,
  rangedStepIntervalMs: 1100,
  rangedAttackIntervalMs: 1900,
  rangedWindupMs: 480,
  rangedSecondsToContact: 3.3,
  spawnSeparation: 1,
  playerTileSeparation: 1,
});

export const FIRST_DELVE_ENCOUNTER = D114_FIRST_DELVE_PRESSURE;

export const FIRST_DELVE_PRESSURE_CURVE = Object.freeze([
  Object.freeze({ id: 'learn', minKills: 0, maxPackSize: 1, ranged: false }),
  Object.freeze({ id: 'win', minKills: 1, maxPackSize: 2, ranged: false }),
  Object.freeze({ id: 'pressure', minKills: 2, maxPackSize: 3, ranged: true }),
  Object.freeze({ id: 'reward', minKills: 4, maxPackSize: 4, ranged: true }),
]);

export const isFirstDelve = ({ depth, theme, layout } = {}) => (
  Number(depth) === FIRST_DELVE_ENCOUNTER.depth
  && theme === FIRST_DELVE_ENCOUNTER.theme
  && layout === FIRST_DELVE_ENCOUNTER.layout
);

export const firstDelvePackCap = (roomIndex) => {
  if (!Number.isFinite(roomIndex) || roomIndex < FIRST_DELVE_ENCOUNTER.openingRoomIndex) {
    return null;
  }

  const offset = Math.floor(roomIndex - FIRST_DELVE_ENCOUNTER.openingRoomIndex);
  if (offset >= FIRST_DELVE_ENCOUNTER.earlyPackCaps.length) {
    return FIRST_DELVE_PRESSURE_CURVE[FIRST_DELVE_PRESSURE_CURVE.length - 1].maxPackSize;
  }
  return FIRST_DELVE_ENCOUNTER.earlyPackCaps[offset];
};

export const firstDelveStageForRoom = (roomIndex) => {
  if (roomIndex <= FIRST_DELVE_ENCOUNTER.openingRoomIndex) {
    return FIRST_DELVE_PRESSURE_CURVE[0];
  }
  if (roomIndex === FIRST_DELVE_ENCOUNTER.openingRoomIndex + 1) {
    return FIRST_DELVE_PRESSURE_CURVE[1];
  }
  if (roomIndex === FIRST_DELVE_ENCOUNTER.openingRoomIndex + 2) {
    return FIRST_DELVE_PRESSURE_CURVE[2];
  }
  return FIRST_DELVE_PRESSURE_CURVE[3];
};

const sceneEncounter = scene => scene && scene.metadata && scene.metadata.encounter;

export const isEncounterLockedRanged = monster => Boolean(
  monster
  && monster.behaviour
  && monster.behaviour.encounterRole === 'ranged'
  && monster.behaviour.encounterLocked === true,
);

export const isEncounterActorActive = monster => Boolean(
  monster && (!monster.behaviour || monster.behaviour.encounterInactive !== true),
);

const deactivateEncounterActor = (monster) => {
  if (!monster || !monster.behaviour || monster.behaviour.encounterInactive !== true) {
    return false;
  }
  monster.state = monster.state || {};
  monster.state.mode = 'dormant';
  monster.state.targetId = null;
  monster.state.pendingAttack = null;
  if (monster.ai && typeof monster.ai.update === 'function'
    && !monster.state.encounterPausedUpdate) {
    monster.state.encounterPausedUpdate = monster.ai.update;
    monster.ai.update = () => false;
  }
  return true;
};

const activateEncounterActor = (monster) => {
  if (!monster || !monster.behaviour || monster.behaviour.encounterInactive !== true) {
    return false;
  }
  monster.behaviour.encounterInactive = false;
  monster.state = monster.state || {};
  if (monster.ai && monster.state.encounterPausedUpdate) {
    monster.ai.update = monster.state.encounterPausedUpdate;
    delete monster.state.encounterPausedUpdate;
  }
  monster.state.mode = 'idle';
  monster.state.targetId = null;
  monster.state.pendingAttack = null;
  monster.state.lastGlideAt = 0;
  return true;
};

/**
 * Switch a generated ranged actor from its safe opening melee shell to the
 * normal ranged behaviour.  Monster AI exposes setBehaviour specifically for
 * runtime behaviour changes, so no client or AI-controller changes are needed.
 */
export const unlockRangedMonster = (monster) => {
  if (!isEncounterLockedRanged(monster)) {
    return false;
  }

  const unlock = monster.behaviour.encounterUnlock || {};
  monster.behaviour.type = 'ranged';
  monster.behaviour.encounterLocked = false;
  monster.behaviour.attack = {
    ...monster.behaviour.attack,
    range: Number.isFinite(unlock.range)
      ? unlock.range
      : FIRST_DELVE_ENCOUNTER.rangedPreferredRange,
    minimumRange: Number.isFinite(unlock.minimumRange)
      ? unlock.minimumRange
      : FIRST_DELVE_ENCOUNTER.rangedMinimumRange,
  };
  if (Number.isFinite(unlock.aggressionRange)) {
    monster.behaviour.aggressionRange = unlock.aggressionRange;
  }
  if (Number.isFinite(unlock.pursuitRange)) {
    monster.behaviour.pursuitRange = unlock.pursuitRange;
  }
  monster.encounterUnlocked = true;
  if (monster.ai && typeof monster.ai.setBehaviour === 'function') {
    monster.ai.setBehaviour('ranged');
  }
  return true;
};

export const unlockRangedInScene = (scene, killCount) => {
  const encounter = sceneEncounter(scene);
  if (!encounter || !Number.isFinite(killCount)) {
    return 0;
  }

  const threshold = Number.isFinite(encounter.rangedUnlockKills)
    ? encounter.rangedUnlockKills
    : FIRST_DELVE_ENCOUNTER.rangedUnlockKills;
  if (killCount < threshold) {
    return 0;
  }

  encounter.rangedUnlocked = true;
  return (Array.isArray(scene.monsters) ? scene.monsters : [])
    .filter(monster => isEncounterActorActive(monster)
      && isEncounterLockedRanged(monster)
      && killCount >= (Number(monster.behaviour.encounterMinKills) || 0))
    .reduce((count, monster) => count + (unlockRangedMonster(monster) ? 1 : 0), 0);
};

/**
 * Pause actors whose authored stage has not been earned and activate every
 * stage now covered by the scene-wide kill count. Pausing replaces the AI
 * update function, so dormant monsters cannot patrol, target, heal, buff, or
 * attack while they wait. Player combat also filters them through
 * isEncounterActorActive.
 */
export const advanceEncounterStage = (scene, killCount = null) => {
  const encounter = sceneEncounter(scene);
  if (!encounter) {
    return { kills: 0, stage: null, activated: 0, rangedUnlocked: 0 };
  }

  const kills = Number.isFinite(killCount)
    ? Math.max(0, Math.floor(killCount))
    : Math.max(0, Math.floor(Number(encounter.kills) || 0));
  encounter.kills = kills;
  let activated = 0;

  (Array.isArray(scene.monsters) ? scene.monsters : []).forEach((monster) => {
    const minKills = Number(monster?.behaviour?.encounterMinKills) || 0;
    if (kills >= minKills) {
      activated += activateEncounterActor(monster) ? 1 : 0;
    } else {
      deactivateEncounterActor(monster);
    }
  });

  const activeStage = [...FIRST_DELVE_PRESSURE_CURVE]
    .reverse()
    .find(stage => kills >= stage.minKills) || FIRST_DELVE_PRESSURE_CURVE[0];
  encounter.activeStage = activeStage.id;
  const rangedUnlocked = unlockRangedInScene(scene, kills);
  return {
    kills,
    stage: activeStage.id,
    activated,
    rangedUnlocked,
  };
};

/**
 * Record a kill in the current delve.  The first qualifying kill belongs to
 * the player who earned it, but the ranged reveal is shared by the scene so a
 * party does not receive contradictory threat rules.
 */
export const recordEncounterKill = (player, scene) => {
  const encounter = sceneEncounter(scene);
  if (!player || !encounter) {
    return { kills: 0, unlocked: false };
  }

  const combat = player.combat || (player.combat = {});
  const sceneId = player.sceneId || scene.id || 'scene';
  const byScene = combat.encounterKillsByScene || (combat.encounterKillsByScene = {});
  const kills = (Number(encounter.kills) || 0) + 1;
  encounter.kills = kills;
  byScene[sceneId] = kills;
  combat.encounterKills = kills;
  const progression = advanceEncounterStage(scene, kills);
  return {
    kills,
    stage: progression.stage,
    activated: progression.activated,
    unlocked: progression.rangedUnlocked > 0 || encounter.rangedUnlocked === true,
    newlyUnlocked: progression.rangedUnlocked,
  };
};

const sameTile = (a, b) => (
  a && b && Math.round(a.x || 0) === Math.round(b.x || 0)
  && Math.round(a.y || 0) === Math.round(b.y || 0)
);

const isAlive = actor => actor && actor.isAlive !== false
  && (actor.stats?.resources?.health?.current === undefined
    || actor.stats.resources.health.current > 0);

const walkableAt = (scene, x, y) => {
  if (!scene || !scene.map || x < 0 || y < 0) {
    return false;
  }
  const width = Number(scene.map.width) || 200;
  const height = Number(scene.map.height) || width;
  if (x >= width || y >= height) {
    return false;
  }
  const index = (y * width) + x;
  const background = scene.map.background?.[index];
  const foreground = scene.map.foreground?.[index];
  // Generated maps use dungeon GIDs, while unit doubles often expose a
  // boolean walkability map. Runtime map checks remain authoritative in
  // movement code, but the same semantic helper keeps this repair safe.
  if (background === undefined) {
    return true;
  }
  return UI.tileWalkable(background - 1)
    && UI.tileWalkable((foreground || 0) - 1, 'foreground');
};

const separationCandidates = (scene, x, y) => {
  const width = Number(scene?.map?.width) || 200;
  const height = Number(scene?.map?.height) || width;
  const candidates = [];
  const maxRadius = Math.max(width, height);

  // Search complete square rings in a stable order. An adjacent tile wins in
  // normal play, while the wider search means a crowded doorway still repairs
  // the invariant instead of silently leaving two actors stacked.
  for (let radius = 1; radius < maxRadius; radius += 1) {
    for (let dy = -radius; dy <= radius; dy += 1) {
      for (let dx = -radius; dx <= radius; dx += 1) {
        if (Math.max(Math.abs(dx), Math.abs(dy)) !== radius) continue;
        candidates.push({ x: x + dx, y: y + dy });
      }
    }
  }
  return candidates;
};

/**
 * Resolve an accidental player/monster tile overlap before the next combat
 * decision.  Normal movement already avoids occupied tiles; this is the
 * server-side invariant for spawn fallbacks, reconnects, and continuous glide
 * rounding.  It never moves a player and returns the number of corrections.
 */
export const separateSceneActors = (scene) => {
  const players = Array.isArray(scene?.players) ? scene.players.filter(isAlive) : [];
  const monsters = Array.isArray(scene?.monsters) ? scene.monsters.filter(isAlive) : [];
  let corrected = 0;

  monsters.forEach((monster, monsterIndex) => {
    const overlapsPlayer = players.some(candidate => sameTile(monster, candidate));
    const overlapsMonster = monsters.some((candidate, candidateIndex) => (
      candidateIndex < monsterIndex && sameTile(monster, candidate)
    ));
    if (!overlapsPlayer && !overlapsMonster) {
      return;
    }
    const x = Math.round(monster.x || 0);
    const y = Math.round(monster.y || 0);
    const occupied = new Set(monsters
      .filter(other => other !== monster)
      .map(other => `${Math.round(other.x || 0)}:${Math.round(other.y || 0)}`));
    const candidate = separationCandidates(scene, x, y)
      .find(spot => walkableAt(scene, spot.x, spot.y)
        && !occupied.has(`${spot.x}:${spot.y}`)
        && !players.some(other => sameTile(spot, other)));
    if (!candidate) {
      return;
    }
    monster.x = candidate.x;
    monster.y = candidate.y;
    monster.state = monster.state || {};
    monster.state.lastGlideAt = 0;
    corrected += 1;
  });

  return corrected;
};

export default {
  D114_FIRST_DELVE_PRESSURE,
  FIRST_DELVE_ENCOUNTER,
  FIRST_DELVE_PRESSURE_CURVE,
  isFirstDelve,
  firstDelvePackCap,
  firstDelveStageForRoom,
  isEncounterActorActive,
  isEncounterLockedRanged,
  unlockRangedMonster,
  unlockRangedInScene,
  advanceEncounterStage,
  recordEncounterKill,
  separateSceneActors,
};
