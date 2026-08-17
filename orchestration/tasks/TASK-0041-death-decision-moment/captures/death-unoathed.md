# TASK-0041 capture — unoathed death

Variant: soft-return Scion (`mode: soft`, `state: awaiting-respawn`).

The private `player:death-summary` frame rendered the full-screen **You have
fallen** overlay with:

- `What leaves this run`: No carried value is lost.
- `Protected on return`: Bronze Spear, Linen Sash
- `Next`: old-barrow:entrance
- no succession text or mortal-oath consequence
- one primary action: **Continue**

The action dismisses the overlay and refocuses the game canvas. Server-side
respawn remains authoritative; the client does not move the player or alter
the recovery pool.

Evidence: `tests/unit/death-decision.spec.js` (unoathed D-106 projection and
overlay input assertions), focused run 5/5; full unit run 121 files /
773 tests.
