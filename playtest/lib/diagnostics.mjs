/**
 * TASK-0062: playtest diagnostics. Installed by the runner. Does not change
 * wait budgets, retries, or pass/fail — it only records what already happened.
 */

import fs from 'node:fs';
import path from 'node:path';

const DEFAULT_TIMEOUT_MS = 8000;

const waitsOf = (player) => {
  if (!player._protocolWaits) player._protocolWaits = [];
  return player._protocolWaits;
};

const recordWait = (player, entry) => {
  waitsOf(player).push(entry);
};

export const installPlaytestDiagnostics = (HeadlessPlayer) => {
  if (HeadlessPlayer.prototype._verdigrisDiagInstalled) return;
  HeadlessPlayer.prototype._verdigrisDiagInstalled = true;

  const originalWaitFor = HeadlessPlayer.prototype.waitFor;
  HeadlessPlayer.prototype.waitFor = async function waitForDiagnosed(predicate, options = {}) {
    const started = Date.now();
    const label = options.label || 'condition';
    const timeoutBudgetMs = options.timeoutMs ?? DEFAULT_TIMEOUT_MS;
    try {
      const result = await originalWaitFor.call(this, predicate, options);
      recordWait(this, {
        label,
        timeoutBudgetMs,
        elapsedMs: Date.now() - started,
        ok: true,
      });
      return result;
    } catch (error) {
      recordWait(this, {
        label,
        timeoutBudgetMs,
        elapsedMs: Date.now() - started,
        ok: false,
      });
      throw error;
    }
  };
};

export const lastEnvelopes = (player, count = 5) => {
  const events = Array.isArray(player.events) ? player.events : [];
  return events.slice(-count).map((entry) => ({
    event: entry.event,
    at: entry.at,
  }));
};

export const collectFailureDiagnosis = (players) => {
  const waits = [];
  const envelopes = [];
  for (const player of players) {
    if (!player) continue;
    waits.push(...waitsOf(player));
    envelopes.push(...lastEnvelopes(player, 5));
  }
  const lastFive = envelopes.slice(-5);
  return { waits, lastEnvelopes: lastFive };
};

export const formatFailureDiagnosis = (name, wallMs, diagnosis) => {
  const waits = JSON.stringify(diagnosis.waits);
  const envelopes = JSON.stringify(diagnosis.lastEnvelopes);
  return `  DIAG ${name} wall=${wallMs}ms waits=${waits} lastEnvelopes=${envelopes}`;
};

export const timingLogEnabled = () => /^(1|true|yes)$/i.test(process.env.PLAYTEST_TIMING_LOG || '');

export const timingLogPath = (projectRoot) => {
  if (process.env.PLAYTEST_TIMING_LOG_PATH) return process.env.PLAYTEST_TIMING_LOG_PATH;
  return path.join(
    projectRoot,
    'orchestration',
    'tasks',
    'TASK-0062-playtest-flake-triage',
    'timing.jsonl',
  );
};

export const appendTimingLog = (filePath, record) => {
  fs.mkdirSync(path.dirname(filePath), { recursive: true });
  fs.appendFileSync(filePath, `${JSON.stringify(record)}\n`);
};
