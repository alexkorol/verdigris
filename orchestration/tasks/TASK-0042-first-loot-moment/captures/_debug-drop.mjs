import { HeadlessPlayer } from '../../../../playtest/harness.mjs';

const p = await HeadlessPlayer.connect({
  url: 'ws://localhost:9881',
  guestId: 'debug-first-find',
  houseName: 'House Debug',
  scionName: 'Debugborn',
});
try {
  await p.enterZone('dungeon', 'warren');
  await p.devSetLevel(5);
  await p.devHeal();
  let s = await p.state();
  console.log('scene:', s.sceneName, s.sceneType, 'monsters:', s.monsters.length);
  console.log('sceneMetadata:', JSON.stringify(s.sceneMetadata && {
    depth: s.sceneMetadata.depth,
    theme: s.sceneMetadata.theme,
    layout: s.sceneMetadata.layout,
    encounter: s.sceneMetadata.encounter && s.sceneMetadata.encounter.id,
  }));

  let kills = 0;
  const drop = await p.waitFor(async () => {
    const st = await p.state();
    if (st.lifecycle !== 'alive') await p.devHeal();
    const found = st.groundItems.find(item => item.firstFind);
    if (found) return found;
    const slain = p.messages.filter(m => m.includes('slain')).length;
    if (slain !== kills) {
      kills = slain;
      console.log(`kills=${kills} groundItems=${st.groundItems.map(i => i.id).join(',')}`);
    }
    const target = st.monsters
      .filter(m => m.rarity !== 'elite')
      .sort((a, b) => (Math.abs(a.x - st.x) + Math.abs(a.y - st.y))
        - (Math.abs(b.x - st.x) + Math.abs(b.y - st.y)))[0];
    if (target) {
      await p.devTeleport(Math.round(target.x) + 1, Math.round(target.y));
      await p.attack(target);
    }
    return false;
  }, { timeoutMs: 45000, intervalMs: 500, label: 'first find drop' });

  console.log('FIRST FIND:', JSON.stringify({
    id: drop.id, name: drop.displayName, firstFind: drop.firstFind, x: drop.x, y: drop.y,
  }));
  console.log('messages:', p.messages.slice(-5));
} finally {
  p.close();
}
