/**
 * Real-protocol first-delve evidence driver.
 *
 * Run against a development server:
 *   PLAYTEST_WS_URL=ws://localhost:6520 node drive-first-delve.mjs
 *
 * Development teleports shorten traversal only. Every recorded progression
 * step is produced by a real primary attack and authoritative monster death.
 */

import HeadlessPlayer from '../../../../playtest/harness.mjs';

const url = process.env.PLAYTEST_WS_URL || 'ws://localhost:6520';
const sleep = ms => new Promise(resolve => { setTimeout(resolve, ms); });

const summarize = (state) => {
  const active = state.monsters.filter(monster => monster.state.mode !== 'dormant');
  const dormant = state.monsters.filter(monster => monster.state.mode === 'dormant');
  const activeMarksmen = active.filter(monster => monster.name === 'Ashen Marksman');
  return {
    living: state.monsters.length,
    active: active.length,
    dormant: dormant.length,
    activeMarksmen: activeMarksmen.length,
    rangedMarksmen: activeMarksmen.filter(monster => monster.behaviour.type === 'ranged').length,
  };
};

const killOneActive = async (player) => {
  let state = await player.state();
  const target = state.monsters
    .filter(monster => monster.state.mode !== 'dormant')
    .sort((a, b) => (a.rarity === 'elite') - (b.rarity === 'elite'))[0];
  if (!target) throw new Error('No active staged actor was available');

  for (let attempt = 0; attempt < 80; attempt += 1) {
    state = await player.state();
    const live = state.monsters.find(monster => monster.uuid === target.uuid);
    if (!live) {
      await sleep(500);
      return target.name;
    }
    player.devHeal();
    player.devTeleport(Math.round(live.x) + 1, Math.round(live.y));
    await player.attack(live);
    await sleep(250);
  }
  throw new Error(`Timed out killing ${target.name}`);
};

const player = await HeadlessPlayer.connect({
  url,
  guestId: `task-0040-driven-transcript-${Date.now()}`,
  houseName: 'House Transcript',
  scionName: 'Cadence Witness',
});

try {
  await player.enterZone('dungeon', 'warren');
  player.devHeal();

  const initial = await player.waitFor(async () => {
    const state = await player.state();
    return summarize(state).active === 1 ? state : false;
  }, { timeoutMs: 10000, label: 'one active opening actor' });
  const initialPayload = Array.isArray(player.scene?.monsters) ? player.scene.monsters : [];
  const opener = initial.monsters.find(monster => monster.state.mode !== 'dormant');

  process.stdout.write(`initial-payload roster=${initialPayload.length} marksmen=${initialPayload.filter(monster => monster.name === 'Ashen Marksman').length}\n`);
  process.stdout.write(`scene-kills=0 opener=${opener.name} rarity=${opener.rarity} ${JSON.stringify(summarize(initial))}\n`);

  for (let encounter = 1; encounter <= 4; encounter += 1) {
    const defeated = await killOneActive(player);
    const state = await player.state();
    const sceneKills = initial.monsters.length - state.monsters.length;
    process.stdout.write(`scene-kills=${sceneKills} defeated=${defeated} ${JSON.stringify(summarize(state))}\n`);
  }
} finally {
  player.close();
}
