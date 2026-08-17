const ANIMATION_TIMELINE_GUARD = Symbol('verdigris.animationTimelineGuard');

const sameAnimationState = (controller, snapshot) => (
  snapshot
  && typeof snapshot === 'object'
  && Number.isFinite(snapshot.sequence)
  && controller.sequence === snapshot.sequence
  && controller.state === (snapshot.state || controller.state)
  && controller.direction === (snapshot.direction || controller.direction)
);

/**
 * Keep SpriteAnimator's frame clock alive when a movement packet carries no
 * new animation state.  The historical movement path calls
 * ensureAnimationController() for those packets, which re-applies the actor's
 * last snapshot. SpriteAnimator intentionally resets on every application,
 * so this narrow player-event seam skips only an unchanged state and still
 * forwards all new sequences through the original implementation.
 */
export const installAnimationTimelineGuard = (actor) => {
  const controller = actor && actor.animationController;
  if (!controller || typeof controller.applyServerState !== 'function') {
    return false;
  }

  if (controller[ANIMATION_TIMELINE_GUARD]) {
    return true;
  }

  const applyServerState = controller.applyServerState;
  const guardedApplyServerState = function guardedApplyServerState(snapshot = {}) {
    if (sameAnimationState(this, snapshot)) {
      if (Number.isFinite(snapshot.speed)) {
        this.speed = snapshot.speed;
      }
      if (Number.isFinite(snapshot.duration)) {
        this.duration = snapshot.duration;
      }
      if (Object.prototype.hasOwnProperty.call(snapshot, 'skillId')) {
        this.skillId = snapshot.skillId || null;
      }
      if (Object.prototype.hasOwnProperty.call(snapshot, 'holdState')) {
        this.holdState = snapshot.holdState || null;
      }
      return true;
    }

    return applyServerState.call(this, snapshot);
  };

  Object.defineProperty(controller, 'applyServerState', {
    configurable: true,
    value: guardedApplyServerState,
    writable: true,
  });
  Object.defineProperty(controller, ANIMATION_TIMELINE_GUARD, {
    configurable: false,
    value: true,
  });

  return true;
};

export default installAnimationTimelineGuard;
