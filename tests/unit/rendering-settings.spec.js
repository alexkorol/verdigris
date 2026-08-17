/** @vitest-environment node */

import { readFileSync } from 'node:fs';
import { fileURLToPath, URL } from 'node:url';
import { describe, expect, it } from 'vitest';

const settingsSource = readFileSync(
  fileURLToPath(new URL('../../src/components/slots/Settings.vue', import.meta.url)),
  'utf8',
);

describe('ambient lighting settings surface', () => {
  it('exposes one persisted opt-in cycle control in the existing settings pane', () => {
    expect(settingsSource.match(/id="day-night-cycle"/g)).toHaveLength(1);
    expect(settingsSource).toContain('v-model="selected.dayNightCycle"');
    expect(settingsSource).toContain('this.selected.dayNightCycle = isAmbientCycleEnabled();');
    expect(settingsSource).toContain('setAmbientCycleEnabled(enabled);');
    expect(settingsSource).toContain('Opt in to changing ambient light');
  });
});
