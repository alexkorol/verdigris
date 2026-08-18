import {
  beforeEach,
  describe,
  expect,
  it,
  vi,
} from 'vitest';

import {
  ACTION_DEFINITIONS,
  CONTROLS_STORAGE_KEY,
  addActionBinding,
  bindingConflicts,
  bindingLookup,
  defaultBindings,
  displayBinding,
  findBindingOwner,
  getActionBindings,
  getBindings,
  loadBindings,
  normaliseBinding,
  primaryBindingLabel,
  removeActionBinding,
  resetBindings,
  subscribeBindings,
} from '../../src/core/config/controls.js';
import InputController from '../../src/core/utilities/input-controller.js';

const createStorage = () => {
  const data = new Map();
  return {
    getItem: (key) => (data.has(key) ? data.get(key) : null),
    setItem: (key, value) => { data.set(key, String(value)); },
    removeItem: (key) => { data.delete(key); },
    dump: () => Object.fromEntries(data),
  };
};

beforeEach(() => {
  // Every test starts from factory defaults against a fresh storage stub.
  vi.stubGlobal('window', { localStorage: createStorage() });
  resetBindings();
});

describe('controls binding map', () => {
  it('ships the D-007 default shape', () => {
    const bindings = getBindings();
    expect(bindings['primary-attack']).toContain('mouse0');
    expect(bindings['primary-attack']).toContain('1');
    expect(bindings['ability-1']).toContain('mouse2');
    expect(bindings.dash).toContain(' ');
    expect(bindings.dash).toContain('shift');
    expect(bindings['ability-2']).toContain('e');
    expect(bindings['ability-3']).toContain('r');
    expect(bindings['ability-4']).toContain('f');
    // No two actions share a binding out of the box.
    expect(bindingConflicts()).toEqual([]);
  });

  it('derives one action per registered skill with registry labels', () => {
    const ids = ACTION_DEFINITIONS.map((action) => action.id);
    expect(ids).toEqual([
      'primary-attack',
      'dash',
      'ability-1',
      'ability-2',
      'ability-3',
      'ability-4',
    ]);
    const primary = ACTION_DEFINITIONS.find((action) => action.id === 'primary-attack');
    expect(primary.label).toBe('Bronze Arc');
  });

  it('normalises keyboard and mouse binding values', () => {
    expect(normaliseBinding('Space')).toBe(' ');
    expect(normaliseBinding('Spacebar')).toBe(' ');
    expect(normaliseBinding('Q')).toBe('q');
    expect(normaliseBinding('Shift')).toBe('shift');
    expect(normaliseBinding('mouse0')).toBe('mouse0');
    expect(normaliseBinding('MOUSE2')).toBe('mouse2');
    expect(normaliseBinding('')).toBe('');
    expect(normaliseBinding(null)).toBe('');
  });

  it('labels bindings for display', () => {
    expect(displayBinding('mouse0')).toBe('LMB');
    expect(displayBinding('mouse2')).toBe('RMB');
    expect(displayBinding(' ')).toBe('Space');
    expect(displayBinding('q')).toBe('Q');
    expect(displayBinding('shift')).toBe('Shift');
    expect(primaryBindingLabel('primary-attack')).toBe('LMB');
    expect(primaryBindingLabel('ability-1')).toBe('RMB');
  });

  it('rejects assigning one binding to two actions', () => {
    const result = addActionBinding('ability-2', 'q');
    expect(result.ok).toBe(false);
    expect(result.conflict).toBe('ability-1');
    expect(getActionBindings('ability-2')).not.toContain('q');
    expect(getActionBindings('ability-1')).toContain('q');
  });

  it('rebinds an action to a fresh key and resolves the lookup', () => {
    expect(removeActionBinding('ability-3', 'r').ok).toBe(true);
    expect(addActionBinding('ability-3', 't').ok).toBe(true);
    expect(findBindingOwner('t')).toBe('ability-3');
    expect(bindingLookup().get('t')).toBe('ability-3');
    expect(bindingLookup().has('r')).toBe(false);
  });

  it('persists rebinding through localStorage and reloads merged', () => {
    addActionBinding('dash', 'x');
    const stored = JSON.parse(window.localStorage.getItem(CONTROLS_STORAGE_KEY));
    expect(stored.dash).toContain('x');

    const reloaded = loadBindings();
    expect(reloaded.dash).toContain('x');
    expect(reloaded.dash).toContain(' '); // defaults survive alongside
    expect(reloaded['primary-attack']).toContain('mouse0');
  });

  it('ignores a corrupt persisted payload and falls back to defaults', () => {
    window.localStorage.setItem(CONTROLS_STORAGE_KEY, '{not json');
    expect(loadBindings()).toEqual(defaultBindings());
  });

  it('notifies subscribers live on rebind and reset', () => {
    const listener = vi.fn();
    const unsubscribe = subscribeBindings(listener);
    addActionBinding('ability-4', 'g');
    expect(listener).toHaveBeenCalledTimes(1);
    resetBindings();
    expect(listener).toHaveBeenCalledTimes(2);
    unsubscribe();
    addActionBinding('ability-4', 'h');
    expect(listener).toHaveBeenCalledTimes(2);
  });

  it('drives the input controller with live mouse-button bindings', () => {
    const onSkill = vi.fn();
    const controller = new InputController({ onSkill });

    expect(controller.getMouseBinding(0).id).toBe('primary-attack');
    expect(controller.getMouseBinding(2).id).toBe('ability-1');
    expect(controller.getMouseBinding(1)).toBe(null);
    expect(controller.getMouseBinding(9)).toBe(null);
    controller.destroy();
  });

  it('applies a rebind to the input controller without a restart', () => {
    const onSkill = vi.fn();
    const controller = new InputController({ onSkill });
    const keyEvent = (key) => ({ key, preventDefault: vi.fn() });

    expect(controller.handleKeyDown(keyEvent('q'))).toBe(true);
    expect(onSkill).toHaveBeenLastCalledWith(expect.objectContaining({ skillId: 'ability-1' }));

    removeActionBinding('ability-1', 'q');
    addActionBinding('ability-1', 't');

    expect(controller.handleKeyDown(keyEvent('t'))).toBe(true);
    expect(onSkill).toHaveBeenLastCalledWith(expect.objectContaining({ skillId: 'ability-1' }));
    expect(controller.getSkillBinding('q')).toBe(null);
    controller.destroy();
  });

  it('lets a rebind move the mouse attack buttons', () => {
    const onSkill = vi.fn();
    const controller = new InputController({ onSkill });

    // Move LMB from primary attack to the heal skill: the button follows.
    removeActionBinding('primary-attack', 'mouse0');
    expect(addActionBinding('ability-4', 'mouse0').ok).toBe(true);
    expect(controller.getMouseBinding(0).id).toBe('ability-4');
    expect(getActionBindings('ability-4')).toContain('mouse0');
    expect(displayBinding('mouse0')).toBe('LMB');
    controller.destroy();
  });
});
