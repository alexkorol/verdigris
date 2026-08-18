// Directive mana-rejection copy (TASK-0049 deliverable 2).
//
// The server still owns the balance numbers (skill costs, regen rate). This
// module mirrors the two recovery constants BY VALUE so the client can turn
// the bare "Not enough mana." rejection into a directive line without a
// server round-trip. This is the same documented "display constants mirror
// core constants" pattern as the quickbar cost labels (0009/0013 watch item);
// the values below must stay in step with
// `server/core/combat/regeneration.js` (REGEN_INTERVAL_MS, MANA_REGEN_FRACTION)
// and `server/shared/skills/index.js` (per-skill resourceCost.mana).
import { getSkillDefinition } from '@shared/skills/index.js';

export const MANA_REGEN_INTERVAL_S = 2;
export const MANA_REGEN_FRACTION = 0.03;

let lastAttemptedSkillId = null;

export const recordSkillAttempt = (skillId) => {
  lastAttemptedSkillId = skillId || null;
};

export const lastAttemptedSkill = () => lastAttemptedSkillId;

/** Mana recovered per tick: at least 1, else 3% of max (server rule). */
export const manaRegenAmount = (max) => Math.max(1, Math.floor((Number(max) || 0) * MANA_REGEN_FRACTION));

export const readMana = (player = {}) => {
  const stats = player.stats || {};
  const resources = stats.resources || {};
  const meter = resources.mana || player.mana || player.mp || {};
  const current = Number.isFinite(Number(meter.current)) ? Number(meter.current) : 0;
  const max = Number.isFinite(Number(meter.max)) ? Number(meter.max) : 0;
  return { current, max };
};

/**
 * Build the directive replacement for the server's "Not enough mana.".
 * Uses the live mana state the client already holds plus the cost of the most
 * recently attempted skill (the client always knows which skill it just
 * asked to cast). Falls back to the bare phrase when nothing is known.
 */
export const formatManaRejection = (player = {}, skillId = lastAttemptedSkillId) => {
  const { current, max } = readMana(player);
  const regen = manaRegenAmount(max);
  const skill = skillId ? getSkillDefinition(skillId) : null;
  const cost = skill && skill.resourceCost && Number.isFinite(skill.resourceCost.mana)
    ? skill.resourceCost.mana
    : 0;
  const missing = cost > 0 ? Math.max(0, cost - current) : 0;
  const lead = missing > 0 ? `Need ${missing} more mana` : 'Not enough mana';
  return `${lead} — recovering ${regen} every ${MANA_REGEN_INTERVAL_S}s.`;
};

export default {
  MANA_REGEN_INTERVAL_S,
  MANA_REGEN_FRACTION,
  recordSkillAttempt,
  lastAttemptedSkill,
  manaRegenAmount,
  readMana,
  formatManaRejection,
};
