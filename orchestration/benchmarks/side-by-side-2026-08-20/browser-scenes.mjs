// D-124 side-by-side benchmark: browser reference scenes matched (loosely)
// to the native 0070 reference set. Visual comparison artifact — NOT a gate;
// staging shortcuts documented inline.
import { chromium } from '@playwright/test';
import { fileURLToPath } from 'node:url';
import path from 'node:path';
import {
  startOwnedServer, stopOwnedServer, runCapture, loginChronicles,
} from '../../../tests/e2e/lib/capture-harness.mjs';

const outDir = path.dirname(fileURLToPath(import.meta.url));
const PORT = Number(process.env.CAPTURE_PORT);
if (!Number.isInteger(PORT) || PORT <= 0) throw new Error('CAPTURE_PORT required');

const shoot = (page, name) =>
  page.screenshot({ path: path.join(outDir, `browser-${name}-1920x1080.png`) });

runCapture(async () => {
  const server = await startOwnedServer({ port: PORT });
  const browser = await chromium.launch({ headless: true });
  const checks = {};
  try {
    const context = await browser.newContext({ viewport: { width: 1920, height: 1080 } });
    const page = await context.newPage();
    await loginChronicles(page, {
      baseUrl: `http://127.0.0.1:${PORT}`, house: 'Verdigris', scion: 'Benchmark',
    });
    const canvas = page.locator('canvas[aria-label="Game world"]');
    await page.waitForTimeout(1500);
    checks['login'] = true;
    await shoot(page, '01-route-entrance');

    // Enter the adventure zone and wander into combat.
    try {
      await page.getByRole('button', { name: 'Adventure', exact: true }).click({ timeout: 8000 });
      await page.waitForTimeout(1200);
    } catch {}
    const bounds = await canvas.boundingBox();
    for (const key of ['KeyD', 'KeyD', 'KeyS', 'KeyD', 'KeyW', 'KeyD']) {
      await page.keyboard.down(key);
      await page.waitForTimeout(700);
      await page.keyboard.up(key);
      // Attack toward the movement direction while traveling (LMB skill).
      if (bounds) {
        await canvas.click({
          position: { x: Math.round(bounds.width * 0.6), y: Math.round(bounds.height * 0.5) },
        }).catch(() => {});
      }
      await page.waitForTimeout(200);
    }
    checks['combat-walk'] = true;
    await shoot(page, '02-pack-combat');

    // A few more strikes then the aftermath shot (stands in for telegraph).
    for (let i = 0; i < 6; i++) {
      if (bounds) {
        await canvas.click({
          position: { x: Math.round(bounds.width * 0.55), y: Math.round(bounds.height * 0.45) },
        }).catch(() => {});
      }
      await page.waitForTimeout(350);
    }
    await shoot(page, '03-combat-aftermath');
    checks['aftermath'] = true;

    // Inventory open (matches native scene 4's gear pane).
    await page.keyboard.press('KeyI');
    await page.waitForTimeout(900);
    await shoot(page, '04-inventory-open');
    checks['inventory'] = true;
    await page.keyboard.press('Escape');

    // HUD-focused shot (orbs/quickbar closeup context = native scene 5 slot).
    await page.waitForTimeout(400);
    await shoot(page, '05-hud-state');
    checks['hud'] = true;

    await context.close();
  } finally {
    await browser.close();
    stopOwnedServer(server);
  }
  return checks;
}, { evidencePath: path.join(outDir, 'browser-scenes-evidence.json') });
