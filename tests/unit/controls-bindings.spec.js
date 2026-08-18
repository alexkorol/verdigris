import { describe, it, expect, beforeEach, vi } from 'vitest';

const store = new Map();
vi.stubGlobal('window', {
  localStorage: {
    getItem: (k) => (store.has(k) ? store.get(k) : null),
    setItem: (k, v) => store.set(k, String(v)),
    removeItem: (k) => store.delete(k),
  },
});

const {
  getActionBindings,
  skillBindings,
  findBindingOwner,
  bindingConflicts,
  addActionBinding,
  removeActionBinding,
  resetBindings,
  normaliseBinding,
  displayBinding,
  primaryBindingLabel,
  subscribeBindings,
  ACTION_DEFINITIONS,
} = await import('../../src/core/config/controls.js');

describe('controls bindings config', () => {
  beforeEach(() => {
    store.clear();
    resetBindings();
  });

  it('exposes the six combat actions with D-007 defaults', () => {
    const ids = ACTION_DEFINITIONS.map((d) => d.id);
    expect(ids).toEqual(['primary-attack', 'dash', 'ability-1', 'ability-2', 'ability-3', 'ability-4']);
    expect(getActionBindings('primary-attack')).toEqual(['mouse0', '1']);
    expect(getActionBindings('dash')).toEqual([' ', 'shift', '2']);
    expect(getActionBindings('ability-1')).toEqual(['mouse2', 'q', '3']);
  });

  it('labels actions from the shared skill registry', () => {
    const primary = ACTION_DEFINITIONS.find((d) => d.id === 'primary-attack');
    expect(primary.label).toBe('Bronze Arc');
    const ability1 = ACTION_DEFINITIONS.find((d) => d.id === 'ability-1');
    expect(ability1.label).toBe('Cinder Fan');
  });

  it('maps skill ids to binding rows for the input controller', () => {
    const rows = skillBindings();
    const primary = rows.find((r) => r.id === 'primary-attack');
    expect(primary.keys).toEqual(['mouse0', '1']);
    expect(primary.type).toBe('press');
  });

  it('finds binding owners including mouse buttons', () => {
    expect(findBindingOwner('mouse0')).toBe('primary-attack');
    expect(findBindingOwner('q')).toBe('ability-1');
    expect(findBindingOwner('unbound-key')).toBeNull();
  });

  it('ships defaults without conflicts', () => {
    expect(bindingConflicts()).toEqual([]);
  });

  it('refuses to add a binding that conflicts with another action', () => {
    const result = addActionBinding('dash', 'q');
    expect(result.ok).toBe(false);
    expect(result.conflict).toBe('ability-1');
    expect(getActionBindings('dash')).not.toContain('q');
  });

  it('adds, removes, persists, and resets bindings', () => {
    expect(addActionBinding('ability-2', 't').ok).toBe(true);
    expect(getActionBindings('ability-2')).toContain('t');
    expect(store.get('verdigris:controls:v1')).toContain('"t"');
    removeActionBinding('ability-2', 't');
    expect(getActionBindings('ability-2')).not.toContain('t');
    addActionBinding('ability-2', 't');
    resetBindings();
    expect(getActionBindings('ability-2')).not.toContain('t');
  });

  it('notifies subscribers on change', () => {
    const seen = [];
    const unsubscribe = subscribeBindings((map) => seen.push(map));
    addActionBinding('ability-3', 'g');
    expect(seen.length).toBe(1);
    expect(seen[0]['ability-3']).toContain('g');
    unsubscribe();
    addActionBinding('ability-3', 'h');
    expect(seen.length).toBe(1);
  });

  it('normalises and formats bindings for display', () => {
    expect(normaliseBinding('Space')).toBe(' ');
    expect(normaliseBinding('Q')).toBe('q');
    expect(displayBinding('mouse0')).toBe('LMB');
    expect(displayBinding('mouse2')).toBe('RMB');
    expect(displayBinding(' ')).toBe('Space');
    expect(displayBinding('shift')).toBe('Shift');
    expect(primaryBindingLabel('primary-attack')).toBe('LMB');
    expect(primaryBindingLabel('ability-1')).toBe('RMB');
  });
});
