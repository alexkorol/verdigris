import { monitorEventLoopDelay } from 'node:perf_hooks';

// The playtest server and the client harness share a machine.  A busy CI
// worker can therefore delay both sides without changing the behaviour being
// tested.  Keep the authored deadlines as the floor and add only the amount
// of scheduler slack we have actually observed in this run.  A cap is
// intentional: a genuinely missing event must still fail in a bounded time.
const DEFAULT_MAX_FACTOR = 1.75;
const DEFAULT_LAG_BASELINE_MS = 20;
const DEFAULT_LAG_MULTIPLIER = 4;
const DEFAULT_RESOLUTION_MS = 20;

const monitor = monitorEventLoopDelay({ resolution: DEFAULT_RESOLUTION_MS });
monitor.enable();

const finite = (value, fallback) => Number.isFinite(value) ? value : fallback;

export const adaptiveTimeoutMs = (baseMs, {
  maxFactor = DEFAULT_MAX_FACTOR,
  lagBaselineMs = DEFAULT_LAG_BASELINE_MS,
  lagMultiplier = DEFAULT_LAG_MULTIPLIER,
} = {}) => {
  const base = Math.max(1, Math.ceil(Number(baseMs) || 1));
  const percentileMs = finite(monitor.percentile(99) / 1e6, lagBaselineMs);
  const maxMs = finite(monitor.max / 1e6, lagBaselineMs);
  // The maximum catches a single pre-empted scheduler turn; the percentile
  // represents sustained pressure.  Down-weight the former so one incidental
  // GC pause cannot turn an eight-second regression gate into a long sleep.
  const observedLagMs = Math.max(0, Math.max(percentileMs, maxMs * 0.25) - lagBaselineMs);
  const slackMs = Math.min(base * Math.max(0, maxFactor - 1), observedLagMs * lagMultiplier);
  return Math.ceil(base + slackMs);
};

export const timingDiagnostics = () => ({
  p99EventLoopLagMs: finite(monitor.percentile(99) / 1e6, 0),
  maxEventLoopLagMs: finite(monitor.max / 1e6, 0),
});

export const resetTimingDiagnostics = () => monitor.reset();

