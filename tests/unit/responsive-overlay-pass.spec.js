/** @vitest-environment node */

import { readFileSync } from 'node:fs';
import { fileURLToPath, URL } from 'node:url';
import { describe, expect, it } from 'vitest';

const readSource = relativePath => readFileSync(
  fileURLToPath(new URL(`../../${relativePath}`, import.meta.url)),
  'utf8',
);

describe('TASK-0059 compact overlay stack', () => {
  it('anchors the guide banner beside the minimap only at compact laptop widths', () => {
    const source = readSource('src/components/layout/GameContainer.vue');
    expect(source).toContain('/* TASK-0059: compact laptop stack (1280x720 / 1366x768). 1920x1080 is unchanged. */');
    expect(source).toContain('@media (width <= 1366px)');
    expect(source).toContain('left: 186px');
    expect(source).toContain('max-width: calc(100% - 186px - 316px)');
    expect(source).toContain('white-space: normal');
    const desktopGuide = source.indexOf('.game-container__guide-banner {');
    const compactQuery = source.indexOf('@media (width <= 1366px)');
    expect(desktopGuide).toBeGreaterThan(-1);
    expect(compactQuery).toBeGreaterThan(desktopGuide);
    expect(source.slice(desktopGuide, compactQuery)).toContain("transform: translateX(-50%)");
  });

  it('keeps inventory from covering the HUD orbs and the full canvas at compact widths', () => {
    const source = readSource('src/components/ui/panes/PaneHost.vue');
    expect(source).toContain('@media (width <= 1366px)');
    expect(source).toContain('--pane-host-panel-bottom: calc(var(--hud-orb-size, 152px) + 120px)');
    expect(source).toContain('width: min(calc(100vw - 24px), 680px)');
    expect(source).toContain('z-index: 88');
    expect(source).toContain('max-height: calc(100dvh - var(--pane-host-panel-top) - var(--pane-host-panel-bottom) - 8px)');
    expect(source).toContain('min(calc(100vw - (var(--pane-host-panel-gutter) * 2)), 1240px)');
  });

  it('drops the loot toast below the guide banner at compact heights', () => {
    const source = readSource('src/components/ui/LootMoment.vue');
    expect(source).toContain('padding-top: clamp(24px, 12vh, 120px)');
    expect(source).toContain('@media (width <= 1366px)');
    expect(source).toContain('padding-top: clamp(96px, 18vh, 160px)');
  });
});
