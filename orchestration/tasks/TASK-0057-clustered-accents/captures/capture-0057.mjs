// TASK-0057 captures: the overworld village and one generated zone, showing
// clustered floor accents (seeded blobs) instead of one-cell checkerboard noise.
// Run with the game server on 127.0.0.1:<PLAYWRIGHT_BASE_URL port>. Saves PNGs
// here and exits non-zero unless the on-screen assertions hold.
import { chromium } from '@playwright/test';
import { fileURLToPath } from 'node:url';
import path from 'node:path';
import fs from 'node:fs';

const base = process.env.PLAYWRIGHT_BASE_URL || 'http://127.0.0.1:6541';
const outDir = path.dirname(fileURLToPath(import.meta.url));

const login = async (page) => {
  await page.goto(base, { waitUntil: 'domcontentloaded' });
  const guest = page.getByRole('button', { name: 'Play as Guest', exact: true });
  if (await guest.isVisible().catch(() => false)) {
    await guest.click();
  }
  await page.waitForTimeout(1200);
  const chronicles = page.getByRole('heading', { name: 'Chronicles' });
  if (await chronicles.isVisible().catch(() => false)) {
    const houseName = page.getByLabel('Found a House');
    if (await houseName.isVisible().catch(() => false)) {
      await houseName.fill('Clustergate');
      await page.getByRole('button', { name: 'Inscribe' }).click();
    }
    const scionName = page.getByLabel('Name a new Scion');
    if ((await scionName.isVisible().catch(() => false))
      && (await page.locator('.chronicles__scion').count()) === 0) {
      await scionName.fill('Wayfarer');
      await page.getByRole('button', { name: 'Add Scion' }).click();
    }
    await page.getByRole('button', { name: /^Set Out as / }).click();
  }
  await page.locator('canvas[aria-label="Game world"]').waitFor({ timeout: 15_000 });
};

// A real render has varied pixel colours, not a blank/cleared canvas.
const canvasHasContent = async (canvas) => canvas.evaluate((el) => {
  const ctx = el.getContext('2d');
  if (!ctx) return false;
  const width = el.width;
  const height = el.height;
  if (width < 8 || height < 8) return false;
  const data = ctx.getImageData(0, 0, width, height).data;
  const colours = new Set();
  for (let i = 0; i < data.length; i += 4096) { // stride-sample
    colours.add((data[i] << 16) | (data[i + 1] << 8) | data[i + 2]);
    if (colours.size > 16) return true;
  }
  return colours.size > 3;
});

const run = async () => {
  const browser = await chromium.launch();
  const context = await browser.newContext({ viewport: { width: 1366, height: 768 } });
  const page = await context.newPage();

  await login(page);
  const canvas = page.locator('canvas[aria-label="Game world"]');
  const minimap = page.getByLabel('World minimap');

  await minimap.waitFor({ timeout: 10_000 });
  // 01 — overworld village (Delaford) before entering any generated zone.
  await page.screenshot({ path: path.join(outDir, '01-village-overworld.png') });
  const villageRendered = await canvasHasContent(canvas);

  // Enter a generated zone through the real Adventure-menu transition.
  const zoneMenu = page.getByLabel('Choose a zone');
  if (!(await zoneMenu.isVisible().catch(() => false))) {
    await page.getByRole('button', { name: 'Adventure', exact: true }).click();
  }
  await zoneMenu.waitFor({ timeout: 10_000 });
  await zoneMenu.getByRole('button', { name: /Verdant Grove/ }).click();
  await minimap.getByText(/Verdant Grove/).waitFor({ timeout: 20_000 });
  await page.waitForTimeout(800); // let the zone render fully

  // 02 — generated zone (grove/clearings) with clustered dirt accents.
  await page.screenshot({ path: path.join(outDir, '02-zone-clustered-accents.png') });
  const zoneRendered = await canvasHasContent(canvas);

  const evidence = {
    checks: {
      villageRendered,
      zoneRendered,
      zoneEntered: await minimap.getByText(/Verdant Grove/).isVisible().catch(() => false),
    },
  };
  fs.writeFileSync(path.join(outDir, 'capture-evidence.json'), JSON.stringify(evidence, null, 2));

  await browser.close();

  if (!evidence.checks.villageRendered) throw new Error('village canvas did not render content');
  if (!evidence.checks.zoneEntered) throw new Error('Verdant Grove zone did not load');
  if (!evidence.checks.zoneRendered) throw new Error('zone canvas did not render content');
  console.log('CAPTURES OK', JSON.stringify(evidence.checks));
};

run().catch((error) => {
  console.error(error);
  process.exit(1);
});
