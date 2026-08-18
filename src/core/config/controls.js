import { PLAYER_MOVE_SAMPLE_MS } from '@shared/movement.js';
import { listSkills } from '@shared/skills/index.js';

export const MOVEMENT_BINDINGS = {
  up: ['w', 'arrowup'],
  down: ['s', 'arrowdown'],
  left: ['a', 'arrowleft'],
  right: ['d', 'arrowright'],
};

export const DIAGONAL_BINDINGS = [
  ['up', 'right', 'up-right'],
  ['down', 'right', 'down-right'],
  ['up', 'left', 'up-left'],
  ['down', 'left', 'down-left'],
];

export const MOVEMENT_REPEAT = {
  initialDelayMs: PLAYER_MOVE_SAMPLE_MS,
  repeatDelayMs: PLAYER_MOVE_SAMPLE_MS,
};

// ── Rebindable action map (TASK-0038) ────────────────────────────────────────
//
// Default shape follows D-007: LMB primary attack, RMB weapon skill, Space
// dodge/dash, Q/E/R/F skill slots, number-row aliases kept for the quickbar.
// Bindings are persisted client-side (localStorage) and applied live.

export const CONTROLS_STORAGE_KEY = 'verdigris:controls:v1';

// Mouse buttons are first-class binding values so the two world-attack
// buttons can be remapped like any key.
export const MOUSE_BUTTON_TO_BINDING = {
  0: 'mouse0',
  1: 'mouse1',
  2: 'mouse2',
};

const MOUSE_BINDING_LABELS = {
  mouse0: 'LMB',
  mouse1: 'MMB',
  mouse2: 'RMB',
};

const DEFAULT_ACTION_BINDINGS = {
  'primary-attack': ['mouse0', '1'],
  dash: [' ', 'shift', '2'],
  'ability-1': ['mouse2', 'q', '3'],
  'ability-2': ['e', '4'],
  'ability-3': ['r', '5'],
  'ability-4': ['f', '6'],
};

// One rebindable action per registered skill, labelled from the shared skill
// registry so the settings UI and the quickbar speak the same names.
export const ACTION_DEFINITIONS = listSkills().map((skill) => ({
  id: skill.id,
  label: skill.label || skill.name || skill.id,
  type: 'press',
  defaults: [...(DEFAULT_ACTION_BINDINGS[skill.id] || [])],
}));

const ACTION_IDS = new Set(ACTION_DEFINITIONS.map((action) => action.id));

const SPECIAL_KEY_ALIASES = {
  space: ' ',
  spacebar: ' ',
};

const getStorage = () => {
  try {
    if (typeof window !== 'undefined' && window.localStorage) {
      return window.localStorage;
    }
  } catch (error) {
    // Access to localStorage can throw in hardened contexts; bindings then
    // simply run on defaults for the session.
  }
  return null;
};

/** Normalise a KeyboardEvent.key / mouse pseudo-key into a binding value. */
export const normaliseBinding = (value) => {
  if (typeof value !== 'string') {
    return '';
  }
  const lower = value.toLowerCase();
  if (SPECIAL_KEY_ALIASES[lower] !== undefined) {
    return SPECIAL_KEY_ALIASES[lower];
  }
  if (lower === ' ') {
    return ' ';
  }
  const trimmed = lower.trim();
  if (!trimmed) {
    return '';
  }
  if (Object.prototype.hasOwnProperty.call(MOUSE_BINDING_LABELS, trimmed)) {
    return trimmed;
  }
  return trimmed;
};

/** Human-readable label for a binding value (LMB, RMB, Space, Q, Shift…). */
export const displayBinding = (binding) => {
  const value = normaliseBinding(binding);
  if (!value) {
    return '';
  }
  if (MOUSE_BINDING_LABELS[value]) {
    return MOUSE_BINDING_LABELS[value];
  }
  if (value === ' ') {
    return 'Space';
  }
  if (value.length === 1) {
    return value.toUpperCase();
  }
  return value.charAt(0).toUpperCase() + value.slice(1);
};

const sanitiseBindingList = (list) => {
  if (!Array.isArray(list)) {
    return null;
  }
  const seen = new Set();
  const result = [];
  list.forEach((entry) => {
    const value = normaliseBinding(entry);
    if (value && !seen.has(value)) {
      seen.add(value);
      result.push(value);
    }
  });
  return result;
};

export const defaultBindings = () => Object.fromEntries(
  ACTION_DEFINITIONS.map((action) => [action.id, [...action.defaults]]),
);

/** Defaults merged with the persisted override (unknown actions ignored). */
export const loadBindings = () => {
  const map = defaultBindings();
  const storage = getStorage();
  if (!storage) {
    return map;
  }
  try {
    const raw = storage.getItem(CONTROLS_STORAGE_KEY);
    if (!raw) {
      return map;
    }
    const stored = JSON.parse(raw);
    ACTION_DEFINITIONS.forEach((action) => {
      const list = sanitiseBindingList(stored && stored[action.id]);
      if (list) {
        map[action.id] = list;
      }
    });
  } catch (error) {
    // A corrupt payload must never break input; fall back to defaults.
  }
  return map;
};

