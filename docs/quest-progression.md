# Quest progression

Quest progress is server-authored and persisted with the player. Definitions
live in `server/shared/quests.js` so server rules and the Vue journal use the
same titles, objectives, and reward copy.

## Campaign vertical slice

`aldwyns-charge` advances only in this order:

1. move through Delaford;
2. strike a hostile creature;
3. slay it;
4. pick up a real world item;
5. enter an Adventure realm.

The existing gameplay handlers report these events through
`notifyProgression`, which advances both the onboarding prompts and the quest
without letting either client author completion. On completion the server:

- adds one point to the quest portion of the 123-point passive-tree economy;
- records `Answered Aldwyn's Charge` on the living Scion;
- adds five authoritative House renown, idempotently by deed;
- force-saves player progress and pushes a live quest-journal update.

`proof-of-temper` begins immediately afterward. It requires a real elite kill,
guarantees one native Vessel only while that exact quest objective is current,
then verifies the same item entered the inventory and was truly equipped. It
awards the second passive point and ten House renown.

`the-pale-crown` is the first contextual campaign contract:

1. enter **Weir Crypt** specifically (another crypt-themed layout does not
   count);
2. defeat **The Pale Sovereign** in a crypt instance;
3. use the opened stairs to reach floor two of that crypt expedition.

Delve and kill events carry server-derived zone, layout, theme, depth, and
monster identity context. Objective criteria are matched only on the server
and are stripped from the client journal snapshot. The contract awards the
third passive point, 15 House renown, and the Scion deed
`Broke the Pale Sovereign's seal`.

`rot-in-the-reeds` continues the campaign into **Marsh of Reeds**. It requires
the exact named zone, the marsh boss **The Rotfather**, and an authoritative
return from that expedition to the surface. Capturing the departed zone before
the instance is torn down prevents another marsh-like realm—or a client-authored
message—from satisfying the return. The commission awards the fourth passive
point, 20 House renown, and the deed `Ended the rot beneath the reeds`.

## Four-road covenant

The second campaign act carries the House standard across the procedural world
web in a fixed narrative order:

1. **Oath of Tin** — enter a Tin Road holding, defeat its generated Warden,
   and return to the Crossroads;
2. **The Salt Reckoning** — secure a Salt Road holding and return its eastern
   trade route to Rhea's ledger;
3. **The Chalk Vigil** — enter the southern grave road, defeat its Warden,
   and return alive;
4. **The Copper Testament** — claim a western holding and return to seal the
   four-road covenant.

Each commission awards one passive-tree quest point and 25/30/35/40 House
renown respectively. Completing Copper raises the campaign total to eight
quest points and opens the deeper covenant; it does not prematurely seal the
campaign or mint an endgame tablet.

Road progress is House-persistent. Cleared nodes restore on later Scion
admissions, and a server-side parent check rejects direct requests for barred
deeper holdings. Re-entering an already-cleared node satisfies the clear rite
without inventing another Warden, so a resumed campaign cannot deadlock.

## The Deep Roads

The third campaign act returns to the same four House-charted roads at tier
two, where each branch has a canonical story Warden rather than a generic
elite identity:

1. **The Quarry Saint's Canon** — break the Quarry Saint beneath Tin;
2. **The Brine Widow's Tithe** — end the Brine Widow's levy beyond Salt;
3. **The Bell Beneath Chalk** — still the Ossuary Bell under the graves;
4. **The Cinder Judgment** — answer the Cinder Judge beyond Copper.

Each commission requires exact tier-two entry, that holding's authoritative
Warden clear, and a living return to the Crossroads. Road tier is also the
instance depth used by monster and loot scaling, so these revisits are
mechanically deeper rather than renamed tier-one floors. They award one quest
point and 45/50/55/60 House renown respectively. The Cinder return raises the
campaign total to twelve, seals the House campaign, and grants the first
consumable charted tablet. Quest snapshots also publish the campaign total and
current act name/progress so clients do not infer chapter boundaries. A later
Scion born into that sealed House inherits the twelve campaign quest points,
completed Chronicle record, and endgame access; succession cannot strand the
reserved passive-tree budget on a fallen predecessor.

## Wayfinder Mastery

Campaign completion opens a House-wide endgame ledger containing 64 distinct
objectives: four charted-tablet families across tiers 1-16. The exact family,
tier, modifiers, and objective key travel with the consumable tablet. Killing
its Seal-Bound Warden records the objective only on its first clear, grants
three House renown per map tier, and continues to count repeat expeditions
without duplicating mastery rewards.

The mastery set is validated and persisted on the House Chronicle, so every
later Scion inherits it. The next-tablet tier-ascent chance starts at 35% and
rises by one percentage point per two mastered objectives, capped at 65%.
This makes family breadth a real sustain strategy rather than a cosmetic
checklist. The authoritative client snapshot identifies new versus mastered
tablets before the owner consumes them.

Guest saves include the quest state. Account saves send it as `questsData`.
Malformed or unknown persisted quest entries are discarded and quest points
are clamped to the reserved 23-point quest budget.

## Native client contract

The native protocol publishes only presentation-safe quest metadata; trigger
names, zone criteria, boss matching, and depth requirements remain server-only.
The remote client mirrors admission, snapshot, and live update envelopes into
the `ClientQuestState` journal model. `J` opens Chronicle Commissions, while
the compact town tracker shows the same current objective.

For named Chronicles characters, the current quest index, objective cursor,
completed IDs, and awarded quest points are checkpointed on the living Scion.
Re-admitting that Scion restores the exact checkpoint. House renown and the
campaign-complete/endgame unlock remain House-wide.
