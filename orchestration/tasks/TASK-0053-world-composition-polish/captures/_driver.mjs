/**
 * TASK-0053 capture driver: real rendered evidence for the world-composition
 * deliverables, against a booted server (never port 6500):
 *   01-barrow-interior.jpg — Old Barrow room: wall faces on exposed cells,
 *                            dark room-mass beyond
 *   02-grove-treeline.jpg  — Grove clearing edge: dense tree-line boundary
 *
 * Usage: GAME_URL=http://127.0.0.1:9881 node _driver.mjs
 * Hard-fails if the game never reaches the expected scenes (0038 pattern).
 */
import { chromium } from 'playwright';
import fs from 'node:fs';
import path from 'node:path';
import { fileURLToPath } from 'node:url';

const GAME_URL = process.env.GAME_URL || 'http://127.0.0.1:9881';
const OUT_DIR = path.dirname(fileURLToPath(import.meta.url));
const PREFIX = process.env.CAPTURE_PREFIX || '';
const MAX_SHOT_BYTES = 250 * 1024;

const sleep = ms => new Promise(resolve => setTimeout(resolve, ms));

const saveShot = async (page, name) => {
  const file = path.join(OUT_DIR, `${PREFIX}${name}`);
  for (const quality of [72, 60, 48]) {
    await page.screenshot({ path: file, type: 'jpeg', quality });
    if (fs.statSync(file).size <= MAX_SHOT_BYTES) return;
  }
  throw new Error(`${name} exceeds ${MAX_SHOT_BYTES} bytes even at lowest quality`);
};

const main = async () => {
  const browser = await chromium.launch();
  const page = await browser.newPage({ viewport: { width: 1440, height: 1000 } });
  page.on('pageerror', error => console.warn('[pageerror]', error.message));

  await page.goto(GAME_URL, { waitUntil: 'domcontentloaded' });

  // Guest login + Chronicles onboarding (same flow as the e2e critical loop).
  await page.getByRole('button', { name: 'Play as Guest', exact: true }).click();
  const chroniclesHeading = page.getByRole('heading', { name: 'Chronicles' });
  await chroniclesHeading.waitFor({ state: 'visible', timeout: 15000 }).catch(() => {});
  if (await chroniclesHeading.isVisible()) {
    const houseName = page.getByLabel('Found a House');
    if (await houseName.isVisible()) {
      await houseName.fill('House Composition');
      await page.getByRole('button', { name: 'Inscribe' }).click();
    }
    const scionName = page.getByLabel('Name a new Scion');
    await scionName.waitFor({ state: 'visible', timeout: 10000 });
    if (await page.locator('.chronicles__scion').count() === 0) {
      await scionName.fill('Wayfarer Mason');
      await page.getByRole('button', { name: 'Add Scion' }).click();
    }
    await page.getByRole('button', { name: /^Set Out as / }).click();
  }

  const canvas = page.locator('canvas[aria-label="Game world"]');
  await canvas.waitFor({ state: 'visible', timeout: 20000 });

  // Development-only wire seam (same envelope the playtest harness uses).
  await page.evaluate(() => {
    window.__dev = { state: null, requestId: 0 };
    window.ws.addEventListener('message', (event) => {
      try {
        const message = JSON.parse(event.data);
        if (message.event === 'dev:state' && message.data) {
          window.__dev.state = {
            requestId: message.data.requestId,
            ...(message.data.state || {}),
          };
        }
      } catch { /* non-JSON frames are not dev states */ }
    });
    window.__devSend = (event, data) => {
      window.ws.send(JSON.stringify({ event, data }));
    };
  });
  const devSend = (event, data = {}) => page.evaluate(
    ([name, payload]) => window.__devSend(name, payload),
    [event, data],
  );
  const devState = async () => {
    const requestId = await page.evaluate(() => {
      window.__dev.requestId += 1;
      window.__devSend('dev:state', { requestId: window.__dev.requestId });
      return window.__dev.requestId;
    });
    await page.waitForFunction(
      id => window.__dev.state && window.__dev.state.requestId === id,
      requestId,
      { timeout: 8000 },
    );
    return page.evaluate(() => window.__dev.state);
  };

  const enterZone = async (buttonName) => {
    await page.getByRole('button', { name: 'Adventure', exact: true }).click();
    await page.getByRole('button', { name: buttonName }).click();
    for (let attempt = 0; attempt < 30; attempt += 1) {
      await sleep(700);
      const state = await devState();
      if (state.sceneType === 'instance') return state;
    }
    throw new Error(`never reached instance via ${buttonName}`);
  };

  await devSend('dev:setlevel', { level: 5 });
  await devSend('dev:heal');

  // 01 — Old Barrow interior: exposed wall faces + dark room mass.
  const barrow = await enterZone(/The Old Barrow/);
  console.log('in instance:', barrow.sceneName, 'at', barrow.x, barrow.y);
  await devSend('dev:teleport', { x: Math.round(barrow.x) + 6, y: Math.round(barrow.y) });
  await sleep(1200); // let the camera settle and billboards draw
  await saveShot(page, '01-barrow-interior.jpg');

  // 02 — Grove boundary: return to town, enter the Verdant Grove, walk
  // toward the northern treeline and face it.
  await devSend('party:returnToTown', {});
  await sleep(2000);
  const grove = await enterZone(/Verdant Grove/);
  console.log('in instance:', grove.sceneName, 'at', grove.x, grove.y);
  const north = { x: Math.round(grove.x), y: Math.round(grove.y) - 14 };
  await devSend('dev:teleport', north);
  await sleep(400);
  const at = await devState();
  console.log('teleported toward treeline:', Math.round(at.x), Math.round(at.y));
  await sleep(1200);
  await saveShot(page, '02-grove-treeline.jpg');

  await browser.close();
  console.log('captures written to', OUT_DIR);
};

main().catch((error) => {
  console.error(error);
  process.exit(1);
});
