// TASK-0115 browser panel + typography inventory capture driver.
//
// Starts an owned production server on a REQUIRED capsule port (this lane:
// loopback 6620-6639 only, never 6500), drives every persistent/situational
// game panel visible through tests/e2e/lib/capture-harness.mjs's shared
// Chronicles login, proves each panel visible (bounding box + computed
// typography), screenshots it at 1920x1080 and 1366x768, and records the raw
// measurements to capture-0115-evidence.json. runCapture() hard-fails (exit 1)
// unless every panel-visible check is true.
//
//   CAPTURE_PORT=6620 node orchestration/tasks/TASK-0115-browser-panel-typography-inventory/captures/capture-0115.mjs
//   SKIP_BUILD=1          reuse existing dist/
//   NEGATIVE_CONTROL=1    redirect ALL output into .tmp-negative-control/ and
//                         inject one intentionally false visibility assertion
//                         (must exit non-zero)
import { chromium } from '@playwright/test';
import { fileURLToPath } from 'node:url';
import path from 'node:path';
import fs from 'node:fs';
import {
  startOwnedServer,
  stopOwnedServer,
  loginChronicles,
  boxOf,
  screenshotPath,
  runCapture,
} from '../../../../tests/e2e/lib/capture-harness.mjs';

const taskDir = path.resolve(path.dirname(fileURLToPath(import.meta.url)), '..');
let outDir = path.join(taskDir, 'captures');
const NEGATIVE_CONTROL = process.env.NEGATIVE_CONTROL === '1';
if (NEGATIVE_CONTROL) {
  outDir = path.join(outDir, '.tmp-negative-control');
  fs.mkdirSync(outDir, { recursive: true });
}

const PORT = Number(process.env.CAPTURE_PORT);
if (!Number.isInteger(PORT) || PORT <= 0) {
  throw new Error('CAPTURE_PORT is required (lane capsule 6620-6639 only)');
}
if (PORT === 6500) {
  throw new Error('CAPTURE_PORT 6500 is the owner live server — never touch');
}
if (PORT < 6620 || PORT > 6639) {
  throw new Error(`CAPTURE_PORT must be in lane capsule 6620-6639, got ${PORT}`);
}

const VIEWPORTS = [
  { width: 1920, height: 1080, name: '1920x1080' },
  { width: 1366, height: 768, name: '1366x768' },
];

// Panel registry: id -> { label, css } where css is the DOM anchor whose
// visibility must be proven before its PNG counts and whose computed
// typography is probed. Trigger flows live in capturePanels().
const PANELS = [
  { id: 'hud-chrome', css: '.game-container__hud' },
  { id: 'world-minimap', css: '.world-minimap' },
  { id: 'chat-peek', css: '.game-container__chat-peek' },
  { id: 'panel-nav', css: '.game-container__pane-menu' },
  { id: 'guide-banner', css: '.guide-banner' },
  { id: 'party-panel', css: '.party-panel' },
  { id: 'zone-menu', css: '[aria-label="Choose a zone"]' },
  { id: 'roads-chart', css: '[aria-label="Choose a road"]' },
  { id: 'chatbox-expanded', css: '.game-container__chat-overlay:not(.game-container__chat-overlay--collapsed) .chatbox' },
  { id: 'character-pane', css: '[aria-label="Character panel"]' },
  { id: 'inventory-pane', css: '[aria-label="Inventory panel"]' },
  { id: 'quests-pane', css: '[aria-label="Quests overlay"]' },
  { id: 'skill-tree-overlay', css: '[aria-label="Skill Tree overlay"]' },
  { id: 'settings-overlay', css: '[aria-label="Settings overlay"]' },
  { id: 'logout-overlay', css: '[aria-label="Logout overlay"]' },
  { id: 'loot-moment', css: '[aria-label="First find"]' },
  { id: 'death-overlay', css: '.death-overlay__panel' },
  { id: 'escape-menu', css: '.escape-menu' },
  { id: 'context-menu', css: '#context-menu #actions' },
];

