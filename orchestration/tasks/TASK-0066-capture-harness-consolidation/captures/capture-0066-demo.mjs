// TASK-0066 demo: proves tests/e2e/lib/capture-harness.mjs by reproducing
// TASK-0059's compact-viewport (1366x768) assertion set through the shared
// helper instead of a standalone script. This does NOT touch or rewrite
// TASK-0059's own evidence — it is a fresh, independent capture against the
// current build, using the same on-screen checks 0059 defined.
//
//   CAPTURE_PORT=<your capsule port> node orchestration/tasks/TASK-0066-capture-harness-consolidation/captures/capture-0066-demo.mjs
//   SKIP_BUILD=1  — reuse existing dist/
import { chromium } from '@playwright/test';
import { fileURLToPath } from 'node:url';
import path from 'node:path';
import {
  startOwnedServer,
  stopOwnedServer,
  loginChronicles,
  boxOf,
  boxesOverlap,
  overflowsViewport,
  screenshotPath,
  runCapture,
} from '../../../../tests/e2e/lib/capture-harness.mjs';

const outDir = path.dirname(fileURLToPath(import.meta.url));
const repoRoot = path.resolve(outDir, '..', '..', '..', '..');
const PORT = Number(process.env.CAPTURE_PORT);
if (!Number.isInteger(PORT) || PORT <= 0) {
  throw new Error('CAPTURE_PORT is required (your port capsule only) — capture-harness capsule discipline');
}

const VIEWPORT = { width: 1366, height: 768, name: '1366x768' };
const suffix = Date.now().toString(36).slice(-5);

const collectChrome = async (page) => {
  const locators = {
    guide: page.locator('.guide-banner'),
    party: page.locator('.game-container__party-overlay'),
    zoneMenu: page.getByLabel('Choose a zone'),
    minimap: page.locator('.world-minimap'),
    chatPeek: page.locator('.game-container__chat-peek'),
    identity: page.locator('.hud-shell__identity'),
    hpOrb: page.locator('.hud-shell__orb--left'),
    mpOrb: page.locator('.hud-shell__orb--right'),
    quickbar: page.locator('.hud-shell__quickbar'),
  };
  const boxes = {};
  for (const [name, locator] of Object.entries(locators)) {
    boxes[name] = await boxOf(locator);
  }
  return boxes;
};

const closeOpenPanes = async (page) => {
  const continueBtn = page.getByRole('button', { name: 'Continue', exact: true });
  for (let i = 0; i < 6; i += 1) {
    const inventory = await page.getByLabel('Inventory panel').isVisible().catch(() => false);
    const skill = await page.getByLabel('Skill Tree overlay').isVisible().catch(() => false);
    const loot = await page.getByLabel('First find').isVisible().catch(() => false);
    const death = await page.locator('.death-overlay').isVisible().catch(() => false);
    if (death) {
      await continueBtn.click().catch(() => {});
      await page.waitForTimeout(150);
      continue;
    }
    if (inventory || skill || loot) {
      await page.keyboard.press('Escape');
      await page.waitForTimeout(150);
      continue;
    }
    break;
  }
};

const ensureGuideAndAdventure = async (page) => {
  await page.evaluate(() => {
    if (window.__verdigrisOverlayCapture?.setGuide) {
      window.__verdigrisOverlayCapture.setGuide(`capture-harness demo (${Date.now()})`);
    }
  });
  await page.waitForTimeout(200);
  if (!(await page.getByLabel('Choose a zone').isVisible().catch(() => false))) {
    await page.getByRole('button', { name: 'Adventure', exact: true }).click();
  }
  await page.getByLabel('Choose a zone').waitFor({ timeout: 10_000 });
  await page.locator('.guide-banner').waitFor({ timeout: 8_000 });
};

const pairResult = (boxes, left, right) => ({
  overlap: boxesOverlap(boxes[left], boxes[right]),
  missing: !boxes[left] || !boxes[right],
});

runCapture(async () => {
  const server = await startOwnedServer({ port: PORT, repoRoot, runLabel: 'task0066-demo' });
  const browser = await chromium.launch({ headless: true });
  const checks = {};
  try {
    const context = await browser.newContext({ viewport: VIEWPORT });
    const page = await context.newPage();
    await loginChronicles(page, {
      baseUrl: `http://127.0.0.1:${PORT}`,
      house: `Ch${suffix}`,
      scion: `Cs${suffix}`,
    });
    await ensureGuideAndAdventure(page);
    await page.locator('.hud-shell__identity').waitFor({ timeout: 15_000 });
    const chromeBoxes = await collectChrome(page);
    await page.screenshot({ path: screenshotPath(outDir, 'demo', VIEWPORT.name, 'chrome') });

    await page.keyboard.press('i');
    const inventory = page.getByLabel('Inventory panel');
    await inventory.waitFor({ timeout: 10_000 });
    const inventoryBoxes = { ...await collectChrome(page), inventory: await boxOf(inventory) };
    await page.screenshot({ path: screenshotPath(outDir, 'demo', VIEWPORT.name, 'inventory') });
    await closeOpenPanes(page);

    const pairs = {
      'guide-vs-party': pairResult(chromeBoxes, 'guide', 'party'),
      'guide-vs-minimap': pairResult(chromeBoxes, 'guide', 'minimap'),
      'zoneMenu-vs-quickbar': pairResult(chromeBoxes, 'zoneMenu', 'quickbar'),
      'zoneMenu-vs-mpOrb': pairResult(chromeBoxes, 'zoneMenu', 'mpOrb'),
      'identity-vs-chatPeek': pairResult(chromeBoxes, 'identity', 'chatPeek'),
      'identity-vs-hpOrb': pairResult(chromeBoxes, 'identity', 'hpOrb'),
      'chatPeek-vs-hpOrb': pairResult(chromeBoxes, 'chatPeek', 'hpOrb'),
      'inventory-vs-hpOrb': pairResult(inventoryBoxes, 'inventory', 'hpOrb'),
      'inventory-vs-mpOrb': pairResult(inventoryBoxes, 'inventory', 'mpOrb'),
      'inventory-vs-quickbar': pairResult(inventoryBoxes, 'inventory', 'quickbar'),
    };
    for (const [name, result] of Object.entries(pairs)) {
      checks[`${VIEWPORT.name}.${name}`] = !result.missing && !result.overlap;
    }

    const overflowTargets = {
      zoneMenu: chromeBoxes.zoneMenu,
      party: chromeBoxes.party,
      guide: chromeBoxes.guide,
      inventory: inventoryBoxes.inventory,
    };
    for (const [name, box] of Object.entries(overflowTargets)) {
      checks[`${VIEWPORT.name}.${name}-in-viewport`] = overflowsViewport(box, VIEWPORT) === false;
    }

    checks[`${VIEWPORT.name}.zoneMenu-fits-party-column`] = Boolean(chromeBoxes.zoneMenu)
      && Boolean(chromeBoxes.party)
      && chromeBoxes.zoneMenu.width <= chromeBoxes.party.width + 12;
    checks[`${VIEWPORT.name}.inventory-leaves-canvas`] = Boolean(inventoryBoxes.inventory)
      && inventoryBoxes.inventory.width <= 700;

    await context.close();
  } finally {
    await browser.close();
    stopOwnedServer(server);
  }
  return checks;
}, { evidencePath: path.join(outDir, 'capture-0066-demo-evidence.json') });
