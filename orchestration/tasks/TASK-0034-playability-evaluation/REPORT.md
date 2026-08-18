---
task: TASK-0034
state: REVIEW_REQUESTED
coordinator: kimi
worker: kimi-code-cli
branch: codex/TASK-0034-playability-evaluation
base_commit: 056746b
---

# TASK-0034 report — first-session playability evaluation (D-110)

Two full session arcs played against a fresh build of the program tip
(`056746b`, `npm run build`, dev server on :9777 — the owner's :6500 instance
untouched): **Arc A** guest quickstart (`?play`) and **Arc B** the Chronicles
House/Scion path, each ~10 minutes of driven play with 46 timestamped captures
(`captures/`) and a beat-by-beat `captures/session-log.md`. Headless Chromium;
where headless fidelity is doubtful I say so and downgrade the claim. Every
friction item was re-verified against the code before ranking. Read-only:
`git status` shows only this task folder.

## 1. Session narrative (compressed)

**Arc A — guest quickstart.** Landing page (`01`) is handsome and the CTA is
unambiguous. `?play` drops a stranger straight into the village with Aldwyn's
tutorial chat (`02`). The first minute shows a full HUD with zero explanation
of it (`03`). WASD works. Talking to Aldwyn failed — right-clicking him (and
everything else) only ever offered "Walk here / Examine / Cancel" (`05`).
Walking north to the Old Wood gate never left the village (`06`). "Fight #1"
was 77 s of skill spam at no monster (`07-08`). Looting failed completely —
nine ground items, no Take action anywhere (`09`). Inventory and equip work
well (`10-12`). The quest panel is excellent (`13`). The Adventure menu is
excellent (`14`) — but entering Verdant Grove bounced the party back to the
village within seconds: "The party returns to the surface." (`15`, chat at
07:23). "Fight #2" was again in the village, producing 13 "Not enough mana."
messages (`16-17`). Three minutes standing in wolf territory produced no
death — HP never dropped (`18-19`, HP 110/110 throughout).

