import { chromium } from 'playwright';

const baseUrl = process.env.PLAYWRIGHT_BASE_URL || 'http://127.0.0.1:6547';
const browser = await chromium.launch({ headless: true });
const page = await browser.newPage({ viewport: { width: 1440, height: 1000 } });

await page.goto(`${baseUrl}/`, { waitUntil: 'domcontentloaded' });
await page.getByRole('button', { name: 'Play as Guest', exact: true }).click();
await page.getByRole('heading', { name: 'Chronicles' }).waitFor({ timeout: 30000 });
const house = page.getByLabel('Found a House');
if (await house.isVisible()) {
  await house.fill('Wire Ember');
  await page.getByRole('button', { name: 'Inscribe' }).click();
}
await page.locator('.chronicles__mortal-checkbox').check();
await page.getByLabel('Name a new Scion').fill('Wire Asha');
await page.getByRole('button', { name: 'Add Scion' }).click();
await page.getByRole('button', { name: /^Set Out as / }).click();
await page.locator('canvas[aria-label="Game world"]').waitFor({ state: 'visible', timeout: 30000 });
await page.getByLabel('World minimap').waitFor({ state: 'visible', timeout: 30000 });
await page.getByRole('button', { name: 'Adventure', exact: true }).click();
const menu = page.getByLabel('Choose a zone');
await menu.getByRole('button', { name: /Old Barrow/ }).click();
await page.getByLabel('World minimap').getByText('The Old Barrow').waitFor({ timeout: 30000 });
await page.waitForTimeout(2500);

await page.evaluate(() => {
  window.__wire = [];
  const socket = window.ws;
  if (!socket) return;
  const send = socket.send.bind(socket);
  socket.send = (value) => {
    try {
      const message = JSON.parse(value);
      if (message.event === 'player:skill:trigger' || message.event === 'player:move') {
        window.__wire.push({ direction: 'out', event: message.event, data: message.data, at: Date.now() });
      }
    } catch { /* non-JSON frame */ }
    return send(value);
  };
  socket.addEventListener('message', (event) => {
    try {
      const message = JSON.parse(event.data);
      if (['combat:hit', 'game:send:message', 'player:stats:update', 'player:movement', 'monster:state'].includes(message.event)) {
        window.__wire.push({ direction: 'in', event: message.event, data: message.data, at: Date.now() });
      }
    } catch { /* non-JSON frame */ }
  });
});

const canvas = page.locator('canvas[aria-label="Game world"]');
const bounds = await canvas.boundingBox();
const keys = ['KeyD', 'KeyS', 'KeyA', 'KeyW'];
for (let index = 0; index < 14; index += 1) {
  const key = keys[index % keys.length];
  await page.keyboard.down(key);
  await page.waitForTimeout(700);
  await page.keyboard.up(key);
  await page.keyboard.press('Space');
  await canvas.click({
    button: 'left',
    position: { x: Math.round(bounds.width * 0.55), y: Math.round(bounds.height * 0.52) },
  });
  await page.waitForTimeout(1200);
}

const result = await page.evaluate(() => ({
  wire: window.__wire,
  body: document.body.innerText.slice(-5000),
  ws: { url: window.ws?.url || null, readyState: window.ws?.readyState ?? null },
}));
process.stdout.write(`${JSON.stringify(result, null, 2)}\n`);
await browser.close();
