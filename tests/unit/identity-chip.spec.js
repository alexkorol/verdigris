/** @vitest-environment node */

import { readFileSync } from 'node:fs';
import { fileURLToPath, URL } from 'node:url';
import { describe, expect, it } from 'vitest';

const readSource = relativePath => readFileSync(
  fileURLToPath(new URL(`../../${relativePath}`, import.meta.url)),
  'utf8',
);

describe('TASK-0055 identity chip', () => {
  it('caps chip width, exposes the full label on hover, and sits above the HP orb', () => {
    const hud = readSource('src/components/layout/GameHUD.vue');
    expect(hud).toContain(':title="identityLabel"');
    expect(hud).toContain('pointer-events: auto');
    expect(hud).toContain('max-width: min(28vw, 240px)');
    expect(hud).toContain('bottom: calc(var(--hud-orb-size, 152px) + 28px)');
    expect(hud).not.toContain('bottom: calc(var(--hud-orb-size, 152px) * 0.6)');
  });
});
