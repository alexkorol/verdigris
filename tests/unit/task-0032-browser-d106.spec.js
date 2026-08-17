/** @vitest-environment node */

import fs from 'node:fs';
import os from 'node:os';
import path from 'node:path';
import { afterEach, describe, expect, it } from 'vitest';
import { ChroniclesRepository } from '#server/core/repositories/chronicles-repository.js';
import { ChroniclesStore } from '#server/core/services/chronicles-store.js';
import { collectCarriedRecovery } from '#server/core/services/chronicles.js';
import world from '#server/core/world.js';

const disposables = [];
afterEach(() => {
  disposables.splice(0).forEach((file) => fs.rmSync(file, { force: true, recursive: true }));
});

const item = (uuid, id = 'bronze-sword') => ({
  id, uuid, name: id, stackable: id === 'coins', trophies: id === 'bronze-sword' ? [{ trophyId: 'boar' }] : [],
});

describe('TASK-0032 browser D-106/D-109 alignment', () => {
  it('transfers every equipped/pack item and carried trophy without losing socketed data', () => {
    const result = collectCarriedRecovery({
      scionId: 'scion-1', username: 'Morrow', houseId: 'house-1',
      wear: { right_hand: item('equipped-1') },
      inventory: { slots: [item('pack-1'), item('coins-1', 'coins')] },
      trophies: [{ trophyId: 'fragment-1', quantity: 2 }],
    });
    expect(result.items.map(entry => entry.uuid)).toEqual(['equipped-1', 'pack-1', 'coins-1']);
    expect(result.items[0].trophies).toEqual([{ trophyId: 'boar' }]);
    expect(result.trophies).toEqual([{ trophyId: 'fragment-1', quantity: 2 }]);
  });

  it('persists an all-carried death transfer and claims relics/trophies atomically', () => {
    const repository = new ChroniclesRepository({ dbFile: ':memory:' });
    const house = repository.foundHouse('account:task32', 'Ashford');
    const scion = repository.createScion('account:task32', house.houseId, 'Morrow');
    repository.beginRun('account:task32', house.houseId);
    const burial = repository.entombScion({
      accountId: 'account:task32', houseId: house.houseId, scionId: scion.scionId,
      level: 8, relicItems: [item('equipped-1'), item('pack-1')],
      trophies: [{ trophyId: 'boar', quantity: 2 }],
    });
    expect(burial.relicCount).toBe(2);
    expect(burial.trophyCount).toBe(1);
    expect(repository.entombScion({
      accountId: 'account:task32', houseId: house.houseId, scionId: scion.scionId,
    }).idempotent).toBe(true);
    repository.beginRun('account:task32', house.houseId);
    repository.beginRun('account:task32', house.houseId);
    repository.beginRun('account:task32', house.houseId);
    const relic = repository.drawEligibleRelic(['account:task32']);
    expect(repository.claimRelic(relic.id, { scionId: 'successor' })).toBe(true);
    const trophy = repository.drawEligibleTrophy(['account:task32']);
    expect(repository.claimTrophy(trophy.id, { scionId: 'successor' })).toBe(true);
    repository.close();
  });

  it('migrates an old SQLite relic table and removes duplicate UUID rows deterministically', () => {
    const temp = fs.mkdtempSync(path.join(os.tmpdir(), 'verdigris-task32-migration-'));
    const file = path.join(temp, 'chronicles.sqlite');
    const first = new ChroniclesRepository({ dbFile: file });
    const house = first.foundHouse('account:migration', 'Ashford');
    const scion = first.createScion('account:migration', house.houseId, 'Morrow');
    first.db.prepare(`INSERT INTO chronicle_relics
      (id, house_id, source_scion_id, origin_scion_name, item_json, status, eligible_run, created_at)
      VALUES (?, ?, ?, ?, ?, 'circulating', 0, ?), (?, ?, ?, ?, ?, 'circulating', 0, ?)`)
      .run('r1', house.houseId, scion.scionId, 'Morrow', JSON.stringify(item('same-uuid')), '2026-01-01',
        'r2', house.houseId, scion.scionId, 'Morrow', JSON.stringify(item('same-uuid')), '2026-01-02');
    first.close();
    const migrated = new ChroniclesRepository({ dbFile: file });
    expect(migrated.db.prepare('SELECT COUNT(*) AS count FROM chronicle_relics WHERE item_uuid = ?').get('same-uuid').count).toBe(1);
    migrated.close();
    disposables.push(temp);
  });

  it('keeps JSON Chronicle pools compatible with old saves and requeues one candidate once', () => {
    const temp = fs.mkdtempSync(path.join(os.tmpdir(), 'verdigris-task32-json-'));
    const store = new ChroniclesStore({ storeFile: path.join(temp, 'chronicles.json') });
    const state = {
      version: 3,
      houses: [{ id: 'house-old', name: 'Ashford', scions: [{ id: 'scion-old', name: 'Morrow' }], crypt: [] }],
      activeHouseId: 'house-old', activeScionId: 'scion-old',
    };
    expect(store.save('account:old', state).ok).toBe(true);
    const burial = store.entomb('account:old', { houseId: 'house-old', scionId: 'scion-old' }, {
      relicItems: [item('pool-1')], trophies: [{ trophyId: 'trophy-1' }],
    });
    expect(burial.state.houses[0].relicCandidates).toHaveLength(1);
    const dropped = store.beginRelicDrop('account:old', { houseId: 'house-old' });
    expect(store.requeueCandidate('account:old', dropped.relic.id)).toMatchObject({ ok: true });
    expect(store.requeueCandidate('account:old', dropped.relic.id).ok).toBe(false);
    disposables.push(temp);
  });

  it('retires an instance with active membership and one-time candidate requeue', () => {
    const scene = world.createInstance('task32-party', {
      items: [
        { uuid: 'ground-relic', chroniclesRelic: { id: 'relic-1' } },
        { uuid: 'ground-trophy', chroniclesTrophy: { id: 'trophy-1', trophyId: 'boar' } },
      ],
    });
    scene.players.push({ uuid: 'member-1' });
    const result = world.retireInstance('task32-party');
    expect(result.activeMembers).toEqual(['member-1']);
    expect(result.requeuedCandidates).toEqual([
      expect.objectContaining({ id: 'relic-1', kind: 'relic' }),
      expect.objectContaining({ id: 'trophy-1', kind: 'trophy' }),
    ]);
    expect(scene.items.every(item => item.recoveryRetired === true)).toBe(true);
    expect(world.retireInstance('task32-party').retired).toBe(false);
  });
});
