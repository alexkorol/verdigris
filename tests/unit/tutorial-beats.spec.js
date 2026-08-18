import { describe, it, expect } from 'vitest';

import {
  GUIDE_PREFIX,
  isGuideMessage,
  stripGuidePrefix,
  shouldSurfaceGuideBeat,
  DEFAULT_MAX_GUIDE_BEATS,
} from '../../src/core/tutorial-beats.js';

describe('tutorial beats', () => {
  it('detects Aldwyn guide messages', () => {
    expect(GUIDE_PREFIX).toBe('Aldwyn the Guide:');
    expect(isGuideMessage('Aldwyn the Guide: Welcome.')).toBe(true);
    expect(isGuideMessage('You hit a monster.')).toBe(false);
    expect(isGuideMessage('')).toBe(false);
    expect(isGuideMessage(null)).toBe(false);
  });

  it('strips the speaker prefix for the banner', () => {
    expect(stripGuidePrefix('Aldwyn the Guide: Take a walk.')).toBe('Take a walk.');
    expect(stripGuidePrefix('plain line')).toBe('plain line');
  });

  it('caps transient surfacing at the first N beats', () => {
    expect(shouldSurfaceGuideBeat('Aldwyn the Guide: x', 0)).toBe(true);
    expect(shouldSurfaceGuideBeat('Aldwyn the Guide: x', DEFAULT_MAX_GUIDE_BEATS)).toBe(false);
    expect(shouldSurfaceGuideBeat('not a guide', 0)).toBe(false);
  });
});
