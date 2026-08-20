// Shared hard-fail browser-capture harness for orchestration task evidence.
//
// Extracted from TASK-0055 and TASK-0059's capture scripts (see
// orchestration/tasks/TASK-0055-browser-followups/captures/capture-0055.mjs
// and .../TASK-0059-responsive-overlay-pass/captures/capture-0059.mjs) so
// future SPECs can say "use capture-harness" instead of restating server
// lifecycle, Chronicles login, bounding-box math, and the hard-fail JSON
// summary pattern.
//
// Usage template for a new orchestration/tasks/TASK-NNNN-*/captures/capture-NNNN.mjs:
//
//   import { chromium } from '@playwright/test';
//   import { fileURLToPath } from 'node:url';
//   import path from 'node:path';
//   import {
//     startOwnedServer, stopOwnedServer, loginChronicles, boxOf,
//     boxesOverlap, overflowsViewport, screenshotPath, runCapture,
//   } from '../../../../tests/e2e/lib/capture-harness.mjs';
//
//   const outDir = path.dirname(fileURLToPath(import.meta.url));
//   // REQUIRED — no default. Use your coordinator's port capsule only
//   // (see orchestration/ORCHESTRATION.md "Resource capsules"); never 6500.
//   const PORT = Number(process.env.CAPTURE_PORT);
//   if (!Number.isInteger(PORT) || PORT <= 0) {
//     throw new Error('CAPTURE_PORT is required (your port capsule only)');
//   }
//
//   runCapture(async () => {
//     const server = await startOwnedServer({ port: PORT });
//     const browser = await chromium.launch({ headless: true });
//     const checks = {};
//     try {
//       const context = await browser.newContext({ viewport: { width: 1366, height: 768 } });
//       const page = await context.newPage();
//       await loginChronicles(page, { baseUrl: `http://127.0.0.1:${PORT}`, house: 'Demo', scion: 'DemoScion' });
//       const a = await boxOf(page.locator('.some-element'));
//       const b = await boxOf(page.locator('.other-element'));
//       checks['1366x768.no-overlap'] = Boolean(a) && Boolean(b) && !boxesOverlap(a, b);
//       await page.screenshot({ path: screenshotPath(outDir, 'after', '1366x768', 'demo') });
//       await context.close();
//     } finally {
//       await browser.close();
//       stopOwnedServer(server);
//     }
//     return checks;
//   }, { evidencePath: path.join(outDir, 'capture-NNNN-evidence.json') });
//
// Exit semantics: runCapture() writes the evidence JSON, then exits non-zero
// (via process.exit(1)) if any check is falsy or the capture function threw;
// otherwise prints "CAPTURES OK <checks>" and exits 0. This mirrors 0038's
// hard-fail pattern exactly — no silent partial success.

import { spawn } from 'node:child_process';
import path from 'node:path';
import fs from 'node:fs';
import http from 'node:http';
import os from 'node:os';

/** Rounds a Playwright bounding box to 1 decimal place; passes null through. */
export const roundBox = (box) => {
  if (!box) return null;
  return {
    x: Math.round(box.x * 10) / 10,
    y: Math.round(box.y * 10) / 10,
    width: Math.round(box.width * 10) / 10,
    height: Math.round(box.height * 10) / 10,
  };
};

/** True if two boxes overlap by more than `gap` px on both axes. Null-safe (returns false). */
export const boxesOverlap = (a, b, gap = 0) => {
  if (!a || !b) return false;
  return a.x < b.x + b.width - gap
    && a.x + a.width > b.x + gap
    && a.y < b.y + b.height - gap
    && a.y + a.height > b.y + gap;
};

/** True if a box extends outside the viewport by more than `pad` px, or is missing. */
export const overflowsViewport = (box, viewport, pad = 1) => {
  if (!box) return true;
  return box.x < -pad
    || box.y < -pad
    || box.x + box.width > viewport.width + pad
    || box.y + box.height > viewport.height + pad;
};

/** Waits for `locator` to be visible, then returns its rounded bounding box (null on timeout — never throws). */
export const boxOf = async (locator, timeoutMs = 2500) => {
  try {
    await locator.waitFor({ state: 'visible', timeout: timeoutMs });
  } catch {
    return null;
  }
  return roundBox(await locator.boundingBox());
};

/** Polls `url` with plain HTTP GETs until it answers with status < 500. */
export const waitForHttp = (url, timeoutMs = 60_000) => new Promise((resolve, reject) => {
  const started = Date.now();
  const attempt = () => {
    const req = http.get(url, (res) => {
      res.resume();
      if (res.statusCode && res.statusCode < 500) {
        resolve();
        return;
      }
      retry();
    });
    req.on('error', retry);
    req.setTimeout(2000, () => {
      req.destroy();
      retry();
    });
  };
  const retry = () => {
    if (Date.now() - started > timeoutMs) {
      reject(new Error(`server at ${url} did not become ready in ${timeoutMs}ms`));
      return;
    }
    setTimeout(attempt, 400);
  };
  attempt();
});

/** Runs `command` to completion; rejects with combined stdout+stderr on non-zero exit. */
export const runCommand = (command, args, { cwd, env } = {}) => new Promise((resolve, reject) => {
  const child = spawn(command, args, {
    cwd,
    env: { ...process.env, ...env },
    stdio: ['ignore', 'pipe', 'pipe'],
    shell: process.platform === 'win32' && command !== process.execPath,
  });
  let stdout = '';
  let stderr = '';
  child.stdout.on('data', (chunk) => { stdout += chunk.toString(); });
  child.stderr.on('data', (chunk) => { stderr += chunk.toString(); });
  child.on('error', reject);
  child.on('close', (code) => {
    if (code === 0) {
      resolve({ stdout, stderr });
      return;
    }
    reject(new Error(`${command} ${args.join(' ')} exited ${code}\n${stderr}\n${stdout}`));
  });
});

