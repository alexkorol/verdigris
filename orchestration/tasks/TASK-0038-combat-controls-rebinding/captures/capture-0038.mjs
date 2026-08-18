// TASK-0038 captures: rebinding UI, persistence across reload, LMB/RMB attacks.
// Run with the game server on 127.0.0.1:6500. Saves PNGs + frame evidence here.
import { chromium } from '@playwright/test';
import { fileURLToPath } from 'node:url';
import path from 'node:path';
import fs from 'node:fs';

const base = process.env.PLAYWRIGHT_BASE_URL || 'http://127.0.0.1:6500';
const outDir = path.dirname(fileURLToPath(import.meta.url));

const login = async (page) => {
  await page.goto(base, { waitUntil: 'domcontentloaded' });
  const guest = page.getByRole('button', { name: 'Play as Guest', exact: true });
  if (await guest.isVisible().catch(() => false)) {
    await guest.click();
  }
  await page.waitForTimeout(1500);
  const chronicles = page.getByRole('heading', { name: 'Chronicles' });
  if (await chronicles.isVisible().catch(() => false)) {
    const houseName = page.getByLabel('Found a House');
    if (await houseName.isVisible().catch(() => false)) {
      await houseName.fill('Gateward');
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

const openSettingsControls = async (page) => {
  await page.keyboard.press('Escape'); // close anything stacked
  await page.waitForTimeout(250);
  const escapeMenu = page.locator('.escape-menu');
  if (await escapeMenu.isVisible().catch(() => false)) {
    await escapeMenu.getByRole('button', { name: 'Settings', exact: true }).click();
  } else {
    await page.getByRole('button', { name: 'Settings', exact: true }).first().click();
  }
  const settings = page.getByLabel('Settings overlay');
  await settings.waitFor({ timeout: 10_000 });
  const bindings = settings.locator('.bindings');
  await bindings.scrollIntoViewIfNeeded();
  await bindings.waitFor();
  return { settings, bindings };
};

const run = async () => {
  const browser = await chromium.launch();
  const context = await browser.newContext({ viewport: { width: 1280, height: 800 } });
  const page = await context.newPage();

  const skillFrames = [];
  page.on('websocket', (ws) => {
    ws.on('framesent', (frame) => {
      if (typeof frame.payload === 'string' && frame.payload.includes('player:skill:trigger')) {
        skillFrames.push({ dir: 'sent', payload: frame.payload.slice(0, 400) });
      }
    });
    ws.on('framereceived', (frame) => {
      if (typeof frame.payload === 'string' && /skill|combat/.test(frame.payload)) {
        skillFrames.push({ dir: 'recv', payload: frame.payload.slice(0, 400) });
      }
    });
  });

  await login(page);
  const canvas = page.locator('canvas[aria-label="Game world"]');
  const bounds = await canvas.boundingBox();
  if (!bounds) throw new Error('canvas has no bounds');

  // 01 — quickbar shows the live mouse bindings (LMB/RMB labels).
  await page.screenshot({ path: path.join(outDir, '01-quickbar-mouse-bindings.png') });

  // 02 — LMB world click fires the primary attack at the cursor.
  await canvas.click({ position: { x: bounds.width * 0.62, y: bounds.height * 0.45 } });
  await page.waitForTimeout(250);
  await page.screenshot({ path: path.join(outDir, '02-lmb-primary-attack.png') });

  // 03 — plain RMB world click casts the weapon skill (no context menu).
  await canvas.click({ button: 'right', position: { x: bounds.width * 0.4, y: bounds.height * 0.6 } });
  await page.waitForTimeout(300);
  const menuVisible = await page.locator('#actions').isVisible().catch(() => false);
  await page.screenshot({ path: path.join(outDir, '03-rmb-weapon-skill.png') });

  // 04 — rebinding UI inside Settings → Controls.
  const { settings, bindings } = await openSettingsControls(page);
  await page.screenshot({ path: path.join(outDir, '04-settings-controls-bindings.png') });

  // 05 — rebind Cairn Ward (ability-3) to T via the capture flow.
  const cairnRow = bindings.locator('.bindings__row', { hasText: 'Cairn Ward' });
  await cairnRow.locator('.bindings__add').click();
  await page.waitForTimeout(150);
  await page.keyboard.press('t');
  await page.waitForTimeout(250);
  const chipT = cairnRow.locator('.bindings__chip', { hasText: 'T' });
  await chipT.waitFor({ timeout: 5_000 });
  await page.screenshot({ path: path.join(outDir, '05-rebind-cairn-ward-to-T.png') });

  // 06 — the rebind survives a full page reload (localStorage persistence).
  await page.reload({ waitUntil: 'domcontentloaded' });
  await login(page);
  const reopened = await openSettingsControls(page);
  const persistedRow = reopened.bindings.locator('.bindings__row', { hasText: 'Cairn Ward' });
  await persistedRow.locator('.bindings__chip', { hasText: 'T' }).waitFor({ timeout: 5_000 });
  await page.screenshot({ path: path.join(outDir, '06-rebind-persists-after-reload.png') });

  const evidence = {
    rmbContextMenuOpened: menuVisible,
    skillFrames,
    checks: {
      primaryAttackSent: skillFrames.some((f) => f.dir === 'sent' && f.payload.includes('primary-attack')),
      weaponSkillSent: skillFrames.some((f) => f.dir === 'sent' && f.payload.includes('ability-1')),
    },
  };
  fs.writeFileSync(path.join(outDir, 'attack-frame-evidence.json'), JSON.stringify(evidence, null, 2));

  await browser.close();

  if (!evidence.checks.primaryAttackSent) throw new Error('no primary-attack skill frame captured for LMB');
  if (!evidence.checks.weaponSkillSent) throw new Error('no ability-1 skill frame captured for RMB');
  if (menuVisible) throw new Error('plain RMB still opened the context menu');
  console.log('CAPTURES OK', JSON.stringify(evidence.checks));
};

run().catch((error) => {
  console.error(error);
  process.exit(1);
});
