/**
 * Core loop: FIGHT. Enter a zone whose packs include a support healer, engage
 * a pack, and actually kill something within a sane time while surviving.
 * Regression: healers used to out-heal the player 3-20x — nothing could die.
 */
export default async function combat({ connect, assert }) {
  const p = await connect();
  try {
    await p.enterZone('grove', 'clearings'); // grove packs include Grovekeeper healers
    p.devSetLevel(5);
    p.devHeal();

    const scene = await p.state();
    assert(scene.monsters.length >= 20, `instance is populated (${scene.monsters.length} monsters)`);

    // Walk up to the nearest non-elite monster.
    const target = scene.monsters
      // Keep the target in the pack but leave the support/buffer caster alive
      // to exercise the healer regression without asking the harness to kill
      // the healer itself under scheduler contention.
      .filter(m => m.rarity !== 'elite' && !['support', 'buffer'].includes(m.behaviour?.type))
      .sort((a, b) => (Math.abs(a.x - scene.x) + Math.abs(a.y - scene.y))
        - (Math.abs(b.x - scene.x) + Math.abs(b.y - scene.y)))[0];
    assert(target, 'found a trash monster to fight');
    p.devTeleport(target.x + 1, target.y);

    const aliveBefore = scene.monsters.length;

    // Swing; auto-attack sustains the fight afterwards.
    await p.attack(target);

    const won = await p.waitFor(async () => {
      let s;
      try {
        s = await p.state();
      } catch (error) {
        // Keep the authored kill window alive for a transient diagnostic
        // frame miss while the loaded server resolves healer/combat ticks.
        if (/dev:state timed out/.test(error?.message || '')) return false;
        throw error;
      }
      if (s.lifecycle !== 'alive') {
        return 'died';
      }
      if (s.monsters.length < aliveBefore) {
        return 'killed';
      }
      // Keep swinging at the originally selected UUID. Retargeting the
      // nearest pack member on every sample can bounce between a healer and
      // its target under scheduler pressure, turning a real kill into a
      // bounded timeout without weakening the healer regression assertion.
      const live = s.monsters.find(monster => monster.uuid === target.uuid);
      if (live && Math.abs(live.x - s.x) <= 1 && Math.abs(live.y - s.y) <= 1) {
        await p.attack(live);
      } else if (live) {
        await p.devTeleport(live.x + 1, live.y);
        await p.attack(live);
      }
      return false;
    }, { timeoutMs: 30000, intervalMs: 400, label: 'a monster kill' });

    assert(won === 'killed', 'killed a pack member within 30s (healer race is winnable)');

    const after = await p.state();
    assert(after.lifecycle === 'alive', `survived the pack (hp ${after.hp.current}/${after.hp.max})`);
    // dev:state can observe the scene removal one WebSocket tick before the
    // combat:hit broadcast reaches the harness. Wait for that real protocol
    // event instead of turning delivery order into a flaky assertion.
    await p.waitFor(() => p.hits.some(hit => hit.died), {
      timeoutMs: 2000,
      intervalMs: 25,
      label: 'combat kill event',
    });
    assert(p.hits.some(hit => hit.died), 'combat log recorded the kill');
  } finally {
    p.close();
  }
}
