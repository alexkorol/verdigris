import { describe, it, expect, beforeEach } from 'vitest';

import {
  MANA_REGEN_INTERVAL_S,
  MANA_REGEN_FRACTION,
  recordSkillAttempt,
  lastAttemptedSkill,
  manaRegenAmount,
  readMana,
  formatManaRejection,
} from '../../src/core/mana-directive.js';

describe('mana directive', () => {
  beforeEach(() => {
    recordSkillAttempt(null);
  });

  it('mirrors the server recovery constants by value', () => {
    expect(MANA_REGEN_INTERVAL_S).toBe(2);
    expect(MANA_REGEN_FRACTION).toBe(0.03);
  });

  it('computes regen as max(1, floor(max * fraction))', () => {
    expect(manaRegenAmount(90)).toBe(2);
    expect(manaRegenAmount(0)).toBe(1);
    expect(manaRegenAmount(300)).toBe(9);
  });

  it('records and recalls the last attempted skill', () => {
    expect(lastAttemptedSkill()).toBeNull();
    recordSkillAttempt('ability-2');
    expect(lastAttemptedSkill()).toBe('ability-2');
  });

  it('reads the mana meter from the live resource state', () => {
    const player = { stats: { resources: { mana: { current: 10, max: 90 } } } };
    expect(readMana(player)).toEqual({ current: 10, max: 90 });
  });

  it('states the missing amount and recovery cadence for the last skill', () => {
    recordSkillAttempt('ability-1'); // Cinder Fan costs 12 mana
    const player = { stats: { resources: { mana: { current: 10, max: 90 } } } };
    expect(formatManaRejection(player)).toBe('Need 2 more mana — recovering 2 every 2s.');
  });

  it('still states the cadence when no specific skill is known', () => {
    const player = { stats: { resources: { mana: { current: 10, max: 90 } } } };
    expect(formatManaRejection(player)).toBe('Not enough mana — recovering 2 every 2s.');
  });
});