/**
 * Builds (unless skipBuild) and spawns the app's own server on a REQUIRED,
 * caller-supplied port — capsule discipline, no default, never 6500 (owner's
 * live server). Isolates guest-save/Chronicles state per port+run so
 * parallel/serial captures never collide with a developer's own save.
 * Returns the child process; pass it to stopOwnedServer() when done.
 */
export const startOwnedServer = async ({
  port,
  repoRoot = process.cwd(),
  bindHost = '127.0.0.1',
  skipBuild = process.env.SKIP_BUILD === '1',
  nodeEnv = 'production',
  runLabel = 'capture',
  extraEnv = {},
  readyTimeoutMs = 90_000,
}) => {
  if (!Number.isInteger(port) || port <= 0) {
    throw new Error('startOwnedServer: port is required (capsule discipline — no default)');
  }
  if (port === 6500) {
    throw new Error('startOwnedServer: port 6500 is the owner\'s live server — never touch');
  }
  if (!skipBuild) {
    console.log(`[capture-harness] production build (PORT ${port})`);
    await runCommand(
      process.platform === 'win32' ? 'npx.cmd' : 'npx',
      ['vite', 'build', '--mode', 'production'],
      { cwd: repoRoot, env: { NODE_ENV: 'production' } },
    );
  }
  const stateDir = path.join(os.tmpdir(), `verdigris-${runLabel}-${port}-${Date.now().toString(36)}`);
  const child = spawn(process.execPath, ['server/index.js'], {
    cwd: repoRoot,
    env: {
      ...process.env,
      ...extraEnv,
      PORT: String(port),
      NODE_ENV: nodeEnv,
      VERDIGRIS_BIND_HOST: bindHost,
      GUEST_SAVE_DIR: stateDir,
      CHRONICLES_DB_FILE: `${stateDir}.sqlite`,
      CHRONICLES_STORE_FILE: `${stateDir}.json`,
    },
    stdio: ['ignore', 'pipe', 'pipe'],
  });
  let output = '';
  child.stdout.on('data', (chunk) => { output += chunk.toString(); });
  child.stderr.on('data', (chunk) => { output += chunk.toString(); });
  try {
    await waitForHttp(`http://${bindHost}:${port}`, readyTimeoutMs);
  } catch (error) {
    stopOwnedServer(child);
    throw new Error(`${error.message}\n${output}`);
  }
  return child;
};

/** Terminates a server started by startOwnedServer(). Safe to call more than once. */
export const stopOwnedServer = (child) => {
  if (!child || child.killed || !child.pid) {
    return;
  }
  if (process.platform === 'win32') {
    spawn('taskkill', ['/pid', String(child.pid), '/f', '/t'], { stdio: 'ignore' });
    return;
  }
  child.kill('SIGTERM');
};

/**
 * Drives the shared Chronicles/guest-login flow: Play as Guest, found a
 * House (if not already), swear the mortal oath, name a Scion, set out —
 * waits for the game canvas before returning. `names` are truncated to the
 * game's 20-char field limit the same way the original capture scripts did.
 */
export const loginChronicles = async (page, { baseUrl, house, scion }) => {
  await page.goto(baseUrl, { waitUntil: 'domcontentloaded' });
  const guest = page.getByRole('button', { name: 'Play as Guest', exact: true });
  if (await guest.isVisible().catch(() => false)) {
    await guest.click();
  }
  await page.getByRole('heading', { name: 'Chronicles' }).waitFor({ timeout: 30_000 });
  const houseField = page.getByLabel('Found a House');
  if (await houseField.isVisible().catch(() => false)) {
    await houseField.fill(String(house).slice(0, 20));
    await page.getByRole('button', { name: 'Inscribe' }).click();
  }
  await page.locator('.chronicles__mortal-checkbox').check();
  await page.getByLabel('Name a new Scion').fill(String(scion).slice(0, 20));
  await page.getByRole('button', { name: 'Add Scion' }).click();
  await page.getByRole('button', { name: /^Set Out as / }).click();
  await page.locator('canvas[aria-label="Game world"]').waitFor({ state: 'visible', timeout: 30_000 });
};

/** Builds the standard `<outDir>/<prefix>-<viewportName>-<label>.png` capture path. */
export const screenshotPath = (outDir, prefix, viewportName, label) => (
  path.join(outDir, `${prefix}-${viewportName}-${label}.png`)
);

/**
 * Runs `fn`, which must return a flat `{ [checkName]: boolean }` map (or
 * throw). Writes that map verbatim to `evidencePath`, then hard-fails: any
 * falsy check, or a thrown error, prints it and exits 1; an all-true map
 * prints "CAPTURES OK <checks>" and returns normally (exit 0). Never leaves
 * a partial/ambiguous exit code — there is no "PARTIAL" outcome.
 */
export const runCapture = async (fn, { evidencePath }) => {
  try {
    const checks = await fn();
    fs.writeFileSync(evidencePath, JSON.stringify(checks, null, 2));
    const failures = Object.entries(checks).filter(([, ok]) => !ok).map(([name]) => name);
    if (failures.length) {
      throw new Error(`CAPTURE FAILED: ${failures.join(', ')}`);
    }
    console.log('CAPTURES OK', JSON.stringify(checks));
  } catch (error) {
    console.error(error);
    process.exit(1);
  }
};
