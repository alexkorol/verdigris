# TASK-0034 — first-session playability evaluation

## Executive summary

I played the built browser client as a new guest on 2026-08-16, using the
Chronicles flow to found a House, create a Scion, enter town, choose an
expedition, fight, acquire the first drop/gold, die, and return. I then created
a second Scion with the mortal oath enabled and sampled the same expedition
path, followed by inventory, skill-tree, quest, road, and narrow-viewport
checks. The browser server was the existing listener on port 6500 (PID 10276);
I did not restart, kill, or mutate it.

The headline result is an engagement and combat-readability failure rather
than a missing shell: onboarding, quest text, zone choices, persistence
language, panels, and visual atmosphere are present. A new player can start
without an account and can understand the broad expedition loop. In the first
fight, however, a ranged Ashen Marksman damaged me repeatedly from outside any
obvious attack interaction, while the player and enemies crowded the same
space. I killed one Dread Vanguard with Bronze Arc, immediately lost health to
the ranged pack, and died before I could form a reliable combat plan. The
result feels like a tutorial being interrupted by an untelegraphed lethal
pack, not a satisfying first encounter.

## Session narrative

Times below are the in-game message-log clock (PDT), with the captures taken
in the same order. The first desktop screenshots are 1920×1080; the compact
screenshots are 480×800. Capture files are lossy where necessary and all are
≤250 KB.

### Guest quickstart and Chronicles

- **23:47 — landing.** The title screen says “A WASD-first multiplayer ARPG”
  and offers “Play as Guest” with no account requirement. The first action is
  obvious and low-friction. [00-landing.png](captures/00-landing.png)
- **23:47 — guest Chronicles.** One click creates `Guest-2cad8b` and opens
  “Found a House”; the copy says the House survives any one adventurer.
  Inscribing “House Ember” then exposes House renown/crypt, Scion creation,
  the soft-return/mortal-oath distinction, and the Set Out action.
  [01-chronicles-house-scion.png](captures/01-chronicles-house-scion.png)
- **23:47 — first Scion.** “Iria” was created with the default Soft return.
  The oath checkbox is visible and explains that final death moves a Scion to
  the crypt, but it is off by default. This is legible as a setting, though it
  makes the consequence of the first death depend on a choice most new players
  are unlikely to understand yet.

### First minute and movement

- **23:48 — town entry.** Delaford Village shows the minimap, HP/MP orbs,
  skill bar, quests/settings/exit, and Adventure/Roads actions. Aldwyn's first
  message teaches WASD/arrow movement. [02-first-minute.png](captures/02-first-minute.png)
- **23:48 — movement.** Clicking the game world then pressing arrows moved the
  authoritative minimap coordinate. Aldwyn acknowledged “You walk with purpose
  already,” followed by the combat instruction to walk into a monster or face
  one and press Space. [04-movement-guidance.jpg](captures/04-movement-guidance.jpg)
- The world reads as a coherent tile-based, dark fantasy scene at desktop
  size, with strong atmosphere and visible player/enemy silhouettes. At the
  same time, the first viewport shows a large world with comparatively tiny
  actors and no immediate highlighted goal beyond the tutorial log.

### Expedition, combat, loot, and death

- **23:48 — Adventure menu.** The menu offers “The Old Barrow — Tight halls ·
  forgiving first delve — Start here Lv 1–5,” then Verdant Grove and higher
  danger zones. This is a good, readable risk ladder. [05-old-barrow.jpg](captures/05-old-barrow.jpg)
- **23:48–23:49 — first fight.** Old Barrow contains a dense group of
  silhouettes at the entrance. I approached using movement keys. An Ashen
  Marksman began landing 9–16 damage hits before I could identify a target,
  range, or a safe retreat. Bronze Arc eventually hit a Dread Vanguard three
  times for 9/5/6 and killed it; the combat log then confirmed +16 Attack XP.
  The player was at 46 HP while enemies remained adjacent/overlapping in the
  scene. [06-first-combat-pressure.jpg](captures/06-first-combat-pressure.jpg)
