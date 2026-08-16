import assert from 'node:assert/strict';
import {chromium} from 'playwright';

const WAIT = 30_000;

function trackedPage(context, errors) {
  const page = context.newPage();
  return page.then(result => {
    result.on('pageerror', error => errors.push(`pageerror: ${error.message}`));
    result.on('console', message => {
      if (message.type() === 'error') errors.push(`console: ${message.text()}`);
    });
    return result;
  });
}

async function waitForBoot(page) {
  await page.waitForSelector('#btnnew', {state: 'visible', timeout: WAIT});
  await page.waitForFunction(() => Boolean(window.__V && window.__V.state === 'title'), null, {timeout: WAIT});
}

async function begin(page, direction) {
  await page.locator('#btnnew').click();
  await page.locator(`.choice[data-dir="${direction}"]`).click();
  await page.waitForFunction(() => window.__V?.state === 'instance', null, {timeout: WAIT});
}

async function tick(page, seconds) {
  await page.evaluate(duration => {
    const count = Math.ceil(duration * 60);
    for (let i = 0; i < count; i += 1) window.__V.tick(1 / 60);
  }, seconds);
}

async function openDebug(page) {
  const panel = page.locator('#debug');
  if (await panel.evaluate(element => getComputedStyle(element).display !== 'block')) {
    await page.keyboard.press('`');
  }
  await panel.waitFor({state: 'visible'});
}

async function clearWaves(page) {
  await openDebug(page);
  // Clearing is a debug-panel operation; keep the harness Scion alive while
  // real-time enemy telegraphs run between the scheduled waves.
  await page.evaluate(() => { if (window.__V.player) window.__V.player.life = 1_000_000; });
  const clear = page.getByRole('button', {name: 'clear node'});
  for (let attempt = 0; attempt < 8; attempt += 1) {
    const overlay = await page.locator('#overlay').evaluate(element => ({display: getComputedStyle(element).display, text: element.textContent.slice(0, 180)}));
    if (overlay.display !== 'none') throw new Error(`overlay appeared while clearing wave ${attempt}: ${JSON.stringify(overlay)}`);
    await clear.click();
    await tick(page, 0.2);
    // A killed enemy may drop loot beside the Scion, masking the standard
    // prompt. Move to the known standard location and try the interaction;
    // pressing twice covers the case where the first press picks up a drop.
    await page.evaluate(() => { window.__V.player.x = 0; window.__V.player.y = -720; });
    await tick(page, 0.1);
    await page.keyboard.press('e');
    await tick(page, 0.1);
    await page.keyboard.press('e');
    if (await page.locator('#btnmap, #housename').count()) return;
    // checkWave schedules the next wave with a real-time timeout.
    await page.waitForTimeout(1_050);
  }
  const diagnostic = await page.evaluate(() => ({
    state: window.__V.state,
    alive: window.__V.entities.filter(entity => entity.alive).length,
    entities: window.__V.entities.length,
    prompt: document.querySelector('#prompt')?.textContent,
    debug: getComputedStyle(document.querySelector('#debug')).display,
  }));
  throw new Error(`clear node did not reach the standard prompt: ${JSON.stringify(diagnostic)}`);
}

async function plantStandard(page) {
  await page.evaluate(() => {
    window.__V.player.x = 0;
    window.__V.player.y = -720;
  });
  await page.keyboard.press('e');
}

function result(name, errors, detail = '') {
  return {name, ok: errors.length === 0, detail: errors.length ? errors.join(' | ') : detail};
}

async function loadCheck(baseUrl) {
  const errors = [];
  const browser = await chromium.launch({headless: true});
  try {
    const context = await browser.newContext();
    const page = await trackedPage(context, errors);
    await page.goto(baseUrl, {waitUntil: 'domcontentloaded'});
    await waitForBoot(page);
    await page.waitForTimeout(2_000);
    return result('load: boot with no console errors', errors);
  } finally {
    await browser.close();
  }
}

async function directionCheck(baseUrl) {
  const errors = [];
  const stats = {};
  const browser = await chromium.launch({headless: true});
  try {
    for (const direction of ['str', 'dex', 'int']) {
      const context = await browser.newContext();
      const page = await trackedPage(context, errors);
      await page.goto(baseUrl, {waitUntil: 'domcontentloaded'});
      await waitForBoot(page);
      await begin(page, direction);
      stats[direction] = await page.evaluate(() => ({str: window.__V.scion.str, dex: window.__V.scion.dex, int: window.__V.scion.int}));
      await context.close();
    }
    assert.notDeepEqual(stats.str, stats.dex, 'strength and dexterity starts should differ');
    assert.notDeepEqual(stats.dex, stats.int, 'dexterity and intelligence starts should differ');
    assert.notDeepEqual(stats.str, stats.int, 'strength and intelligence starts should differ');
    return result('fresh-house arc: all three directions have distinct stats', errors, JSON.stringify(stats));
  } finally {
    await browser.close();
  }
}