**Arc B — Chronicles.** Onboarding copy is clear; House founding and Scion
naming are smooth; the mortal oath is honestly explained ("OFF BY DEFAULT
WHILE BALANCE IS STILL BEING TUNED") (`22-27`). Set Out works (`28`). In-game,
the House/Scion identity vanishes from the HUD (`29`). Same looting failure,
same mana spam, same unkillable-at-1 (`30-38`). Natural death being
unreachable, death was forced via the playtest `dev:kill` path: the client
faded directly to the Chronicles screen — no death banner, no memorial beat,
no chat message (`40-41`). The crypt ledger is genuinely good: "Mortalis,
level 1 — Heirloom: Bronze Dagger · awaiting an heir" (`42`). The successor
"Mortalis II" spawns fresh with a starter dagger; nothing else carries over
(`43-45`).

## 2. Ranked friction inventory

### Blockers (a new player stalls or is ejected in the first 5 minutes)

**B1. The tutorial quest is uncompletable: ground pickup is undiscoverable.**
Quest step "Claim an item from the ground" (capture `13`); both arcs failed it
— right-click on ground items offers only "Walk here | Cancel" (`09`, `35`),
and the sole pickup path is the undocumented Z/G "grab" hotkey
(`src/components/GameCanvas.vue:530-535` → `player:take:underfoot`,
`server/player/handlers/actions/index.js:1277`). Nothing in Aldwyn's guidance,
the HUD, or any menu names it. *Fix direction: add "Take" to the ground-item
context menu, surface the grab key in Aldwyn's script and a HUD hint.*

**B2. The first Adventure-zone entry ejects you.** `14-15` + chat: clicking
Verdant Grove ("READY Lv 1–6") landed the player in the instance and returned
them to the village within seconds ("The party returns to the surface." —
party-framed, for a solo player). Mechanism confirmed:
`checkStairTransitions` auto-returns a depth-1 party standing on the
`stairsUp` tile (`server/player/handlers/party.js:598-638`); the entry spawn
is on/adjacent to the entry stairs. `playtest/scenarios/movement.mjs` guards
a mid-walk variant of this regression, but menu-entry in a fresh session
still bounced. *Fix direction: spawn off the stairs tile, entry grace period,
solo-appropriate copy.* (Headless driver may have held a movement key —
reproduce on hardware before speccing.)

### Major (the loop technically works but feels broken or absent)

**M1. Combat never visibly connected.** Across ~5 minutes of fights in two
arcs: 0 kills, no damage numbers, no monster HP bars, no hit feedback visible
in any capture (`07-08`, `16-17`, `33-34`, `37-38`). Some of this is the
driver never reaching a real monster (see B2) — but a level-1 player also
cannot tell whether contact+Space is doing anything. *Fix direction: verify
on hardware; if real, damage numbers/HP feedback on contact hits are the
cheapest fix.* **Combat-fun verdict: unproven, trending negative.**

**M2. The mana economy fights the onboarding kit.** Cinder Fan costs 12 mana
on a 6 s cooldown (`server/shared/skills/index.js:32-41`); level-1 pool 90;
regen is 3 %/2 s ≈ 1 mana/s (`server/core/combat/regeneration.js:4-9`). Seven
on-cooldown casts empty the pool, then the quickbar shows the skill READY
while the server answers "Not enough mana." — 13–25 chat spams per session
(`17` MP orb black at 8/90; readiness reflects cooldown only, rejection at
`server/core/combat/index.js:651-655`). *Fix direction: dim unaffordable
skills on the bar; align cooldown with sustainable regen.*

**M3. Death — the product's only loss event (D-109) — is unreachable at level
1.** Standing defenseless in wolf territory for 3 minutes: HP 110/110 at the
end, both arcs (`18-19`, `39`). Out-of-combat regen (2 %/2 s after 8 s,
`regeneration.js:6-8`) outpaces starter-monster damage. A new player never
sees the respawn flow, the ward, or any stakes. *Fix direction: starter-zone
damage vs regen tuning pass.*

### Minor (polish/legibility)

- **m1. Tutorial chat is truncated** behind a one-line ticker ("Aldwyn the
  Guide: Steel ans… ＋badge", `03`, `07`) — the only instructions the game
  gives are easy to miss. *Fix: expandable chat or tutorial toast queue.*
- **m2. House/Scion identity vanishes in-game** — after Set Out the only
  identity on screen is the username; the Chronicles fantasy disappears until
  you die (`29`). *Fix: House name in the HUD/status line.*
- **m3. Permadeath has no ceremony** — fade straight to the ledger; the
  client has no listener for `chronicles:scion-fallen` (confirmed in the
  TASK-0031 audit); the death moment is silent (`40-41`). *Fix: a death
  interstitial ("Mortalis has fallen…") before the ledger.*
- **m4. Dark navy/black rectangles read as holes or bugs** in the village
  plaza and the pond slab (`03`, `07`, `14`, `19`). *Fix: identify whether
  shadow, water, or unlit bake; adjust.*
- **m5. Movement speed unverifiable headless** — observed ~1.1 tiles/s, but
  the server cadence is 150 ms/tile ≈ 6.7 tiles/s
  (`server/shared/movement.js:1`); the headless client throttles input.
  Verify feel on hardware; note 6.7 t/s is far above the 2.2 t/s pace the
  native/slice ARPG reference settled on.

## 3. What already WORKS (keep-list)

Landing page and guest CTA (`01`); Chronicles onboarding flow and mortal-oath
honesty (`22-27`); Aldwyn's guide script and the Aldwyn's Charge quest panel
with objectives + reward (`13`); the Adventure expedition menu — level bands,
DANGER labels, "START HERE" (`14`); quickbar tooltips with hotkeys/cooldowns
(`03`); HP/MP orb HUD; inventory equip flow (`10-12`); the crypt ledger with
heirloom provenance (`42`); D-109 disconnect safety (TASK-0020, not re-tested
here); renderer toggle F6. None of the fixes above should disturb these.

## 4. The single biggest statement

**This is not yet a game because the tutorial's own verb chain is broken at
three of five links**: it tells you to fight (combat never visibly connects),
to claim an item (pickup is undiscoverable), and to enter an Adventure realm
(the first entry throws you back out). A stranger following the game's own
instructions stalls inside five minutes — and the one thing that should
teach stakes, death, never happens.

## 5. Explicit answers

- **Is the goal of the game communicated?** Yes on paper — Aldwyn's Charge is
  explicit and well-written — but its steps can't be completed (B1/B2), so
  the communicated goal is unachievable.
- **Is combat fun for more than 5 minutes?** Unproven; no verified kill in
  either arc, no visible feedback (M1). Currently it is not fun because it
  is not legible.
- **Is loot exciting?** It can't be: loot is visible on the ground and
  unobtainable without an undocumented hotkey (B1).
- **Is death comprehensible?** Soft death effectively never happens (M3);
  hard death works mechanically but arrives silently (m3). Neither teaches
  anything.
- **Is there a reason to keep playing?** The Chronicles frame (House,
  heirloom, crypt, renown) is compelling and already real — but none of it
  is reachable in-session until the verb chain above is repaired.

## Method notes / evidence caveats

- Headless Chromium (software GL): canvas rendered correctly throughout; the
  perspective renderer was active (fog/horizon visible in `19`). Combat
  connection and movement-rate claims are flagged where headless input
  throttling could be the cause; both have hardware-verify fix directions.
- Server: dev build of `056746b`, in-memory SQLite, temp guest saves; no
  repo files modified outside this task folder (`git status` proof:
  untracked `orchestration/tasks/TASK-0034-playability-evaluation/` only).
- Driver scripts preserved for reproduction: `captures/_driver.mjs`,
  `_arc-a.mjs`, `_arc-b.mjs`; console logs alongside.