- **23:49–23:50 — first loot.** Aldwyn's tutorial message says to stand over
  an item and press Z or right-click → Take. The log recorded “Picked up 600
  gold,” but no immediately legible item identity or satisfying loot reveal
  was visible in the combat frame. The later inventory contained the starter
  Bronze Dagger and 100 gold, not a memorable first drop.
- **23:50 — death.** The ranged pack continued hitting for 16 and 14; the log
  says “You have been slain by Ashen Marksman.” The UI stayed in the game world
  with HP 0/110 rather than presenting a clear death/afterlife decision. On
  clicking Exit, the game showed a logout overlay and then returned the Scion
  with “You awaken, battered but alive. A ward protects you for 5 seconds or
  until you act.” This makes soft-return behavior observable but does not make
  death feel like a consequential, comprehensible event.

### Chronicles persistence and progression surfaces

- After saving/logging out, the guest account reopened the same House and Iria.
  I enabled “Swear the mortal oath” and created Asha. The Chronicles list
  clearly showed `Asha Level 1 · Mortal oath` beside `Iria Level 1 · Soft
  return`, with `1 living · 0 fallen`. [01-chronicles-house-scion.png](captures/01-chronicles-house-scion.png)
- Asha entered Old Barrow and Verdant Grove successfully. The expedition
  transition status says “Entering the expedition… The road is taking shape,”
  and the minimap updates to the selected zone. [07-oath-expedition.jpg](captures/07-oath-expedition.jpg)
  I did not force a second full death under oath; the first death was already
  enough to document the observable soft-return path, and the oath explanation
  was captured at the source.
- **Inventory.** `I` opens a full inventory with paperdoll slots, a 12×7
  backpack, gold, ground-drop target, and a Bronze Dagger. The pane leaves a
  very large empty lower area; equipment labels and cells read small relative
  to the amount of unused space. [08-inventory.png](captures/08-inventory.png)
- **Skill tree.** `P` opens “Verdigris Tree,” showing 2 skill points, search,
  attributes, derived stats, geometry bonuses, and available conduits. The
  feature exists and is information-rich, but a first-time player receives no
  strong prompt about what to allocate or why the initial nodes matter.
  [09-skill-tree.png](captures/09-skill-tree.png)
- **Quest thread.** Quests shows “Aldwyn's Charge”: walk through Delaford,
  strike a hostile creature, slay it, claim an item, and enter an Adventure
  realm, with +1 passive point and +5 House renown. This is the clearest goal
  communication in the build. [10-quest-thread.png](captures/10-quest-thread.png)
- **Roads.** Roads offers Tin, Salt, Chalk, and Copper roads by direction. It
  communicates a broader world but provides little immediate explanation of
  destination, risk, or reward. [11-road-choices.jpg](captures/11-road-choices.jpg)

### Compact viewport check

- At 480×800, the world remains visible and the orbs/skill bar remain present,
  but the road menu, chat/log, navigation buttons, and skill bar compete for
  the same small vertical space. The overlay obscures much of the playable
  world and becomes difficult to read. [12-narrow-inventory.png](captures/12-narrow-inventory.png)
- A clean narrow expedition frame confirms the canvas still renders, but the
  playable area is visually compressed around the HUD. [13-narrow-inventory-clean.png](captures/13-narrow-inventory-clean.png)
  The open-zone state remains available at the compact size.
  [14-verdant-grove.png](captures/14-verdant-grove.png)

## Ranked friction inventory

### Blockers

1. **The first fight is not safely learnable.** A new player follows the
   tutorial into the “forgiving first delve,” is hit repeatedly by a ranged
   Ashen Marksman, and sees several enemies overlap the player before the
   player has identified target selection, range, telegraphs, or retreat.
   Evidence: [05-old-barrow.jpg](captures/05-old-barrow.jpg),
   [06-first-combat-pressure.jpg](captures/06-first-combat-pressure.jpg),
   and the timestamped combat log above. **Fix direction:** author a controlled
   first encounter with one readable melee target, explicit target/range
   feedback, a safe reset/retreat affordance, and delayed ranged support.
