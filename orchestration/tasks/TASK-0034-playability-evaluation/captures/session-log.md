

# Arc A — guest quickstart (?play) — started 2026-08-17 14:17:36
Client: headless Chromium (Playwright 1.56.1), 1440x1000 viewport. Server: 127.0.0.1:9777 dev build.

### 2026-08-17 14:17:41 — Arc A beat 1 — landing page (plain URL, logged out)
- Did: loaded http://127.0.0.1:9777/ and looked at what a stranger sees.
- Page text: "BEGIN A LIVING CHRONICLE\n\nPLAY AS GUEST\nONE CLICK. YOUR OWN PERSISTENT HOUSE.\n\nNo account needed. Your House and Scions stay with this browser.\n\nSIGN IN TO AN EXISTING ACCOUNT\n+"
- Elements: {"heading":true,"backdropCanvas":true,"guestBtn":true,"accountToggle":true}
- Capture: 01-a-landing-page.jpg

### 2026-08-17 14:17:48 — Arc A beat 2 — ?play one-URL quickstart
- Did: navigated to /?play (one-URL quickstart hook in Login.vue).
- Happened: game canvas visible = true. Spawn state: {"uuid":"e9bcb246-887b-4925-a84e-76661279dbcf","username":"Wanderer 500f7f","x":34,"y":122,"level":1}
- Minimap says: "Delaford Village\n34, 122"
- Chat so far: "Welcome to Verdigris.\n07:17 AM\nYour House's wagon rolls in with the dawn market. The quartermaster counts 100 gold into the ledger — the day's road purse.\n07:17 AM\nAldwyn the Guide: Welcome to Delaford, Wanderer 500f7f. I am Aldwyn, and it falls to me to keep new blood alive long enough to matter.\n07:17 AM\nAldwyn the Guide: First things first: use W, A, S and D (or the arrow keys) to take a walk around town.\n07:17 AM"
- Capture: 02-a-quickstart-arrival.jpg

### 2026-08-17 14:17:53 — Arc A beat 3 — first minute orientation
- Did: nothing for ~5s, just read the screen like a new player.
- Visible overlays/panels/hints: []
- Quickbar: "Space\nBronze Arc\nBRONZE ARC\nCarve a broad weapon arc through the three tiles ahead.\nSPACE / 1\nShift\nGhostroad Step\n8s\nGHOSTROAD STEP\nSlip three tiles along the ghostroad, stopping before danger.\nSHIFT / 2 · 8S RECOVERY\nQ\nCinder Fan\n6s\nCINDER FAN\nHurl a searing cinder down the facing lane to strike at range.\nQ / 3 · 12 MANA · 6S RECOVERY\nE\nRimebreak\n12s\nRIMEBREAK\nBreak a ring of rime around you, da"
- Minimap: "Delaford Village\n34, 122"
- Capture: 03-a-first-minute-hud.jpg

### 2026-08-17 14:18:01 — Arc A beat 4 — move around town (WASD)
- Did: clicked canvas to focus, held D/S/A/W ~0.6s each.
- Happened: pos {"x":34,"y":122} -> {"x":34.666666,"y":122}
- Capture: 04-a-move-around-town.jpg

### 2026-08-17 14:18:03 — Arc A beat 5a — walk toward Aldwyn the Guide
- Did: held A until x<=35. Result: {"ok":true,"pos":{"uuid":"e9bcb246-887b-4925-a84e-76661279dbcf","username":"Wanderer 500f7f","x":34,"y":122,"level":1},"steps":1}

### 2026-08-17 14:19:26 — Arc A beat 5b — talk to Aldwyn (find a goal)
- Did: right-clicked around screen looking for a Talk action. Menus seen: (0.5,0.5): Wagon House Wayfarers 500f7f Wagon | Walk here | Examine House Wayfarers 500f7f Wagon | Cancel ;; (0.42,0.5): Bank Rhea of the Countinghouse | Walk here | Examine Rhea of the Countinghouse | Cancel ;; (0.58,0.5): Walk here | Cancel ;; (0.35,0.5): Walk here | Cancel ;; (0.65,0.5): Walk here | Cancel ;; (0.42,0.42): Walk here | Cancel ;; (0.58,0.42): Walk here | Cancel ;; (0.42,0.58): Walk here | Cancel ;; (0.58,0.58): Walk here | Cancel ;; (0.3,0.45): Walk here | Cancel ;; (0.7,0.45): Walk here | Cancel
- Happened: Talk clicked = false. Aldwyn said: ""
- Capture: 05-a-after-aldwyn-talk.jpg

