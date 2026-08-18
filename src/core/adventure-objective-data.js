// Mirrored server constants for the Adventure zone preview (TASK-0049).
//
// These mirror `server/core/map.js` (TEMPLATE_THEMES + THEME_MONSTERS boss
// names + instanceItemLevelForDepth) and `server/player/handlers/party.js`
// (startSoloInstance always enters at depth 1). They are presentation-only:
// the server stays authoritative and these must be re-synced if those core
// tables ever change. Kept in one file so the mirror is auditable at a glance.
export const TEMPLATE_BOSS_NAMES = Object.freeze({
  dungeon: 'Warden of the Deep',
  crypt: 'The Pale Sovereign',
  grove: 'The Elder Oak',
  wilds: 'Alpha of the Wilds',
  marsh: 'The Rotfather',
  // The remaining themes mirror TEMPLATE_THEMES/THEME_MONSTERS for future
  // zones that reuse them via a template; solo zones currently use the five
  // templates above.
  stone: 'Warden of the Deep',
  sand: 'Tomb King Ahmenet',
  volcanic: 'Furnace Tyrant',
});

export const SOLO_DELVE_DEPTH = 1;

export const instanceItemLevelForDepth = (depth) => Math.min(
  80,
  10 + ((Math.max(1, Math.floor(Number(depth) || 1)) - 1) * 10),
);

export default {
  TEMPLATE_BOSS_NAMES,
  SOLO_DELVE_DEPTH,
  instanceItemLevelForDepth,
};
