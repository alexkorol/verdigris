// TASK-0034 Arc B — Chronicles path session (Play as Guest -> Found a House ->
// Name a Scion (mortal oath) -> Set Out -> play -> permadeath -> crypt -> successor).
import {
  BASE, chatDiff, chatText, clickMenuAction, closeMenu, createSession,
  flushConsole, hudSummary, logBeat, logLine, minimapText, myPlayer,
  shot, walk, walkToward, canvasRightClick, worldItems, ts,
} from './_driver.mjs';

const { page, browser, consoleLog } = await createSession();
const die = (msg) => { logLine(`\n**DRIVER NOTE (${ts()}): ${msg}**`); };

try {
  logLine(`\n\n# Arc B — Chronicles path — started ${ts()}`);
  logLine('Client: headless Chromium (Playwright 1.56.1), 1440x1000 viewport. Server: 127.0.0.1:9777 dev build.');

  // ---- Beat 1: landing -> Play as Guest -> Chronicles onboarding ----------
  await page.goto(BASE, { waitUntil: 'domcontentloaded' });
  await page.waitForTimeout(2000);
  const s1 = await shot(page, 'b-landing-page');
  await page.getByRole('button', { name: 'Play as Guest', exact: true }).click();
  const chroniclesHeading = page.getByRole('heading', { name: 'Chronicles' });
  let chroniclesUp = true;
  try {
    await chroniclesHeading.waitFor({ state: 'visible', timeout: 20000 });
  } catch { chroniclesUp = false; }
  await page.waitForTimeout(1000);
  const s2 = await shot(page, 'b-chronicles-onboarding');
  const onboardingText = await page.evaluate(() => {
    const el = document.querySelector('.chronicles');
    return el ? el.innerText.trim().slice(0, 900) : '(no .chronicles element)';
  });
  logBeat('Arc B beat 1 — landing -> Play as Guest -> Chronicles onboarding', [
    `- Did: loaded ${BASE}/, clicked "Play as Guest".`,
    `- Happened: Chronicles screen visible = ${chroniclesUp}.`,
    `- Onboarding copy verbatim: ${JSON.stringify(onboardingText)}`,
    `- Captures: ${s1}, ${s2}`,
  ]);

  // ---- Beat 2: Found a House ----------------------------------------------
  const houseName = page.getByLabel('Found a House');
  let houseFlow = [];
  if (await houseName.isVisible().catch(() => false)) {
    await houseName.fill('Gateward');
    const s3a = await shot(page, 'b-found-house-filled');
    await page.getByRole('button', { name: 'Inscribe' }).click();
    await page.waitForTimeout(1200);
    houseFlow.push(`house founded, capture: ${s3a}`);
  } else {
    houseFlow.push('no "Found a House" form — house may already exist for this guest');
  }
  const s3 = await shot(page, 'b-house-ledger');
  const ledgerText = await page.evaluate(() => {
    const el = document.querySelector('.chronicles__ledger');
    return el ? el.innerText.trim().slice(0, 600) : '(no ledger)';
  });
  logBeat('Arc B beat 2 — Found a House', [
    `- Did: filled "Found a House" with "Gateward", clicked Inscribe. ${houseFlow.join(' ')}`,
    `- Ledger now reads: ${JSON.stringify(ledgerText)}`,
    `- Capture: ${s3}`,
  ]);

  // ---- Beat 3: Name a Scion with the mortal oath ---------------------------
  const scionName = page.getByLabel('Name a new Scion');
  await scionName.waitFor({ state: 'visible', timeout: 10000 });
  await scionName.fill('Mortalis');
  const mortalBox = page.locator('.chronicles__mortal-checkbox');
  const oathCopy = await page.locator('.chronicles__mortal-option').innerText().catch(() => '(no oath copy)');
  await mortalBox.check();
  const s4a = await shot(page, 'b-scion-mortal-oath');
  await page.getByRole('button', { name: 'Add Scion' }).click();
  await page.waitForTimeout(1200);
  const s4 = await shot(page, 'b-scion-added');
  const rosterText = await page.evaluate(() => {
    const el = document.querySelector('.chronicles__roster');
    return el ? el.innerText.trim().slice(0, 400) : '(no roster)';
  });
  logBeat('Arc B beat 3 — Name a Scion (mortal oath sworn)', [
    '- Did: named scion "Mortalis", checked "Swear the mortal oath", clicked Add Scion.',
    `- Oath copy verbatim: ${JSON.stringify(oathCopy)}`,
    `- Roster now: ${JSON.stringify(rosterText)}`,
    `- Captures: ${s4a}, ${s4}`,
  ]);

  // ---- Beat 4: Set Out ------------------------------------------------------
  const setOut = page.getByRole('button', { name: /^Set Out as / });
  await setOut.waitFor({ state: 'visible', timeout: 10000 });
  const setOutLabel = await setOut.innerText();
  await setOut.click();
  const canvas = page.locator('canvas[aria-label="Game world"]');
  let arrived = true;
  try {
    await canvas.waitFor({ state: 'visible', timeout: 25000 });
  } catch { arrived = false; }
  await page.waitForTimeout(2500);
  const s5 = await shot(page, 'b-set-out-arrival');
  const chat0 = await chatText(page);
  const mini0 = await minimapText(page);
  const pos0 = await myPlayer();
  logBeat('Arc B beat 4 — Set Out', [
    `- Did: clicked "${setOutLabel.trim()}".`,
    `- Happened: canvas visible = ${arrived}. Spawn: ${JSON.stringify(pos0)}`,
    `- Minimap: ${JSON.stringify(mini0)}`,
    `- Chat: ${JSON.stringify(chat0)}`,
    `- Capture: ${s5}`,
  ]);

  // ---- Beat 5: first minute in the Chronicles framing -----------------------
  await page.waitForTimeout(3000);
  const s6 = await shot(page, 'b-first-minute-hud');
  const hud = await hudSummary(page);
  const identityBits = await page.evaluate(() => document.body.innerText.match(/Mortalis|Gateward|House/g)?.slice(0, 6) || []);
  logBeat('Arc B beat 5 — first minute orientation (Chronicles framing)', [
    '- Did: read the screen. Looked for any sign of my Scion name / House in the HUD.',
    `- HUD quickbar: ${JSON.stringify(hud.quickbar)}`,
    `- Identity references on screen: ${JSON.stringify(identityBits)}`,
    `- Capture: ${s6}`,
  ]);

  // ---- Beat 6: move around town ---------------------------------------------
  const before = await myPlayer();
  await canvas.click({ position: { x: 400, y: 400 } }).catch(() => {});
  for (const key of ['KeyD', 'KeyS', 'KeyA', 'KeyW']) await walk(page, key, 600);
  const after = await myPlayer();
  const s7 = await shot(page, 'b-move-around-town');
  logBeat('Arc B beat 6 — move around town (WASD)', [
    `- Did: D/S/A/W ~0.6s each. Pos ${JSON.stringify(before && { x: before.x, y: before.y })} -> ${JSON.stringify(after && { x: after.x, y: after.y })}`,
    `- Capture: ${s7}`,
  ]);

  // ---- Beat 7: talk to Aldwyn ------------------------------------------------
  const chatBeforeAldwyn = await chatText(page);
  await walkToward(page, 'KeyA', p => p.x <= 35, { maxSteps: 20 });
  let talked = false; const scanNotes = [];
  const scanPoints = [
    [0.5, 0.5], [0.42, 0.5], [0.58, 0.5], [0.35, 0.5], [0.65, 0.5],
    [0.42, 0.42], [0.58, 0.42], [0.42, 0.58], [0.58, 0.58], [0.3, 0.45], [0.7, 0.45],
  ];
  for (const [fx, fy] of scanPoints) {
    const menu = await canvasRightClick(page, fx, fy);
    if (menu.open) {
      scanNotes.push(`(${fx},${fy}): ${menu.actions.join(' | ')}`);
      if (menu.actions.some(a => /^Talk/.test(a))) {
        const s8a = await shot(page, 'b-aldwyn-context-menu');
        scanNotes.push(`menu capture: ${s8a}`);
        talked = await clickMenuAction(page, /^Talk/);
        break;
      }
      await closeMenu(page);
    }
  }
  await page.waitForTimeout(1200);
  const aldwynSaid = chatDiff(chatBeforeAldwyn, await chatText(page));
  const s8 = await shot(page, 'b-after-aldwyn-talk');
  logBeat('Arc B beat 7 — talk to Aldwyn (find a goal)', [
    `- Did: right-click scanned for Talk. Menus: ${scanNotes.join(' ;; ') || 'none'}`,
    `- Happened: Talk clicked = ${talked}. Aldwyn said: ${JSON.stringify(aldwynSaid)}`,
    `- Capture: ${s8}`,
  ]);

  // ---- Beat 8: north gate -> Old Wood -> fight #1 ----------------------------
  const p8 = await myPlayer();
  if (p8 && p8.x > 39) await walkToward(page, 'KeyA', p => p.x <= 38, { maxSteps: 20 });
  else if (p8 && p8.x < 37) await walkToward(page, 'KeyD', p => p.x >= 38, { maxSteps: 20 });
  await walkToward(page, 'KeyW', p => p.y <= 93, { maxSteps: 30 });
  await page.waitForTimeout(1500);
  const p8b = await myPlayer();
  const miniGate = await minimapText(page);
  const s9 = await shot(page, 'b-old-wood-entry');
  logBeat('Arc B beat 8 — north gate zone transition', [
    `- Did: walked north through the gate. Pos now ${JSON.stringify(p8b && { x: p8b.x, y: p8b.y })}; minimap: ${JSON.stringify(miniGate)}`,
    `- Capture: ${s9}`,
  ]);

  const chatBeforeFight = await chatText(page);
  await walkToward(page, 'KeyW', p => p.y <= 172, { maxSteps: 15 });
  let fightLog = ''; let kills = 0; const fightShots = [];
  const fightStart = Date.now();
  let midShotDone = false;
  while (Date.now() - fightStart < 75000 && kills < 2) {
    await page.keyboard.press('Space');
    await page.waitForTimeout(700);
    await page.keyboard.press('KeyQ');
    await page.waitForTimeout(700);
    await page.keyboard.press('KeyE');
    await page.waitForTimeout(500);
    if (!midShotDone && Date.now() - fightStart > 6000) {
      fightShots.push(await shot(page, 'b-fight-wolf-mid'));
      midShotDone = true;
    }
    fightLog = chatDiff(chatBeforeFight, await chatText(page));
    kills = (fightLog.match(/died\./g) || []).length;
    if (kills >= 1 && Date.now() - fightStart > 15000) break;
  }
  fightShots.push(await shot(page, 'b-fight-wolf-end'));
  logBeat('Arc B beat 9 — fight #1 (Old Wood Wolf), Space/Q/E rotation', [
    `- Combat chat: ${JSON.stringify(fightLog)}`,
    `- Kills: ${kills}. Duration: ${Math.round((Date.now() - fightStart) / 1000)}s. Captures: ${fightShots.join(', ')}`,
  ]);

  // ---- Beat 10: loot + equip -------------------------------------------------
  const itemsBefore = await worldItems();
  let tookSomething = false; const takeScans = [];
  const lootPoints = [
    [0.5, 0.5], [0.53, 0.5], [0.47, 0.5], [0.5, 0.53], [0.5, 0.47],
    [0.56, 0.5], [0.44, 0.5], [0.5, 0.56], [0.5, 0.44], [0.56, 0.56], [0.44, 0.44],
  ];
  for (const [fx, fy] of lootPoints) {
    const menu = await canvasRightClick(page, fx, fy);
    if (menu.open) {
      takeScans.push(`(${fx},${fy}): ${menu.actions.join(' | ')}`);
      if (menu.actions.some(a => /^Take/.test(a))) {
        const s10a = await shot(page, 'b-take-context-menu');
        takeScans.push(`menu capture: ${s10a}`);
        tookSomething = await clickMenuAction(page, /^Take/);
        await page.waitForTimeout(800);
        break;
      }
      await closeMenu(page);
    }
  }
  const itemsAfter = await worldItems();
  await page.keyboard.press('KeyI');
  await page.waitForTimeout(700);
  const inv = page.getByLabel('Inventory panel');
  const invVisible = await inv.isVisible().catch(() => false);
  const s10 = await shot(page, 'b-inventory-open');
  let invItems = [];
  if (invVisible) {
    invItems = await inv.locator('.inventory-item').evaluateAll(els => els.map(e => e.getAttribute('aria-label')));
    const firstItem = inv.locator('.inventory-item').first();
    if (await firstItem.count()) {
      await firstItem.click({ button: 'right' });
      await page.waitForTimeout(500);
      await clickMenuAction(page, /^Equip/);
      await page.waitForTimeout(500);
    }
  }
  const s10b = await shot(page, 'b-after-equip');
  await page.keyboard.press('Escape');
  logBeat('Arc B beat 10 — loot a drop, open inventory, equip', [
    `- Menus seen while looting: ${takeScans.join(' ;; ') || 'none'}. Take clicked = ${tookSomething}. Ground items ${itemsBefore.length} -> ${itemsAfter.length}`,
    `- Inventory visible = ${invVisible}. Items: ${JSON.stringify(invItems)}`,
    `- Captures: ${s10}, ${s10b}`,
  ]);

  // ---- Beat 11: fight #2 — stay in combat longer to judge the loop -----------
  const chatBeforeFight2 = await chatText(page);
  let fight2Log = ''; let kills2 = 0; const f2Shots = [];
  const f2Start = Date.now();
  let f2Mid = false;
  while (Date.now() - f2Start < 90000 && kills2 < 2) {
    await page.keyboard.press('Space');
    await page.waitForTimeout(600);
    await page.keyboard.press('KeyQ');
    await page.waitForTimeout(600);
    await page.keyboard.press('KeyE');
    await page.waitForTimeout(600);
    if (!f2Mid && Date.now() - f2Start > 20000) {
      f2Shots.push(await shot(page, 'b-fight2-mid'));
      f2Mid = true;
    }
    fight2Log = chatDiff(chatBeforeFight2, await chatText(page));
    kills2 = (fight2Log.match(/died\./g) || []).length;
    if (/You died/i.test(fight2Log)) break;
  }
  f2Shots.push(await shot(page, 'b-fight2-end'));
  logBeat('Arc B beat 11 — fight #2 (extended, ~90s cap)', [
    `- Combat chat: ${JSON.stringify(fight2Log.slice(-800))}`,
    `- Kills: ${kills2}. Duration: ${Math.round((Date.now() - f2Start) / 1000)}s. Captures: ${f2Shots.join(', ')}`,
  ]);

  // ---- Beat 12: die on purpose (mortal oath -> permadeath) -------------------
  // Arc A lesson: a level-1 character cannot die naturally (soft-mode regen /
  // weak monsters — 3 min standing in monster territory, 0 threat). So: a brief
  // natural-death attempt for the record, then the same dev:kill websocket
  // message the playtest mortality scenario uses (playtest/scenarios/mortality.mjs).
  const chatBeforeDeath = await chatText(page);
  await page.waitForTimeout(20000); // stand in wolf territory, no fighting back
  const hpCheck = await page.evaluate(() => document.body.innerText.match(/HP\s*\n?(\d+)\s*\/\s*(\d+)/)?.[0] || '(no HP readout)');
  const s12a = await shot(page, 'b-death-natural-attempt');
  await page.evaluate(() => {
    window.ws.send(JSON.stringify({ event: 'dev:kill', data: {} }));
  });
  let dead = false; let returnToChronicles = false;
  const deathStart = Date.now();
  while (Date.now() - deathStart < 30000) {
    await page.waitForTimeout(1000);
    const d = chatDiff(chatBeforeDeath, await chatText(page));
    if (/You died|final death|permadead/i.test(d)) dead = true;
    if (await page.getByRole('heading', { name: 'Chronicles' }).isVisible().catch(() => false)) {
      returnToChronicles = true;
      break;
    }
    if (dead && Date.now() - deathStart > 4000) {
      returnToChronicles = await page.getByRole('heading', { name: 'Chronicles' }).isVisible().catch(() => false);
      break;
    }
  }
  const s12 = await shot(page, 'b-death-moment');
  const deathChat = chatDiff(chatBeforeDeath, await chatText(page));
  await page.waitForTimeout(3000);
  const s12b = await shot(page, 'b-after-permadeath-screen');
  const deathUi = await page.evaluate(() => document.body.innerText.slice(0, 1400));
  logBeat('Arc B beat 12 — die on purpose (mortal oath)', [
    '- Did: stood in wolf territory ~20s without fighting (natural-death attempt), then sent the playtest dev:kill websocket message (same path as playtest/scenarios/mortality.mjs).',
    `- Natural-death check: vitals read ${JSON.stringify(hpCheck)} right before dev:kill — corroborates Arc A finding that natural death is unreachable at level 1.`,
    `- Happened: death/final-death seen in chat = ${dead}; returned to Chronicles screen = ${returnToChronicles}.`,
    `- Death chat: ${JSON.stringify(deathChat.slice(-600))}`,
    `- Screen after death: ${JSON.stringify(deathUi)}`,
    `- Captures: ${s12a}, ${s12}, ${s12b}`,
  ]);

  // ---- Beat 13: crypt / memorial ---------------------------------------------
  const crypt = page.locator('.chronicles__crypt');
  let cryptText = '(no crypt element)';
  if (await crypt.count()) {
    const summary = crypt.locator('summary');
    if (await summary.isVisible().catch(() => false)) await summary.click();
    await page.waitForTimeout(500);
    cryptText = (await crypt.innerText().catch(() => '(unreadable)')).slice(0, 500);
  }
  const s13 = await shot(page, 'b-crypt-open');
  const houseStats = await page.evaluate(() => {
    const el = document.querySelector('.chronicles__house-heading');
    return el ? el.innerText.trim() : '(no house heading)';
  });
  logBeat('Arc B beat 13 — crypt / memorial', [
    '- Did: opened the crypt details on the Chronicles screen.',
    `- Crypt contents: ${JSON.stringify(cryptText)}`,
    `- House heading (renown etc.): ${JSON.stringify(houseStats)}`,
    `- Capture: ${s13}`,
  ]);

  // ---- Beat 14: create a successor, note what carries over -------------------
  const scionName2 = page.getByLabel('Name a new Scion');
  let successorFlow = [];
  if (await scionName2.isVisible().catch(() => false)) {
    await scionName2.fill('Mortalis II');
    // Successor stays on the default "soft return" so we can contrast death UX.
    await page.getByRole('button', { name: 'Add Scion' }).click();
    await page.waitForTimeout(1200);
    const s14a = await shot(page, 'b-successor-added');
    successorFlow.push(`successor added, capture: ${s14a}`);
    const setOut2 = page.getByRole('button', { name: /^Set Out as / });
    await setOut2.waitFor({ state: 'visible', timeout: 10000 });
    successorFlow.push(`Set Out button: "${(await setOut2.innerText()).trim()}"`);
    await setOut2.click();
    try {
      await page.locator('canvas[aria-label="Game world"]').waitFor({ state: 'visible', timeout: 25000 });
      successorFlow.push('canvas visible after Set Out');
    } catch { successorFlow.push('canvas did NOT appear after Set Out'); }
    await page.waitForTimeout(2500);
  } else {
    successorFlow.push('no "Name a new Scion" form visible — successor flow blocked');
  }
  const s14 = await shot(page, 'b-successor-arrival');
  const posSucc = await myPlayer();
  const chatSucc = await chatText(page);
  await page.keyboard.press('KeyI');
  await page.waitForTimeout(700);
  const inv2 = page.getByLabel('Inventory panel');
  let succItems = [];
  if (await inv2.isVisible().catch(() => false)) {
    succItems = await inv2.locator('.inventory-item').evaluateAll(els => els.map(e => e.getAttribute('aria-label')));
  }
  const s14b = await shot(page, 'b-successor-inventory');
  await page.keyboard.press('Escape');
  logBeat('Arc B beat 14 — successor: what carries over', [
    `- Did: ${successorFlow.join(' ')}`,
    `- Successor spawn: ${JSON.stringify(posSucc)}`,
    `- Chat on arrival: ${JSON.stringify(chatSucc.slice(-500))}`,
    `- Successor inventory: ${JSON.stringify(succItems)}`,
    `- Captures: ${s14}, ${s14b}`,
  ]);

  // ---- Beat 15: a few more minutes as the successor ---------------------------
  const chatBeforeWind = await chatText(page);
  await page.locator('canvas[aria-label="Game world"]').click({ position: { x: 400, y: 400 } }).catch(() => {});
  for (const key of ['KeyD', 'KeyS', 'KeyS', 'KeyA']) await walk(page, key, 800);
  const windStart = Date.now();
  while (Date.now() - windStart < 40000) {
    await page.keyboard.press('Space');
    await page.waitForTimeout(800);
    await page.keyboard.press('KeyQ');
    await page.waitForTimeout(800);
  }
  const s15 = await shot(page, 'b-final-minutes');
  logBeat('Arc B beat 15 — a few more minutes as the successor', [
    `- Chat: ${JSON.stringify(chatDiff(chatBeforeWind, await chatText(page)).slice(-500))}`,
    `- Capture: ${s15}`,
  ]);

  logLine(`\n# Arc B ended ${ts()}`);
} catch (err) {
  die(`Arc B driver crashed: ${err.message}\n${err.stack}`);
  try { await shot(page, 'b-crash-state'); } catch { /* ignore */ }
} finally {
  flushConsole(consoleLog, 'arc-b');
  await browser.close();
}
