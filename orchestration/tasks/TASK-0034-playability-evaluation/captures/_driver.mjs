// TASK-0034 playability session driver — shared helpers.
// Everything this writes stays inside this captures/ directory.
import { chromium } from 'playwright';
import fs from 'node:fs';
import path from 'node:path';
import { fileURLToPath } from 'node:url';

export const BASE = process.env.PLAYWRIGHT_BASE_URL || 'http://127.0.0.1:9777';
export const CAPTURES = path.dirname(fileURLToPath(import.meta.url));
export const LOG = path.join(CAPTURES, 'session-log.md');

export const ts = () => new Date().toISOString().replace('T', ' ').slice(0, 19);

export function logLine(text = '') {
  fs.appendFileSync(LOG, `${text}\n`);
}

export function logBeat(title, lines) {
  logLine(`\n### ${ts()} — ${title}`);
  for (const line of [].concat(lines)) logLine(line);
}

export function nextShotNumber() {
  let max = 0;
  for (const f of fs.readdirSync(CAPTURES)) {
    const m = f.match(/^(\d+)-.*\.jpg$/);
    if (m) max = Math.max(max, parseInt(m[1], 10));
  }
  return max + 1;
}

export async function shot(page, slug) {
  const n = nextShotNumber();
  const name = `${String(n).padStart(2, '0')}-${slug}.jpg`;
  const p = path.join(CAPTURES, name);
  for (const q of [75, 60, 45]) {
    await page.screenshot({ path: p, type: 'jpeg', quality: q });
    if (fs.statSync(p).size <= 250 * 1024) break;
  }
  return name;
}

export async function createSession() {
  const browser = await chromium.launch({ headless: true });
  const context = await browser.newContext({
    viewport: { width: 1440, height: 1000 },
    ignoreHTTPSErrors: true,
  });
  const page = await context.newPage();
  const consoleLog = [];
  page.on('console', (msg) => {
    if (msg.type() === 'error' || msg.type() === 'warning') {
      consoleLog.push(`[${ts()}] ${msg.type()}: ${msg.text()}`);
    }
  });
  page.on('pageerror', (err) => consoleLog.push(`[${ts()}] PAGEERROR: ${err.message}`));
  page.on('requestfailed', (req) => {
    consoleLog.push(`[${ts()}] REQFAIL: ${req.url()} ${req.failure()?.errorText || ''}`);
  });
  return { browser, context, page, consoleLog };
}

export function flushConsole(consoleLog, name) {
  fs.writeFileSync(
    path.join(CAPTURES, `console-${name}.log`),
    consoleLog.length ? consoleLog.join('\n') : '(no console errors/warnings captured)',
  );
}

export async function api(pathname) {
  const res = await fetch(`${BASE}${pathname}`);
  if (!res.ok) throw new Error(`${pathname} -> ${res.status}`);
  return res.json();
}

export const players = () => api('/world/players');
export const myPlayer = async () => (await players()).at(-1) || null;
export const worldItems = () => api('/world/items');

export async function walk(page, key, ms = 700) {
  await page.keyboard.down(key);
  await page.waitForTimeout(ms);
  await page.keyboard.up(key);
  await page.waitForTimeout(300);
}

// Hold `key` in steps until predicate(player) is true or steps run out.
export async function walkToward(page, key, predicate, { stepMs = 700, maxSteps = 45 } = {}) {
  for (let i = 0; i < maxSteps; i += 1) {
    await walk(page, key, stepMs);
    const p = await myPlayer();
    if (!p) return { ok: false, pos: null, steps: i + 1 };
    if (predicate(p)) return { ok: true, pos: p, steps: i + 1 };
  }
  return { ok: false, pos: await myPlayer(), steps: maxSteps };
}

export async function canvasRightClick(page, fx = 0.5, fy = 0.5) {
  const canvas = page.locator('canvas[aria-label="Game world"]');
  const bounds = await canvas.boundingBox();
  if (!bounds) return { open: false, actions: [], error: 'no canvas bounds' };
  await canvas.click({
    button: 'right',
    position: { x: Math.round(bounds.width * fx), y: Math.round(bounds.height * fy) },
  });
  await page.waitForTimeout(500);
  const menu = page.locator('#actions');
  if (await menu.isVisible()) {
    const actions = (await menu.locator('.action').allTextContents()).map(a => a.trim());
    return { open: true, actions };
  }
  return { open: false, actions: [] };
}

export async function clickMenuAction(page, regex) {
  const item = page.locator('#actions .action', { hasText: regex });
  if ((await item.count()) === 0) return false;
  await item.first().click();
  await page.waitForTimeout(300);
  return true;
}

export async function closeMenu(page) {
  const menu = page.locator('#actions');
  if (!(await menu.isVisible())) return;
  const cancel = menu.locator('.action', { hasText: 'Cancel' });
  if (await cancel.count()) await cancel.first().click();
  else await page.keyboard.press('Escape');
  await page.waitForTimeout(200);
}

export async function chatText(page) {
  return page.evaluate(() => {
    const el = document.querySelector('.chatbox__messages');
    return el ? el.innerText.trim() : '';
  });
}

// Newest chat lines not present in a previous snapshot.
export function chatDiff(before, after) {
  const beforeLines = new Set((before || '').split('\n'));
  return (after || '').split('\n').filter(l => l.trim() && !beforeLines.has(l)).join('\n');
}

export async function minimapText(page) {
  const el = page.getByLabel('World minimap');
  if (!(await el.count())) return '';
  return (await el.innerText()).trim();
}

export async function hudSummary(page) {
  return page.evaluate(() => {
    const grab = sel => {
      const el = document.querySelector(sel);
      return el ? el.innerText.trim().slice(0, 400) : null;
    };
    return {
      minimap: grab('.world-minimap'),
      quickbar: grab('.quickbar'),
      vitals: grab('.vitals, .hud-vitals, [class*=health]'),
    };
  });
}
