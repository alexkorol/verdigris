import { describe, expect, it } from 'vitest';
import { frameDeltaSeconds, MAX_FRAME_DT_SECONDS } from '../../src/core/hud/wizard-orb-renderer.js';

describe('Wizard orb frame clock', () => {
  it('clamps negative and oversized RAF deltas without producing NaN', () => {
    expect(frameDeltaSeconds(100, 200)).toBe(0);
    expect(frameDeltaSeconds(200, 100)).toBeCloseTo(0.05, 10);
    expect(frameDeltaSeconds(1000, 0)).toBe(MAX_FRAME_DT_SECONDS);
    expect(frameDeltaSeconds(Number.NaN, 0)).toBe(0);
  });

  it('preserves ordinary frame timing', () => {
    expect(frameDeltaSeconds(1016.666, 1000)).toBeCloseTo(0.016666, 6);
  });
});
