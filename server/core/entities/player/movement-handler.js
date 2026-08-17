import MapUtils from '#shared/map-utils.js';
import Socket from '#server/socket.js';
import UI from '#shared/ui.js';
import config from '#server/config.js';
import playerEvent from '#server/player/handlers/actions/index.js';
import { isAllowedActionId } from '#server/core/data/action-list.js';
import world from '#server/core/world.js';
import { transitionPlayerIfOnPortal } from '#server/core/world-transitions.js';
import { autoPickupCurrency } from '#server/core/items/pickup.js';
import {
  DEFAULT_FACING_DIRECTION,
  DEFAULT_ANIMATION_DURATIONS,
  DEFAULT_ANIMATION_HOLDS,
} from '#shared/combat.js';
import {
  PLAYER_MOVE_SAMPLE_MS,
  PLAYER_TILE_TRAVEL_MS,
  directionVector,
  occupiedTile,
  playerMovementDelta,
  roundPosition,
} from '#shared/movement.js';

export const BASE_MOVE_DURATION = PLAYER_TILE_TRAVEL_MS;

// Movement samples are frequent, but a client should only re-apply an
// animation state when that state actually changes.  SpriteAnimator treats
// every server-state application as a new timeline, so repeating the same
// sequence in each movement packet would reset its frame and elapsed time.
// This is presentation bookkeeping only; the movementStep and player payload
// remain server-authoritative and unchanged.
const lastBroadcastAnimationSignature = new WeakMap();

/**
 * Movement feel is deliberately derived from the shared authoritative cadence.
 * Keep this table beside the resolver so a change to pace cannot silently
 * leave the animation/interpolation assumptions behind (D-114).
 *
 * The server still resolves each sample and the existing movementStep wire
 * envelope is unchanged.  The acceleration value is presentation-facing: it
 * controls the run animation ramp, while collision and position remain
 * server-authoritative samples.
 */
export const MOVEMENT_FEEL = Object.freeze({
  speedTilesPerSecond: 1000 / PLAYER_TILE_TRAVEL_MS,
  accelerationSeconds: 0.2,
  accelerationTilesPerSecondSquared: (1000 / PLAYER_TILE_TRAVEL_MS) / 0.2,
  initialAnimationSpeed: 0.25,
  interpolationWindowMs: PLAYER_MOVE_SAMPLE_MS * 2,
  screenWidthTiles: 24,
  secondsToCrossScreen: 24 / (1000 / PLAYER_TILE_TRAVEL_MS),
});

export const computeStepDuration = (deltaX, deltaY, baseDuration = BASE_MOVE_DURATION) => {
  const diagonal = Math.abs(deltaX) === 1 && Math.abs(deltaY) === 1;
  const multiplier = diagonal ? Math.SQRT2 : 1;
  return Math.round(baseDuration * multiplier);
};

export const directionDelta = (direction) => {
  return directionVector(direction);
};

const resolveFacing = (direction, fallback = DEFAULT_FACING_DIRECTION) => {
  if (!direction) {
    return fallback;
  }

  const mapping = {
    'up-right': 'right',
    'down-right': 'right',
    'up-left': 'left',
    'down-left': 'left',
  };

  const candidate = mapping[direction] || direction;
  if (['up', 'down', 'left', 'right'].includes(candidate)) {
    return candidate;
  }

  return fallback;
};

const setFacing = (player, direction) => {
  player.facing = resolveFacing(direction, player.facing || DEFAULT_FACING_DIRECTION);
  return player.facing;
};

const clearAnimationTimer = (player) => {
  if (player.animationTimer) {
    clearTimeout(player.animationTimer);
    player.animationTimer = null;
  }
};