### 2026-08-17 14:20:14 — Arc A beat 6a — north gate zone transition to Old Wood
- Did: walked north through the gate at (38,94).
- Happened: pos now {"x":38,"y":115.666667}; minimap: "Delaford Village\n38, 116"
- Chat: ""
- Capture: 06-a-old-wood-entry.jpg

### 2026-08-17 14:21:33 — Arc A beat 6b — fight #1 (Old Wood Wolf)
- Did: walked into wolf spawn, spammed Space (primary) and Q.
- Combat chat log: ""
- Kills observed: 0. Duration so far: 77s
- Captures: 07-a-fight-wolf-mid.jpg, 08-a-fight-wolf-end.jpg

### 2026-08-17 14:22:56 — Arc A beat 7 — loot a drop
- Did: right-click scanned tiles around the kill for a Take action. Menus seen: (0.5,0.5): Drink from the Crossroads Fountain | Walk here | Cancel ;; (0.53,0.5): Walk here | Cancel ;; (0.47,0.5): Walk here | Cancel ;; (0.5,0.53): Walk here | Cancel ;; (0.5,0.47): Walk here | Cancel ;; (0.56,0.5): Walk here | Cancel ;; (0.44,0.5): Walk here | Cancel ;; (0.5,0.56): Walk here | Cancel ;; (0.5,0.44): Walk here | Cancel ;; (0.56,0.56): Walk here | Cancel ;; (0.44,0.44): Walk here | Cancel
- Happened: Take clicked = false. Ground items before=9 after=9
- Capture: 09-a-after-loot.jpg

### 2026-08-17 14:23:05 — Arc A beat 8 — open inventory, equip something
- Did: pressed I. Inventory visible=true. Items: ["Bronze Dagger (1 x 2)"]
- Right-click first item menu: ["Equip Bronze Dagger","Examine Bronze Dagger","Drop Bronze Dagger","Cancel","capture: 11-a-inventory-item-menu.jpg"]
- Captures: 10-a-inventory-open.jpg, 12-a-after-equip.jpg

### 2026-08-17 14:23:08 — Arc A beat 9 — look for a goal (J = quests)
- Did: pressed J. Quest panel text: "QUESTS\nClose\n×\n\nACTIVE COMMISSION\n\nAldwyn's Charge\n\nLearn the rhythm of Delaford, then cross the threshold into an Adventure realm.\n\n◆\nWalk through Delaford\n◇\nStrike a hostile creature\n·\nSlay a hostile creature\n·\nClaim an item from the ground\n·\nEnter an Adventure realm\nREWARD\n+1 passive point\n+5 House renown"
- Capture: 13-a-quests-panel.jpg

### 2026-08-17 14:23:19 — Arc A beat 10 — zone transition via Adventure button
- Did: clicked Adventure. zone menu options: "CHOOSE AN EXPEDITION\n\nYou: Lv 1\nThe Old Barrow\nTight halls · forgiving first delve\nSTART HERE\nLv 1–5\nVerdant Grove\nOpen clearings · roaming packs\nREADY\nLv 1–6\nSunken Colonnade\nA narrow, punishing gauntlet\nDANGER\nLv 3–8\nWeir Crypt\nDense rooms · little retreat\nDANGER\nLv 4–9\nThe Wilds\nBroad hunting grounds\nDANGER\nLv 6–12\nMarsh of Reeds\nHostile wetlands · elite packs\nDANGER\nLv 8–14" menu capture: 14-a-zone-menu.jpg
- Minimap now: "Verdant Grove\n100, 100"
- Capture: 15-a-after-zone-transition.jpg

