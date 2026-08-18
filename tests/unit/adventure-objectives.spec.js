import { describe, it, expect } from 'vitest';

import { zoneObjective } from '../../src/core/adventure-objectives.js';

describe('adventure zone objectives', () => {
  it('names the dungeon theme boss with first-delve treasure and depth', () => {
    const objective = zoneObjective({ template: 'dungeon' });
    expect(objective.warden).toBe('Warden of the Deep');
    expect(objective.itemLevel).toBe(10);
    expect(objective.depth).toBe(1);
    expect(objective.line).toBe('Warden of the Deep · item-level 10 gear · depth 1');
  });

  it('maps every solo template to its generated theme boss', () => {
    expect(zoneObjective({ template: 'grove' }).warden).toBe('The Elder Oak');
    expect(zoneObjective({ template: 'crypt' }).warden).toBe('The Pale Sovereign');
    expect(zoneObjective({ template: 'wilds' }).warden).toBe('Alpha of the Wilds');
    expect(zoneObjective({ template: 'marsh' }).warden).toBe('The Rotfather');
  });

  it('keeps treasure and depth when a template is unknown', () => {
    const objective = zoneObjective({ template: 'nope' });
    expect(objective.warden).toBeNull();
    expect(objective.line).toBe('item-level 10 gear · depth 1');
  });
});
