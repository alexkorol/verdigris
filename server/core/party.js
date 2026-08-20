import { THEME_MONSTERS, instanceItemLevelForDepth } from '#server/core/map.js';

// Solo Adventure always enters at floor 1 (party.js startSoloInstance →
// enterFloor(1)). Preview depth must stay in lockstep with that call.
export const SOLO_DELVE_DEPTH = 1;

// Template → INSTANCE_THEMES key. Mirrors the lookup generateInstance uses
// from map.js TEMPLATE_THEMES so boss names come from THEME_MONSTERS, not a
// second display table. Adventure templates are the keys that matter here.
const TEMPLATE_THEME = Object.freeze({
  dungeon: 'stone',
  stone: 'stone',
  crypt: 'crypt',
  tomb: 'crypt',
  sand: 'sand',
  desert: 'sand',
  volcanic: 'volcanic',
  hell: 'volcanic',
  marsh: 'marsh',
  swamp: 'marsh',
  grove: 'grove',
  forest: 'grove',
  wilds: 'wilds',
  wilderness: 'wilds',
});

export const themeForTemplate = (template) => {
  const key = String(template || '').toLowerCase();
  if (TEMPLATE_THEME[key]) {
    return TEMPLATE_THEME[key];
  }
  if (THEME_MONSTERS[key]) {
    return key;
  }
  return 'stone';
};

export const zonePreviewFields = (zone = {}) => {
  const depth = SOLO_DELVE_DEPTH;
  const theme = themeForTemplate(zone.template);
  const roster = THEME_MONSTERS[theme] || THEME_MONSTERS.stone;
  return {
    bossDisplayName: roster.boss,
    treasureItemLevel: instanceItemLevelForDepth(depth),
    depth,
  };
};

export const adventureZonePayload = (zones = []) => zones.map((zone) => ({
  id: zone.id,
  name: zone.name,
  template: zone.template,
  layout: zone.layout,
  levelHint: zone.levelHint,
  ...zonePreviewFields(zone),
}));

export default {
  SOLO_DELVE_DEPTH,
  themeForTemplate,
  zonePreviewFields,
  adventureZonePayload,
};