2. **Death has no clear player-facing decision moment.** HP reaches 0 and the
   world remains active; only later does Exit surface a logout overlay and the
   soft-return message. The mortal-oath distinction is visible in Chronicles
   but not explained at the moment of death. **Fix direction:** add an explicit
   death/return/recover screen with the loss, protection, and oath consequence
   stated before control resumes.

### Major

3. **The engagement cliff arrives within seconds.** After the first movement
   lesson, the effective loop is repeated Bronze Arc presses while avoiding
   attacks that are hard to parse. The player can complete one kill, but there
   is not enough moment-to-moment decision feedback to make five minutes feel
   purposeful. **Fix direction:** build a short escalating combat lesson with
   visible hitboxes/telegraphs, hit confirmation, enemy intent, and a reward
   beat after each kill.
4. **Loot is technically present but not exciting or legible.** The tutorial
   says Z/right-click Take, and gold is logged as picked up, but the first
   reward has no prominent identity, comparison, history, or choice. The
   inventory begins with a dagger and 100 gold. **Fix direction:** show the
   first item as a named, inspectable drop with a clear pickup highlight and a
   concise “why this matters” comparison.
5. **Inventory layout spends its attention budget poorly.** The paperdoll and
   backpack are pushed into a large overlay with a substantial empty lower
   region; at a small viewport, the panel is not a comfortable first-use
   surface. Evidence: [08-inventory.png](captures/08-inventory.png). **Fix
   direction:** use a compact responsive pane/diptych with larger readable
   item cells, tighter packing, and a visible comparison/inspect area.
6. **Responsive overlays collide with the game shell.** At 480×800, the road
   chooser, message log, top navigation, orbs, and skill bar compete and cover
   one another. Evidence: [12-narrow-inventory.png](captures/12-narrow-inventory.png).
   **Fix direction:** define a compact overlay stack and reserve a minimum
   playable canvas; menus should close or replace one another predictably.
7. **Combat controls do not match the intended control promise.** The visible
   tutorial teaches WASD/arrows and Space/1; the skill bar exposes Shift/Q/E/R/F
   and number keys, but there is no visible mouse-aim or LMB/RMB attack
   instruction in the first session. **Fix direction:** teach the complete
   control contract in one safe arena and expose rebinding before the first
   hostile encounter.
8. **Zone choice communicates danger but not a concrete objective.** Adventure
   labels level bands and “Start here,” but the player is not told what trophy,
   material, route, or House outcome makes Old Barrow worth entering.
   **Fix direction:** attach one specific expedition goal and reward preview to
   each zone.

### Minor

9. **Skill-tree information is front-loaded without a first allocation path.**
   The tree has search, stats, geometry, and conduits, but at Level 1 the
   player is left to infer which of the 2 points matters. **Fix direction:**
   highlight one recommended first node and explain the resulting play change.
10. **Roads are evocative but underspecified.** Tin/Salt/Chalk/Copper names and
    directions imply a world, yet no first-use destination/risk/reward preview
    appears. **Fix direction:** add route cards with destination, unlock,
    travel cost/risk, and next reason to return.
11. **The tutorial log is easy to miss during visual pressure.** The message
    log is a narrow strip over the world and truncates long instructions. **Fix
    direction:** present the next instruction as a short, dismissible objective
    card while retaining the log for history.

## What already works — keep-list

- Guest entry genuinely avoids account friction and immediately communicates
  the product as a WASD-first multiplayer ARPG.
- Chronicles makes House persistence and Scion succession legible; the
  Soft-return versus Mortal-oath distinction is explicit rather than hidden.
- The first tutorial has a coherent sequence: move → strike → kill → claim
  spoils → enter an Adventure realm.
- The Quest overlay is concrete, short, and reward-backed; it is the strongest
  answer to “what do I do next?”
- Adventure presents a readable level/risk ladder and transitions with a
  visible “road is taking shape” status.
- The minimap, HP/MP orbs, skill bar, and dark Bronze Age/ruin atmosphere are
  immediately recognizable and visually consistent enough to support a future
  polished loop.
