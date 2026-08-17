/** @vitest-environment node */

import {
  afterEach,
  beforeEach,
  describe,
  expect,
  it,
  vi,
} from 'vitest';

import Socket from '#server/socket.js';
import world from '#server/core/world.js';
import InputController from '../../src/core/utilities/input-controller.js';
import { MOVEMENT_REPEAT } from '../../src/core/config/controls.js';
import playerEvents from '../../src/core/player/events/player.js';
import createPlayerMovementHandler, {
  MOVEMENT_FEEL,
  broadcastMovement,
} from '#server/core/entities/player/movement-handler.js';

const sceneId = 'test:movement-feel';

const openMap = () => ({
  background: new Array(200 * 200).fill(1),
  foreground: new Array(200 * 200).fill(0),
});

const makePlayer = () => {
  const player = {
    uuid: 'movement-feel-player',
    socket_id: 'movement-feel-socket',
    username: 'Movement Feel Tester',
    x: 10,
    y: 10,
    sceneId,
    facing: 'right',
    moving: false,
    action: false,
    queue: [{ action: { actionId: 'stale-path-action' } }],
    blocked: { foreground: null, background: null },
    path: {
      current: {
        length: 2,
        path: {
          walking: [[10, 10], [11, 10], [12, 10]],
          set: [[10, 10], [12, 10]],
        },
        step: 1,
        walkable: true,
        interrupted: false,
        walkId: 4,
      },
    },
    movementStep: {
      sequence: 0,
      startedAt: 0,
      duration: 0,
      walkId: null,
      stepIndex: null,
      steps: null,
      direction: null,
      blocked: false,
      interrupted: false,
    },
    animation: {
      state: 'run',
      direction: 'right',
      sequence: 7,
      startedAt: 900,
      duration: 50,
      speed: 1,
      skillId: null,
      holdState: null,
    },
  };

  player.movement = createPlayerMovementHandler(player);
  player.move = player.movement.move;
  player.walkPath = player.movement.walkPath;
  return player;
};

