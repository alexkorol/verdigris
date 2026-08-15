/** @vitest-environment node */

import { readFileSync } from 'node:fs';
import { fileURLToPath, URL } from 'node:url';
import { describe, expect, it } from 'vitest';

import {
  buildItemTooltipModel,
  getItemRarity,
  getItemTooltipPosition,
  itemTooltipAriaLabel,
} from '@/core/inventory/item-tooltip.js';

describe('inventory item tooltip', () => {
  it('turns Vesselforge lines into styled client-safe rows', () => {
    const item = {
      name: 'Bronze Hunger',
      type: 'weapon',
      equipSlot: 'right_hand',
      boundTo: 'house-account',
      chroniclesRelic: { houseName: 'Vaelmont', scionName: 'Vesper' },
      stats: { attack: { stab: 4, slash: -2, crush: 0 } },
      vessel: {
        item: {
          brands: [{ id: 'keen' }],
          bonds: [{ id: 'battle-rhythm' }],
          trophies: [],
        },
        lines: [
          { section: 'name', text: 'Bronze Hunger', tone: 'bonded' },
          { section: 'kind', text: 'One-hand weapon · Bronze (tier 3)' },
          { section: 'brand', text: '✦ +12% increased Physical Damage' },
          { section: 'bond', text: '◈ Battle Rhythm', tone: 'bond' },
        ],
      },
    };

    const model = buildItemTooltipModel(item, { width: 1, height: 3 });

    expect(model).toMatchObject({
      name: 'Bronze Hunger',
      rarity: 'rare',
      meta: ['Weapon', 'Right Hand', '1 × 3'],
      binding: 'House Vaelmont heirloom · carried by Vesper',
    });
    expect(model.vesselLines).toEqual([
      { section: 'kind', text: 'One-hand weapon · Bronze (tier 3)', tone: 'normal' },
      { section: 'brand', text: '✦ +12% increased Physical Damage', tone: 'normal' },
      { section: 'bond', text: '◈ Battle Rhythm', tone: 'bond' },
    ]);
    expect(model.statLines).toEqual(['+4 Stab Attack', '-2 Slash Attack']);
  });

  it('classifies generated and legacy affixed gear without trusting arbitrary rarity values', () => {
    expect(getItemRarity({ vessel: { item: { awakened: { power: 'Echo' } } } })).toBe('awakened');
    expect(getItemRarity({ affixes: { brand: { id: 'keen' } } })).toBe('magic');
    expect(getItemRarity({ rarity: 'mythic' })).toBe('normal');
    expect(itemTooltipAriaLabel({ name: 'Bronze Pickaxe' }, { width: 1, height: 3 }))
      .toBe('Bronze Pickaxe (1 x 3)');
  });

  it('keeps the shared tooltip inside the viewport on either side of the pointer', () => {
    expect(getItemTooltipPosition(
      { clientX: 100, clientY: 100 },
      { width: 1280, height: 720 },
    )).toEqual({ left: 116, top: 116, bottom: null, maxHeight: 592 });
    expect(getItemTooltipPosition(
      { clientX: 1200, clientY: 680 },
      { width: 1280, height: 720 },
    )).toEqual({ left: 858, top: null, bottom: 56, maxHeight: 652 });
  });

  it('uses one floating tooltip for backpack and equipped items', () => {
    const gridSource = readFileSync(
      fileURLToPath(new URL('../../src/components/inventory/InventoryGrid.vue', import.meta.url)),
      'utf8',
    );
    const equipmentSource = readFileSync(
      fileURLToPath(new URL('../../src/components/sub/EquipmentSlot.vue', import.meta.url)),
      'utf8',
    );
    const containerSource = readFileSync(
      fileURLToPath(new URL('../../src/components/layout/GameContainer.vue', import.meta.url)),
      'utf8',
    );
    const tooltipSource = readFileSync(
      fileURLToPath(new URL('../../src/components/inventory/ItemTooltip.vue', import.meta.url)),
      'utf8',
    );

    expect(gridSource).toContain('<ItemTooltip');
    expect(gridSource).toContain(':aria-label="itemAriaLabel(item)"');
    expect(gridSource).toContain('@focus="showTooltip($event, item)"');
    expect(gridSource).not.toContain(':title="itemTooltip(item)"');
    expect(equipmentSource).toContain('<ItemTooltip');
    expect(equipmentSource).toContain(':dimensions="tooltipDimensions"');
    expect(equipmentSource).not.toContain('InventoryItemTooltip');
    expect(equipmentSource).toContain('this.clearContextHint();');
    expect(containerSource).toContain('.game-container--right-pane-open :deep(.first-action)');
    expect(tooltipSource).toContain('role="tooltip"');
    expect(tooltipSource).toContain('item-tooltip__line--tone-');
  });
});
