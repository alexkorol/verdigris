// The authored daytime grade is the default presentation. The full clock is
// deliberately opt-in so a new expedition opens in a stable, readable light.
export const AMBIENT_CYCLE_STORAGE_KEY = 'verdigris.ambient-cycle-enabled';

const hasStorage = () => (
  typeof window !== 'undefined'
  && window.localStorage
  && typeof window.localStorage.getItem === 'function'
);

export const isAmbientCycleEnabled = () => (
  hasStorage() && window.localStorage.getItem(AMBIENT_CYCLE_STORAGE_KEY) === 'true'
);

export const setAmbientCycleEnabled = (enabled) => {
  const value = Boolean(enabled);
  if (hasStorage() && typeof window.localStorage.setItem === 'function') {
    window.localStorage.setItem(AMBIENT_CYCLE_STORAGE_KEY, String(value));
  }
  return value;
};