### 2026-08-17 14:24:34 — Arc A beat 11 — fight #2 (post-transition zone)
- Did: wandered to find a monster, then Space/E/Q rotation for up to 60s.
- Combat chat: "The party returns to the surface.\n07:23 AM\nNot enough mana.\n07:24 AM\nNot enough mana.\n07:24 AM\nNot enough mana.\n07:24 AM\nNot enough mana.\n07:24 AM\nNot enough mana.\n07:24 AM\nNot enough mana.\n07:24 AM\nNot enough mana.\n07:24 AM\nNot enough mana.\n07:24 AM"
- Kills: 0. Captures: 16-a-fight2-contact.jpg, 17-a-fight2-end.jpg

### 2026-08-17 14:27:40 — Arc A beat 12 — die on purpose, observe death UX
- Did: stood in monster territory without fighting back for up to 3 minutes.
- Happened: dead=false. Death chat: ""
- Screen after death: "Walk here\nDelaford Village\n53, 99\nQUESTS\nSETTINGS\nEXIT\nPARTY\nADVENTURE\nROADS\nNot enough mana.\n13\nCHAT\n13\nPin\nWelcome to Verdigris.\n07:17 AM\nYour House's wagon rolls in with the dawn market. The quartermaster counts 100 gold into the ledger — the day's road purse.\n07:17 AM\nAldwyn the Guide: Welcome to Delaford, Wanderer 500f7f. I am Aldwyn, and it falls to me to keep new blood alive long enough to matter.\n07:17 AM\nAldwyn the Guide: First things first: use W, A, S and D (or the arrow keys) to take a walk around town.\n07:17 AM\nAldwyn the Guide: You walk with purpose already.\n07:17 AM\nAldwyn the Guide: Steel answers steel out here. Walk into a monster to strike it, or face one and press SPACE (or 1).\n07:17 AM\nThe party returns to the surface.\n07:23 AM\nNot enough mana.\n07:24 AM\nNot enough mana.\n07:24 AM\nNot enough mana.\n07:24 AM\nNot enough mana.\n07:24 AM\nNot enough mana.\n07:24 AM\nNot enough mana.\n07:24 AM\nNot enough mana.\n07:24 AM\nNot enough mana.\n07:24 AM\nSend\nHP\n110 / 110\nSpace\nBronze Arc\nBRONZE ARC\nCarve a broad weapon arc through the three tiles ahead.\nSPACE / 1\nShift\nGhostroad Step\n8s\nGHOSTROAD STEP\nSlip three tiles along the ghostroad, stopping before danger.\nSHIFT / 2 · 8S RECOVE"
- Captures: 18-a-death-moment.jpg, 19-a-death-screen.jpg

### 2026-08-17 14:28:38 — Arc A beat 13 — post-death flow + a few more minutes
- Did: clicked death-screen CTA (none found), walked, fought for ~45s.
- Chat: ""
- Captures: 20-a-after-death-cta.jpg, 21-a-final-minutes.jpg

# Arc A ended 2026-08-17 14:28:38


# Arc B — Chronicles path — started 2026-08-18 00:01:53
Client: headless Chromium (Playwright 1.56.1), 1440x1000 viewport. Server: 127.0.0.1:9777 dev build.

### 2026-08-18 00:01:59 — Arc B beat 1 — landing -> Play as Guest -> Chronicles onboarding
- Did: loaded http://127.0.0.1:9777/, clicked "Play as Guest".
- Happened: Chronicles screen visible = true.
- Onboarding copy verbatim: "V\n\nTHE LIVING RECORD\n\nCHRONICLES\n\nAccount: Guest-24f7fc\n\nYour House endures beyond any one adventurer. Name its first Scion, then set out into Delaford.\n\nFOUND A HOUSE\nInscribe"
- Captures: 22-b-landing-page.jpg, 23-b-chronicles-onboarding.jpg

### 2026-08-18 00:02:01 — Arc B beat 2 — Found a House
- Did: filled "Found a House" with "Gateward", clicked Inscribe. house founded, capture: 24-b-found-house-filled.jpg
- Ledger now reads: "HOUSE RECORDS\n\nHouse Gateward\n0 living · 0 fallen\n+ Found another House\n\nHOUSE OF\n\nGateward\nRENOWN\n0\nCRYPT\n0\n\nLIVING SCIONS\n\nNo living names are written here yet.\n\nNAME A NEW SCION\nAdd Scion\nSWEAR THE MORTAL OATH\nFINAL DEATH MOVES THIS SCION TO THE CRYPT. OFF BY DEFAULT WHILE BALANCE IS STILL BEING TUNED.\nCHOOSE A SCION"
- Capture: 25-b-house-ledger.jpg

