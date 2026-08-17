// TASK-0027 evidence capture: boots a guest session against a locally running
// game server (http://127.0.0.1:6500). Supersedes the 0024 harness.
//
// Changes vs 0024:
//  1. Night shots use a COHERENT virtual clock. The 0024 harness shifted
//     performance.now() by +240 s but not the requestAnimationFrame timestamp
//     timeline; WizardOrbRenderer seeds its animation clock from
//     performance.now() and diffs it against rAF timestamps, so the naive
//     patch fed it dt ~= -240 s, overflowed its flash envelope to Infinity,
//     and NaN'd the orb shader output — the "orbs render nearly black at
//     night" defect carried out of the 0024 review was this harness artifact,
//     not a compositing bug (see probe-orbs.mjs / probe-night-real.mjs and
//     REPORT.md). Here, performance.now and rAF are shifted together.
//  2. Adds Phase-4 DoF evidence: wheel-zoomed close (miniature blend) and
//     wide (crisp floor) captures.
//
// Usage: node capture.mjs <prefix> [shots...]
//   <prefix>-arpg.jpg         ARPG default at the village spawn (day)
//   <prefix>-edge-<dir>.jpg   walked to a map edge, horizon + atmosphere
//   <prefix>-night.jpg        coherent-clock night grade (t ~ 0.80), orbs lit
//   <prefix>-zoom-close.jpg   wheel to max zoom: DoF miniature blend
//   <prefix>-zoom-wide.jpg    wheel to min zoom: DoF zero floor, crisp
//   reference-demo.jpg        D-108 reference
//   <prefix>-vs-reference.jpg side-by-side composite

import { chromium } from 'playwright';
import fs from 'node:fs';
import path from 'node:path';
import { fileURLToPath } from 'node:url';

const here = path.dirname(fileURLToPath(import.meta.url));
const repoRoot = path.resolve(here, '..', '..', '..', '..');
const gameUrl = process.env.PLAYWRIGHT_BASE_URL || 'http://127.0.0.1:6500';
const prefix = process.argv[2] || 'after';
const NIGHT_OFFSET_MS = 0.80 * 300 * 1000;

// Shifts BOTH web clock surfaces the client consumes. performance.now drives
// the renderer day cycle (via core/config/movement.js `now()`); rAF timestamps
// drive the HUD orb animation envelopes. Shifting only the first poisons any
// consumer that mixes the two.
const coherentClockInit = (offset) => {
  const originalNow = performance.now.bind(performance);
  performance.now = () => originalNow() + offset;
  const originalRaf = window.requestAnimationFrame.bind(window);
  window.requestAnimationFrame = cb => originalRaf(ts => cb(ts + offset));
};

const shot = async (page, name, quality = 72) => {
  const file = path.join(here, name);
  await page.screenshot({ path: file, type: 'jpeg', quality });
  console.log('captured', name);
};

