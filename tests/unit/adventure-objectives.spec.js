/** @vitest-environment node */

import { describe, it, expect, beforeEach } from 'vitest';

import {
  zoneObjective,
  ingestAdventureZones,
  resetAdventureZonePayload,
} from '../../src/core/adventure-objectives.js';

const dungeonPayload = {
  id: 'old-barrow',
  template: 'dungeon',
  bossDisplayName: 'Warden of the Deep',
  treasureItemLevel: 10,
  depth: 1,
};

describe('adventure zone objectives', () => {
  beforeEach(() => {
    resetAdventureZonePayload();
  });

  it('names the dungeon theme boss from the server payload', () => {
    ingestAdventureZones([dungeonPayload]);
    const objective = zoneObjective({ template: 'dungeon' });
    expect(objective.warden).toBe('Warden of the Deep');
    expect(objective.itemLevel).toBe(10);
    expect(objective.depth).toBe(1);
    expect(objective.line).toBe('Warden of the Deep · item-level 10 gear · depth 1');
  });

  it('maps every solo template from the ingested server payload', () => {
    ingestAdventureZones([
      { template: 'grove', bossDisplayName: 'The Elder Oak', treasureItemLevel: 10, depth: 1 },
      { template: 'crypt', bossDisplayName: 'The Pale Sovereign', treasureItemLevel: 10, depth: 1 },
      { template: 'wilds', bossDisplayName: 'Alpha of the Wilds', treasureItemLevel: 10, depth: 1 },
      { template: 'marsh', bossDisplayName: 'The Rotfather', treasureItemLevel: 10, depth: 1 },
    ]);
    expect(zoneObjective({ template: 'grove' }).warden).toBe('The Elder Oak');
    expect(zoneObjective({ template: 'crypt' }).warden).toBe('The Pale Sovereign');
    expect(zoneObjective({ template: 'wilds' }).warden).toBe('Alpha of the Wilds');
    expect(zoneObjective({ template: 'marsh' }).warden).toBe('The Rotfather');
  });

  it('does not invent a client-side boss mirror when the payload is missing', () => {
    const objective = zoneObjective({ template: 'dungeon' });
    expect(objective.warden).toBeNull();
    expect(objective.itemLevel).toBeNull();
    expect(objective.depth).toBeNull();
    expect(objective.line).toBe('');
  });

  it('prefers fields already present on the zone over the ingested catalog', () => {
    ingestAdventureZones([dungeonPayload]);
    const objective = zoneObjective({
      template: 'dungeon',
      bossDisplayName: 'A Different Warden',
      treasureItemLevel: 20,
      depth: 2,
    });
    expect(objective.warden).toBe('A Different Warden');
    expect(objective.itemLevel).toBe(20);
    expect(objective.depth).toBe(2);
  });
});