export const broadcastMovement = (player, players = null) => {
  if (!player) {
    return;
  }

  const meta = {};
  if (player.movementStep) {
    meta.movementStep = player.movementStep;
  }
  const animation = player.animation;
  const animationSignature = animation
    ? JSON.stringify([
      animation.sequence,
      animation.state,
      animation.direction,
      animation.speed,
      animation.duration,
      animation.skillId,
      animation.holdState,
    ])
    : null;
  const previousSignature = lastBroadcastAnimationSignature.get(player);
  if (animation && animationSignature !== previousSignature) {
    meta.animation = player.animation;
    lastBroadcastAnimationSignature.set(player, animationSignature);
  }
  const recipients = players || world.getScenePlayers(player.sceneId);
  // `playerMovement` falls back to the animation on the movement payload when
  // meta.animation is absent.  Remove that redundant fallback too, otherwise
  // the client would still re-apply the same sequence on every sample.
  const movementPayload = meta.animation
    ? player
    : (() => {
      const payload = { ...player };
      delete payload.animation;
      return payload;
    })();
  Socket.broadcast('player:movement', movementPayload, recipients, { meta });
};

export const broadcastAnimation = (player, players = null) => {
  if (!player || !player.animation) {
    return;
  }

  const recipients = players || world.getScenePlayers(player.sceneId);
  Socket.broadcast('player:animation', {
    playerId: player.uuid,
    animation: player.animation,
  }, recipients);
};

const createInitialAnimation = (player, overrides = {}) => {
  const direction = resolveFacing(overrides.direction, DEFAULT_FACING_DIRECTION);
  return {
    state: overrides.state || 'idle',
    direction,
    sequence: Number.isFinite(overrides.sequence) ? overrides.sequence : 0,
    startedAt: Number.isFinite(overrides.startedAt) ? overrides.startedAt : Date.now(),
    duration: Number.isFinite(overrides.duration) ? overrides.duration : 0,
    speed: Number.isFinite(overrides.speed) ? overrides.speed : 1,
    skillId: overrides.skillId || null,
    holdState: overrides.holdState || null,
  };
};

const setAnimationState = (player, state, options = {}) => {
  const resolvedState = state || 'idle';
  const direction = resolveFacing(options.direction, player.facing || DEFAULT_FACING_DIRECTION);
  const nowTs = Number.isFinite(options.startedAt) ? options.startedAt : Date.now();
  const previousSequence = player.animation && typeof player.animation.sequence === 'number'
    ? player.animation.sequence
    : 0;
  const sequence = Number.isFinite(options.sequence) ? options.sequence : previousSequence + 1;
  const duration = Number.isFinite(options.duration)
    ? options.duration
    : (DEFAULT_ANIMATION_DURATIONS[resolvedState] || 0);
  const holdState = options.holdState !== undefined
    ? options.holdState
    : (DEFAULT_ANIMATION_HOLDS[resolvedState] || null);

  player.animation = {
    state: resolvedState,
    direction,
    sequence,
    startedAt: nowTs,
    duration,
    speed: Number.isFinite(options.speed) ? options.speed : 1,
    skillId: options.skillId || null,
    holdState,
  };

  clearAnimationTimer(player);

  if (holdState && duration > 0 && options.autoHold !== false) {
    player.animationTimer = setTimeout(() => {
      if (!player.animation || player.animation.sequence !== sequence) {
        return;
      }

      setAnimationState(player, holdState, {
        direction,
        sequence: sequence + 0.1,
        startedAt: Date.now(),
        duration: DEFAULT_ANIMATION_DURATIONS[holdState] || 0,
        holdState: DEFAULT_ANIMATION_HOLDS[holdState] || null,
        autoHold: false,
      });
      broadcastAnimation(player);
    }, duration);
  }

  return player.animation;
};

const registerMovementStep = (player, step = {}) => {
  const currentSequence = player.movementStep && typeof player.movementStep.sequence === 'number'
    ? player.movementStep.sequence
    : 0;

  const interruption = step.interrupted !== undefined
    ? step.interrupted
    : Boolean(player.path && player.path.current && player.path.current.interrupted);

  player.movementStep = {
    sequence: currentSequence + 1,
    startedAt: typeof step.startedAt === 'number' ? step.startedAt : Date.now(),
    duration: typeof step.duration === 'number' ? step.duration : 0,
    walkId: step.walkId ?? null,
    stepIndex: step.stepIndex ?? null,
    steps: step.steps ?? null,
    direction: step.direction || null,
    blocked: Boolean(step.blocked),
    interrupted: interruption,
  };

  return player.movementStep;
};

/**
 * Invalidate a click-to-walk route without putting the actor through an idle
 * animation state.  Continuous WASD samples arrive every 50ms; the old path
 * cancellation helper reset idle → run on every sample, restarting the sprite
 * and making an otherwise interpolated walk look like a repeated tile step.
 */