const fontOf = async (page, css) => page.evaluate((selector) => {
  const el = document.querySelector(selector);
  if (!el) return null;
  const cs = window.getComputedStyle(el);
  return {
    fontFamily: cs.fontFamily,
    fontSize: cs.fontSize,
    fontWeight: cs.fontWeight,
    color: cs.color,
  };
}, css);

const shotAndMeasure = async (page, viewportName, panel) => {
  const locator = page.locator(panel.css);
  const box = await boxOf(locator, 6000);
  const shot = screenshotPath(outDir, 'panel', viewportName, panel.id);
  await page.screenshot({ path: shot });
  const font = await fontOf(page, panel.css);
  return { box, font, shot: path.basename(shot), visible: Boolean(box) };
};

// 'Game panels' nav hosts Quests/Settings/Exit; 'World actions' hosts
// Party/Adventure/Roads (GameContainer.vue:58-104).
const panelNavButton = (page, options) => page
  .getByLabel('Game panels')
  .getByRole('button', options);
const worldNavButton = (page, options) => page
  .getByLabel('World actions')
  .getByRole('button', options);

const capturePanels = async (page, viewportName, results, checks) => {
  const record = async (panelId) => {
    const panel = PANELS.find((p) => p.id === panelId);
    results[viewportName][panelId] = await shotAndMeasure(page, viewportName, panel);
    checks[`${viewportName}.${panelId}-visible`] = results[viewportName][panelId].visible;
  };

  // --- persistent chrome (nothing opened yet) ---
  await record('hud-chrome');
  await record('world-minimap');
  await record('chat-peek');
  await record('panel-nav');

  // guide banner via the production capture hook (GameContainer.vue:780)
  await page.evaluate(() => window.__verdigrisOverlayCapture?.setGuide(
    'Walk the road. Open Adventure when you are ready for the Old Barrow.',
  ));
  await record('guide-banner');

  // party panel toggle
  await worldNavButton(page, { name: 'Party', exact: true }).click();
  await record('party-panel');
  await worldNavButton(page, { name: 'Party', exact: true }).click();

  // expedition zone menu
  await worldNavButton(page, { name: 'Adventure', exact: true }).click();
  await record('zone-menu');
  await worldNavButton(page, { name: 'Adventure', exact: true }).click();

  // roads chart menu
  await worldNavButton(page, { name: 'Roads', exact: true }).click();
  await record('roads-chart');
  await worldNavButton(page, { name: 'Roads', exact: true }).click();

  // expanded chatbox via '/' (Delaford.vue:1291)
  await page.keyboard.press('/');
  await record('chatbox-expanded');
  await page.keyboard.press('Escape');

  // docked side panes: hotkeys c / i (Delaford.vue:1271); same key toggles off
  await page.keyboard.press('c');
  await record('character-pane');
  await page.keyboard.press('c');

  await page.keyboard.press('i');
  await record('inventory-pane');
  await page.keyboard.press('i');

  // overlay panes: j / p hotkeys, Settings + Exit nav buttons
  await page.keyboard.press('j');
  await record('quests-pane');
  await page.keyboard.press('Escape');

  await page.keyboard.press('p');
  await record('skill-tree-overlay');
  await page.keyboard.press('Escape');

  await panelNavButton(page, { name: 'Settings', exact: true }).click();
  await record('settings-overlay');
  await page.keyboard.press('Escape');

  await panelNavButton(page, { name: 'Exit', exact: true }).click();
  await record('logout-overlay');
  await page.keyboard.press('Escape');

  // first-find loot moment via production hook (auto-dismisses after 7s)
  await page.evaluate(() => window.__verdigrisOverlayCapture?.showLoot());
  await record('loot-moment');
  await page.keyboard.press('Escape');

  // death overlay via production hook; dismissed with Continue
  await page.evaluate(() => window.__verdigrisOverlayCapture?.showDeath());
  await record('death-overlay');
  const cont = page.getByRole('button', { name: 'Continue', exact: true });
  if (await cont.isVisible().catch(() => false)) {
    await cont.click();
  }

  // escape menu: only opens when every other pane/chat layer is closed
  await page.keyboard.press('Escape');
  await record('escape-menu');
  await page.keyboard.press('Escape');

  // context menu: real server round-trip. Plain RMB is bound to ability-1
  // (controls.js D-007); Shift+RMB is the menu trigger, dispatched here as
  // the same trusted-shape DOM MouseEvent the browser produces. The menu
  // only opens when the server answers with >=1 action ("Walk here" always
  // applies), so scan a spot grid until #actions mounts.
  const canvas = page.locator('canvas[aria-label="Game world"]');
  const cb = await canvas.boundingBox();
  let ctxSeen = false;
  if (cb) {
    const cx = cb.x + cb.width / 2;
    const cy = cb.y + cb.height / 2;
    const spots = [
      { x: cx, y: cy },
      { x: cx - cb.width * 0.25, y: cy - cb.height * 0.2 },
      { x: cx + cb.width * 0.25, y: cy - cb.height * 0.2 },
      { x: cx - cb.width * 0.25, y: cy + cb.height * 0.2 },
      { x: cx + cb.width * 0.25, y: cy + cb.height * 0.2 },
      { x: cb.x + 130, y: cb.y + 80 },
      { x: cb.x + cb.width - 130, y: cb.y + 80 },
      { x: cb.x + 130, y: cb.y + cb.height - 120 },
      { x: cb.x + cb.width - 130, y: cb.y + cb.height - 120 },
    ];
    for (const spot of spots) {
      await page.evaluate(([evX, evY]) => {
        const el = document.querySelector('#game-map');
        if (!el) return;
        el.dispatchEvent(new MouseEvent('contextmenu', {
          bubbles: true,
          cancelable: true,
          clientX: evX,
          clientY: evY,
          button: 2,
          shiftKey: true,
        }));
      }, [spot.x, spot.y]);
      const box = await boxOf(page.locator('#context-menu #actions'), 2000);
      if (box) {
        ctxSeen = true;
        console.log(`[capture-0115] context menu hit at ${Math.round(spot.x)},${Math.round(spot.y)}`);
        break;
      }
    }
  }
  await record('context-menu');
  if (!ctxSeen) {
    checks[`${viewportName}.context-menu-visible`] = false;
  } else {
    await page.keyboard.press('c'); // opens Character pane; openPane emits contextmenu:close
    await page.keyboard.press('c'); // close it again
  }
};

