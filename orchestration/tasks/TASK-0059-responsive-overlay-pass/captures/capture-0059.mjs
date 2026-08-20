// TASK-0059 captures: compact overlay collisions at 1366x768 and 1280x720,
// plus an unchanged-layout check at 1920x1080. This script starts and owns
// its production server on 127.0.0.1:6582 (CAPTURE_PORT override, cursor
// range 6580-6599 only; never 6500).
//
//   node orchestration/tasks/TASK-0059-responsive-overlay-pass/captures/capture-0059.mjs
//   CAPTURE_PHASE=before  — record defects, do not hard-fail on overlap
//   SKIP_BUILD=1          — reuse existing dist/
//
// Hard-fail (default): exits non-zero unless bounding-box non-overlap
// assertions hold for the pairs this task fixed.
import { chromium } from '@playwright/test';
import { spawn } from 'node:child_process';
import { fileURLToPath } from 'node:url';
import path from 'node:path';
import fs from 'node:fs';
import http from 'node:http';

const outDir = path.dirname(fileURLToPath(import.meta.url));
const repoRoot = path.resolve(outDir, '..', '..', '..', '..');
const PHASE = process.env.CAPTURE_PHASE === 'before' ? 'before' : 'after';
const PORT = Number(process.env.CAPTURE_PORT || 6582);
const BIND_HOST = '127.0.0.1';
const prefix = PHASE === 'before' ? 'before' : 'after';

if (!Number.isInteger(PORT) || PORT < 6580 || PORT > 6599) {
  throw new Error(`CAPTURE_PORT must be in cursor range 6580-6599, got ${PORT}`);
}

const base = `http://${BIND_HOST}:${PORT}`;
const suffix = Date.now().toString(36).slice(-5);

const VIEWPORTS = [
  { width: 1366, height: 768, name: '1366x768' },
  { width: 1280, height: 720, name: '1280x720' },
  { width: 1920, height: 1080, name: '1920x1080' },
];

const boxesOverlap = (a, b, gap = 0) => {
  if (!a || !b) return false;
  return a.x < b.x + b.width - gap
    && a.x + a.width > b.x + gap
    && a.y < b.y + b.height - gap
    && a.y + a.height > b.y + gap;
};

const overflowsViewport = (box, viewport, pad = 1) => {
  if (!box) return true;
  return box.x < -pad
    || box.y < -pad
    || box.x + box.width > viewport.width + pad
    || box.y + box.height > viewport.height + pad;
};

const roundBox = (box) => {
  if (!box) return null;
  return {
    x: Math.round(box.x * 10) / 10,
    y: Math.round(box.y * 10) / 10,
    width: Math.round(box.width * 10) / 10,
    height: Math.round(box.height * 10) / 10,
  };
};

const waitForHttp = (url, timeoutMs = 60_000) => new Promise((resolve, reject) => {
  const started = Date.now();
  const attempt = () => {
    const req = http.get(url, (res) => {
      res.resume();
      if (res.statusCode && res.statusCode < 500) {
        resolve();
        return;
      }
      retry();
    });
    req.on('error', retry);
    req.setTimeout(2000, () => {
      req.destroy();
      retry();
    });
  };
  const retry = () => {
    if (Date.now() - started > timeoutMs) {
      reject(new Error(`server at ${url} did not become ready in ${timeoutMs}ms`));
      return;
    }
    setTimeout(attempt, 400);
  };
  attempt();
});

const runCommand = (command, args, env) => new Promise((resolve, reject) => {
  const child = spawn(command, args, {
    cwd: repoRoot,
    env: { ...process.env, ...env },
    stdio: ['ignore', 'pipe', 'pipe'],
    shell: process.platform === 'win32' && command !== process.execPath,
  });
  let stdout = '';
  let stderr = '';
  child.stdout.on('data', (chunk) => { stdout += chunk.toString(); });
  child.stderr.on('data', (chunk) => { stderr += chunk.toString(); });
  child.on('error', reject);
  child.on('close', (code) => {
    if (code === 0) {
      resolve({ stdout, stderr });
      return;
    }
    reject(new Error(`${command} ${args.join(' ')} exited ${code}\n${stderr}\n${stdout}`));
  });
});