let liveBindings = loadBindings();
const listeners = new Set();

const persistBindings = () => {
  const storage = getStorage();
  if (!storage) {
    return;
  }
  try {
    storage.setItem(CONTROLS_STORAGE_KEY, JSON.stringify(liveBindings));
  } catch (error) {
    // Quota/serialisation failures leave the in-memory map authoritative.
  }
};

const notifyBindings = () => {
  listeners.forEach((listener) => {
    try {
      listener(getBindings());
    } catch (error) {
      // A broken subscriber must not break input for everyone else.
    }
  });
};

/** Subscribe to live binding changes. Returns an unsubscribe function. */
export const subscribeBindings = (listener) => {
  listeners.add(listener);
  return () => listeners.delete(listener);
};

/** Current bindings as { actionId: [binding, …] } (defensive copy). */
export const getBindings = () => Object.fromEntries(
  Object.entries(liveBindings).map(([id, list]) => [id, [...list]]),
);

export const getActionBindings = (actionId) => [...(liveBindings[actionId] || [])];

/** Skill-binding rows in the shape the input controller consumes. */
export const skillBindings = () => ACTION_DEFINITIONS.map((action) => ({
  id: action.id,
  label: action.label,
  type: action.type,
  keys: getActionBindings(action.id),
}));

// Back-compat snapshot for legacy consumers; live readers should use
// skillBindings()/getActionBindings() instead.
export const SKILL_BINDINGS = skillBindings();

/** binding -> actionId. First action in registry order wins a collision. */
export const bindingLookup = () => {
  const lookup = new Map();
  ACTION_DEFINITIONS.forEach((action) => {
    getActionBindings(action.id).forEach((binding) => {
      if (!lookup.has(binding)) {
        lookup.set(binding, action.id);
      }
    });
  });
  return lookup;
};

/** The action currently holding a binding, or null. */
export const findBindingOwner = (binding) => {
  const value = normaliseBinding(binding);
  if (!value) {
    return null;
  }
  for (const action of ACTION_DEFINITIONS) {
    if (getActionBindings(action.id).includes(value)) {
      return action.id;
    }
  }
  return null;
};

/** All bindings held by more than one action: [{ binding, actions }]. */
export const bindingConflicts = (map = liveBindings) => {
  const owners = new Map();
  Object.entries(map).forEach(([actionId, list]) => {
    (list || []).forEach((binding) => {
      const value = normaliseBinding(binding);
      if (!value) {
        return;
      }
      if (!owners.has(value)) {
        owners.set(value, []);
      }
      owners.get(value).push(actionId);
    });
  });
  return [...owners.entries()]
    .filter(([, actions]) => actions.length > 1)
    .map(([binding, actions]) => ({ binding, actions }));
};

/**
 * Add a binding to an action. Refuses (without mutating) when the binding is
 * already held by another action — no two actions share one binding.
 * Returns { ok: true } or { ok: false, conflict: actionId }.
 */
export const addActionBinding = (actionId, binding) => {
  if (!ACTION_IDS.has(actionId)) {
    return { ok: false, reason: 'unknown-action' };
  }
  const value = normaliseBinding(binding);
  if (!value) {
    return { ok: false, reason: 'invalid-binding' };
  }
  const owner = findBindingOwner(value);
  if (owner === actionId) {
    return { ok: true, alreadyBound: true };
  }
  if (owner) {
    return { ok: false, conflict: owner };
  }
  liveBindings = { ...liveBindings, [actionId]: [...getActionBindings(actionId), value] };
  persistBindings();
  notifyBindings();
  return { ok: true };
};

export const removeActionBinding = (actionId, binding) => {
  if (!ACTION_IDS.has(actionId)) {
    return { ok: false, reason: 'unknown-action' };
  }
  const value = normaliseBinding(binding);
  const next = getActionBindings(actionId).filter((entry) => entry !== value);
  liveBindings = { ...liveBindings, [actionId]: next };
  persistBindings();
  notifyBindings();
  return { ok: true };
};

/** Replace every binding with the factory defaults (and persist that). */
export const resetBindings = () => {
  liveBindings = defaultBindings();
  persistBindings();
  notifyBindings();
  return getBindings();
};

/** Display label for an action's primary binding (quickbar/skill bar). */
export const primaryBindingLabel = (actionId) => {
  const [first] = getActionBindings(actionId);
  return first ? displayBinding(first) : '';
};

export default {
  MOVEMENT_BINDINGS,
  DIAGONAL_BINDINGS,
  MOVEMENT_REPEAT,
  SKILL_BINDINGS,
  ACTION_DEFINITIONS,
  CONTROLS_STORAGE_KEY,
  MOUSE_BUTTON_TO_BINDING,
  normaliseBinding,
  displayBinding,
  defaultBindings,
  loadBindings,
  getBindings,
  getActionBindings,
  skillBindings,
  bindingLookup,
  findBindingOwner,
  bindingConflicts,
  addActionBinding,
  removeActionBinding,
  resetBindings,
  subscribeBindings,
  primaryBindingLabel,
};
