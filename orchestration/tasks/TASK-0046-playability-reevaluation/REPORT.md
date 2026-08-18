---
task: TASK-0046
state: REVIEW_REQUESTED
branch: codex/TASK-0046-playability-reevaluation
commits:
  - 8dd9aee8
  - f0b6300f
  - 65b51a9a
  - 1de6e45b
  - e3c3ca57
  - ec42e843
base_commit: 45846af7
architect_review_required: true
---

## Verdict

The browser now has a credible first-session scaffold, and the post-friction
guest arc is playable for ten minutes: the tutorial produces readable melee
kills, XP, and a gold pickup. It is not yet fun-adjacent across both required
arcs. The mortal-oath Chronicles arc reaches a real House/Scion and Old Barrow
but never produces a readable hit, kill, loot, or death despite sustained
movement and attack attempts. Resource feedback also degenerates into repeated
`Not enough mana.` messages. This remains `REVIEW_REQUESTED`, not accepted.

## Method and socket proof

Current candidate was rebuilt from the isolated worktree at `c3988b29`, a
descendant of program tip `45846af7`. Production servers used only disposable
loopback ports; owner port 6500 was never used.

- `npm ci --no-audit --no-fund`: 834 packages installed.
- `npm run build`: Vite 5.4.21, 377 modules transformed, passed.
- Guest first-minute page-context proof on port 6548:
  `window.ws.url=ws://127.0.0.1:6548/`, readyState 1, page age 3.81s.
- Chronicles first-minute page-context proof on port 6547:
  `window.ws.url=ws://127.0.0.1:6547/`, readyState 1, page age 56.73s.
- The authoritative correction run used Playwright Chromium headless on
  disposable port 6550 for both arcs, with `ARC_MS=600000`; the driver,
  page-world socket values, and server-owned login/disconnect correlation are
  preserved in `captures/playwright-arcs-2026-08-18.txt`. The earlier in-app
  exploratory arc remains useful for richer qualitative notes, but it is not
  the wire-proof or duration claim.

## Arc A — guest quickstart (~10m, Playwright correction)

The headless driver opened `http://127.0.0.1:6550/?play`, selected Old Barrow,
kept a real canvas session active for the configured 600,000 ms, and recorded
`window.ws.url=ws://127.0.0.1:6550/`, readyState `1`. Server output correlated
the same arc to `Wanderer c43dc4` logging in and later leaving. The earlier
in-app run supplies the richer combat observations below; this run closes the
Playwright/duration/wire-proof requirement.

Qualitative observations from the earlier exploratory in-app client pass
(supplemental, not the Playwright wire proof):

- Opening: Delaford tutorial, Adventure panel, Old Barrow transition.
- ~00:18: first Dread Vanguard kill; Aldwyn explains XP, loot, and the next
  Adventure objective; 60 gold was picked up.
- ~04:05: movement into the next pack produced incoming Dread damage, repeated
  Bronze Arc hits, a second kill, and Attack XP.
- ~04:10–05:15: additional melee contact produced a third kill and XP.
- ~05:30 onward: movement continued through the dungeon, but repeated skill
  attempts produced `Not enough mana.` while the quickbar remained visually
  usable; no clear recovery/next-reward beat appeared.
- End state at ~10m14s: Old Barrow, position around 111,83, HP 110/110.

## Arc B — Chronicles House/Scion with mortal oath (~10m, Playwright correction)

- The headless driver opened the root page, clicked `Play as Guest`, inscribed
  `Playwright Ember`, and added `Playwright Asha` with `Swear the mortal oath`
  checked. Server output correlated `Playwright Asha (lvl 1)` on port 6550.
- The driver selected Old Barrow and kept the canvas session active for the
  configured 600,000 ms. The earlier Chronicles exploration recorded the
  card text `Asha Level 1 · Mortal oath` and the setup surfaces below.
- In the exploratory Chronicles pass, Set Out reached Delaford and Old Barrow.
  The first-minute socket proof was
  captured before/around this transition.
- Quest overlay clearly listed Aldwyn's Charge and its five-step verb chain.
- Roads exposed Tin/Salt/Chalk/Copper names and directions but no concrete
  destination/risk/reward preview.
- Inventory exposed Bronze Dagger and 100 gold; skill tree exposed 2 points,
  1 node, and no recommended first allocation.
