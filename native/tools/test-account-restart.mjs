// Real-process durability gate: no in-process session object survives a restart.
import assert from 'node:assert/strict';
import { spawn } from 'node:child_process';
import { once } from 'node:events';
import { mkdtemp, readdir, readFile, writeFile, rename, mkdir, rmdir } from 'node:fs/promises';
import { tmpdir } from 'node:os';
import path from 'node:path';
import net from 'node:net';
import WebSocket from 'ws';

const exe = path.resolve(process.argv[2] || 'native/build/verdigris_server.exe');
const directory = await mkdtemp(path.join(tmpdir(), 'verdigris-restart-'));
let child, socket, sequence = 0;
const waiters = new Set();
const logs = [];
const identity = 'restart-proof';
async function port() {
  const probe = net.createServer();
  await new Promise(resolve => probe.listen(0, '127.0.0.1', resolve));
  const value = probe.address().port;
  await new Promise(resolve => probe.close(resolve));
  return value;
}
async function boot() {
  const p = await port();
  child = spawn(exe, [String(p), '--save-dir', directory], { windowsHide: true, stdio: ['pipe', 'pipe', 'pipe'] });
  await new Promise((resolve, reject) => {
    const timer = setTimeout(() => reject(new Error('server readiness timeout')), 10000);
    child.once('error', reject);
    child.once('exit', code => { clearTimeout(timer); reject(new Error(`server exited ${code}: ${logs.join('')}`)); });
    child.stderr.on('data', b => logs.push(b.toString()));
    child.stdout.on('data', b => {
      logs.push(b.toString());
      if (b.toString().includes('listening on')) { clearTimeout(timer); resolve(); }
    });
  });
  socket = new WebSocket(`ws://127.0.0.1:${p}`);
  await once(socket, 'open');
  socket.on('message', bytes => {
    const message = JSON.parse(bytes);
    for (const waiter of [...waiters]) if (waiter.accept(message)) {
      waiters.delete(waiter); clearTimeout(waiter.timer); waiter.resolve(message);
    }
  });
  return p;
}
function response(accept) {
  return new Promise((resolve, reject) => {
    const waiter = { accept, resolve, timer: setTimeout(() => { waiters.delete(waiter); reject(new Error('protocol response timeout')); }, 7000) };
    waiters.add(waiter);
  });
}
function send(event, data = {}) { socket.send(JSON.stringify({ event, data })); }
async function state() {
  const requestId = `restart-${++sequence}`;
  const result = response(e => e.event === 'dev:state' && e.data.requestId === requestId);
  send('dev:state', { requestId });
  return (await result).data.state;
}
async function login() {
  const result = response(e => e.event === 'player:chronicles:ready');
  send('player:login', { guestId: identity, awaitChronicles: true });
  await result;
}
async function stop() {
  socket?.terminate(); socket = null;
  if (child && child.exitCode === null) { const exited = once(child, 'exit'); child.kill('SIGKILL'); await exited; }
  child = null;
}
const select = id => send('player:chronicles:select', { houseId: 'house-restart', scionId: id, scionName: id, mortal: false });
const action = (actionId, item) => send('player:context-menu:action', { queueItem: { action: { actionId }, item } });
const purse = s => s.inventory.filter(i => i.id === 'coins').reduce((n, i) => n + i.qty, 0);
const checkpoint = s => ({ inventory: s.inventoryDetails, wear: s.wearDetails, tree: s.passiveTree,
  quests: s.quests, questPoints: s.questPoints, xp: s.xp, house: s.chroniclesRecord,
  bank: s.bank, investment: s.houseInvestment, endgame: s.endgame });