const startOwnedServer = async () => {
  if (process.env.SKIP_BUILD !== '1') {
    console.log(`[capture-0059] production build (PORT ${PORT})`);
    await runCommand(
      process.platform === 'win32' ? 'npx.cmd' : 'npx',
      ['vite', 'build', '--mode', 'production'],
      { NODE_ENV: 'production' },
    );
  }
  const guestDir = path.join(process.env.TEMP || '/tmp', `verdigris-capture-${PORT}-${PHASE}`);
  const child = spawn(process.execPath, ['server/index.js'], {
    cwd: repoRoot,
    env: {
      ...process.env,
      PORT: String(PORT),
      NODE_ENV: 'production',
      VERDIGRIS_BIND_HOST: BIND_HOST,
      GUEST_SAVE_DIR: guestDir,
      CHRONICLES_DB_FILE: `${guestDir}.sqlite`,
      CHRONICLES_STORE_FILE: `${guestDir}.json`,
    },
    stdio: ['ignore', 'pipe', 'pipe'],
  });
  let output = '';
  child.stdout.on('data', (chunk) => { output += chunk.toString(); });
  child.stderr.on('data', (chunk) => { output += chunk.toString(); });
  try {
    await waitForHttp(base, 90_000);
  } catch (error) {
    stopOwnedServer(child);
    throw new Error(`${error.message}\n${output}`);
  }
  return child;
};

const stopOwnedServer = (child) => {
  if (!child || child.killed || !child.pid) {
    return;
  }
  if (process.platform === 'win32') {
    spawn('taskkill', ['/pid', String(child.pid), '/f', '/t'], { stdio: 'ignore' });
    return;
  }
  child.kill('SIGTERM');
};

const loginChronicles = async (page, names) => {
  await page.goto(base, { waitUntil: 'domcontentloaded' });
  const guest = page.getByRole('button', { name: 'Play as Guest', exact: true });
  if (await guest.isVisible().catch(() => false)) {
    await guest.click();
  }
  await page.getByRole('heading', { name: 'Chronicles' }).waitFor({ timeout: 30_000 });
  const house = page.getByLabel('Found a House');
  if (await house.isVisible().catch(() => false)) {
    await house.fill(names.house);
    await page.getByRole('button', { name: 'Inscribe' }).click();
  }
  await page.locator('.chronicles__mortal-checkbox').check();
  await page.getByLabel('Name a new Scion').fill(names.scion);
  await page.getByRole('button', { name: 'Add Scion' }).click();
  await page.getByRole('button', { name: /^Set Out as / }).click();
  await page.locator('canvas[aria-label="Game world"]').waitFor({ state: 'visible', timeout: 30_000 });
};

const boxOf = async (locator, timeoutMs = 2500) => {
  try {
    await locator.waitFor({ state: 'visible', timeout: timeoutMs });
  } catch {
    return null;
  }
  return roundBox(await locator.boundingBox());
};

const closeOpenPanes = async (page) => {
  const continueBtn = page.getByRole('button', { name: 'Continue', exact: true });
  if (await continueBtn.isVisible().catch(() => false)) {
    await continueBtn.click().catch(() => {});
    await page.waitForTimeout(200);
  }
  for (let i = 0; i < 6; i += 1) {
    const inventory = await page.getByLabel('Inventory panel').isVisible().catch(() => false);
    const settings = await page.getByLabel('Settings overlay').isVisible().catch(() => false);
    const skill = await page.getByLabel('Skill Tree overlay').isVisible().catch(() => false);
    const loot = await page.getByLabel('First find').isVisible().catch(() => false);
    const death = await page.locator('.death-overlay').isVisible().catch(() => false);
    const escapeMenu = await page.locator('.escape-menu').isVisible().catch(() => false);
    if (death) {
      await continueBtn.click().catch(() => {});
      await page.waitForTimeout(150);
      continue;
    }
    if (inventory || settings || skill || loot || escapeMenu) {
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
      window.__verdigrisOverlayCapture.setGuide(
        `Walk the road. Open Adventure when you are ready for the Old Barrow. (${Date.now()})`,
      );
    }
  });
  await page.waitForTimeout(200);
  if (!(await page.getByLabel('Choose a zone').isVisible().catch(() => false))) {
    await page.getByRole('button', { name: 'Adventure', exact: true }).click();
  }
  await page.getByLabel('Choose a zone').waitFor({ timeout: 10_000 });
  await page.locator('.guide-banner').waitFor({ timeout: 8_000 });
};

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
    hud: page.locator('.game-container__hud'),
    quickbar: page.locator('.hud-shell__quickbar'),
  };
  const boxes = {};
  for (const [name, locator] of Object.entries(locators)) {
    boxes[name] = await boxOf(locator);
  }
  return boxes;
};

const pairResult = (boxes, left, right) => ({
  left,
  right,
  overlap: boxesOverlap(boxes[left], boxes[right]),
  missing: !boxes[left] || !boxes[right],
});

