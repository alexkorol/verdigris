// TASK-0049 captures: first-session UI wave (house identity, directive mana
// copy, guide banner, zone objective preview, skill-tree first-allocation).
// Run against a built game server on 127.0.0.1:6500. Hard-fail: exits non-zero
// unless every on-screen check passes; saves PNGs + a checks JSON here.
import { chromium } from 'playwright';
import { fileURLToPath } from 'node:url';
import path from 'node:path';
import fs from 'node:fs';

const base = process.env.PLAYWRIGHT_BASE_URL || 'http://127.0.0.1:6500';
const outDir = path.dirname(fileURLToPath(import.meta.url));

// Unique per run so a persistent server DB never collides across re-runs.
const suffix = Date.now().toString(36).slice(-5);
const HOUSE_NAME = `Ember-${suffix}`;
const SCION_NAME = `Asha-${suffix}`;

const shot = (page, name) => page.screenshot({ path: path.join(outDir, name) });

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

const run = async () => {
  const browser = await chromium.launch({ headless: true });
  const page = await browser.newPage({ viewport: { width: 1440, height: 1000 } });
  const checks = {};

  await loginChronicles(page);

  const readMana = () => page.evaluate(() => new Promise((resolve, reject) => {
    const socket = window.ws;
    const requestId = `mana-${Date.now()}-${Math.random().toString(36).slice(2)}`;
    const timer = window.setTimeout(() => {
      socket.removeEventListener('message', onMessage);
      reject(new Error('dev:state timeout'));
    }, 5000);
    const onMessage = (event) => {
      try {
        const message = JSON.parse(event.data);
        if (message.event === 'dev:state' && message.data?.requestId === requestId) {
          window.clearTimeout(timer);
          socket.removeEventListener('message', onMessage);
          resolve(message.data.state);
        }
      } catch { /* ignore non-JSON frames */ }
    };
    socket.addEventListener('message', onMessage);
    socket.send(JSON.stringify({ event: 'dev:state', data: { requestId } }));
  }));

  // 03 — guide banner (Aldwyn speaks ~2.5s after admission, banner lasts ~9s).
  await page.waitForTimeout(3400);
  const banner = page.locator('.guide-banner');
  const bannerText = await banner.locator('.guide-banner__text').textContent().catch(() => '');
  checks.guideBanner = await banner.isVisible().catch(() => false)
    && /First things first|use W, A, S and D|Welcome to Delaford/.test(bannerText);
  await shot(page, '03-guide-banner.png');

  // 01 — house/scion identity stays visible in the world HUD.
  const identity = page.locator('.hud-shell__identity');
  const identityText = await identity.textContent().catch(() => '');
  checks.houseIdentity = identityText.includes(`House ${HOUSE_NAME}`)
    && identityText.includes(SCION_NAME)
    && identityText.includes('Mortal oath');
  await shot(page, '01-house-identity.png');

  // 04 — Adventure zone rows state the concrete draw.
  await page.getByRole('button', { name: 'Adventure', exact: true }).click();
  await page.getByLabel('Choose a zone').waitFor({ timeout: 10_000 });
  const zoneText = await page.getByLabel('Choose a zone').textContent();
  checks.zoneObjective = zoneText.includes('Warden of the Deep · item-level 10 gear · depth 1');
  await shot(page, '04-zone-objective.png');
  await page.getByRole('button', { name: 'Adventure', exact: true }).click();

  // 05 — skill tree first-allocation hint.
  await page.keyboard.press('p');
  await page.getByText('Verdigris Tree').waitFor({ timeout: 10_000 });
  const hint = page.locator('.first-allocation-hint');
  const hintText = await hint.textContent().catch(() => '');
  checks.skillTreeHint = hintText.includes('Start here')
    && /Light Step|Firm Grip|First Lesson|First Rite|First Cut|First Throw/.test(hintText);
  await shot(page, '05-skill-tree-hint.png');
  await page.keyboard.press('p');
  await page.waitForTimeout(400);

  // 02 — directive mana rejection. Drain mana through the real quickbar
  // hotkeys (3..6 = Cinder Fan/Rimebreak/Cairn Ward/Dawn Rite), then trigger
  // one more cast and assert the directive line in the chat log. Poll the
  // authoritative mana meter so the drain is deterministic under regen.
  await page.waitForTimeout(300);

  const castDrain = async () => {
    await page.keyboard.press('3'); await page.waitForTimeout(350);
    await page.keyboard.press('4'); await page.waitForTimeout(350);
    await page.keyboard.press('5'); await page.waitForTimeout(350);
    await page.keyboard.press('6'); await page.waitForTimeout(350);
  };
  await castDrain();

  let state = await readMana();
  let mana = state && state.mana ? Number(state.mana.current) : 90;
  if (!Number.isFinite(mana) || mana >= 80) {
    await castDrain(); // first volley missed; retry once
    state = await readMana();
    mana = state && state.mana ? Number(state.mana.current) : 90;
  }

  let guard = 0;
  const castAndRead = async () => {
    await page.waitForTimeout(6300);
    await page.keyboard.press('3');
    await page.waitForTimeout(600);
    state = await readMana();
    return state && state.mana ? Number(state.mana.current) : 0;
  };

  while (mana >= 12 && guard < 20) {
    mana = await castAndRead();
    guard += 1;
  }
  while (mana >= 6 && guard < 20) {
    mana = await castAndRead();
    guard += 1;
  }

  // Mana is below 6; after Cinder Fan's cooldown the next cast is rejected.
  await page.waitForTimeout(6300);
  await page.keyboard.press('3');
  await page.waitForTimeout(900);
  const chatMessages = page.locator('.chatbox__messages');
  const chatText = await chatMessages.textContent().catch(() => '');
  checks.manaDirective = /Need \d+ more mana — recovering \d+ every 2s/.test(chatText);
  await shot(page, '02-mana-directive.png');

  const evidence = { checks, bannerText, identityText, hintText, finalMana: mana, chatSnippet: chatText.slice(-400) };
  fs.writeFileSync(path.join(outDir, 'capture-0049-checks.json'), JSON.stringify(evidence, null, 2));

  await browser.close();

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
