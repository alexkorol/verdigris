// TASK-0053 frame-time evidence, same method as 0029 phase-5
// (synchronous RAF callback duration, 1440x1000, 12s WASD workload) but the
// measurement happens INSIDE the Old Barrow — the wall-billboard path this
// task changes. Usage (server already listening on a free port, never 6500):
//   GAME_URL=http://127.0.0.1:9881 node _measure-frame-time.mjs before
//   GAME_URL=http://127.0.0.1:9881 node _measure-frame-time.mjs after

import { chromium } from 'playwright';

const gameUrl = process.env.GAME_URL || 'http://127.0.0.1:9881';
const label = process.argv[2] || 'run';
const workloadMs = 12_000;

const completeChronicles = async (page) => {
  const chronicles = page.getByRole('heading', { name: 'Chronicles' });
  await page.getByRole('button', { name: 'Play as Guest', exact: true }).click();
  await page.waitForFunction(() => (
    Boolean(document.querySelector('#game-map'))
    || [...document.querySelectorAll('h1,h2,h3')]
      .some(element => element.textContent?.trim() === 'Chronicles')
  ), null, { timeout: 15_000 });
  if (await chronicles.isVisible()) {
    const houseName = page.getByLabel('Found a House');
    if (await houseName.isVisible()) {
      await houseName.fill('Gateward');
      await page.getByRole('button', { name: 'Inscribe' }).click();
    }
    const scionName = page.getByLabel('Name a new Scion');
    if (await scionName.isVisible() && await page.locator('.chronicles__scion').count() === 0) {
      await scionName.fill('Wayfarer');
      await page.getByRole('button', { name: 'Add Scion' }).click();
    }
    await page.getByRole('button', { name: /^Set Out as / }).click();
  }
  await page.locator('canvas[aria-label="Game world"]').waitFor({
    state: 'visible',
    timeout: 15_000,
  });
};

const enterOldBarrow = async (page) => {
  await page.getByRole('button', { name: 'Adventure', exact: true }).click();
  await page.getByRole('button', { name: /The Old Barrow/ }).click();
  await page.waitForTimeout(3_000); // instance transition + first frames
};

const browser = await chromium.launch();
try {
  const context = await browser.newContext({ viewport: { width: 1440, height: 1000 } });
  await context.addInitScript(() => {
    const nativeRaf = window.requestAnimationFrame.bind(window);
    const samples = [];
    let collecting = false;
    let startedAt = 0;
    window.__startFrameMeasurement = () => {
      samples.length = 0;
      startedAt = performance.now();
      collecting = true;
    };
    window.__stopFrameMeasurement = () => {
      collecting = false;
      return { startedAt, samples: [...samples] };
    };
    window.requestAnimationFrame = callback => nativeRaf(timestamp => {
      const callbackStart = performance.now();
      callback(timestamp);
      const callbackDuration = performance.now() - callbackStart;
      if (collecting && callbackStart >= startedAt) samples.push(callbackDuration);
    });
  });
  const page = await context.newPage();
  await page.goto(`${gameUrl}/`);
  await completeChronicles(page);
  await enterOldBarrow(page);

  await page.evaluate(() => window.__startFrameMeasurement());
  const keys = ['KeyD', 'KeyS', 'KeyA', 'KeyW'];
  for (const key of keys) {
    await page.keyboard.down(key);
    await page.waitForTimeout(workloadMs / keys.length);
    await page.keyboard.up(key);
  }
  const measurement = await page.evaluate(() => window.__stopFrameMeasurement());
  const samples = measurement.samples.filter(Number.isFinite);
  if (samples.length < 100) {
    throw new Error(`Expected at least 100 frame samples, received ${samples.length}.`);
  }
  const sorted = [...samples].sort((a, b) => a - b);
  const percentile = (fraction) => sorted[Math.min(
    sorted.length - 1,
    Math.ceil(sorted.length * fraction) - 1,
  )];
  const mean = samples.reduce((sum, value) => sum + value, 0) / samples.length;
  const result = {
    label,
    viewport: '1440x1000',
    scene: 'old-barrow-interior',
    workloadSeconds: workloadMs / 1000,
    sampleCount: samples.length,
    meanMs: Number(mean.toFixed(3)),
    p95Ms: Number(percentile(0.95).toFixed(3)),
    maxMs: Number(Math.max(...samples).toFixed(3)),
  };
  console.log(JSON.stringify(result));
  await context.close();
} finally {
  await browser.close();
}