const run = () => runCapture(async () => {
  const server = await startOwnedServer({ port: PORT });
  const browser = await chromium.launch({ headless: true });
  const suffix = Date.now().toString(36).slice(-5);
  const results = {};
  const checks = {};
  try {
    for (const viewport of VIEWPORTS) {
      const name = viewport.name;
      results[name] = {};
      console.log(`[capture-0115] ${name}`);
      const context = await browser.newContext({ viewport });
      const page = await context.newPage();
      await loginChronicles(page, {
        baseUrl: `http://127.0.0.1:${PORT}`,
        house: `Panel${suffix}`.slice(0, 20),
        scion: `Inv${suffix}`.slice(0, 20),
      });
      await capturePanels(page, name, results, checks);
      await context.close();
    }
  } finally {
    await browser.close();
    stopOwnedServer(server);
  }

  if (NEGATIVE_CONTROL) {
    checks['negative-control.false-visibility-inventory'] = false;
  }

  fs.writeFileSync(
    path.join(outDir, 'capture-0115-evidence.json'),
    JSON.stringify({
      task: 'TASK-0115',
      port: PORT,
      bindHost: '127.0.0.1',
      negativeControl: NEGATIVE_CONTROL,
      viewports: VIEWPORTS.map((v) => v.name),
      panels: PANELS.map((p) => p.id),
      results,
    }, null, 2),
  );

  return checks;
}, { evidencePath: path.join(outDir, 'capture-0115-checks.json') });

run();
