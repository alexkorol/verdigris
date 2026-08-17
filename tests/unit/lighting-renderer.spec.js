import { afterEach, describe, expect, it } from 'vitest';
import {
  AMBIENT_CYCLE_STORAGE_KEY,
  isAmbientCycleEnabled,
  setAmbientCycleEnabled,
} from '../../src/core/config/ambient-clock.js';
import {
  DAY_LENGTH_SECONDS,
  getNightFactor,
  MIDDAY_AMBIENT,
  sampleAmbientForClock,
  sampleAmbient,
} from '../../src/core/rendering/lighting-renderer.js';

describe('2.5D day/night lighting', () => {
  const previousWindow = globalThis.window;

  afterEach(() => {
    if (previousWindow === undefined) {
      delete globalThis.window;
    } else {
      globalThis.window = previousWindow;
    }
  });

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

  it('opens on the stable midday grade when the cycle is not opted in', () => {
    expect(sampleAmbientForClock(0, false)).toEqual(MIDDAY_AMBIENT);
    expect(sampleAmbientForClock(DAY_LENGTH_SECONDS * 0.80, false)).toEqual(MIDDAY_AMBIENT);
  });

  it('restores the authored day/night curve when opted in', () => {
    expect(sampleAmbientForClock(DAY_LENGTH_SECONDS * 0.80, true))
      .toEqual(sampleAmbient(DAY_LENGTH_SECONDS * 0.80));
  });

  it('persists the opt-in setting so a fresh read survives reload', () => {
    const values = new Map();
    globalThis.window = {
      localStorage: {
        getItem: key => values.get(key) ?? null,
        setItem: (key, value) => values.set(key, value),
      },
    };

    expect(isAmbientCycleEnabled()).toBe(false);
    setAmbientCycleEnabled(true);
    expect(values.get(AMBIENT_CYCLE_STORAGE_KEY)).toBe('true');
    expect(isAmbientCycleEnabled()).toBe(true);
    setAmbientCycleEnabled(false);
    expect(isAmbientCycleEnabled()).toBe(false);
  });
});