describe('browser movement feel contract', () => {
  beforeEach(() => {
    vi.spyOn(Socket, 'broadcast').mockImplementation(() => {});
    vi.spyOn(Socket, 'emit').mockImplementation(() => {});
    world.clients = [];
    world._players = [];
  });

  afterEach(() => {
    vi.restoreAllMocks();
    world.scenes.delete(sceneId);
    world._players = [];
  });

  it('derives one coherent speed, acceleration, interpolation, and screen-crossing table', () => {
    expect(MOVEMENT_FEEL.speedTilesPerSecond).toBeCloseTo(1000 / 150, 8);
    expect(MOVEMENT_FEEL.accelerationTilesPerSecondSquared).toBeCloseTo(
      MOVEMENT_FEEL.speedTilesPerSecond / MOVEMENT_FEEL.accelerationSeconds,
      8,
    );
    expect(MOVEMENT_FEEL.interpolationWindowMs).toBe(100);
    expect(MOVEMENT_FEEL.secondsToCrossScreen).toBeCloseTo(
      MOVEMENT_FEEL.screenWidthTiles / MOVEMENT_FEEL.speedTilesPerSecond,
      8,
    );
  });

  it('invalidates a legacy queued path without restarting a held run animation', () => {
    const player = makePlayer();
    const scene = world.ensureScene(sceneId, {
      type: 'test',
      map: openMap(),
      monsters: [],
      metadata: { portals: [], spawnPoints: [{ x: 10, y: 10 }] },
    });
    scene.players = [player];
    world.players.push(player);

    expect(player.move('right', { startedAt: 1_000 })).toBe(true);

    expect(player.path.current.path.walking).toEqual([]);
    expect(player.path.current.path.set).toEqual([]);
    expect(player.path.current.interrupted).toBe(true);
    expect(player.queue).toEqual([]);
    expect(player.animation.state).toBe('run');
    expect(player.animation.direction).toBe('right');
    expect(player.animation.sequence).toBe(7);
  });

  it('ramps the run animation on held samples without changing authoritative displacement', () => {
    const player = makePlayer();
    const scene = world.ensureScene(sceneId, {
      type: 'test',
      map: openMap(),
      monsters: [],
      metadata: { portals: [], spawnPoints: [{ x: 10, y: 10 }] },
    });
    scene.players = [player];
    world.players.push(player);

    player.move('right', { startedAt: 1_000 });
    const firstSpeed = player.animation.speed;
    player.move('right', { startedAt: 1_050 });
    const secondSpeed = player.animation.speed;
    player.move('right', { startedAt: 1_100 });
    const thirdSpeed = player.animation.speed;

    expect(firstSpeed).toBe(MOVEMENT_FEEL.initialAnimationSpeed);
    expect(secondSpeed).toBeGreaterThan(firstSpeed);
    expect(thirdSpeed).toBeGreaterThan(secondSpeed);
    expect(player.x).toBeCloseTo(10 + (3 * (50 / 150)), 6);
    expect(player.y).toBe(10);
    expect(player.movementStep.duration).toBe(50);
  });

  it('resets the feel ramp when continuous input stops', () => {
    const player = makePlayer();
    const scene = world.ensureScene(sceneId, {
      type: 'test',
      map: openMap(),
      monsters: [],
      metadata: { portals: [], spawnPoints: [{ x: 10, y: 10 }] },
    });
    scene.players = [player];
    world.players.push(player);

    player.move('right', { startedAt: 1_000 });
    player.move('right', { startedAt: 1_050 });
    player.movement.stopMovement({ player: { socket_id: player.socket_id } });
    player.move('right', { startedAt: 2_000 });

    expect(player.animation.speed).toBe(MOVEMENT_FEEL.initialAnimationSpeed);
  });

  it('does not rebroadcast an unchanged animation sequence with movement samples', () => {
    const player = makePlayer();
    const recipients = [player];

    broadcastMovement(player, recipients);
    broadcastMovement(player, recipients);

    expect(Socket.broadcast).toHaveBeenCalledTimes(2);
    const firstPayload = Socket.broadcast.mock.calls[0][1];
    const secondPayload = Socket.broadcast.mock.calls[1][1];
    const firstMeta = Socket.broadcast.mock.calls[0][3].meta;
    const secondMeta = Socket.broadcast.mock.calls[1][3].meta;
    expect(firstPayload.animation).toEqual(expect.objectContaining({ sequence: 7 }));
    expect(secondPayload.animation).toBeUndefined();
    expect(firstMeta.animation).toEqual(expect.objectContaining({ sequence: 7 }));
    expect(secondMeta.animation).toBeUndefined();
    expect(secondMeta.movementStep).toEqual(player.movementStep);

    player.animation = { ...player.animation, sequence: 8 };
    broadcastMovement(player, recipients);
    const thirdMeta = Socket.broadcast.mock.calls[2][3].meta;
    expect(thirdMeta.animation).toEqual(expect.objectContaining({ sequence: 8 }));
  });

  it('ensureRepeat schedules from the new deadline state', () => {
    vi.useFakeTimers();
    const onMove = vi.fn();
    const controller = new InputController({ onMove });
    controller.activeDirection = 'right';

    controller.ensureRepeat();
    expect(controller.repeatTimeout).not.toBeNull();

    vi.advanceTimersByTime(MOVEMENT_REPEAT.initialDelayMs);
    expect(onMove).toHaveBeenCalledWith('right', { repeated: true });

    controller.destroy();
    vi.useRealTimers();
  });

  it('guards the event path from resetting an omitted same-sequence animation', () => {
    const actor = {
      uuid: 'movement-event-player',
      animation: null,
      animationController: null,
    };
    const makeController = () => ({
      state: 'idle',
      direction: 'right',
      sequence: 0,
      speed: 1,
      duration: 0,
      frameIndex: 0,
      elapsedMs: 0,
      applyCalls: 0,
      applyServerState(snapshot) {
        this.applyCalls += 1;
        this.state = snapshot.state || this.state;
        this.direction = snapshot.direction || this.direction;
        this.sequence = snapshot.sequence;
        this.speed = snapshot.speed;
        this.duration = snapshot.duration;
        this.frameIndex = 0;
        this.elapsedMs = 0;
        return true;
      },
      toJSON() {
        return {
          state: this.state,
          direction: this.direction,
          sequence: this.sequence,
          speed: this.speed,
          duration: this.duration,
        };
      },
    });
    const context = {
      game: {
        player: actor,
        map: { players: [] },
      },
      playerMovement: (payload, meta) => {
        if (!actor.animationController) {
          actor.animationController = makeController();
        }
        const snapshot = meta.animation || payload.animation || actor.animation;
        actor.animationController.applyServerState(snapshot);
        actor.animation = actor.animationController.toJSON();
      },
    };
    const firstAnimation = {
      state: 'run',
      direction: 'right',
      sequence: 11,
      speed: 0.75,
      duration: 50,
    };

    playerEvents['player:movement']({
      data: { uuid: actor.uuid, animation: firstAnimation },
      meta: { animation: firstAnimation },
    }, context);
    expect(actor.animationController.applyCalls).toBe(1);

    actor.animationController.frameIndex = 2;
    actor.animationController.elapsedMs = 23;
    playerEvents['player:movement']({
      data: { uuid: actor.uuid },
      meta: {},
    }, context);

    expect(actor.animationController.applyCalls).toBe(1);
    expect(actor.animationController.frameIndex).toBe(2);
    expect(actor.animationController.elapsedMs).toBe(23);

    const nextAnimation = { ...firstAnimation, sequence: 12, speed: 1 };
    playerEvents['player:movement']({
      data: { uuid: actor.uuid },
      meta: { animation: nextAnimation },
    }, context);
    expect(actor.animationController.applyCalls).toBe(2);
    expect(actor.animationController.frameIndex).toBe(0);
  });
});