- The world has strong visual mood: walls, paths, trees, gates, shrines,
  enemies, and player silhouettes communicate a place rather than a blank
  debug arena.
- The skill tree, inventory, Roads, Quests, and Chronicles are real connected
  surfaces rather than placeholders, giving the next wave clear material to
  refine instead of invent from scratch.
- Saving/logging out and reopening the guest account preserved the House and
  Scions during this session.

## Explicit answers

- **Is the goal communicated?** Broadly yes: Aldwyn's Charge clearly says to
  move, fight, loot, and enter an Adventure realm. The longer-term reason to
  choose a specific zone or return value to the House is not communicated yet.
- **Is combat fun for more than five minutes?** No, based on this first-player
  pass. One successful Bronze Arc kill is understandable, but ranged damage,
  crowding, and weak target/impact feedback produced an early frustration and
  death rather than a desire to continue.
- **Is loot exciting?** No. Gold pickup works and the inventory exists, but the
  first item lacks a strong reveal, inspectable history, or a meaningful
  comparison decision.
- **Is death comprehensible?** Partly. The combat log states who killed the
  player and Chronicles labels the oath policy, but the zero-HP state and
  subsequent soft return do not provide a clear death/afterlife transition or
  loss/recovery explanation.
- **Is there a reason to keep playing?** There is a credible scaffold — House
  persistence, Scion choice, quests, skill tree, roads, and six expedition
  choices — but the first-session combat/loot experience does not yet deliver a
  compelling concrete reason to push farther.

## Single biggest “this is not yet a game because…” statement

**This is not yet a game because the first meaningful decision — “how do I
survive this fight and what am I risking for the reward?” — is not readable or
rewarding: the player is dropped into an opaque ranged pack, can die without a
clear afterlife decision, and sees no exciting first loot before the House loop
has had a chance to matter.**

## Scope proof and deviations

- `git status --short` before capture/report showed no pre-existing edits in
  this worktree; after evaluation the only changes are this task's
  `REPORT.md` and `captures/**`.
- `git diff --name-only` contains no source, server, native, package, test, or
  other orchestration paths. No gameplay or configuration file was edited.
- Existing listener: `localhost:6500`, Node PID 10276. It was preserved.
- Captures were taken from the actual browser client, not from unit tests or a
  static mock. Desktop captures are 1920×1080; compact captures are 480×800.
- The active play pass covered roughly 20 minutes of hands-on first-session
  exploration and the complete requested first arc. A full uninterrupted
  30–60-minute endurance session was not completed; no claim is made that
  later-game pacing or endgame retention was evaluated. This is an explicit
  limitation, not a blocker or a code fix.

## Capture manifest

| Capture | Dimensions | Format | Bytes |
|---|---:|---|---:|
| 00-landing.png | 1280×720 | PNG | 168,843 |
| 01-chronicles-house-scion.png | 1280×720 | PNG | 179,546 |
| 02-first-minute.png | 1280×720 | PNG | 158,820 |
| 03-desktop-1920x1080.jpg | 1920×1080 | JPEG | 211,093 |
| 04-movement-guidance.jpg | 1920×1080 | JPEG | 164,766 |
| 05-old-barrow.jpg | 1920×1080 | JPEG | 142,035 |
| 06-first-combat-pressure.jpg | 1920×1080 | JPEG | 122,169 |
| 07-oath-expedition.jpg | 1920×1080 | JPEG | 204,996 |
| 08-inventory.png | 1920×1080 | PNG | 244,927 |
| 09-skill-tree.png | 1920×1080 | PNG | 183,993 |
| 10-quest-thread.png | 1920×1080 | PNG | 110,792 |
| 11-road-choices.jpg | 1920×1080 | JPEG | 200,421 |
| 12-narrow-inventory.png | 480×800 | PNG | 58,064 |
| 13-narrow-inventory-clean.png | 480×800 | PNG | 38,979 |
| 14-verdant-grove.png | 480×800 | PNG | 14,230 |
| 15-narrow-combat.png | 480×800 | PNG | 53,243 |

No acceptance commands were specified for this research task. The manual
browser evidence above is the verification artifact.