const interruptPathfinding = (player) => {
  const { path } = player;
  if (path && path.current) {
    if (typeof path.current.walkId === 'number') {
      path.current.walkId += 1;
    } else {
      path.current.walkId = 1;
    }

    if (path.current.path) {
      path.current.path.walking = [];
      path.current.path.set = [];
    }
    path.current.length = 0;
    path.current.step = 0;
    path.current.walkable = false;
    path.current.interrupted = true;
  }

  player.moving = false;
  player.queue = [];
  player.action = false;
};

const cancelPathfinding = (player) => {
  interruptPathfinding(player);
  if (player.movementFeel) {
    player.movementFeel = { velocity: 0, timestamp: Date.now() };
  }
  setAnimationState(player, 'idle', { direction: player.facing });
};

const getMovementAnimationSpeed = (player, timestamp) => {
  const previous = player.movementFeel || { velocity: 0, timestamp: null };
  const elapsedSeconds = Number.isFinite(previous.timestamp)
    ? Math.max(0, Math.min(0.5, (timestamp - previous.timestamp) / 1000))
    : 0;
  const velocity = Math.min(
    MOVEMENT_FEEL.speedTilesPerSecond,
    previous.velocity + (MOVEMENT_FEEL.accelerationTilesPerSecondSquared * elapsedSeconds),
  );

  // Keep transient feel state off the serialized Player object.  It is not
  // gameplay state and must never become part of the network contract.
  if (!Object.prototype.hasOwnProperty.call(player, 'movementFeel')) {
    Object.defineProperty(player, 'movementFeel', {
      value: { velocity, timestamp },
      configurable: true,
      enumerable: false,
      writable: true,
    });
  } else {
    player.movementFeel = { velocity, timestamp };
  }

  return Math.max(
    MOVEMENT_FEEL.initialAnimationSpeed,
    velocity / MOVEMENT_FEEL.speedTilesPerSecond,
  );
};

const setRunAnimation = (player, direction, duration, startedAt) => {
  const facing = resolveFacing(direction, player.facing || DEFAULT_FACING_DIRECTION);
  const speed = getMovementAnimationSpeed(player, startedAt);
  const current = player.animation;

  if (current && current.state === 'run' && current.direction === facing) {
    // Preserve the current frame/timeline while held input continues.  Only
    // refresh the existing timing fields; a new direction gets a normal state
    // transition below.
    current.duration = duration;
    current.speed = speed;
    return current;
  }

  return setAnimationState(player, 'run', {
    direction: facing,
    duration,
    speed,
    startedAt,
  });
};

const hasBlockingHealth = (actor) => {
  const current = actor && actor.stats && actor.stats.resources
    && actor.stats.resources.health
    && actor.stats.resources.health.current;

  return !Number.isFinite(current) || current > 0;
};

const hasLivingMonsterAt = (player, tileX, tileY) => {
  const scene = world.getSceneForPlayer(player);
  const monsters = scene && Array.isArray(scene.monsters) ? scene.monsters : [];

  // Monsters hold continuous positions; they block the tile they stand on.
  return monsters.some(monster => monster
    && Math.round(monster.x) === tileX
    && Math.round(monster.y) === tileY
    && monster.isAlive !== false
    && hasBlockingHealth(monster));
};

const canMoveTo = (player, tileX, tileY) => {
  const { size } = config.map;

  if (tileX < 0 || tileY < 0 || tileX > size.x - 1 || tileY > size.y - 1) {
    return false;
  }

  const occupiedX = Math.round(tileX);
  const occupiedY = Math.round(tileY);

  if (hasLivingMonsterAt(player, occupiedX, occupiedY)) {
    return false;
  }

  const tileIndex = (occupiedY * size.x) + occupiedX;
  const scene = world.getSceneForPlayer(player);
  const mapLayers = scene && scene.map ? scene.map : world.map;
  const steppedOn = {
    background: mapLayers.background[tileIndex] - 1,
    foreground: mapLayers.foreground[tileIndex] - 1,
  };

  const tiles = {
    background: steppedOn.background,
    foreground: steppedOn.foreground,
  };

  return MapUtils.gridWalkable(tiles, player, tileIndex, 0, 0, mapLayers) === 0;
};

