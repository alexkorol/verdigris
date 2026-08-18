import {
  MOVEMENT_BINDINGS,
  DIAGONAL_BINDINGS,
  MOVEMENT_REPEAT,
  MOUSE_BUTTON_TO_BINDING,
  normaliseBinding,
  skillBindings,
  subscribeBindings,
} from '../config/controls.js';

const MOVEMENT_KEYS = new Set(
  Object.values(MOVEMENT_BINDINGS)
    .reduce((acc, keys) => acc.concat(keys), [])
    .map((key) => key.toLowerCase()),
);

class InputController {
  constructor(options = {}) {
    this.onMove = options.onMove || null;
    this.onStop = options.onStop || null;
    this.onSkill = options.onSkill || null;
    // Tests may inject a fixed binding table; production reads the live,
    // rebindable map from config/controls.js and follows rebinds instantly.
    this.fixedBindings = Array.isArray(options.bindings) ? options.bindings : null;

    this.pressedKeys = new Set();
    this.activeDirection = null;
    this.repeatTimeout = null;
    this.nextRepeatAt = null;

    this.rebuildSkillLookup();
    this.unsubscribeBindings = this.fixedBindings
      ? null
      : subscribeBindings(() => this.rebuildSkillLookup());
  }

  destroy() {
    this.clearRepeat();
    this.pressedKeys.clear();
    if (typeof this.unsubscribeBindings === 'function') {
      this.unsubscribeBindings();
      this.unsubscribeBindings = null;
    }
  }

  rebuildSkillLookup() {
    const bindings = this.fixedBindings || skillBindings();
    this.skillLookup = bindings.reduce((acc, binding) => {
      (binding.keys || []).forEach((key) => {
        const value = normaliseBinding(key);
        if (value && !acc.has(value)) {
          acc.set(value, binding);
        }
      });
      return acc;
    }, new Map());
  }

  normaliseKey(rawKey) {
    return normaliseBinding(rawKey);
  }

  handleKeyDown(event) {
    const key = this.normaliseKey(event.key);
    if (!key) {
      return false;
    }

    if (this.isMovementKey(key)) {
      if (!this.pressedKeys.has(key)) {
        this.pressedKeys.add(key);
        this.updateMovement(true);
      }
      return true;
    }

    const binding = this.getSkillBinding(key);
    if (binding) {
      if (event && event.repeat) {
        return true;
      }
      this.triggerSkill(binding, 'start', event);
      return true;
    }

    return false;
  }

  handleKeyUp(event) {
    const key = this.normaliseKey(event.key);
    if (!key) {
      return false;
    }

    let handled = false;
    if (this.pressedKeys.has(key)) {
      this.pressedKeys.delete(key);
      this.updateMovement(false);
      handled = true;
    }

    const binding = this.getSkillBinding(key);
    if (binding && binding.type === 'hold') {
      this.triggerSkill(binding, 'end', event);
      handled = true;
    }

    if (binding && binding.type !== 'hold') {
      handled = true;
    }

    return handled;
  }

  isMovementKey(key) {
    return MOVEMENT_KEYS.has(key);
  }

  getSkillBinding(key) {
    return this.skillLookup.get(key) || null;
  }

  /**
   * Resolve a DOM mouse button index (0 = LMB, 2 = RMB) to the skill binding
   * the player has mapped to it, or null when the button is unbound.
   */
  getMouseBinding(button) {
    const pseudoKey = MOUSE_BUTTON_TO_BINDING[button];
    if (!pseudoKey) {
      return null;
    }
    return this.getSkillBinding(pseudoKey);
  }

  updateMovement(initialTrigger = false) {
    const direction = this.computeDirection();

    if (!direction) {
      if (this.activeDirection && typeof this.onStop === 'function') {
        this.onStop(this.activeDirection);
      }
      this.activeDirection = null;
      this.clearRepeat();
      return;
    }

    if (direction !== this.activeDirection || initialTrigger) {
      this.activeDirection = direction;
      if (typeof this.onMove === 'function') {
        this.onMove(direction, { initial: true });
      }
      this.restartRepeat();
      return;
    }

    this.ensureRepeat();
  }

  computeDirection() {
    const has = (keys) => keys.some((key) => this.pressedKeys.has(key));

    const up = has(MOVEMENT_BINDINGS.up);
    const down = has(MOVEMENT_BINDINGS.down);
    const left = has(MOVEMENT_BINDINGS.left);
    const right = has(MOVEMENT_BINDINGS.right);

    if ((up && down) || (left && right)) {
      return null;
    }

    for (let i = 0; i < DIAGONAL_BINDINGS.length; i += 1) {
      const [vertical, horizontal, diagonal] = DIAGONAL_BINDINGS[i];
      if (vertical === 'up' && horizontal === 'right' && up && right) {
        return diagonal;
      }
      if (vertical === 'down' && horizontal === 'right' && down && right) {
        return diagonal;
      }
      if (vertical === 'up' && horizontal === 'left' && up && left) {
        return diagonal;
      }
      if (vertical === 'down' && horizontal === 'left' && down && left) {
        return diagonal;
      }
    }

    if (up) return 'up';
    if (down) return 'down';
    if (left) return 'left';
    if (right) return 'right';

    return null;
  }

  getRepeatDelay() {
    return MOVEMENT_REPEAT.repeatDelayMs;
  }

  restartRepeat() {
    this.clearRepeat();
    if (!this.activeDirection) {
      return;
    }

    // A self-scheduling timeout keeps the cadence anchored to a monotonic
    // deadline.  setInterval accumulated timer drift and could bunch a
    // movement sample against the next rAF after a busy frame, which exposed
    // the server's otherwise smooth interpolation as a visible hitch.
    this.nextRepeatAt = Date.now() + MOVEMENT_REPEAT.initialDelayMs;
    this.scheduleRepeat();
  }

  scheduleRepeat() {
    if (!this.activeDirection || !Number.isFinite(this.nextRepeatAt)) {
      return;
    }

    const delay = Math.max(0, this.nextRepeatAt - Date.now());
    this.repeatTimeout = setTimeout(() => {
      this.repeatTimeout = null;
      if (!this.activeDirection) {
        return;
      }

      if (typeof this.onMove === 'function') {
        this.onMove(this.activeDirection, { repeated: true });
      }

      const cadence = this.getRepeatDelay();
      const now = Date.now();
      // If the tab was backgrounded or the main thread was blocked, resume
      // from the current frame instead of firing a burst of stale samples.
      this.nextRepeatAt = Math.max(this.nextRepeatAt + cadence, now + cadence);
      this.scheduleRepeat();
    }, delay);
  }

  ensureRepeat() {
    if (this.repeatTimeout !== null || Number.isFinite(this.nextRepeatAt)) {
      return;
    }
    this.restartRepeat();
  }

  clearRepeat() {
    if (this.repeatTimeout !== null) {
      clearTimeout(this.repeatTimeout);
      this.repeatTimeout = null;
    }
    this.nextRepeatAt = null;
  }

  triggerSkill(binding, phase, event) {
    if (!binding) {
      return;
    }

    if (binding.type !== 'hold' && phase !== 'start') {
      return;
    }

    if (typeof this.onSkill === 'function') {
      this.onSkill({
        skillId: binding.id,
        phase,
        event,
      });
    }
  }
}

export default InputController;
