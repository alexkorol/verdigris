/** @vitest-environment node */

import { describe, expect, it } from 'vitest';
import { existsSync, readFileSync } from 'node:fs';
import { fileURLToPath } from 'node:url';
import { THEME_MONSTERS, instanceItemLevelForDepth } from '#server/core/map.js';
import {
  SOLO_DELVE_DEPTH,
  themeForTemplate,
  zonePreviewFields,
  adventureZonePayload,
} from '#server/core/party.js';

const ADVENTURE_ZONES = [
  { id: 'old-barrow', name: 'The Old Barrow', template: 'dungeon', layout: 'warren', levelHint: '1–5' },
  { id: 'verdant-grove', name: 'Verdant Grove', template: 'grove', layout: 'clearings', levelHint: '1–6' },
  { id: 'sunken-colonnade', name: 'Sunken Colonnade', template: 'crypt', layout: 'gauntlet', levelHint: '3–8' },
  { id: 'weir-crypt', name: 'Weir Crypt', template: 'crypt', layout: 'warren', levelHint: '4–9' },
  { id: 'the-wilds', name: 'The Wilds', template: 'wilds', layout: 'clearings', levelHint: '6–12' },
  { id: 'marsh-of-reeds', name: 'Marsh of Reeds', template: 'marsh', layout: 'clearings', levelHint: '8–14' },
];

describe('TASK-0055 server-side zone preview payload', () => {
  it('pulls boss names from THEME_MONSTERS via the same theme map generateInstance uses', () => {
    expect(zonePreviewFields({ template: 'dungeon' }).bossDisplayName)
      .toBe(THEME_MONSTERS.stone.boss);
    expect(zonePreviewFields({ template: 'grove' }).bossDisplayName)
      .toBe(THEME_MONSTERS.grove.boss);
    expect(zonePreviewFields({ template: 'crypt' }).bossDisplayName)
      .toBe(THEME_MONSTERS.crypt.boss);
    expect(zonePreviewFields({ template: 'wilds' }).bossDisplayName)
      .toBe(THEME_MONSTERS.wilds.boss);
    expect(zonePreviewFields({ template: 'marsh' }).bossDisplayName)
      .toBe(THEME_MONSTERS.marsh.boss);
  });

  it('uses the solo-delve depth and instanceItemLevelForDepth from map.js', () => {
    expect(SOLO_DELVE_DEPTH).toBe(1);
    const preview = zonePreviewFields({ template: 'dungeon' });
    expect(preview.depth).toBe(SOLO_DELVE_DEPTH);
    expect(preview.treasureItemLevel).toBe(instanceItemLevelForDepth(SOLO_DELVE_DEPTH));
    expect(preview.treasureItemLevel).toBe(10);
  });

  it('attaches additive preview fields onto every Adventure zone without renaming existing ones', () => {
    const payload = adventureZonePayload(ADVENTURE_ZONES);
    expect(payload).toHaveLength(ADVENTURE_ZONES.length);
    payload.forEach((entry, index) => {
      const source = ADVENTURE_ZONES[index];
      expect(entry.id).toBe(source.id);
      expect(entry.name).toBe(source.name);
      expect(entry.template).toBe(source.template);
      expect(entry.layout).toBe(source.layout);
      expect(entry.levelHint).toBe(source.levelHint);
      expect(entry.bossDisplayName).toBe(
        THEME_MONSTERS[themeForTemplate(source.template)].boss,
      );
      expect(entry.treasureItemLevel).toBe(10);
      expect(entry.depth).toBe(1);
    });
  });

  it('deletes the client-side adventure-objective-data mirror', () => {
    const mirror = fileURLToPath(new URL('../../src/core/adventure-objective-data.js', import.meta.url));
    expect(existsSync(mirror)).toBe(false);
  });

  it('wires the payload onto the existing player:login and party:update envelopes', () => {
    const handler = readFileSync(
      fileURLToPath(new URL('../../server/player/handlers/party.js', import.meta.url)),
      'utf8',
    );
    expect(handler).toContain("from '#server/core/party.js'");
    expect(handler).toContain('adventureZonePayload');
    expect(handler).toContain('attachAdventureZones');
    expect(handler).toContain("event === 'player:login'");
    expect(handler).toContain('emitPartyUpdate');
  });
});
