/**
 * Chronicles mortal-oath opener: use the live protocol to admit a Scion,
 * enter Old Barrow, approach the authoritative first encounter actor, and
 * prove the first melee hit and kill. Dev teleport is setup-only; combat is
 * resolved by the ordinary player:skill:trigger path.
 */
export default async function chroniclesFirstCombat({ connect, assert }) {
  const guestId = `playtest-chronicles-combat-${Date.now()}`;
  const houseId = `house-combat-${Date.now()}`;
  const scionId = `scion-combat-${Date.now()}`;
  const p = await connect({
    guestId,
    loginPayload: {
      useGuestAccount: true,
      guestId,
      awaitChronicles: true,
    },
  });

  try {
    p.protocolErrors = [];
    p.ws.on('message', raw => {
      try {
        const message = JSON.parse(raw.toString());
        if (message.event === 'player:chronicles:error') p.protocolErrors.push(message.data);
      } catch { /* harness already owns malformed frames */ }
    });
    const waitUpdate = async () => p.waitFor(
      () => p.chroniclesUpdateCount > (p.__chroniclesUpdateBefore || 0),
      { timeoutMs: 5000, label: 'Chronicles mutation acknowledgement' },
    );

    p.__chroniclesUpdateBefore = p.chroniclesUpdateCount;
    await p.emit('player:chronicles:mutate', {
      type: 'found-house',
      house: {
        id: houseId,
        name: 'Combat Ember',
        foundedAt: new Date().toISOString(),
      },
    });
    try {
      await waitUpdate();
    } catch (error) {
      throw new Error(`${error.message}; errors=${JSON.stringify(p.protocolErrors)}; events=${p.events.slice(-12).map(event => event.event).join(',')}`);
    }

    p.__chroniclesUpdateBefore = p.chroniclesUpdateCount;
    await p.emit('player:chronicles:mutate', {
      type: 'add-scion',
      houseId,
      scion: {
        id: scionId,
        name: 'Mortal Asha',
        mortal: true,
      },
    });
    try {
      await waitUpdate();
    } catch (error) {
      throw new Error(`${error.message}; errors=${JSON.stringify(p.protocolErrors)}; events=${p.events.slice(-12).map(event => event.event).join(',')}`);
    }

    const chronicles = p.chroniclesUpdate.chronicles;
    const admitted = await p.selectScion({
      houseId,
      scionId,
      scionName: chronicles.houses
        .find(house => house.id === houseId).scions
        .find(scion => scion.id === scionId).name,
      mortal: true,
    });
    assert(admitted.chronicles?.mortal === true, 'mortal oath is carried into world admission');

    await p.enterZone('dungeon', 'warren');
    const opening = await p.state();
    const target = opening.monsters.find(monster => (
      monster.isAlive !== false
      && monster.behaviour?.encounterInactive !== true
    )) || opening.monsters.find(monster => monster.isAlive !== false);
    assert(target, 'Old Barrow exposes a live opening encounter actor');

    p.devTeleport(Math.round(target.x) + 1, Math.round(target.y));
    await p.waitFor(async () => {
      const state = await p.state();
      return Math.abs(state.x - (Math.round(target.x) + 1)) <= 1
        && Math.abs(state.y - Math.round(target.y)) <= 1;
    }, { timeoutMs: 5000, label: 'approach opening encounter actor' });

    const beforeHits = p.hits.length;
    const startedAt = Date.now();
    let hit = null;
    for (let attempt = 0; attempt < 6 && !hit; attempt += 1) {
      await p.attack(target);
      hit = await p.waitFor(
        () => p.hits.slice(beforeHits).find(event => (
          event.targetId === target.uuid && Number(event.amount) > 0
        )),
        { timeoutMs: 1200, intervalMs: 100, label: 'first mortal-oath combat hit' },
      ).catch(() => null);
    }
    assert(hit, 'mortal-oath first attack deals authoritative damage');

    const kill = await p.waitFor(
      () => p.hits.slice(beforeHits).find(event => event.targetId === target.uuid && event.died),
      { timeoutMs: 8000, intervalMs: 250, label: 'mortal-oath first kill' },
    );
    assert(kill, `mortal-oath first kill resolves within ${Date.now() - startedAt}ms`);
    assert(p.messages.some(message => /slain|defeated/i.test(message)), 'first kill emits a readable server message');
  } finally {
    p.close();
  }
}