const playerPosition = async () => {
  const response = await fetch(`${gameUrl}/world/players`);
  const players = await response.json();
  return players.find(candidate => candidate.username === 'Wayfarer') || players.at(-1);
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

const walkToEdge = async (page, key, label) => {
  let last = await playerPosition();
  let still = 0;
  await page.keyboard.down(key);
  for (let i = 0; i < 26 && still < 3; i += 1) {
    await page.waitForTimeout(1200);
    const current = await playerPosition();
    if (current && last && Math.abs(current.x - last.x) < 0.01 && Math.abs(current.y - last.y) < 0.01) {
      still += 1;
    } else {
      still = 0;
    }
    last = current;
  }
  await page.keyboard.up(key);
  await page.waitForTimeout(800);
  console.log(`${label} edge position`, JSON.stringify(last));
  await shot(page, `${prefix}-edge-${label}.jpg`);
  return last;
};

const gameShots = async (browser, { night = false } = {}) => {
  const context = await browser.newContext({ viewport: { width: 1440, height: 1000 } });
  if (night) {
    // Evidence-only clock offset: start the day cycle at t=0.80 (deep night),
    // coherently across performance.now and rAF (see header note 1).
    await context.addInitScript(coherentClockInit, NIGHT_OFFSET_MS);
  }
  const page = await context.newPage();
  await onboard(page);

  // If a previous session left the Scion inside a combat zone, leave it for
  // the safety of the village before walking long distances.
  const minimap = page.getByLabel('World minimap');
  await page.waitForTimeout(2000);
  const zone = await minimap.textContent();
  console.log('current zone:', zone && zone.trim());
  if (zone && !zone.includes('Village')) {
    const exitButton = page.getByRole('button', { name: 'EXIT', exact: true });
    if (await exitButton.isVisible()) {
      await exitButton.click();
      await page.waitForTimeout(2500);
      console.log('zone after exit:', (await minimap.textContent()).trim());
    }
  }

  if (night) {
    await shot(page, `${prefix}-night.jpg`);
    await context.close();
    return;
  }

  // Day cycle starts at page load; ~12 s in is full morning light.
  await page.waitForTimeout(10000);
  await shot(page, `${prefix}-arpg.jpg`);

  const start = await playerPosition();
  const north = await walkToEdge(page, 'KeyW', 'north');
  // Return toward spawn, then probe the other horizontal directions.
  if (north && start) {
    await page.keyboard.down('KeyS');
    const steps = Math.min(20, Math.ceil(Math.abs(north.y - start.y) / 1.2));
    for (let i = 0; i < steps; i += 1) await page.waitForTimeout(1000);
    await page.keyboard.up('KeyS');
  }
  await walkToEdge(page, 'KeyD', 'east');
  await context.close();
};

// Phase-4 DoF evidence: the wheel range is clamped to [0.72, 1.6] around the
// ARPG base 0.85; the camera blends dofStrength 0 -> 0.82 across [0.85, 1.6].
const zoomShots = async (browser) => {
  const context = await browser.newContext({ viewport: { width: 1440, height: 1000 } });
  const page = await context.newPage();
  await onboard(page);
  await page.waitForTimeout(10000); // same morning-light regime as -arpg
  await page.mouse.move(720, 500);

  // Wheel up to the 1.6 ceiling: 0.85 * 1.08^9 ~= 1.70 -> clamped to 1.6.
  for (let i = 0; i < 9; i += 1) {
    await page.mouse.wheel(0, -120);
    await page.waitForTimeout(120);
  }
  await page.waitForTimeout(800);
  await shot(page, `${prefix}-zoom-close.jpg`);

  // Wheel down to the 0.72 floor: 1.6 * 0.92^10 ~= 0.70 -> clamped to 0.72.
  for (let i = 0; i < 10; i += 1) {
    await page.mouse.wheel(0, 120);
    await page.waitForTimeout(120);
  }
  await page.waitForTimeout(800);
  await shot(page, `${prefix}-zoom-wide.jpg`);
  await context.close();
};

const referenceShot = async (browser) => {
  const page = await browser.newPage({ viewport: { width: 1440, height: 1000 } });
  const demoPath = path.join(repoRoot, 'docs', 'reference', '25d-overhaul', 'dist', 'songs-of-the-mire.html');
  await page.goto(`file://${demoPath.replace(/\\/g, '/')}`);
  await page.locator('#startBtn').click();
  await page.waitForTimeout(4000); // demo starts at dayT=22 (morning light)
  await shot(page, 'reference-demo.jpg');
  await page.close();
};

const sideBySide = async (browser, leftName) => {
  const page = await browser.newPage({ viewport: { width: 1440, height: 520 } });
  const encode = name => `data:image/jpeg;base64,${fs.readFileSync(path.join(here, name)).toString('base64')}`;
  await page.setContent(`
    <body style="margin:0;background:#000;display:flex;gap:4px">
      <img src="${encode(leftName)}" style="width:718px;height:500px;object-fit:cover;object-position:top">
      <img src="${encode('reference-demo.jpg')}" style="width:718px;height:500px;object-fit:cover;object-position:top">
    </body>`);
  await page.waitForTimeout(500);
  await shot(page, `${prefix}-vs-reference.jpg`);
  await page.close();
};

const browser = await chromium.launch();
try {
  if (process.argv[3] === 'refs') {
    await referenceShot(browser);
    await sideBySide(browser, `${prefix}-edge-north.jpg`);
  } else {
    await gameShots(browser);
    await zoomShots(browser);
    await gameShots(browser, { night: true });
    await referenceShot(browser);
    await sideBySide(browser, `${prefix}-edge-north.jpg`);
  }
} finally {
  await browser.close();
}
