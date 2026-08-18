import { chromium } from 'playwright';

const baseUrl = process.env.PLAYWRIGHT_BASE_URL || 'http://127.0.0.1:6554';
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
await page.getByRole('button', { name: 'Adventure', exact: true }).click();
await page.getByLabel('Choose a zone').getByRole('button', { name: /Old Barrow/ }).click();
await page.getByLabel('World minimap').getByText('The Old Barrow').waitFor({ timeout: 30000 });
await page.waitForTimeout(1200);

const result = await page.evaluate(async () => {
  const socket = window.ws;
  const wire = [];
  const waitForState = () => new Promise((resolve, reject) => {
    const requestId = `rendered-${Date.now()}-${Math.random()}`;
    const timeout = window.setTimeout(() => {
      socket.removeEventListener('message', onMessage);
      reject(new Error('dev:state timed out'));
    }, 5000);
    const onMessage = (event) => {
      try {
        const message = JSON.parse(event.data);
        if (message.event === 'dev:state' && message.data?.requestId === requestId) {
          window.clearTimeout(timeout);
          socket.removeEventListener('message', onMessage);
          resolve(message.data.state);
        }
      } catch { /* ignore non-JSON frames */ }
    };
    socket.addEventListener('message', onMessage);
    socket.send(JSON.stringify({ event: 'dev:state', data: { requestId } }));
  });
  const onMessage = (event) => {
    try {
      const message = JSON.parse(event.data);
      if (['combat:hit', 'game:send:message', 'player:stats:update', 'monster:state'].includes(message.event)) {
        wire.push({ direction: 'in', event: message.event, data: message.data, at: Date.now() });
      }
    } catch { /* ignore non-JSON frames */ }
  };
  socket.addEventListener('message', onMessage);
  const opening = await waitForState();
  const target = opening.monsters?.[0];
  if (!target) throw new Error('No opening monster in authoritative state');
  const teleport = { x: Math.round(target.x) + 1, y: Math.round(target.y) };
  socket.send(JSON.stringify({ event: 'dev:teleport', data: teleport }));
  await new Promise(resolve => window.setTimeout(resolve, 400));
  const trigger = () => {
    const data = {
      id: opening.uuid,
      skillId: 'primary-attack',
      direction: 'left',
      issuedAt: Date.now(),
      modifiers: { globalCooldownMs: 350 },
      phase: 'start',
    };
    wire.push({ direction: 'out', event: 'player:skill:trigger', data, at: Date.now() });
    socket.send(JSON.stringify({ event: 'player:skill:trigger', data }));
  };
  trigger();
  await new Promise(resolve => window.setTimeout(resolve, 650));
  trigger();
  await new Promise(resolve => window.setTimeout(resolve, 1300));
  socket.removeEventListener('message', onMessage);
  return {
    target: { uuid: target.uuid, name: target.name, x: target.x, y: target.y },
    teleport,
    wire,
    body: document.body.innerText.slice(-7000),
    ws: { url: socket.url, readyState: socket.readyState },
  };
});

const hits = result.wire.filter((frame) => frame.event === 'combat:hit');
if (!hits.some((frame) => frame.data?.targetId === result.target.uuid
  && Number(frame.data?.amount) > 0)) {
  throw new Error('Rendered capture did not record positive damage to the opening actor');
}
if (!hits.some((frame) => frame.data?.targetId === result.target.uuid
  && frame.data?.died === true)) {
  throw new Error('Rendered capture did not record the opening actor death');
}
if (!/slain|died|defeated/i.test(result.body)) {
  throw new Error('Rendered capture body has no readable first-kill confirmation');
}

process.stdout.write(`${JSON.stringify(result, null, 2)}\n`);
await browser.close();