- In Old Barrow, the visible opening actor overlapped the player, but repeated
  WASD movement, directional repositioning, canvas targeting, Bronze Arc,
  number-1, and skill-bar attempts produced no combat log, damage, kill, loot,
  or death. HP remained 110/110 through the ten-minute run.
- End state at ~10m15s: Old Barrow, position around 109,107, HP 110/110.

## TASK-0034 disposition table

| Prior friction | Disposition | Evidence from this pass |
|---|---|---|
| B1 first fight/verb chain is not safely learnable | SURVIVES in Chronicles; improved in guest | Guest produced three melee kills; mortal-oath run still could not connect an opening hit. |
| B2 first Adventure entry ejects the player | FIXED | Both arcs remained in Old Barrow after entry; no bounce to town. |
| M1 combat never visibly connects | PARTIALLY FIXED | Guest has hit/kill/XP logs; Chronicles remains no-hit/no-kill. |
| M2 mana economy fights the onboarding kit | SURVIVES | Repeated `Not enough mana.` messages while skills remained visually actionable. |
| M3 death is unreachable at level 1 | SURVIVES / not reproduced | Both arcs ended at full HP; no death or recovery decision occurred. |
| m1 tutorial chat is easy to miss | SURVIVES | The log remains a narrow ticker/preview; the guest beat text is readable only when expanded. |
| m2 House/Scion identity vanishes in-game | SURVIVES | Chronicles shows Asha during setup, but the in-world HUD does not keep House identity visible. |
| m3 permadeath has no ceremony | NOT-REPRODUCED | Neither arc reached death. |
| m4 dark rectangles read as holes/bugs | SURVIVES as visual risk | The Old Barrow screenshots retain large dark wall/floor masses; no polish change was evaluated. |
| m5 movement speed needs hardware verification | NOT-REPRODUCED | This pass verified reachability and coordinates, not a calibrated hardware feel measurement. |
| Major loot is not exciting/legible | SURVIVES | Guest visibly recorded only a 60-gold pickup; no memorable item reveal/comparison beat occurred. |
| Inventory spends attention budget poorly | SURVIVES | Inventory still exposes a large, information-dense panel with no first-use comparison beat. |
| Responsive overlays collide | NOT-REPRODUCED | No compact viewport pass was run in this task. |
| Zone choice lacks concrete objective | SURVIVES | Old Barrow is labelled forgiving/Start here but gives no specific trophy/material/House reward preview. |
| Skill tree lacks first allocation path | SURVIVES | Two points and one node are visible, but no recommended first change is surfaced. |
| Roads are evocative but underspecified | SURVIVES | Only four road names/directions are shown. |

## Ranked new friction

### Blockers

1. **Chronicles first combat can be visually present but mechanically silent.**
   The mortal-oath arc reaches Old Barrow and shows an overlapping actor, yet
   ten minutes of real movement/attack attempts yields no hit, damage, loot, or
   death. This blocks the core learn→fight→loot loop for the House/Scion path.

### Major

2. **Mana rejection is noisy and non-directive.** The quickbar remains available
   while the server repeatedly rejects casts; the player is not told when or
   how resource recovery will make the next action useful.
3. **The first reward is still mostly currency.** The guest run gets 60 gold,
   but no named item/history/comparison moment that answers why the delve is
   worth repeating.
4. **After the authored tutorial beats, the zone has no next decision.** The
   player can walk and strike, but there is no concrete room objective, reward
   preview, or route reason exposed in the first ten minutes.

### Minor

5. **House identity is absent from the in-world HUD after Set Out.** The setup
   is legible, then the House/Scion fantasy recedes to the Chronicles panel.
6. **The tutorial ticker remains easy to miss under the world view.**

## Scope proof and limitations

Only task-folder reports/captures changed on the coordinator branch. No source,
server, native, playtest, package, or product files were edited. The authoritative
duration and wire-proof run used headless Playwright; the earlier in-app-browser
session is explicitly retained as qualitative exploration only. Canvas actors
are not represented in the DOM, so actor positions and combat readability are
supported by logs, minimap coordinates, HP, and server-backed messages rather
than DOM labels. No death arc or compact viewport pass was forced after the
observed full-HP outcomes, and no screenshot files are claimed by this report.

The architect must review the report and decide whether the Chronicles silent
opener is a real current-tip regression or a presentation/input-surface issue;
no product fix is invented here.
