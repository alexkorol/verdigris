/**
 * TASK-0052 NEGATIVE PROOF (scratch, never committed to scenarios/): the
 * road purse is suppressed — the bounded wait requires a purse that can
 * never land (>= 1e9 gold) — so the hardened house-treasury scenario MUST
 * fail at the wait. If this ever passes, the purse wait has been weakened.
 */
import { loadMode } from '../timing.mjs';

export default async function houseTreasuryNegative({ connect, assert }) {
  const p = await connect({
    guestId: 'playtest-house-treasury-neg',
    houseName: 'Ledger',
    scionName: 'Tala Testborn',
  });
  try {
    // SUPPRESSED road purse: no real scion can carry 1e9 gold, so this
    // stands in for the purse state never arriving.
    const initial = await p.waitFor(async () => {
      const snapshot = await p.state();
      const carried = snapshot.inventory
        .filter(item => item.id === 'coins')
        .reduce((sum, item) => sum + item.qty, 0);
      return carried >= 1e9 ? snapshot : false;
    }, {
      timeoutMs: loadMode ? 12000 : 8000,
      label: 'starting road purse to land',
    });
    const coinsBefore = initial.inventory
      .filter(item => item.id === 'coins')
      .reduce((sum, item) => sum + item.qty, 0);
    assert(coinsBefore >= 100, 'the scion has carried gold available for a House deposit');
  } finally {
    p.close();
  }
}
