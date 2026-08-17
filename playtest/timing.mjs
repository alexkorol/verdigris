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
const LOAD_MODE_MAX_FACTOR = 1.75;

// The server is a child process. Under deliberate whole-machine CPU pressure
// it can be starved even when this client event loop reports little lag. The
// documented load gate opts into the same bounded 1.75x ceiling based on that
// explicit test condition; ordinary runs remain driven by observed lag only.
export const loadMode = /^(1|true|yes)$/i.test(process.env.PLAYTEST_LOAD_MODE || '');

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
  // The maximum catches a pre-empted scheduler turn; the percentile
  // represents sustained pressure. Use whichever observed signal is larger
  // so default mode adapts to contention without an environment flag. The
  // explicit factor cap below still bounds one incidental pause and keeps a
  // genuinely missing event detectable.
  const observedLagMs = Math.max(0, Math.max(percentileMs, maxMs) - lagBaselineMs);
  const measuredSlackMs = observedLagMs * lagMultiplier;
  const loadSlackMs = loadMode ? base * (LOAD_MODE_MAX_FACTOR - 1) : 0;
  const slackMs = Math.min(
    base * Math.max(0, maxFactor - 1),
    Math.max(measuredSlackMs, loadSlackMs),
  );
  return Math.ceil(base + slackMs);
};

export const timingDiagnostics = () => ({
  loadMode,
  p99EventLoopLagMs: finite(monitor.percentile(99) / 1e6, 0),
  maxEventLoopLagMs: finite(monitor.max / 1e6, 0),
});

export const resetTimingDiagnostics = () => monitor.reset();