const captureViewport = async (browser, viewport) => {
  const names = {
    house: `Ov${viewport.width}-${suffix}`.slice(0, 20),
    scion: `Cp${viewport.height}-${suffix}`.slice(0, 20),
  };
  const context = await browser.newContext({ viewport });
  const page = await context.newPage();
  await loginChronicles(page, names);
  await ensureGuideAndAdventure(page);

  await page.locator('.game-container__hud').waitFor({ timeout: 15_000 });
  await page.locator('.hud-shell__identity').waitFor({ timeout: 15_000 });
  const chromeBoxes = await collectChrome(page);
  const chromeShot = path.join(outDir, `${prefix}-${viewport.name}-chrome.png`);
  await page.screenshot({ path: chromeShot });

  await page.keyboard.press('i');
  const inventory = page.getByLabel('Inventory panel');
  await inventory.waitFor({ timeout: 10_000 });
  await page.evaluate(() => {
    if (window.__verdigrisOverlayCapture?.setGuide) {
      window.__verdigrisOverlayCapture.setGuide(
        'Walk the road. Open Adventure when you are ready for the Old Barrow.',
      );
    }
  });
  const inventoryBoxes = {
    ...await collectChrome(page),
    inventory: await boxOf(inventory),
  };
  const inventoryShot = path.join(outDir, `${prefix}-${viewport.name}-inventory.png`);
  await page.screenshot({ path: inventoryShot });
  await closeOpenPanes(page);

  await page.getByLabel('Game panels').getByRole('button', { name: 'Settings', exact: true }).click();
  const settings = page.getByLabel('Settings overlay');
  await settings.waitFor({ timeout: 10_000 });
  const settingsBoxes = {
    settings: await boxOf(settings),
    hud: await boxOf(page.locator('.game-container__hud')),
    party: await boxOf(page.locator('.game-container__party-overlay')),
  };
  const settingsShot = path.join(outDir, `${prefix}-${viewport.name}-settings.png`);
  await page.screenshot({ path: settingsShot });
  await closeOpenPanes(page);

  await page.keyboard.press('p');
  const skillTree = page.getByLabel('Skill Tree overlay');
  await skillTree.waitFor({ timeout: 10_000 });
  const search = page.getByPlaceholder('Search');
  const skillBoxes = {
    skillTree: await boxOf(skillTree),
    search: await boxOf(search),
  };
  const skillShot = path.join(outDir, `${prefix}-${viewport.name}-skill-tree.png`);
  await page.screenshot({ path: skillShot });
  await closeOpenPanes(page);

  await ensureGuideAndAdventure(page);
  await page.evaluate(() => window.__verdigrisOverlayCapture?.showLoot());
  const loot = page.getByLabel('First find');
  await loot.waitFor({ timeout: 10_000 });
  const lootBoxes = {
    loot: await boxOf(loot),
    guide: await boxOf(page.locator('.guide-banner')),
    hud: await boxOf(page.locator('.game-container__hud')),
  };
  const lootShot = path.join(outDir, `${prefix}-${viewport.name}-loot.png`);
  await page.screenshot({ path: lootShot });
  await closeOpenPanes(page);

  await page.evaluate(() => window.__verdigrisOverlayCapture?.showDeath());
  const death = page.locator('.death-overlay');
  await death.waitFor({ timeout: 10_000 });
  const continueBtn = page.getByRole('button', { name: 'Continue', exact: true });
  const deathBoxes = {
    death: await boxOf(death),
    panel: await boxOf(page.locator('.death-overlay__panel')),
    continue: await boxOf(continueBtn),
  };
  const deathShot = path.join(outDir, `${prefix}-${viewport.name}-death.png`);
  await page.screenshot({ path: deathShot });
  await closeOpenPanes(page);

  await context.close();

  const compact = viewport.width <= 1366;
  const pairs = compact
    ? [
      pairResult(chromeBoxes, 'guide', 'party'),
      pairResult(chromeBoxes, 'guide', 'minimap'),
      pairResult(chromeBoxes, 'zoneMenu', 'quickbar'),
      pairResult(chromeBoxes, 'zoneMenu', 'mpOrb'),
      pairResult(chromeBoxes, 'identity', 'chatPeek'),
      pairResult(chromeBoxes, 'identity', 'hpOrb'),
      pairResult(chromeBoxes, 'chatPeek', 'hpOrb'),
      pairResult(inventoryBoxes, 'inventory', 'hpOrb'),
      pairResult(inventoryBoxes, 'inventory', 'mpOrb'),
      pairResult(inventoryBoxes, 'inventory', 'quickbar'),
      pairResult(lootBoxes, 'loot', 'guide'),
    ]
    : [
      pairResult(chromeBoxes, 'identity', 'hpOrb'),
    ];

  const overflowTargets = compact
    ? {
      zoneMenu: chromeBoxes.zoneMenu,
      party: chromeBoxes.party,
      guide: chromeBoxes.guide,
      inventory: inventoryBoxes.inventory,
      settings: settingsBoxes.settings,
      skillSearch: skillBoxes.search,
      loot: lootBoxes.loot,
      deathPanel: deathBoxes.panel,
      continue: deathBoxes.continue,
    }
    : {
      zoneMenu: chromeBoxes.zoneMenu,
      guide: chromeBoxes.guide,
    };

  const overflows = {};
  for (const [name, box] of Object.entries(overflowTargets)) {
    overflows[name] = overflowsViewport(box, viewport);
  }

  return {
    viewport,
    chromeBoxes,
    inventoryBoxes,
    settingsBoxes,
    skillBoxes,
    lootBoxes,
    deathBoxes,
    pairs,
    overflows,
    shots: {
      chrome: path.basename(chromeShot),
      inventory: path.basename(inventoryShot),
      settings: path.basename(settingsShot),
      skillTree: path.basename(skillShot),
      loot: path.basename(lootShot),
      death: path.basename(deathShot),
    },
    desktopUnchanged: viewport.width >= 1920
      ? {
        guideCentered: Boolean(chromeBoxes.guide)
          && Math.abs((chromeBoxes.guide.x + chromeBoxes.guide.width / 2) - (viewport.width / 2)) < 80,
        inventoryWide: Boolean(inventoryBoxes.inventory)
          && inventoryBoxes.inventory.width >= 900,
      }
      : null,
  };
};

