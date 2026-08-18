// TASK-0042 first-loot moment: the server tags the session's one curated
// first-delve drop with `firstFind` (see FIRST_FIND in
// server/core/combat/loot.js). These helpers spot the tagged item crossing
// ground -> inventory and build the one-line "why it matters" comparison
// against whatever the scion currently holds in that slot.
import bus from '../../utilities/bus.js';
import LootMoment from '../../../components/ui/LootMoment.vue';

const COMBAT_STATS = ['stab', 'slash', 'crush', 'range'];

// A persisted first find must not re-toast on every later login refresh.
const presentedPickups = new Set();

const taggedItems = items => (Array.isArray(items) ? items : [])
  .filter(item => item && item.firstFind);

/**
 * The tagged find that just appeared on the ground, or null.
 *
 * @param {array} previousItems Ground items before the broadcast
 * @param {array} nextItems Ground items after the broadcast
 */
export const newFirstFindDrop = (previousItems, nextItems) => {
  const previousUuids = new Set(taggedItems(previousItems).map(item => item.uuid));
  return taggedItems(nextItems).find(item => !previousUuids.has(item.uuid)) || null;
};

/**
 * The tagged find that just entered the inventory, or null.
 *
 * @param {array} previousInventory Inventory slots before the refresh
 * @param {array} nextInventory Inventory slots after the refresh
 */
export const newFirstFindPickup = (previousInventory, nextInventory) => {
  const previousUuids = new Set(taggedItems(previousInventory).map(item => item.uuid));
  return taggedItems(nextInventory).find(item => !previousUuids.has(item.uuid)) || null;
};

const equippedInSlot = (player, item) => {
  const slot = item && (item.equipSlot || item.slotType || item.slot);
  const wear = (player && player.wear) || {};
  return slot ? wear[slot] || null : null;
};

/**
 * One honest line comparing the find against the held item, built from the
 * find's dominant combat category (attack for weapons, defense for armour).
 */
export const buildFirstFindComparison = (item, equipped = null) => {
  const stats = (item && item.stats) || {};
  const categoryTotal = type => COMBAT_STATS
    .reduce((total, key) => total + (Number(stats[type] && stats[type][key]) || 0), 0);
  const category = categoryTotal('defense') > categoryTotal('attack') ? 'defense' : 'attack';
  const heldStats = (equipped && equipped.stats) || {};
  const deltas = COMBAT_STATS
    .map(key => ({
      key,
      delta: (Number(stats[category] && stats[category][key]) || 0)
        - (Number(heldStats[category] && heldStats[category][key]) || 0),
    }))
    .filter(entry => entry.delta !== 0)
    .sort((left, right) => Math.abs(right.delta) - Math.abs(left.delta))
    .slice(0, 2);

  const heldName = equipped && (equipped.displayName || equipped.name);
  if (!deltas.length) {
    return heldName ? `Even trade vs ${heldName}.` : 'No stats to compare.';
  }

  const summary = deltas
    .map(entry => `${entry.delta > 0 ? '+' : ''}${entry.delta} ${entry.key}`)
    .join(', ');
  return heldName
    ? `${summary} ${category} vs ${heldName}`
    : `${summary} ${category} — nothing held in that slot`;
};

/**
 * Drop beat: loot chime through the existing audio seam plus a HUD
 * first-action prompt that names the Take affordance.
 */
export const announceFirstFindDrop = (item) => {
  if (!item) {
    return;
  }

  bus.$emit('sound:loot');
  bus.$emit('game:context-menu:first-only', {
    data: {
      data: {
        count: 0,
        // No `action` field: a plain left-click must keep its default meaning
        // instead of trying to run a fabricated context action.
        firstItem: {
          label: `Take ${item.displayName || item.name} — press Z underfoot`,
        },
      },
    },
  });
};

/**
 * Pickup beat: compact inspect toast with the comparison line, mounted
 * through the existing open:screen seam. Returns the comparison for tests.
 */
export const presentFirstFindPickup = ({ item, player }) => {
  if (!item || presentedPickups.has(item.uuid)) {
    return null;
  }
  presentedPickups.add(item.uuid);

  const comparison = buildFirstFindComparison(item, equippedInSlot(player, item));
  bus.$emit('open:screen', {
    data: {
      screen: LootMoment,
      payload: {
        name: item.displayName || item.name,
        examine: item.examine || '',
        comparison,
      },
    },
  });
  return comparison;
};
