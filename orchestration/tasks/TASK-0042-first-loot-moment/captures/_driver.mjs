/**
 * TASK-0042 capture driver: plays the real built game in a browser through
 * the first-delve first-find moment and captures the three beats as JPEGs:
 *   01-drop-moment.jpg    — the tagged drop on the ground + chat + HUD prompt
 *   02-pickup-prompt.jpg  — standing on the find, Take prompt visible
 *   03-comparison-toast.jpg — the LootMoment toast with the comparison line
 *
 * Usage: boot a development server on a free port (never 6500), then
 *   GAME_URL=http://127.0.0.1:9881 node captures/_driver.mjs
 *
 * Movement/combat use the same development-only WS events the playtest
 * harness uses (dev:setlevel/dev:teleport/dev:state); the pickup itself is
 * the real Z grab key so the toast path is exactly what a player triggers.
 */
import { chromium } from 'playwright';
import fs from 'node:fs';
import path from 'node:path';
import { fileURLToPath } from 'node:url';

const GAME_URL = process.env.GAME_URL || 'http://127.0.0.1:9881';
const OUT_DIR = path.dirname(fileURLToPath(import.meta.url));
const MAX_SHOT_BYTES = 250 * 1024;

const sleep = ms => new Promise(resolve => setTimeout(resolve, ms));

const saveShot = async (page, name) => {
  const file = path.join(OUT_DIR, name);
  for (const quality of [72, 60, 48]) {
    await page.screenshot({ path: file, type: 'jpeg', quality });
    if (fs.statSync(file).size <= MAX_SHOT_BYTES) {
      return;
    }
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
      await houseName.fill('House Firstlight');
      await page.getByRole('button', { name: 'Inscribe' }).click();
    }
    const scionName = page.getByLabel('Name a new Scion');
    await scionName.waitFor({ state: 'visible', timeout: 10000 });
    if (await page.locator('.chronicles__scion').count() === 0) {
      await scionName.fill('Wayfarer Findborn');
      await page.getByRole('button', { name: 'Add Scion' }).click();
    }
    await page.getByRole('button', { name: /^Set Out as / }).click();
  }

  const canvas = page.locator('canvas[aria-label="Game world"]');
  await canvas.waitFor({ state: 'visible', timeout: 20000 });

  // Development-only wire seam: request/response state snapshots plus the
  // dev movement aids, over the client's own socket (envelope {event, data}).
  await page.evaluate(() => {
    window.__dev = { state: null, requestId: 0, droppedItems: [] };
    window.ws.addEventListener('message', (event) => {
      try {
        const message = JSON.parse(event.data);
        if (message.event === 'dev:state' && message.data) {
          window.__dev.state = {
            requestId: message.data.requestId,
            ...(message.data.state || {}),
          };
        }
        // dev:state strips ad-hoc item fields, so the tagged find is tracked
        // from the real world:itemDropped broadcast instead.
        if (message.event === 'world:itemDropped' && Array.isArray(message.data)) {
          window.__dev.droppedItems = message.data;
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

  await devSend('dev:setlevel', { level: 5 });
  await devSend('dev:heal');

  // Enter the first delve through the real Adventure menu.
  await page.getByRole('button', { name: 'Adventure', exact: true }).click();
  await page.getByRole('button', { name: /The Old Barrow/ }).click();
  // Wait for the instance transition via fresh dev states.
  let state = null;
  for (let attempt = 0; attempt < 30; attempt += 1) {
    await sleep(700);
    state = await devState();
    if (state.sceneType === 'instance') break;
  }
  if (!state || state.sceneType !== 'instance') {
    throw new Error('never reached the Old Barrow instance');
  }
  console.log('in instance:', state.sceneName, 'at', state.x, state.y);

  const nearestMonster = s => (s.monsters || [])
    .filter(monster => monster.rarity !== 'elite')
    .sort((a, b) => (Math.abs(a.x - s.x) + Math.abs(a.y - s.y))
      - (Math.abs(b.x - s.x) + Math.abs(b.y - s.y)))[0];

  const attackToward = async (s, target) => {
    const dx = Math.sign((target.x || 0) - s.x);
    const dy = Math.sign((target.y || 0) - s.y);
    const direction = dy < 0 ? (dx < 0 ? 'up-left' : dx > 0 ? 'up-right' : 'up')
      : dy > 0 ? (dx < 0 ? 'down-left' : dx > 0 ? 'down-right' : 'down')
        : (dx < 0 ? 'left' : 'right');
    await devSend('player:skill:trigger', {
      id: s.uuid,
      skillId: 'primary-attack',
      direction,
      issuedAt: Date.now(),
      modifiers: {},
      phase: 'start',
    });
  };

  // Kill the opener until the curated first find lands (first kill decides).
  let find = null;
  for (let attempt = 0; attempt < 90 && !find; attempt += 1) {
    state = await devState();
    if (state.lifecycle !== 'alive') {
      await devSend('dev:heal');
    }
    find = await page.evaluate(() => (
      window.__dev.droppedItems.find(item => item.firstFind) || null
    ));
    if (find) break;
    const target = nearestMonster(state);
    if (!target) {
      await sleep(500);
      continue;
    }
    const distance = Math.abs(target.x - state.x) + Math.abs(target.y - state.y);
    if (distance > 2) {
      await devSend('dev:teleport', { x: Math.round(target.x) + 1, y: Math.round(target.y) });
      state = await devState();
    }
    await attackToward(state, target);
    await sleep(650);
  }
  if (!find) {
    throw new Error('first find never dropped');
  }
  console.log('first find on the ground:', find.displayName || find.name, 'at', find.x, find.y);

  await sleep(900); // let the drop render + chat/prompt settle
  await saveShot(page, '01-drop-moment.jpg');

  // Stand on the find; the HUD prompt names the Take affordance.
  await devSend('dev:teleport', { x: find.x, y: find.y });
  await sleep(900);
  const prompt = await page.locator('.first-action').textContent().catch(() => '');
  console.log('HUD prompt:', (prompt || '').trim());
  await saveShot(page, '02-pickup-prompt.jpg');

  // The real grab key picks it up; the comparison toast follows.
  await canvas.click();
  await page.keyboard.press('z');
  const toast = page.locator('.loot-moment__card');
  await toast.waitFor({ state: 'visible', timeout: 8000 });
  const comparison = await page.locator('.loot-moment__comparison').textContent();
  console.log('comparison toast:', (comparison || '').trim());
  await sleep(400);
  await saveShot(page, '03-comparison-toast.jpg');

  await browser.close();
  console.log('captures written to', OUT_DIR);
};

main().catch((error) => {
  console.error(error);
  process.exit(1);
});
