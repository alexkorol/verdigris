// TASK-0034 Arc A — guest quickstart (?play) session.
import {
  BASE, chatDiff, chatText, clickMenuAction, closeMenu, createSession,
  flushConsole, hudSummary, logBeat, logLine, minimapText, myPlayer,
  players, shot, walk, walkToward, canvasRightClick, worldItems, ts,
} from './_driver.mjs';

const session = await createSession();
const { page, browser, consoleLog } = session;

const die = (msg) => { logLine(`\n**DRIVER NOTE (${ts()}): ${msg}**`); };

try {
  logLine(`\n\n# Arc A — guest quickstart (?play) — started ${ts()}`);
  logLine('Client: headless Chromium (Playwright 1.56.1), 1440x1000 viewport. Server: 127.0.0.1:9777 dev build.');

  // ---- Beat 1: landing page as a stranger -------------------------------
  await page.goto(BASE, { waitUntil: 'domcontentloaded' });
  await page.waitForTimeout(2500);
  let s1 = await shot(page, 'a-landing-page');
  const landingText = await page.evaluate(() => {
    const el = document.querySelector('.login-form') || document.body;
    return el.innerText.trim().slice(0, 800);
  });
  const landingBits = await page.evaluate(() => ({
    heading: !!document.querySelector('h1'),
    backdropCanvas: !!document.querySelector('.login-backdrop__canvas'),
    guestBtn: !!document.querySelector('[aria-label="Play as Guest"]'),
    accountToggle: !!document.querySelector('[class*=account-toggle]'),
  }));
  logBeat('Arc A beat 1 — landing page (plain URL, logged out)', [
    `- Did: loaded ${BASE}/ and looked at what a stranger sees.`,
    `- Page text: ${JSON.stringify(landingText)}`,
    `- Elements: ${JSON.stringify(landingBits)}`,
    `- Capture: ${s1}`,
  ]);

  // ---- Beat 2: ?play quickstart ------------------------------------------
  await page.goto(`${BASE}/?play`, { waitUntil: 'domcontentloaded' });
  const canvas = page.locator('canvas[aria-label="Game world"]');
  let arrived = true;
  try {
    await canvas.waitFor({ state: 'visible', timeout: 25000 });
  } catch {
    arrived = false;
  }
  await page.waitForTimeout(2500);
  const s2 = await shot(page, 'a-quickstart-arrival');
  const chat0 = await chatText(page);
  const mini0 = await minimapText(page);
  const pos0 = await myPlayer();
  logBeat('Arc A beat 2 — ?play one-URL quickstart', [
    `- Did: navigated to /?play (one-URL quickstart hook in Login.vue).`,
    `- Happened: game canvas visible = ${arrived}. Spawn state: ${JSON.stringify(pos0)}`,
    `- Minimap says: ${JSON.stringify(mini0)}`,
    `- Chat so far: ${JSON.stringify(chat0)}`,
    `- Capture: ${s2}`,
  ]);

  // ---- Beat 3: first minute — what is on screen, what tells me what to do
  await page.waitForTimeout(4000);
  const s3 = await shot(page, 'a-first-minute-hud');
  const overlays = await page.evaluate(() => {
    const texts = [];
    document.querySelectorAll('[aria-label$="overlay"], [aria-label$="panel"], .escape-menu, [class*=tutorial], [class*=hint], [class*=toast]').forEach((el) => {
      if (el.offsetParent) texts.push(`${el.getAttribute('aria-label') || el.className}: ${el.innerText.trim().slice(0, 200)}`);
    });
    return texts;
  });
  const hud = await hudSummary(page);
  logBeat('Arc A beat 3 — first minute orientation', [
    '- Did: nothing for ~5s, just read the screen like a new player.',
    `- Visible overlays/panels/hints: ${JSON.stringify(overlays)}`,
    `- Quickbar: ${JSON.stringify(hud.quickbar)}`,
    `- Minimap: ${JSON.stringify(hud.minimap)}`,
    `- Capture: ${s3}`,
  ]);

  // ---- Beat 4: move around town ------------------------------------------
  const before = await myPlayer();
  await page.locator('canvas[aria-label="Game world"]').click({ position: { x: 400, y: 400 } }); // focus
  for (const key of ['KeyD', 'KeyS', 'KeyA', 'KeyW']) await walk(page, key, 600);
  const after = await myPlayer();
  const s4 = await shot(page, 'a-move-around-town');
  logBeat('Arc A beat 4 — move around town (WASD)', [
    '- Did: clicked canvas to focus, held D/S/A/W ~0.6s each.',
    `- Happened: pos ${JSON.stringify(before && { x: before.x, y: before.y })} -> ${JSON.stringify(after && { x: after.x, y: after.y, scene: after.scene || after.sceneId })}`,
    `- Capture: ${s4}`,
  ]);

  // ---- Beat 5: talk to Aldwyn the Guide (quest?) --------------------------
  // Aldwyn stands at (34,116); spawn is (42,115). Walk west to x<=35.
  const chatBeforeAldwyn = await chatText(page);
  const walkWest = await walkToward(page, 'KeyA', p => p.x <= 35, { maxSteps: 20 });
  logBeat('Arc A beat 5a — walk toward Aldwyn the Guide', [
    `- Did: held A until x<=35. Result: ${JSON.stringify(walkWest)}`,
  ]);
  // Scan canvas points near center-left for a menu containing "Talk".
  let talked = false; let talkMenu = null; let scanNotes = [];
  const scanPoints = [
    [0.5, 0.5], [0.42, 0.5], [0.58, 0.5], [0.35, 0.5], [0.65, 0.5],
    [0.42, 0.42], [0.58, 0.42], [0.42, 0.58], [0.58, 0.58], [0.3, 0.45], [0.7, 0.45],
  ];
  for (const [fx, fy] of scanPoints) {
    const menu = await canvasRightClick(page, fx, fy);
    if (menu.open) {
      scanNotes.push(`(${fx},${fy}): ${menu.actions.join(' | ')}`);
      if (menu.actions.some(a => /^Talk/.test(a))) {
        talkMenu = menu;
        const s5a = await shot(page, 'a-aldwyn-context-menu');
        scanNotes.push(`menu capture: ${s5a}`);
        talked = await clickMenuAction(page, /^Talk/);
        break;
      }
      await closeMenu(page);
    }
  }
  await page.waitForTimeout(1200);
  const chatAfterAldwyn = await chatText(page);
  const aldwynSaid = chatDiff(chatBeforeAldwyn, chatAfterAldwyn);
  const s5 = await shot(page, 'a-after-aldwyn-talk');
  logBeat('Arc A beat 5b — talk to Aldwyn (find a goal)', [
    `- Did: right-clicked around screen looking for a Talk action. Menus seen: ${scanNotes.join(' ;; ') || 'none opened'}`,
    `- Happened: Talk clicked = ${talked}. Aldwyn said: ${JSON.stringify(aldwynSaid)}`,
    `- Capture: ${s5}`,
  ]);

  // ---- Beat 6: leave town via north gate, find a wolf, fight #1 ----------
  // Gate 'town-north-old-wood' at (38,94). Align x to 37..39 then walk north.
  const p6 = await myPlayer();
  if (p6 && p6.x > 39) await walkToward(page, 'KeyA', p => p.x <= 38, { maxSteps: 20 });
  else if (p6 && p6.x < 37) await walkToward(page, 'KeyD', p => p.x >= 38, { maxSteps: 20 });
  const toGate = await walkToward(page, 'KeyW', p => p.y <= 93 || (p.scene && String(p.scene).includes('old-wood')) || (p.map && String(p.map).includes('old')), { maxSteps: 30 });
  await page.waitForTimeout(1500);
  const p6b = await myPlayer();
  const miniGate = await minimapText(page);
  const s6 = await shot(page, 'a-old-wood-entry');
  logBeat('Arc A beat 6a — north gate zone transition to Old Wood', [
    `- Did: walked north through the gate at (38,94).`,
    `- Happened: pos now ${JSON.stringify(p6b && { x: p6b.x, y: p6b.y })}; minimap: ${JSON.stringify(miniGate)}`,
    `- Chat: ${JSON.stringify(chatDiff(chatAfterAldwyn, await chatText(page)))}`,
    `- Capture: ${s6}`,
  ]);

  // Wolves spawn around (100,170), entry at (100,176): walk north until aggro.
  const chatBeforeFight = await chatText(page);
  await walkToward(page, 'KeyW', p => p.y <= 172, { maxSteps: 15 });
  // Combat loop: spam primary + Q, watch chat for kills/damage.
  let fightLog = ''; let kills = 0; let fightShots = [];
  const fightStart = Date.now();
  let midShotDone = false;
  while (Date.now() - fightStart < 75000 && kills < 2) {
    await page.keyboard.press('Space');
    await page.waitForTimeout(700);
    await page.keyboard.press('KeyQ');
    await page.waitForTimeout(700);
    if (!midShotDone && Date.now() - fightStart > 6000) {
      fightShots.push(await shot(page, 'a-fight-wolf-mid'));
      midShotDone = true;
    }
    const now = await chatText(page);
    fightLog = chatDiff(chatBeforeFight, now);
    kills = (fightLog.match(/died\./g) || []).length;
    if (kills >= 1 && Date.now() - fightStart > 15000) break;
  }
  fightShots.push(await shot(page, 'a-fight-wolf-end'));
  logBeat('Arc A beat 6b — fight #1 (Old Wood Wolf)', [
    '- Did: walked into wolf spawn, spammed Space (primary) and Q.',
    `- Combat chat log: ${JSON.stringify(fightLog)}`,
    `- Kills observed: ${kills}. Duration so far: ${Math.round((Date.now() - fightStart) / 1000)}s`,
    `- Captures: ${fightShots.join(', ')}`,
  ]);

  // ---- Beat 7: loot the drop ----------------------------------------------
  const itemsBefore = await worldItems();
  let tookSomething = false; let takeScans = [];
  const lootPoints = [
    [0.5, 0.5], [0.53, 0.5], [0.47, 0.5], [0.5, 0.53], [0.5, 0.47],
    [0.56, 0.5], [0.44, 0.5], [0.5, 0.56], [0.5, 0.44], [0.56, 0.56], [0.44, 0.44],
  ];
  for (const [fx, fy] of lootPoints) {
    const menu = await canvasRightClick(page, fx, fy);
    if (menu.open) {
      takeScans.push(`(${fx},${fy}): ${menu.actions.join(' | ')}`);
      if (menu.actions.some(a => /^Take/.test(a))) {
        const s7a = await shot(page, 'a-take-context-menu');
        takeScans.push(`menu capture: ${s7a}`);
        tookSomething = await clickMenuAction(page, /^Take/);
        await page.waitForTimeout(800);
        break;
      }
      await closeMenu(page);
    }
  }
  const itemsAfter = await worldItems();
  const s7 = await shot(page, 'a-after-loot');
  logBeat('Arc A beat 7 — loot a drop', [
    `- Did: right-click scanned tiles around the kill for a Take action. Menus seen: ${takeScans.join(' ;; ') || 'none'}`,
    `- Happened: Take clicked = ${tookSomething}. Ground items before=${itemsBefore.length} after=${itemsAfter.length}`,
    `- Capture: ${s7}`,
  ]);

  // ---- Beat 8: inventory + equip ------------------------------------------
  await page.keyboard.press('KeyI');
  await page.waitForTimeout(700);
  const inv = page.getByLabel('Inventory panel');
  const invVisible = await inv.isVisible().catch(() => false);
  const s8 = await shot(page, 'a-inventory-open');
  let invItems = [];
  let equipMenu = [];
  if (invVisible) {
    invItems = await inv.locator('.inventory-item').evaluateAll(els => els.map(e => e.getAttribute('aria-label')));
    const firstItem = inv.locator('.inventory-item').first();
    if (await firstItem.count()) {
      await firstItem.click({ button: 'right' });
      await page.waitForTimeout(500);
      const menu = page.locator('#actions');
      if (await menu.isVisible()) {
        equipMenu = (await menu.locator('.action').allTextContents()).map(a => a.trim());
        const s8b = await shot(page, 'a-inventory-item-menu');
        equipMenu.push(`capture: ${s8b}`);
        await clickMenuAction(page, /^Equip/);
        await page.waitForTimeout(500);
      }
    }
  }
  const s8c = await shot(page, 'a-after-equip');
  logBeat('Arc A beat 8 — open inventory, equip something', [
    `- Did: pressed I. Inventory visible=${invVisible}. Items: ${JSON.stringify(invItems)}`,
    `- Right-click first item menu: ${JSON.stringify(equipMenu)}`,
    `- Captures: ${s8}, ${s8c}`,
  ]);
  await page.keyboard.press('Escape');

  // ---- Beat 9: look for a goal (quests panel, minimap) --------------------
  await page.keyboard.press('KeyJ');
  await page.waitForTimeout(700);
  const questPanel = page.getByLabel(/quest/i);
  let questText = '';
  if (await questPanel.count()) questText = (await questPanel.first().innerText().catch(() => '')).slice(0, 500);
  const s9 = await shot(page, 'a-quests-panel');
  await page.keyboard.press('Escape');
  logBeat('Arc A beat 9 — look for a goal (J = quests)', [
    `- Did: pressed J. Quest panel text: ${JSON.stringify(questText)}`,
    `- Capture: ${s9}`,
  ]);

  // ---- Beat 10: Adventure button zone transition ---------------------------
  const advBtn = page.getByRole('button', { name: 'Adventure', exact: true });
  let zoneFlow = [];
  if (await advBtn.isVisible().catch(() => false)) {
    await advBtn.click();
    await page.waitForTimeout(800);
    const zoneMenu = page.getByLabel('Choose a zone');
    if (await zoneMenu.isVisible().catch(() => false)) {
      zoneFlow.push(`zone menu options: ${JSON.stringify(await zoneMenu.innerText())}`);
      const s10a = await shot(page, 'a-zone-menu');
      zoneFlow.push(`menu capture: ${s10a}`);
      await zoneMenu.getByRole('button', { name: /Verdant Grove/ }).click().catch((e) => zoneFlow.push(`click failed: ${e.message}`));
      await page.waitForTimeout(2500);
    } else {
      zoneFlow.push('no "Choose a zone" menu appeared');
    }
  } else {
    zoneFlow.push('no Adventure button visible');
  }
  const miniAfterZone = await minimapText(page);
  const s10 = await shot(page, 'a-after-zone-transition');
  logBeat('Arc A beat 10 — zone transition via Adventure button', [
    `- Did: clicked Adventure. ${zoneFlow.join(' ')}`,
  `- Minimap now: ${JSON.stringify(miniAfterZone)}`,
    `- Capture: ${s10}`,
  ]);

  // ---- Beat 11: fight #2 in the new zone ----------------------------------
  const chatBeforeFight2 = await chatText(page);
  // Wander to find a monster.
  for (const key of ['KeyW', 'KeyA', 'KeyS', 'KeyD', 'KeyW', 'KeyW']) {
    await walk(page, key, 900);
    const c = await chatText(page);
    if (/hit|damage|died|attack/i.test(chatDiff(chatBeforeFight2, c))) break;
  }
  const s11a = await shot(page, 'a-fight2-contact');
  let fight2Log = ''; let kills2 = 0;
  const f2Start = Date.now();
  while (Date.now() - f2Start < 60000 && kills2 < 1) {
    await page.keyboard.press('Space');
    await page.waitForTimeout(600);
    await page.keyboard.press('KeyE');
    await page.waitForTimeout(600);
    await page.keyboard.press('KeyQ');
    await page.waitForTimeout(600);
    fight2Log = chatDiff(chatBeforeFight2, await chatText(page));
    kills2 = (fight2Log.match(/died\./g) || []).length;
  }
  const s11 = await shot(page, 'a-fight2-end');
  logBeat('Arc A beat 11 — fight #2 (post-transition zone)', [
    '- Did: wandered to find a monster, then Space/E/Q rotation for up to 60s.',
    `- Combat chat: ${JSON.stringify(fight2Log)}`,
    `- Kills: ${kills2}. Captures: ${s11a}, ${s11}`,
  ]);

  // ---- Beat 12: die on purpose ---------------------------------------------
  const chatBeforeDeath = await chatText(page);
  let dead = false; let deathOverlayText = '';
  const deathStart = Date.now();
  // Stand among monsters and do nothing; if nothing attacks, wander deeper.
  while (Date.now() - deathStart < 180000) {
    const c = await chatText(page);
    const d = chatDiff(chatBeforeDeath, c);
    if (/You died/i.test(d)) { dead = true; break;
    }
    // check for permadeath/chronicles screen
    const chroniclesVisible = await page.getByRole('heading', { name: 'Chronicles' }).isVisible().catch(() => false);
    if (chroniclesVisible) { dead = true; deathOverlayText = 'chronicles screen appeared'; break; }
    // wiggle to stay in aggro range / find attackers
    for (const key of ['KeyW', 'KeyD']) await walk(page, key, 500);
    await page.waitForTimeout(1500);
  }
  await page.waitForTimeout(1500);
  const s12 = await shot(page, 'a-death-moment');
  const deathChat = chatDiff(chatBeforeDeath, await chatText(page));
  // Whatever is on screen now — capture the death/respawn UX.
  const deathUi = await page.evaluate(() => document.body.innerText.slice(0, 1200));
  const s12b = await shot(page, 'a-death-screen');
  logBeat('Arc A beat 12 — die on purpose, observe death UX', [
    `- Did: stood in monster territory without fighting back for up to 3 minutes.`,
    `- Happened: dead=${dead}. Death chat: ${JSON.stringify(deathChat.slice(-600))}`,
    `- Screen after death: ${JSON.stringify(deathUi)}`,
    `- Captures: ${s12}, ${s12b}`,
  ]);

  // ---- Beat 13: what comes next (respawn / successor) + a few more minutes
  // Try to click whatever primary CTA the death/chronicles screen offers.
  const ctaCandidates = [/Set Out as /, /Add Scion/, /Return/, /Respawn/, /Continue/, /Play as Guest/];
  let ctaClicked = null;
  for (const rx of ctaCandidates) {
    const btn = page.getByRole('button', { name: rx });
    if (await btn.first().isVisible().catch(() => false)) {
      // If a "Name a new Scion" field is visible, fill it first.
      const scionName = page.getByLabel('Name a new Scion');
      if (/Add Scion/.test(String(rx)) && await scionName.isVisible().catch(() => false)) {
        await scionName.fill('Wayfarer Two');
      }
      await btn.first().click().catch(() => {});
      ctaClicked = String(rx);
      await page.waitForTimeout(2500);
      break;
    }
  }
  const s13 = await shot(page, 'a-after-death-cta');
  // A few more minutes of play: walk + one more fight attempt.
  const chatBeforeWind = await chatText(page);
  await page.locator('canvas[aria-label="Game world"]').click({ position: { x: 400, y: 400 } }).catch(() => {});
  for (const key of ['KeyD', 'KeyS', 'KeyS', 'KeyA']) await walk(page, key, 800);
  const windStart = Date.now();
  while (Date.now() - windStart < 45000) {
    await page.keyboard.press('Space');
    await page.waitForTimeout(800);
    await page.keyboard.press('KeyQ');
    await page.waitForTimeout(800);
  }
  const s13b = await shot(page, 'a-final-minutes');
  logBeat('Arc A beat 13 — post-death flow + a few more minutes', [
    `- Did: clicked death-screen CTA (${ctaClicked || 'none found'}), walked, fought for ~45s.`,
    `- Chat: ${JSON.stringify(chatDiff(chatBeforeWind, await chatText(page)).slice(-500))}`,
    `- Captures: ${s13}, ${s13b}`,
  ]);

  logLine(`\n# Arc A ended ${ts()}`);
} catch (err) {
  die(`Arc A driver crashed: ${err.message}\n${err.stack}`);
  try { await shot(page, 'a-crash-state'); } catch { /* ignore */ }
} finally {
  flushConsole(consoleLog, 'arc-a');
  await browser.close();
}
