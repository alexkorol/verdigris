/** @vitest-environment node */

import { readFileSync } from 'node:fs';
import { fileURLToPath, URL } from 'node:url';
import { describe, expect, it } from 'vitest';

const readSource = relativePath => readFileSync(
  fileURLToPath(new URL(`../../${relativePath}`, import.meta.url)),
  'utf8',
);

describe('TASK-0049 first-session UI wiring', () => {
  it('keeps House/Scion identity visible in the world HUD', () => {
    const delaford = readSource('src/Delaford.vue');
    const container = readSource('src/components/layout/GameContainer.vue');
    const hud = readSource('src/components/layout/GameHUD.vue');

    expect(delaford).toContain(':house-identity="houseIdentity"');
    expect(delaford).toContain('houseIdentity()');
    expect(container).toContain(':house-identity="houseIdentity"');
    expect(hud).toContain('houseIdentity:');
    expect(hud).toContain('identityLabel()');
    expect(hud).toContain('Mortal oath');
    expect(hud).toContain('aria-label="House and scion"');
  });

  it('turns the bare mana rejection into a directive line', () => {
    const resource = readSource('src/core/player/events/resource.js');
    const canvas = readSource('src/components/GameCanvas.vue');

    expect(resource).toContain("text === 'Not enough mana.'");
    expect(resource).toContain('formatManaRejection(player)');
    expect(resource).toContain("bus.$emit('tutorial:beat'");
    expect(canvas).toContain('recordSkillAttempt(skillId)');
  });

  it('surfaces guide beats and previews zone objectives', () => {
    const container = readSource('src/components/layout/GameContainer.vue');

    expect(container).toContain("bus.$on('tutorial:beat'");
    expect(container).toContain('shouldSurfaceGuideBeat');
    expect(container).toContain('class="game-container__guide-banner"');
    expect(container).toContain('zoneObjective(zone).line');
    expect(container).toContain('game-container__zone-objective');
  });

  it('highlights a first allocation in the skill tree pane', () => {
    const pane = readSource('src/components/passives/GeometricSkillTreePane.vue');
    const tree = readSource('src/core/passives/verdigris-geometric-tree.js');

    expect(tree).toContain('recommendFirstAllocation()');
    expect(tree).toContain('firstAllocationHint: this.recommendFirstAllocation()');
    expect(pane).toContain('firstAllocationHint');
    expect(pane).toContain('recommendedNodeId');
    expect(pane).toContain('node-group.recommended');
  });
});
