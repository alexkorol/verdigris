// TASK-0055 captures: identity chip (long names, no HP-orb overlap, full
// title) at 1366x768 and 1920x1080, plus Adventure preview text matching the
// SERVER-sent adventureZones payload. Hard-fail: exits non-zero unless every
// on-screen assertion holds.
import { chromium } from '@playwright/test';
import { fileURLToPath } from 'node:url';
import path from 'node:path';
import fs from 'node:fs';

const base = process.env.PLAYWRIGHT_BASE_URL || 'http://127.0.0.1:6582';
const outDir = path.dirname(fileURLToPath(import.meta.url));

const suffix = Date.now().toString(36).slice(-5);
const HOUSE_NAME = `Emberglow-${suffix}`.slice(0, 20);
const SCION_NAME = `AshaLong-${suffix}`.slice(0, 20);

const boxesOverlap = (a, b) => {
  if (!a || !b) return false;
  return a.x < b.x + b.width
    && a.x + a.width > b.x
    && a.y < b.y + b.height
    && a.y + a.height > b.y;
};

const loginChronicles = async (page) => {
  await page.goto(base, { waitUntil: 'domcontentloaded' });
  const guest = page.getByRole('button', { name: 'Play as Guest', exact: true });
  if (await guest.isVisible().catch(() => false)) {
    await guest.click();
  }
  await page.getByRole('heading', { name: 'Chronicles' }).waitFor({ timeout: 30_000 });
  const house = page.getByLabel('Found a House');
  if (await house.isVisible().catch(() => false)) {
    await house.fill(HOUSE_NAME);
    await page.getByRole('button', { name: 'Inscribe' }).click();
  }
  await page.locator('.chronicles__mortal-checkbox').check();
  await page.getByLabel('Name a new Scion').fill(SCION_NAME);
  await page.getByRole('button', { name: 'Add Scion' }).click();
  await page.getByRole('button', { name: /^Set Out as / }).click();
  await page.locator('canvas[aria-label="Game world"]').waitFor({ state: 'visible', timeout: 30_000 });
};

const captureViewport = async (browser, viewport, shotPrefix) => {
  const context = await browser.newContext({ viewport });
  const page = await context.newPage();
  const adventureZones = [];
  page.on('websocket', (ws) => {
    ws.on('framereceived', (frame) => {
      if (typeof frame.payload !== 'string' || !frame.payload.includes('adventureZones')) {
        return;
      }
      try {
        const message = JSON.parse(frame.payload);
        const zones = message?.data?.adventureZones;
        if (Array.isArray(zones) && zones.length) {
          adventureZones.push(zones);
        }
      } catch {
        // ignore non-JSON
      }
    });
  });

  await loginChronicles(page);

  const identity = page.locator('.hud-shell__identity');
  await identity.waitFor({ timeout: 15_000 });
  const identityText = (await identity.textContent() || '').trim();
  const identityTitle = (await identity.getAttribute('title')) || '';
  const expectedLabel = `House ${HOUSE_NAME} — ${SCION_NAME} (Mortal oath)`;
  const identityBox = await identity.boundingBox();
  const orbBox = await page.locator('.hud-shell__orb--left').boundingBox();

  await page.screenshot({ path: path.join(outDir, `${shotPrefix}-identity-chip.png`) });

  if (!(await page.getByLabel('Choose a zone').isVisible().catch(() => false))) {
    await page.getByRole('button', { name: 'Adventure', exact: true }).click();
  }
  const zoneMenu = page.getByLabel('Choose a zone');
  await zoneMenu.waitFor({ timeout: 10_000 });
  const payload = adventureZones.at(-1) || [];
  const barrowPayload = payload.find((zone) => zone.id === 'old-barrow') || null;
  const expectedLine = barrowPayload
    ? `${barrowPayload.bossDisplayName} · item-level ${barrowPayload.treasureItemLevel} gear · depth ${barrowPayload.depth}`
    : null;
  if (expectedLine) {
    await zoneMenu.getByText(expectedLine).waitFor({ timeout: 8_000 });
  }
  const zoneText = await zoneMenu.textContent();
  const barrow = page.locator('.game-container__zone', { hasText: 'The Old Barrow' });
  const barrowObjective = (await barrow.locator('.game-container__zone-objective').textContent() || '').trim();
  await page.screenshot({ path: path.join(outDir, `${shotPrefix}-adventure-preview.png`) });

  await context.close();

  return {
    viewport,
    identityText,
    identityTitle,
    expectedLabel,
    identityBox,
    orbBox,
    overlapsOrb: boxesOverlap(identityBox, orbBox),
    zoneText,
    barrowObjective,
    expectedLine,
    payloadCount: payload.length,
    barrowPayload,
    payloadLogged: adventureZones.length > 0,
  };
};

const run = async () => {
  const browser = await chromium.launch({ headless: true });
  const results = [];
  results.push(await captureViewport(browser, { width: 1366, height: 768 }, '01-1366x768'));
  results.push(await captureViewport(browser, { width: 1920, height: 1080 }, '02-1920x1080'));
  await browser.close();

  const checks = {};
  results.forEach((result) => {
    const key = `${result.viewport.width}x${result.viewport.height}`;
    checks[`${key}.houseIdentity`] = result.identityText.includes(`House ${HOUSE_NAME}`)
      && result.identityText.includes(SCION_NAME)
      && result.identityText.includes('Mortal oath');
    checks[`${key}.titleMatchesFullName`] = result.identityTitle === result.expectedLabel
      && result.identityTitle === result.identityText;
    checks[`${key}.chipDoesNotOverlapHpOrb`] = Boolean(result.identityBox)
      && Boolean(result.orbBox)
      && !result.overlapsOrb;
    checks[`${key}.serverPayloadPresent`] = result.payloadLogged && Boolean(result.barrowPayload);
    checks[`${key}.adventureMatchesServer`] = Boolean(result.expectedLine)
      && result.barrowObjective === result.expectedLine
      && result.zoneText.includes(result.expectedLine);
  });

  const evidence = { houseName: HOUSE_NAME, scionName: SCION_NAME, checks, results };
  fs.writeFileSync(path.join(outDir, 'capture-0055-evidence.json'), JSON.stringify(evidence, null, 2));

  const failures = Object.entries(checks).filter(([, ok]) => !ok).map(([name]) => name);
  if (failures.length) {
    throw new Error(`CAPTURE FAILED: ${failures.join(', ')}`);
  }
  console.log('CAPTURES OK', JSON.stringify(checks));
};

run().catch((error) => {
  console.error(error);
  process.exit(1);
});