async function fullLoopCheck(baseUrl) {
  const errors = [];
  const browser = await chromium.launch({headless: true});
  try {
    const context = await browser.newContext();
    const page = await trackedPage(context, errors);
    await page.goto(baseUrl, {waitUntil: 'domcontentloaded'});
    await waitForBoot(page);
    await begin(page, 'str');

    const pickup = await page.evaluate(() => {
      window.__V.player.x = 120;
      window.__V.player.y = 280;
      return {before: Boolean(window.__V.player.weapon), name: window.__V.scion.name};
    });
    await page.keyboard.press('e');
    await page.waitForFunction(() => Boolean(window.__V.player?.weapon), null, {timeout: WAIT});
    assert.equal(pickup.before, false, 'intro should begin without an equipped weapon');

    await tick(page, 7.1);
    await page.waitForTimeout(2_000);
    await page.waitForFunction(() => window.__V.entities.some(entity => entity.alive), null, {timeout: WAIT});
    const crisisSpawned = await page.evaluate(() => window.__V.entities.filter(entity => entity.alive).length);
    assert.ok(crisisSpawned > 0, 'crisis must spawn enemies');

    // Prove the primary attack is a real LMB action against an exposed enemy.
    const melee = await page.evaluate(() => {
      const p = window.__V.player;
      const enemy = window.__V.spawnEnemy('raider', 1, p.x + 70, p.y);
      enemy.speed = 0;
      enemy.moveSpeed = 0;
      enemy.cooldown = 99;
      enemy.life = 100;
      return {x: enemy.x, y: enemy.y, before: enemy.life};
    });
    const viewport = await page.evaluate(() => ({width: innerWidth, height: innerHeight, cam: {...window.__V.cam}, player: {...window.__V.player}}));
    const relY = melee.y - viewport.cam.y;
    const depth = Math.max(.55, Math.min(1.9, 1 + relY * viewport.cam.persp));
    const sx = viewport.width / 2 + (melee.x - viewport.cam.x) * viewport.cam.zoom * depth;
    const sy = viewport.height * viewport.cam.anchor + relY * viewport.cam.zoom * depth * Math.cos(viewport.cam.pitch * Math.PI / 180);
    await page.mouse.move(sx, sy);
    await page.mouse.down();
    await tick(page, 1.2);
    await page.mouse.up();
    const meleeAfter = await page.evaluate(() => window.__V.entities.find(entity => entity.x > window.__V.player.x + 40 && entity.alive)?.life ?? 0);
    assert.ok(meleeAfter < melee.before, `LMB melee should reduce enemy life (${melee.before} -> ${meleeAfter})`);

    await clearWaves(page);
    await plantStandard(page);
    await page.waitForSelector('#btnmap', {state: 'visible', timeout: WAIT});
    const hearthstead = await page.evaluate(() => ({
      cleared: window.__V.house.cleared.includes('hearthstead'),
      unlocked: window.__V.house.unlocked.includes('burning-fields'),
      standing: window.__V.house.standing,
    }));
    assert.deepEqual(hearthstead, {cleared: true, unlocked: true, standing: 10});

    // A carried weapon becomes a relic when raiders kill the Scion.
    const carriedId = await page.evaluate(() => {
      window.__V.enterNode('burning-fields');
      const p = window.__V.player;
      p.life = 1;
      const ids = [];
      for (let i = 0; i < 6; i += 1) {
        const enemy = window.__V.spawnEnemy('raider', 1, p.x + 50 + i * 3, p.y);
        enemy.speed = 0;
        enemy.moveSpeed = 0;
        ids.push(enemy.id);
      }
      return window.__V.scion.weapon.id;
    });
    await tick(page, 2.2);
    await page.waitForSelector('#btnsucc', {state: 'visible', timeout: WAIT});
    const death = await page.evaluate(id => ({
      relic: window.__V.house.relicPool.some(item => item.id === id),
      lineage: window.__V.house.lineage.length,
    }), carriedId);
    assert.equal(death.relic, true, 'the carried weapon should enter the relic pool');
    assert.equal(death.lineage, 1, 'death should append one lineage record');

    await page.locator('#btnsucc').click();
    await page.locator('.choice[data-dir="dex"]').click();
    await page.waitForFunction(() => window.__V.state === 'map', null, {timeout: WAIT});
    const successor = await page.evaluate(() => ({level: window.__V.scion.level, pack: window.__V.scion.pack.length}));
    assert.deepEqual(successor, {level: 1, pack: 0});

    await page.evaluate(() => window.__V.enterNode('wardens-circle'));
    await clearWaves(page);
    await plantStandard(page);
    await page.waitForSelector('#housename', {state: 'visible', timeout: WAIT});
    await page.locator('#housename').fill('Cinderwatch');
    await page.locator('#btnfound').click();
    await page.waitForSelector('#btnmap2', {state: 'visible', timeout: WAIT});
    const founded = await page.evaluate(() => ({founded: window.__V.house.founded, name: window.__V.house.name}));
    assert.deepEqual(founded, {founded: true, name: 'Cinderwatch'});
    await page.locator('#btnmap2').click();
    await page.waitForFunction(() => window.__V.state === 'map', null, {timeout: WAIT});
    await page.reload({waitUntil: 'domcontentloaded'});
    await waitForBoot(page);
    await page.waitForSelector('#btncontinue', {state: 'visible', timeout: WAIT});
    const continueText = await page.locator('#btncontinue').textContent();
    assert.match(continueText, /Cinderwatch/, 'house name must persist across reload');
    return result('full loop: equip → crisis → LMB combat → clear → death/relic → successor → founding', errors, `relic ${carriedId}; ${continueText.trim()}`);
  } finally {
    await browser.close();
  }
}

export async function runSliceChecks(baseUrl) {
  const checks = [];
  for (const check of [loadCheck, directionCheck, fullLoopCheck]) {
    try {
      checks.push(await check(baseUrl));
    } catch (error) {
      checks.push({name: check.name.replace(/Check$/, '').replace(/([a-z])([A-Z])/g, '$1 $2'), ok: false, detail: error.stack || error.message});
    }
  }
  return checks;
}