### 2026-08-18 00:02:03 — Arc B beat 3 — Name a Scion (mortal oath sworn)
- Did: named scion "Mortalis", checked "Swear the mortal oath", clicked Add Scion.
- Oath copy verbatim: "SWEAR THE MORTAL OATH\nFINAL DEATH MOVES THIS SCION TO THE CRYPT. OFF BY DEFAULT WHILE BALANCE IS STILL BEING TUNED."
- Roster now: "LIVING SCIONS\n\nMortalis\nLevel 1 · Mortal oath\n◆"
- Captures: 26-b-scion-mortal-oath.jpg, 27-b-scion-added.jpg

### 2026-08-18 00:02:11 — Arc B beat 4 — Set Out
- Did: clicked "SET OUT AS MORTALIS".
- Happened: canvas visible = true. Spawn: {"uuid":"browser-guest-ff556c2b-934e-4566-b5da-7cf6b624f7fc","username":"Mortalis","x":42,"y":115,"level":1}
- Minimap: "Delaford Village\n42, 115"
- Chat: "Welcome to Verdigris.\n05:02 PM\nAldwyn the Guide: Welcome to Delaford, Mortalis. I am Aldwyn, and it falls to me to keep new blood alive long enough to matter.\n05:02 PM\nAldwyn the Guide: First things first: use W, A, S and D (or the arrow keys) to take a walk around town.\n05:02 PM"
- Capture: 28-b-set-out-arrival.jpg

### 2026-08-18 00:02:15 — Arc B beat 5 — first minute orientation (Chronicles framing)
- Did: read the screen. Looked for any sign of my Scion name / House in the HUD.
- HUD quickbar: "Space\nBronze Arc\nBRONZE ARC\nCarve a broad weapon arc through the three tiles ahead.\nSPACE / 1\nShift\nGhostroad Step\n8s\nGHOSTROAD STEP\nSlip three tiles along the ghostroad, stopping before danger.\nSHIFT / 2 · 8S RECOVERY\nQ\nCinder Fan\n6s\nCINDER FAN\nHurl a searing cinder down the facing lane to strike at range.\nQ / 3 · 12 MANA · 6S RECOVERY\nE\nRimebreak\n12s\nRIMEBREAK\nBreak a ring of rime around you, da"
- Identity references on screen: ["Mortalis"]
- Capture: 29-b-first-minute-hud.jpg

### 2026-08-18 00:02:24 — Arc B beat 6 — move around town (WASD)
- Did: D/S/A/W ~0.6s each. Pos {"x":42,"y":115} -> {"x":42,"y":115}
- Capture: 30-b-move-around-town.jpg

### 2026-08-18 00:04:18 — Arc B beat 7 — talk to Aldwyn (find a goal)
- Did: right-click scanned for Talk. Menus: (0.5,0.5): Walk here | Cancel ;; (0.42,0.5): Walk here | Cancel ;; (0.58,0.5): Walk here | Cancel ;; (0.35,0.5): Walk here | Cancel ;; (0.65,0.5): Walk here | Cancel ;; (0.42,0.42): Walk here | Cancel ;; (0.58,0.42): Walk here | Cancel ;; (0.42,0.58): Walk here | Cancel ;; (0.58,0.58): Walk here | Cancel ;; (0.3,0.45): Walk here | Cancel ;; (0.7,0.45): Walk here | Cancel
- Happened: Talk clicked = false. Aldwyn said: ""
- Capture: 31-b-after-aldwyn-talk.jpg

### 2026-08-18 00:05:00 — Arc B beat 8 — north gate zone transition
- Did: walked north through the gate. Pos now {"x":38.666667,"y":95}; minimap: "Delaford Village\n39, 95"
- Capture: 32-b-old-wood-entry.jpg

