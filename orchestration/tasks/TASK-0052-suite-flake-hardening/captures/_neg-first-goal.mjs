/**
 * TASK-0052 NEGATIVE PROOF (scratch, never committed to scenarios/): the
 * objective push is suppressed — Talk is never actually sent — so the
 * hardened first-goal wait MUST time out. If this ever passes, the bounded
 * wait/assert has been weakened.
 */
import { loadMode } from '../timing.mjs';

export default async function firstGoalNegative({ connect, assert }) {
  const p = await connect({
    guestId: 'playtest-first-goal-neg',
    houseName: 'House Firstlight',
    scionName: 'Quest Testborn',
  });

  try {
    const town = await p.state();
    const aldwyn = town.npcs.find(npc => npc.name === 'Aldwyn the Guide');
    assert(aldwyn, 'Aldwyn is present in the authoritative town scene');
    const approachX = aldwyn.x + 1;
    p.devTeleport(approachX, aldwyn.y);
    await p.waitFor(async () => (await p.state()).x === approachX, { label: 'Aldwyn approach' });

    const menu = await p.rightClick(aldwyn.x, aldwyn.y);
    const talk = menu.find(entry => entry.action?.actionId === 'player:npc:talk');
    assert(talk, 'Aldwyn exposes a Talk interaction in town');

    const messagesBefore = p.messages.length;
    // SUPPRESSED: no Talk frame is ever sent (neither the initial send nor
    // the resend), simulating the objective push never arriving.
    await p.waitFor(async () => {
      const named = p.messages.slice(messagesBefore).some(message => (
        /warden/i.test(message) && /come back|return/i.test(message)
      ));
      return named;
    }, {
      timeoutMs: loadMode ? 12000 : 8000,
      label: 'Aldwyn names the first-Warden objective',
    });
    assert(true, 'the accepted goal explicitly asks for a first Warden');
  } finally {
    p.close();
  }
}
