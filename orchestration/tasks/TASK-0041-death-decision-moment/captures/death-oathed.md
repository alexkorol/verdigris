# TASK-0041 capture — oathed death

Variant: mortal Chronicles Scion (`mode: hard`, `state: permadead`).

The private `player:death-summary` frame rendered the full-screen **The Scion
has fallen** overlay with:

- `What leaves this run`: Bronze Dagger, Coins ×100
- `Recovered to the House pool`: Bronze Dagger, Coins ×100
- `Next`: The Chronicles — choose a successor
- oath framing: “The Chronicles will keep this name. Continue to choose a
  living successor.”
- one primary action: **Return to the Chronicles**

The action emits the existing `player:chronicles:return` envelope with the
authoritative House/Scion IDs. No client-side transfer or succession record is
created.

Evidence: `tests/unit/death-decision.spec.js` (oathed D-106 projection and
final-death handoff assertions), focused run 5/5; full unit run 121 files /
773 tests.

Rendered 1920×1080 JPEG: [death-oathed-1920x1080.jpg](death-oathed-1920x1080.jpg)
