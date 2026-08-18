import { chromium } from 'playwright';

const baseUrl = process.env.PLAYWRIGHT_BASE_URL || 'http://127.0.0.1:6550';
const arcMs = Number(process.env.ARC_MS || 600000);
const log = [];

const note = (entry) => {
  log.push({ at: new Date().toISOString(), ...entry });
};

const waitForMap = async (page) => {
  await page.locator('canvas[aria-label="Game world"]').waitFor({ state: 'visible', timeout: 30000 });
  await page.getByLabel('World minimap').waitFor({ state: 'visible', timeout: 30000 });
};

const chooseOldBarrow = async (page) => {
  const adventure = page.getByRole('button', { name: 'Adventure', exact: true });
  await adventure.click();
  const menu = page.getByLabel('Choose a zone');
  await menu.waitFor({ state: 'visible', timeout: 10000 });
  await menu.getByRole('button', { name: /Old Barrow/ }).click();
  await page.getByLabel('World minimap').getByText('The Old Barrow').waitFor({ timeout: 30000 });
};

const keepAlive = async (page, label) => {
  const canvas = page.locator('canvas[aria-label="Game world"]');
  const bounds = await canvas.boundingBox();
  const keys = ['KeyD', 'KeyS', 'KeyA', 'KeyW'];
  const started = Date.now();
  let index = 0;
  note({ event: 'arc-start', label, url: page.url(), windowWs: await page.evaluate(() => ({
    url: window.ws?.url || null,
    readyState: window.ws?.readyState ?? null,
  })) });
  while (Date.now() - started < arcMs) {
    const key = keys[index % keys.length];
    await page.keyboard.down(key);
    await page.waitForTimeout(700);
    await page.keyboard.up(key);
    await page.keyboard.press('Space').catch(() => {});
    if (bounds) {
      await canvas.click({
        button: 'left',
        position: { x: Math.round(bounds.width * 0.55), y: Math.round(bounds.height * 0.52) },
      }).catch(() => {});
    }
    if (index % 4 === 0) {
      note({
        event: 'arc-checkpoint',
        label,
        elapsedMs: Date.now() - started,
        minimap: await page.getByLabel('World minimap').textContent().catch(() => null),
        hp: await page.locator('.orb--life, [aria-label*="Life"]').first().textContent().catch(() => null),
      });
    }
    index += 1;
    await page.waitForTimeout(4300);
  }
  note({ event: 'arc-end', label, elapsedMs: Date.now() - started, url: page.url() });
};

const startChronicles = async (page) => {
  await page.goto(baseUrl, { waitUntil: 'networkidle' });
  await page.getByRole('button', { name: 'Play as Guest', exact: true }).click();
  await page.getByRole('heading', { name: 'Chronicles' }).waitFor({ timeout: 30000 });
  const house = page.getByLabel('Found a House');
  if (await house.isVisible()) {
    await house.fill('Playwright Ember');
    await page.getByRole('button', { name: 'Inscribe' }).click();
  }
  const mortal = page.locator('.chronicles__mortal-checkbox');
  await mortal.check();
  await page.getByLabel('Name a new Scion').fill('Playwright Asha');
  await page.getByRole('button', { name: 'Add Scion' }).click();
  await page.getByRole('button', { name: /^Set Out as / }).click();
  await waitForMap(page);
  await chooseOldBarrow(page);
};

const startQuickGuest = async (page) => {
  await page.goto(`${baseUrl}/?play`, { waitUntil: 'networkidle' });
  await waitForMap(page);
  await chooseOldBarrow(page);
};

const browser = await chromium.launch({ headless: true });
try {
  const guest = await browser.newPage({ viewport: { width: 1440, height: 1000 } });
  await startQuickGuest(guest);
  await keepAlive(guest, 'guest-quickstart');
  await guest.close();

  const chronicles = await browser.newPage({ viewport: { width: 1440, height: 1000 } });
  await startChronicles(chronicles);
  await keepAlive(chronicles, 'chronicles-mortal-oath');
  await chronicles.close();
} finally {
  await browser.close();
}

process.stdout.write(`${JSON.stringify({ baseUrl, arcMs, log }, null, 2)}\n`);