const isBlocked = (player, direction, delta = null, origin = player) => {
  const vector = delta || directionDelta(direction);

  if (!vector) {
    return true;
  }

  const targetX = origin.x + vector.x;
  const targetY = origin.y + vector.y;

  if (!canMoveTo(player, targetX, targetY)) {
    return true;
  }

  if (vector.x !== 0 && vector.y !== 0) {
    const horizontal = canMoveTo(player, origin.x + vector.x, origin.y);
    const vertical = canMoveTo(player, origin.x, origin.y + vector.y);

    if (!horizontal && !vertical) {
      return true;
    }
  }

  return false;
};

const backgroundBlocked = player => player.blocked.background === true;
const foregroundBlocked = player => player.blocked.foreground === true;

const stopMovement = (player, data) => {
  Socket.emit('player:stopped', { player: data.player });
  player.moving = false;
  if (player.movementFeel) {
    player.movementFeel = { velocity: 0, timestamp: Date.now() };
  }
  setAnimationState(player, 'idle', { direction: player.facing });
  broadcastAnimation(player);
};

const resolvePlayerReference = (reference) => {
  if (reference && typeof reference === 'object') {
    return reference;
  }
  if (typeof reference === 'string') {
    return world.players.find(player => (
      player.uuid === reference || player.socket_id === reference
    )) || null;
  }
  return Number.isInteger(reference) ? world.players[reference] || null : null;
};

export const queueEmpty = (playerReference) => {
  const playerAtIndex = resolvePlayerReference(playerReference);

  if (!playerAtIndex || !Array.isArray(playerAtIndex.queue)) {
    return true;
  }

  return playerAtIndex.queue.length === 0;
};

const move = (player, direction, options = {}) => {
  if (player.disconnecting) {
    return false;
  }

  const context = typeof options === 'boolean' ? { pathfind: options } : options;
  const {
    pathfind = false,
    duration: durationOverride = null,
    walkId = null,
    stepIndex = null,
    steps = null,
    startedAt = Date.now(),
  } = context || {};

  if (!pathfind) {
    // Do not reset the animation timeline for every held-key sample.  The
    // route is still invalidated, so the legacy queued path cannot fight
    // continuous movement, but the visual motion remains continuous.
    interruptPathfinding(player);
  }

  if (pathfind) {
    player.moving = true;
  }

  const delta = pathfind ? directionDelta(direction) : playerMovementDelta(direction);
  const facing = setFacing(player, direction);

  if (!delta) {
    return false;
  }

  const attemptedWalkId = pathfind ? walkId : null;
  const duration = typeof durationOverride === 'number'
    ? durationOverride
    : (pathfind ? computeStepDuration(delta.x, delta.y) : PLAYER_MOVE_SAMPLE_MS);
  const origin = pathfind ? occupiedTile(player) : { x: player.x, y: player.y };

  if (isBlocked(player, direction, delta, origin)) {
    registerMovementStep(player, {
      duration: 0,
      walkId: attemptedWalkId,
      stepIndex,
      steps,
      startedAt,
      direction,
      blocked: true,
    });
    setAnimationState(player, 'idle', { direction: facing });
    return false;
  }

  const previousTile = occupiedTile(player);
  player.x = roundPosition(origin.x + delta.x);
  player.y = roundPosition(origin.y + delta.y);

  registerMovementStep(player, {
    duration,
    walkId: attemptedWalkId,
    stepIndex,
    steps,
    startedAt,
    direction,
    blocked: false,
  });
  setRunAnimation(player, direction, duration, startedAt);
  const currentTile = occupiedTile(player);
  if (currentTile.x !== previousTile.x || currentTile.y !== previousTile.y) {
    transitionPlayerIfOnPortal(player);
  }
  autoPickupCurrency(player);

  return true;
};

