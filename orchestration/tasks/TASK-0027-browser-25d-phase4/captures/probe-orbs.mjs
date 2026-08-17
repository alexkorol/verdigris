// TASK-0027 diagnostic probe: why did the HP/MP orbs render black in the
// 0024 night capture?
//
// Hypothesis: the 0024 night harness shifted `performance.now()` by +240 s
// but NOT the requestAnimationFrame timestamp timeline. WizardOrbRenderer
// seeds `startedAt`/`lastFrameAt` from the shifted performance.now() and then
// diffs them against UNSHIFTED rAF timestamps, producing dt ≈ -240 s. With a
// negative dt, `flash *= Math.exp(-dt * 3.2)` overflows to Infinity and the
// orb shader outputs NaN -> black. The game itself composites the lightmap on
// the world canvas, structurally BELOW the DOM HUD, so real night play shows
// bright orbs.
//
// Context A replays the 0024 patch verbatim (expect: black orbs, ~240 s skew).
// Context B shifts performance.now AND wraps rAF coherently (expect: bright
// orbs at the same night grade).
// Context C is the unpatched day baseline (expect: bright orbs).
//
// Usage: node probe-orbs.mjs   (server must already run on 127.0.0.1:6500)

import { chromium } from 'playwright';
import path from 'node:path';
import { fileURLToPath } from 'node:url';

const here = path.dirname(fileURLToPath(import.meta.url));
const gameUrl = process.env.PLAYWRIGHT_BASE_URL || 'http://127.0.0.1:6500';
const NIGHT_OFFSET_MS = 0.80 * 300 * 1000;

const shot = async (page, name, quality = 72) => {
  const file = path.join(here, name);
  await page.screenshot({ path: file, type: 'jpeg', quality });
  console.log('captured', name);
};

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

// Measures the skew between the (possibly patched) performance.now() and the
// rAF timestamp timeline, plus the ambient grade the renderer would compute.
const probeClocks = async page => page.evaluate(() => {
  const perfNow = performance.now();
  return new Promise((resolve) => {
    requestAnimationFrame((rafTs) => {
      resolve({
        perfNow: Math.round(perfNow),
        rafTs: Math.round(rafTs),
        skewSeconds: Math.round((perfNow - rafTs) / 100) / 10,
      });
    });
  });
});

const run = async () => {
  const browser = await chromium.launch();
  try {
    // Context A — verbatim 0024 night patch (performance.now only).
    const ctxA = await browser.newContext({ viewport: { width: 1440, height: 1000 } });
    await ctxA.addInitScript((offset) => {
      const original = performance.now.bind(performance);
      performance.now = () => original() + offset;
    }, NIGHT_OFFSET_MS);
    const pageA = await ctxA.newPage();
    await onboard(pageA);
    await pageA.waitForTimeout(3000); // let orb rAF loops accumulate the bad dt
    console.log('A (naive patch) clocks:', JSON.stringify(await probeClocks(pageA)));
    await shot(pageA, 'probe-night-naive-clock.jpg');
    await ctxA.close();

    // Context B — coherent virtual clock: shift rAF timestamps identically.
    const ctxB = await browser.newContext({ viewport: { width: 1440, height: 1000 } });
    await ctxB.addInitScript((offset) => {
      const originalNow = performance.now.bind(performance);
      performance.now = () => originalNow() + offset;
      const originalRaf = window.requestAnimationFrame.bind(window);
      window.requestAnimationFrame = cb => originalRaf(ts => cb(ts + offset));
    }, NIGHT_OFFSET_MS);
    const pageB = await ctxB.newPage();
    await onboard(pageB);
    await pageB.waitForTimeout(3000);
    console.log('B (coherent clock) clocks:', JSON.stringify(await probeClocks(pageB)));
    await shot(pageB, 'probe-night-coherent-clock.jpg');
    await ctxB.close();

    // Context C — unpatched day baseline.
    const ctxC = await browser.newContext({ viewport: { width: 1440, height: 1000 } });
    const pageC = await ctxC.newPage();
    await onboard(pageC);
    await pageC.waitForTimeout(3000);
    console.log('C (unpatched day) clocks:', JSON.stringify(await probeClocks(pageC)));
    await shot(pageC, 'probe-day-unpatched.jpg');
    await ctxC.close();
  } finally {
    await browser.close();
  }
};

await run();