### 2026-08-18 00:06:21 — Arc B beat 9 — fight #1 (Old Wood Wolf), Space/Q/E rotation
- Combat chat: "Not enough mana.\n05:05 PM\nNot enough mana.\n05:05 PM\nNot enough mana.\n05:05 PM\nNot enough mana.\n05:05 PM\nNot enough mana.\n05:05 PM\nNot enough mana.\n05:05 PM\nNot enough mana.\n05:05 PM\nNot enough mana.\n05:05 PM\nNot enough mana.\n05:05 PM\nNot enough mana.\n05:05 PM\nNot enough mana.\n05:06 PM\nNot enough mana.\n05:06 PM\nNot enough mana.\n05:06 PM\nNot enough mana.\n05:06 PM\nNot enough mana.\n05:06 PM\nNot enough mana.\n05:06 PM\nNot enough mana.\n05:06 PM\nNot enough mana.\n05:06 PM\nNot enough mana.\n05:06 PM\nNot enough mana.\n05:06 PM"
- Kills: 0. Duration: 80s. Captures: 33-b-fight-wolf-mid.jpg, 34-b-fight-wolf-end.jpg

### 2026-08-18 00:07:51 — Arc B beat 10 — loot a drop, open inventory, equip
- Menus seen while looting: (0.5,0.5): Walk here | Cancel ;; (0.53,0.5): Walk here | Cancel ;; (0.47,0.5): Walk here | Cancel ;; (0.5,0.53): Walk here | Cancel ;; (0.5,0.47): Walk here | Cancel ;; (0.56,0.5): Walk here | Cancel ;; (0.44,0.5): Walk here | Cancel ;; (0.5,0.56): Walk here | Cancel ;; (0.5,0.44): Walk here | Cancel ;; (0.56,0.56): Walk here | Cancel ;; (0.44,0.44): Walk here | Cancel. Take clicked = false. Ground items 9 -> 9
- Inventory visible = true. Items: ["Bronze Dagger (1 x 2)"]
- Captures: 35-b-inventory-open.jpg, 36-b-after-equip.jpg

### 2026-08-18 00:09:25 — Arc B beat 11 — fight #2 (extended, ~90s cap)
- Combat chat: "05:08 PM\n05:08 PM\n05:08 PM\n05:08 PM\n05:08 PM\n05:08 PM\n05:08 PM\n05:08 PM\n05:08 PM\n05:08 PM\n05:08 PM\n05:08 PM\n05:08 PM\n05:08 PM\n05:09 PM\n05:09 PM\n05:09 PM\n05:09 PM\n05:09 PM\n05:09 PM\n05:09 PM\n05:09 PM\n05:09 PM\n05:09 PM\n05:09 PM"
- Kills: 0. Duration: 94s. Captures: 37-b-fight2-mid.jpg, 38-b-fight2-end.jpg

### 2026-08-18 00:09:52 — Arc B beat 12 — die on purpose (mortal oath)
- Did: stood in wolf territory ~20s without fighting (natural-death attempt), then sent the playtest dev:kill websocket message (same path as playtest/scenarios/mortality.mjs).
- Natural-death check: vitals read "HP\n110 / 110" right before dev:kill — corroborates Arc A finding that natural death is unreachable at level 1.
- Happened: death/final-death seen in chat = false; returned to Chronicles screen = true.
- Death chat: ""
- Screen after death: "V\n\nTHE LIVING RECORD\n\nCHRONICLES\n\nAccount: Guest-24f7fc\n\nYour House endures beyond any one adventurer. Name its first Scion, then set out into Delaford.\n\nHOUSE RECORDS\n\nHouse Gateward\n0 living · 1 fallen\n+ Found another House\n\nHOUSE OF\n\nGateward\nRENOWN\n0\nCRYPT\n1\n\nLIVING SCIONS\n\nNo living names are written here yet.\n\nNAME A NEW SCION\nAdd Scion\nSWEAR THE MORTAL OATH\nFINAL DEATH MOVES THIS SCION TO THE CRYPT. OFF BY DEFAULT WHILE BALANCE IS STILL BEING TUNED.\nOpen the crypt (1)\nCHOOSE A SCION"
- Captures: 39-b-death-natural-attempt.jpg, 40-b-death-moment.jpg, 41-b-after-permadeath-screen.jpg

