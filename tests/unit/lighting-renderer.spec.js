import { describe, expect, it } from 'vitest';
import {
  DAY_LENGTH_SECONDS,
  getNightFactor,
  sampleAmbient,
} from '../../src/core/rendering/lighting-renderer.js';

describe('2.5D day/night lighting', () => {
  it('loops seamlessly at the configured day length', () => {
    expect(sampleAmbient(0)).toEqual(sampleAmbient(DAY_LENGTH_SECONDS));
    expect(sampleAmbient(-1)).toEqual(sampleAmbient(DAY_LENGTH_SECONDS - 1));
  });

  it('moves from a warm day grade into a cool night grade', () => {
    const noon = sampleAmbient(DAY_LENGTH_SECONDS * 0.30);
    const dusk = sampleAmbient(DAY_LENGTH_SECONDS * 0.58);
    const night = sampleAmbient(DAY_LENGTH_SECONDS * 0.80);

    expect(noon[0]).toBeGreaterThan(noon[2]);
    expect(dusk[2]).toBeGreaterThan(dusk[0]);
    expect(night[2]).toBeGreaterThan(night[0]);
    expect(getNightFactor(night)).toBeGreaterThan(getNightFactor(noon));
  });

  it('keeps the first minute bright enough for a readable first encounter', () => {
    const firstMinute = sampleAmbient(60);

    expect(DAY_LENGTH_SECONDS).toBeGreaterThanOrEqual(240);
    expect(Math.min(...firstMinute)).toBeGreaterThanOrEqual(210);
  });

  it('keeps the deep reference night curve above a crushed-black floor', () => {
    const night = sampleAmbient(DAY_LENGTH_SECONDS * 0.80);

    // Reference night [110,120,190] renormalized to Verdigris albedo: still
    // a deep blue grade whose darkest channel stays well above black.
    expect(night).toEqual([110, 124, 205]);
    expect(Math.min(...night)).toBeGreaterThanOrEqual(100);
  });
});