try {
  await boot(); await login();
  send('player:chronicles:mutate', { type: 'found-house', house: { id: 'house-restart', name: 'Durable House' } });
  for (const id of ['scion-a', 'scion-b']) send('player:chronicles:mutate', {
    type: 'add-scion', houseId: 'house-restart', scion: { id, name: id, mortal: false }
  });
  const births = [];
  for (const mortal of [false, true]) {
    const receipt = response(e => e.event === 'chronicles:state' && e.data.createdScionId);
    send('chronicles:scion:create', { houseId: 'house-restart', name: mortal ? 'Hardcore Birth' : 'Soft Birth', mortal });
    births.push({ id: (await receipt).data.createdScionId, mortal });
  }
  // The checkbox must be durable at creation, even if the client closes
  // before auto-admission. Do not rely on select-scion repairing the oath.
  await stop(); await boot(); await login();
  const birthRoster = (await state()).chroniclesRecord.state.houses.flatMap(house => house.scions);
  for (const birth of births)
    assert.equal(birthRoster.find(scion => scion.id === birth.id)?.mortal, birth.mortal,
      'Hardcore choice survives restart before first admission');
  select('scion-a');
  send('dev:give', { itemId: 'vessel-handaxe', qty: 1, seed: 53, itemLevel: 18 });
  send('dev:give', { itemId: 'charted-tablet-crown', qty: 1, seed: 37, itemLevel: 5 });
  send('dev:give', { itemId: 'coins', qty: 400 });
  let s = await state();
  const axe = s.inventory.find(i => i.id === 'vessel-handaxe');
  assert.ok(axe?.uuid, 'seeded vessel granted');
  send('item:equip', { item: { uuid: axe.uuid } });
  // Real shop purchase, including canonical pricing rather than a client price.
  send('dev:teleport', { x: 19, y: 113 });
  send('player:npc:trade', { item: { id: 2 } });
  const beforeBuy = await state();
  action('player:shop:buy', { id: 'knife', price: 1 });
  s = await state();
  assert.equal(purse(s), purse(beforeBuy) - 5, 'vendor charges the authoritative price');
  assert.ok(s.inventory.some(i => i.id === 'knife'), 'purchased item exists');
  const boughtKnife = s.inventory.find(i => i.id === 'knife');
  send('dev:setlevel', { level: 7 });
  send('player:skilltree:save', { snapshot: { schemaVersion: 2,
    nodes: ['0,0', '1,0', '2,0'], conduits: [], selectedNodeId: '2,0', classOrder: [] } });
  send('world:zone:enter', { nodeId: 'tin:1:0' });
  send('dev:clear-floor'); await state();
  send('party:returnToTown');
  send('dev:teleport', { x: 31, y: 121 });
  send('house:investment:choose', { choice: 'house_production' });
  action('player:screen:bank', { id: 'bank' });
  action('player:bank:deposit', { uuid: boughtKnife.uuid, qty: 1 });
  s = await state();
  assert.equal(s.passiveTree.nodes.length, 3, 'allocated passive build is nonempty');
  assert.equal(s.houseInvestment.choice, 'house_production', 'earned House investment chosen');
  assert.ok(s.bank.length > 0, 'purchased item deposited into bank');
  const before = checkpoint(s);
  const gold = purse(s);
  select('scion-b'); await state();
  select('scion-a'); s = await state();
  assert.deepEqual(s.inventoryDetails, before.inventory, 'reserve switch preserves inventory');
  assert.deepEqual(s.wearDetails, before.wear, 'reserve switch preserves equipped vessel');
  await stop(); // No graceful shutdown or final-save hook.
  await boot(); await login(); select('scion-a'); s = await state();
  assert.equal(purse(s), gold, 'gold survives an entirely new server process');
  assert.deepEqual(checkpoint(s), before, 'full item rolls, equipment, roster and progress survive restart');
  assert.equal(s.sceneType, 'town', 'reconnect starts in safe town');
  send('instance:enterSolo', { template: 'dungeon', layout: 'warren' });
  await state();
  send('party:returnToTown'); s = await state();
  assert.equal(purse(s), gold, 'gold survives the post-restart round trip');
  assert.deepEqual(s.wearDetails, before.wear, 'equipment survives the post-restart round trip');
  send('dev:give', { itemId: 'vessel-handaxe', qty: 1, seed: 53, itemLevel: 18 });
  s = await state();
  assert.ok(s.inventory.some(i => i.id === 'vessel-handaxe' && i.uuid !== axe.uuid), 'new-process loot never reuses saved UUID');
  send('player:chronicles:mutate', { type: 'add-scion', houseId: 'house-restart',
    scion: { id: 'scion-mortal', name: 'Mortal', mortal: true } });
  send('player:chronicles:select', { houseId: 'house-restart', scionId: 'scion-mortal', scionName: 'Mortal', mortal: true });
  send('dev:give', { itemId: 'garnet-amulet', qty: 1 });
  s = await state();
  const relic = s.inventory.find(i => i.id === 'garnet-amulet');
  send('dev:kill'); s = await state();
  assert.equal(s.lifecycle, 'permadead', 'mortal death committed');
  await stop(); await boot(); await login();
  s = await state();
  assert.equal(s.lifecycle, 'permadead', 'restart does not resurrect a mortal Scion');
  select('scion-mortal'); s = await state();
  assert.equal(s.lifecycle, 'permadead', 'selecting the fallen cannot resurrect them');
  assert.equal(s.chronicles.mortal, true, 're-admission cannot remove an existing mortal oath');
  select('scion-a'); await state();
  send('dev:release-relic');
  send('player:take:underfoot'); s = await state();
  assert.ok(s.inventory.some(i => i.uuid === relic.uuid), 'successor recovers the exact circulating relic after restart');
  // A second process is refused while this one owns the save directory.
  const contender = spawn(exe, [String(await port()), '--save-dir', directory], { windowsHide: true, stdio: 'ignore' });
  const [code] = await once(contender, 'exit');
  assert.equal(code, 1, 'save-directory writer lock prevents clobbering');
  const accountFile = (await readdir(directory)).find(name => name.endsWith('.vgs'));
  const accountPath = path.join(directory, accountFile);
  const lastGood = await readFile(accountPath);
  await rename(accountPath, `${accountPath}.last-good`);
  await mkdir(accountPath); // Test-only empty directory blocks atomic replacement.
  const failedSave = response(e => e.event === 'game:send:message' && e.data.text.includes('Progress could not be saved'));
  send('dev:give', { itemId: 'coins', qty: 999 });
  await failedSave;
  const stoppedWrites = response(e => e.event === 'game:send:message' && e.data.text.includes('save unavailable'));
  send('dev:give', { itemId: 'coins', qty: 999 });
  await stoppedWrites;
  assert.deepEqual(await readFile(`${accountPath}.last-good`), lastGood, 'failed storage preserves last successful checkpoint');
  await rmdir(accountPath); // Remove only the empty fixture directory just created.
  await rename(`${accountPath}.last-good`, accountPath);
  await stop();
  const file = (await readdir(directory)).find(name => name.endsWith('.vgs'));
  const savePath = path.join(directory, file);
  const good = await readFile(savePath);
  await writeFile(`${savePath}.verified-backup`, good);
  const corrupt = Buffer.from(good); corrupt[corrupt.length - 1] ^= 0xff;
  await writeFile(savePath, corrupt);
  await boot();
  const closed = once(socket, 'close');
  send('player:login', { guestId: identity, awaitChronicles: true });
  await Promise.race([closed, new Promise((_, reject) => setTimeout(() => reject(new Error('damaged save was not rejected')), 7000))]);
  assert.deepEqual(await readFile(savePath), corrupt, 'damaged save is preserved, never silently replaced');
  console.log(`PASS: pre-admission Hardcore/soft creation, real-process restart, vendor purchase, reserve Scion, exact vessel/tablet rolls, zone round trip, UUID uniqueness, mortal death, relic recovery, exclusive writer, fail-stopped storage, corrupt-save preservation. Evidence: ${directory}`);
} finally { await stop(); }
