// TASK-0027 ground truth: completely unpatched page, waited out the real
// 300 s day cycle to t ~ 0.80 (deep night). If the orbs are bright here, the
// black-orb defect never existed in the real game — it was a capture-harness
// clock-skew artifact.
//
// Usage: node probe-night-real.mjs   (server must already run on 127.0.0.1:6500)

import { chromium } from 'playwright';
import path from 'node:path';
import { fileURLToPath } from 'node:url';

const here = path.dirname(fileURLToPath(import.meta.url));
const gameUrl = process.env.PLAYWRIGHT_BASE_URL || 'http://127.0.0.1:6500';
const NIGHT_MS = 0.80 * 300 * 1000;

const onboard = async (page) => {
  await page.goto(`${gameUrl}/`);
  await page.getByRole('button', { name: 'Play as Guest', exact: true }).click();
  const chronicles = page.getByRole('heading', { name: 'Chronicles' });
  const deadline = Date.now() + 15000;
  while (Date.now() < deadline) {
    if (await page.locator('#game-map').isVisible()) break;
    if (await chronicles.isVisible()) break;
    await page.waitForTimeout(250);
  }
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
    const setOut = page.getByRole('button', { name: /^Set Out as / });
    await setOut.click();
  }
  const canvas = page.locator('canvas[aria-label="Game world"]');
  await canvas.waitFor({ state: 'visible', timeout: 15000 });
  return canvas;
};

const browser = await chromium.launch();
try {
  const context = await browser.newContext({ viewport: { width: 1440, height: 1000 } });
  const page = await context.newPage();
  await onboard(page);
  // The day cycle runs off performance.now() since navigation; wait until the
  // ambient sampler sits at t ~= 0.80 (deep night), then let a few orb frames
  // render before shooting.
  await page.waitForFunction(
    target => performance.now() >= target,
    NIGHT_MS,
    { timeout: NIGHT_MS + 60000, polling: 500 },
  );
  await page.waitForTimeout(2000);
  const t = await page.evaluate(() => Math.round(performance.now() / 100) / 10);
  console.log('real elapsed seconds at capture:', t);
  await page.screenshot({
    path: path.join(here, 'probe-night-real-unpatched.jpg'),
    type: 'jpeg',
    quality: 72,
  });
  console.log('captured probe-night-real-unpatched.jpg');
  await context.close();
} finally {
  await browser.close();
}
