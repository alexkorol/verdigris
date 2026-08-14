import { defineConfig } from '@playwright/test';
import { existsSync } from 'node:fs';

const localChrome = 'C:/Program Files/Google/Chrome/Application/chrome.exe';
const browserChannel = process.env.PLAYWRIGHT_CHANNEL
  || (process.platform === 'win32' && existsSync(localChrome) ? 'chrome' : null);

export default defineConfig({
  testDir: './tests/e2e',
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
});