### 2026-08-18 00:09:52 — Arc B beat 13 — crypt / memorial
- Did: opened the crypt details on the Chronicles screen.
- Crypt contents: "Open the crypt (1)\nMortalis, level 1\nHeirloom: Bronze Dagger · awaiting an heir"
- House heading (renown etc.): "HOUSE OF\n\nGateward\nRENOWN\n0\nCRYPT\n1"
- Capture: 42-b-crypt-open.jpg

### 2026-08-18 00:10:03 — Arc B beat 14 — successor: what carries over
- Did: successor added, capture: 43-b-successor-added.jpg Set Out button: "SET OUT AS MORTALIS II" canvas visible after Set Out
- Successor spawn: {"uuid":"browser-guest-ff556c2b-934e-4566-b5da-7cf6b624f7fc","username":"Mortalis II","x":42,"y":115,"level":1}
- Chat on arrival: "Welcome to Verdigris.\n05:09 PM\nAldwyn the Guide: Welcome to Delaford, Mortalis II. I am Aldwyn, and it falls to me to keep new blood alive long enough to matter.\n05:09 PM\nAldwyn the Guide: First things first: use W, A, S and D (or the arrow keys) to take a walk around town.\n05:09 PM"
- Successor inventory: ["Bronze Dagger (1 x 2)"]
- Captures: 44-b-successor-arrival.jpg, 45-b-successor-inventory.jpg

### 2026-08-18 00:10:55 — Arc B beat 15 — a few more minutes as the successor
- Chat: "Aldwyn the Guide: You walk with purpose already.\n05:10 PM\nAldwyn the Guide: Steel answers steel out here. Walk into a monster to strike it, or face one and press SPACE (or 1).\n05:10 PM"
- Capture: 46-b-final-minutes.jpg

# Arc B ended 2026-08-18 00:10:55

### 2026-08-18 00:15 — Arc B reviewer notes (human judgment pass over captures 22-b … 46-b)
- Onboarding clarity: the Chronicles screen is gorgeous and the guest account line ("Account: Guest-24f7fc") reassures, but the intro copy "Your House endures beyond any one adventurer. Name its first Scion, then set out" describes step 2 while the actual first control is "FOUND A HOUSE / House name / Inscribe" — a stranger meets the words House and Scion with no definition, and the instruction ordering is backwards. (23-b, 24-b)
- Delight: "Swear the mortal oath" is a great permadeath opt-in, and its subcopy is honest ("Off by default while balance is still being tuned") — but that honesty also tells a new player the marquee feature is unfinished, and the small text is hard to read. (26-b)
- Dead end (same as Arc A): Aldwyn cannot be talked to — right-click scan found only "Walk here | Cancel" on every tile; the quest-giver NPC is scenery. (31-b)
- Combat still yields zero kills headless (fight #1: 80s, fight #2: 94s, chat full of "Not enough mana." spam with no cooldown/mana UI explanation). Consistent with Arc A lesson 1. (33-b, 34-b, 37-b, 38-b)
- Natural death is unreachable at level 1: 20s standing in wolf territory left HP 110/110 (Arc A tried 3 minutes). Permadeath beat therefore used the playtest dev:kill websocket message. This is a balance finding, not a harness shortcut. (39-b)
- Permadeath UX is abrupt to the point of confusion: dev:kill landed and the client cut straight to the Chronicles screen — no death screen, no "Mortalis has fallen" beat, no chat message. Capture 40-b caught the transition mid-fade with the Chronicles panel ghosting translucently over the world (possible compositing artifact). For the game's signature mechanic, the moment has zero ceremony. (40-b, 41-b)
- Delight: the crypt entry is exactly right — "Mortalis, level 1 / Heirloom: Bronze Dagger · awaiting an heir". But nothing explains HOW an heir recovers the heirloom; "awaiting an heir" is a dead-end clue. (42-b)
- Succession works and is instant: Add Scion → "SET OUT AS MORTALIS II" → spawn at the same tile (42,115) with a fresh Bronze Dagger. Carry-over observed: House name, renown (0), crypt record. Nothing in-world acknowledges the fallen Scion — Aldwyn repeats the identical welcome script verbatim, which undercuts the generational fantasy. (43-b, 44-b, 45-b)
- Minor: menu music asset failed to load in both arcs (main_menu-*.mp3 ERR_ABORTED in console-arc-b.log) — likely autoplay policy, but a first-time player gets silence.