const run = async () => {
  const server = await startOwnedServer();
  const browser = await chromium.launch({ headless: true });
  const results = [];
  try {
    for (const viewport of VIEWPORTS) {
      console.log(`[capture-0059] ${PHASE} ${viewport.name}`);
      results.push(await captureViewport(browser, viewport));
    }
  } finally {
    await browser.close();
    stopOwnedServer(server);
  }

  const checks = {};
  results.forEach((result) => {
    const key = result.viewport.name;
    result.pairs.forEach((pair) => {
      checks[`${key}.${pair.left}-vs-${pair.right}`] = !pair.missing && !pair.overlap;
    });
    Object.entries(result.overflows).forEach(([name, overflow]) => {
      checks[`${key}.${name}-in-viewport`] = overflow === false;
    });
    if (result.skillBoxes.search) {
      checks[`${key}.skill-search-visible`] = result.skillBoxes.search.height > 8;
    }
    if (result.deathBoxes.continue) {
      checks[`${key}.death-continue-visible`] = result.deathBoxes.continue.height > 8;
    }
    if (result.desktopUnchanged) {
      checks[`${key}.guide-stays-centered`] = result.desktopUnchanged.guideCentered;
      checks[`${key}.inventory-stays-wide`] = result.desktopUnchanged.inventoryWide;
    }
    if (result.viewport.width <= 1366 && result.chromeBoxes.zoneMenu && result.chromeBoxes.party) {
      checks[`${key}.zoneMenu-fits-party-column`] = result.chromeBoxes.zoneMenu.width
        <= result.chromeBoxes.party.width + 12;
    }
    if (result.viewport.width <= 1366 && result.inventoryBoxes.inventory) {
      checks[`${key}.inventory-leaves-canvas`] = result.inventoryBoxes.inventory.width <= 700;
    }
  });

  const evidence = {
    phase: PHASE,
    port: PORT,
    bindHost: BIND_HOST,
    suffix,
    checks,
    results,
  };
  fs.writeFileSync(path.join(outDir, `capture-0059-${PHASE}-evidence.json`), JSON.stringify(evidence, null, 2));

  const failures = Object.entries(checks).filter(([, ok]) => !ok).map(([name]) => name);
  if (PHASE === 'before') {
    console.log('CAPTURE BEFORE recorded', JSON.stringify({ failures, checkCount: Object.keys(checks).length }));
    return;
  }
  if (failures.length) {
    throw new Error(`CAPTURE FAILED: ${failures.join(', ')}`);
  }
  console.log('CAPTURES OK', JSON.stringify(checks));
};

run().catch((error) => {
  console.error(error);
  process.exit(1);
});