const walkPath = (player) => {
  const { path } = player;
  const baseSpeed = BASE_MOVE_DURATION;

  const isCurrentSession = () => world.players.some(candidate => (
    candidate === player
    && candidate.uuid === player.uuid
    && candidate.socket_id === player.socket_id
  ));

  const executeQueuedAction = () => {
    if (queueEmpty(player)) {
      return;
    }

    // Queued actions carry a client-supplied actionId; dispatch only
    // catalogue actions and drop anything else before it reaches a handler.
    const todo = player.queue[0];
    const actionId = todo && todo.action ? todo.action.actionId : null;
    if (!isAllowedActionId(actionId) || typeof playerEvent[actionId] !== 'function') {
      console.warn(`[queue] Dropped queued action with unknown actionId "${actionId}" for ${player.username || player.uuid}.`);
    } else {
      const result = playerEvent[actionId]({
        todo,
        playerUuid: player.uuid,
        socketId: player.socket_id,
      });
      if (result && typeof result.catch === 'function') {
        result.catch(error => console.error('[movement] Queued action failed:', error));
      }
    }
    player.queue.shift();
  };

  if (!path || !path.current || !Array.isArray(path.current.path.walking)) {
    return;
  }

  if (path.current.path.walking.length <= 1) {
    executeQueuedAction();

    stopMovement(player, { player: { socket_id: player.socket_id } });
    return;
  }

  path.current.walkId = (path.current.walkId || 0) + 1;
  const activeWalkId = path.current.walkId;
  path.current.interrupted = false;

  const scheduleNextStep = () => {
    if (!isCurrentSession()) {
      return;
    }

    if (path.current.walkId !== activeWalkId) {
      return;
    }

    if (path.current.step + 1 >= path.current.path.walking.length) {
      executeQueuedAction();

      stopMovement(player, { player: { socket_id: player.socket_id } });
      return;
    }

    const currentIndex = path.current.step;
    const currentCoordinates = {
      x: path.current.path.walking[currentIndex][0],
      y: path.current.path.walking[currentIndex][1],
    };
    const nextCoordinates = {
      x: path.current.path.walking[currentIndex + 1][0],
      y: path.current.path.walking[currentIndex + 1][1],
    };

    const movement = UI.getMovementDirection({
      current: currentCoordinates,
      next: nextCoordinates,
    });

    const deltaX = Math.abs(nextCoordinates.x - currentCoordinates.x);
    const deltaY = Math.abs(nextCoordinates.y - currentCoordinates.y);
    const stepDuration = computeStepDuration(deltaX, deltaY, baseSpeed);
    const totalSteps = Math.max(0, path.current.path.walking.length - 1);

    setTimeout(() => {
      if (!isCurrentSession()) {
        return;
      }

      if (path.current.walkId !== activeWalkId) {
        return;
      }

      const stepStartedAt = Date.now();
      move(player, movement, {
        pathfind: true,
        duration: stepDuration,
        walkId: activeWalkId,
        stepIndex: currentIndex + 1,
        steps: totalSteps,
        startedAt: stepStartedAt,
        direction: movement,
      });

      broadcastMovement(player);

      if (player.movementStep && player.movementStep.blocked) {
        path.current.interrupted = true;
        path.current.path.walking = [];
        path.current.path.set = [];
        path.current.length = 0;
        path.current.walkable = false;
        player.moving = false;
        return;
      }

      path.current.step += 1;
      scheduleNextStep();
    }, stepDuration);
  };

  scheduleNextStep();
};

const createPlayerMovementHandler = (player) => ({
  resolveFacing: (direction, fallback = player.facing || DEFAULT_FACING_DIRECTION) => (
    resolveFacing(direction, fallback)
  ),
  setFacing: direction => setFacing(player, direction),
  clearAnimationTimer: () => clearAnimationTimer(player),
  createInitialAnimation: overrides => createInitialAnimation(player, overrides),
  setAnimationState: (state, options) => setAnimationState(player, state, options),
  registerMovementStep: step => registerMovementStep(player, step),
  cancelPathfinding: () => cancelPathfinding(player),
  interruptPathfinding: () => interruptPathfinding(player),
  canMoveTo: (tileX, tileY) => canMoveTo(player, tileX, tileY),
  isBlocked: (direction, delta, origin) => isBlocked(player, direction, delta, origin),
  backgroundBlocked: () => backgroundBlocked(player),
  foregroundBlocked: () => foregroundBlocked(player),
  stopMovement: data => stopMovement(player, data),
  move: (direction, options) => move(player, direction, options),
  walkPath: () => walkPath(player),
});

export default createPlayerMovementHandler;
