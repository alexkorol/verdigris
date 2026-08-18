// First-session tutorial beats (TASK-0049 deliverable 3).
//
// Aldwyn the Guide is the only onboarding voice; every first-session beat
// arrives as a chat message prefixed "Aldwyn the Guide: " (server
// core/tutorial.js and core/first-goal.js). Detecting that prefix lets the
// client surface those beats as a transient banner WITHOUT touching server
// code, and combat lines keep flowing through the ticker untouched.
export const GUIDE_PREFIX = 'Aldwyn the Guide:';

export const isGuideMessage = (text = '') => (
  typeof text === 'string' && text.trim().startsWith(GUIDE_PREFIX)
);

/** Remove the speaker prefix so the banner can render the message alone. */
export const stripGuidePrefix = (text = '') => {
  if (!isGuideMessage(text)) {
    return text;
  }
  return text.trim().slice(GUIDE_PREFIX.length).trim();
};

// The authored onboarding is ~10 guide beats (5 tutorial steps + the first
// goal). Cap the transient surfacing so a late reminder never spams the
// banner; the message still lands in the ticker regardless.
export const DEFAULT_MAX_GUIDE_BEATS = 12;

export const shouldSurfaceGuideBeat = (
  text = '',
  surfacedCount = 0,
  max = DEFAULT_MAX_GUIDE_BEATS,
) => isGuideMessage(text) && surfacedCount < max;

export default {
  GUIDE_PREFIX,
  isGuideMessage,
  stripGuidePrefix,
  shouldSurfaceGuideBeat,
  DEFAULT_MAX_GUIDE_BEATS,
};
