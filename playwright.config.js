import { existsSync, realpathSync } from 'node:fs';
import { resolve } from 'node:path';
import { fileURLToPath } from 'node:url';

const localChrome = 'C:/Program Files/Google/Chrome/Application/chrome.exe';
const browserChannel = process.env.PLAYWRIGHT_CHANNEL
  || (process.platform === 'win32' && existsSync(localChrome) ? 'chrome' : null);

// Windows: Node's module cache is case-sensitive while the filesystem is not.
// When the shell cwd casing (e.g. z:\code\games\...) differs from the on-disk
// casing (Z:\Code\Games\...), Playwright's loader process keeps one copy of
// @playwright/test under the real-cased path while ESM specs import a second
// copy under the cwd-cased path, and every test() call fails with "Playwright
// Test did not expect test() to be called here". Canonicalize testDir to the
// on-disk casing so the specs resolve the same module instance, and keep this
// config free of any runtime '@playwright/test' import (defineConfig is an
// identity helper) so the config itself never loads the cwd-cased copy.
const configDir = fileURLToPath(new URL('.', import.meta.url));
const testDir = process.platform === 'win32'
  ? realpathSync.native(resolve(configDir, 'tests/e2e'))
  : './tests/e2e';

/** @type {import('@playwright/test').PlaywrightTestConfig} */
export default {
  testDir,
  // CI runners render the perspective canvas through software WebGL, so give
  // the shared runner one complete game client at a time and enough room to
  // finish the same browser contract. Local feedback remains fast.
  timeout: process.env.CI ? 120_000 : 60_000,
  workers: process.env.CI ? 1 : undefined,
  retries: process.env.CI ? 1 : 0,
  reporter: [['list']],
  use: {
    // The critical-loop spec navigates relative to baseURL and expects the
    // externally booted game server on :6500 (see npm run test:e2e:built).
    // The reconnect/smoke specs boot their own servers on :6512/:6514 and
    // navigate by absolute URL, so this default does not affect them.
    baseURL: process.env.PLAYWRIGHT_BASE_URL || 'http://127.0.0.1:6500',
    ...(browserChannel ? { channel: browserChannel } : {}),
    headless: true,
    screenshot: 'only-on-failure',
    // Action-by-action trace snapshots force a software-WebGL readback and can
    // consume the entire CI timeout. Keep the first attempt representative;
    // if it fails, Playwright records the retry for diagnosis.
    trace: process.env.CI ? 'on-first-retry' : 'retain-on-failure',
  },
};
