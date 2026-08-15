# Handoff — 2026-08-13, post-merge state

Read `AGENTS.md` first (it is the canonical agent guide) and OBEY its
**HARD STOP git preflight** — this very handoff exists because a month of
work was once built on a stale checkout and had to be hand-merged back.

## Recovery update — 2026-08-14

Active recovery work lives on `codex/recover-merge-refinements` and draft PR
[#3](https://github.com/alexkorol/verdigris/pull/3). The canonical remote is
`https://github.com/alexkorol/verdigris`; do not publish to the obsolete
`delaford/game` fork remote.

The owner-tested presentation regressions are now restored and guarded:

- Character and Inventory are symmetric fixed 48vw overlays again. Inventory
  is the right half of the diptych, keeps the world full-size behind it, and
  uses the stacked equipment/12×7 backpack layout. Cells scale 40–54px to fit
  that half; the inventory-only 1240px override and desktop row layout were
  removed.
- Backpack and equipped items use one shared tooltip component and positioning
  model. The obsolete second tooltip was deleted; the legacy world-action hint
  is hidden/cleared while pane item hovers are active.
- The HP/MP numbers are quiet orb-base inscriptions without the old opaque
  label plaque. Clicking the orbs still opens Character/Inventory.
- Terrain now has a restrained, warmer grade, much less mist/haze, crisp
  directional terrain grounding, and no per-tree blur/radial-gradient work.
  Vertical scenery is projected/cut before it reaches the depth-sort/draw pass.
- Decorative orb shaders are capped at 30fps and 2× DPR; the world/input loop
  remains at 60fps.

Verified after the final client change: `npm run lint` clean,
`npm run test:unit` 745/745, `npm run playtest` 31/31,
`npm run test:e2e` 3/3, plus manual browser checks of WASD, both orb shortcuts,
the 48/48 diptych, 12×7 geometry, item art, and identical backpack/equipped
tooltips at 1280×720.

### Camera, lighting, and map-composition correction — 2026-08-14

The next owner playtest found that the first palette pass still left the game
laggy, too far away, and visibly distorted by the shallow 2D-to-2.5D camera.
The follow-up recovery changes the rendering model rather than adding another
colour filter:

- The default camera is closer (`1.34`) with a much more distant horizon. Tile
  proportions remain close to the authored square art instead of stretching
  into a runway, while raised actors, trees, and wall edges retain depth.
- Indoor themes now receive stable scene-specific darkness and the player is a
  persistent warm light source. Outdoor maps keep their day cycle and a much
  quieter player glow; indoor clouds, sun rays, and mist are skipped.
- Only exposed wall-edge tiles become raised billboards. Solid wall interiors
  remain dark mass, so dungeon layouts no longer fill the screen with hundreds
  of repeated brick faces.
- Vertical scenery is culled from projected screen bounds instead of scanning a
  fixed 65×65 square every frame. The flat terrain mesh is 41×41 rather than
  161×161 (3,200 rather than 51,200 triangles), lighting redraws at 30fps, and
  photographic depth blur/antialiasing are effectively removed from pixel art.
- Generated floors choose base-tile variants in coherent 4×4 regions; accent
  floors and water form irregular patches instead of independent square noise.
  Authored world trees and flowers now use clustered placement as well.
- Grove/Wilds wall IDs remain authoritative collision, but their visual ground
  is replaced with theme-matched floor plus deterministic tree-lines. Do not
  render those blocked cells as masonry carpet, and do not skip them into a
  black void; both failures were caught only by the final browser replay.

Treat these as protected visual/performance seams. Do not restore the shallow
horizon, 161×161 flat mesh, full-wall billboard pass, always-on mist, or the
per-cell 12% floor-accent roll while trying to “simplify” the renderer.

Verified on the final renderer in the required order: `npm run test:unit`
747/747, `npm run playtest` 31/31, and `npm run test:e2e` 3/3. The same build
was then played in-browser through Delaford Village, The Old Barrow, and
Verdant Grove; camera scale, player lighting, movement response, dungeon wall
edges, and the outdoor tree-line/ground seam were checked visually.

## Where the project stands

`master` (pushed to origin) is the
reunified line: the 2.5D renderer / four-quest campaign / Chronicles
Houses-and-Scions line merged with the Crossroads world-web / wagons /
security line. Ore mining/smithing is deliberately removed (ARPG direction;
crafting arrives through the Houses meta systems — Vesselforge brand-searing
is live).

Verified green as of this handoff:

- `npm run lint` clean; `npm run test:unit` 696/696
- `npm run playtest` 31/31 (real server, real protocol)
- `npm run test:e2e` — the browser-critical-loop Playwright gate (built
  client: login, Chronicles onboarding, WASD after UI focus, context menus,
  pointer equip/unequip, zone entry)

Deployment: one Node process serves the built client + WS on :6500;
`pm2 start ecosystem.config.cjs`; cloudflared quick tunnel for public play;
`/?play` = one-URL quick start (per-browser guest House); `/stats` = human
status page; `/api/stats` = JSON. HTML shell is `no-cache`, hashed assets
immutable.

## THE ACTUAL PRIORITY: the game looks and plays badly

The owner's verdict after playing on real devices: **"looks and plays
awful."** All the plumbing above is necessary but not sufficient. The next
agent's job is GAME FEEL and VISUAL QUALITY, played by hand in a real
browser, not proven by harness. Concrete starting points observed:

- The 2.5D perspective view has had its first human-driven palette/performance
  pass. Keep tuning it by eye against the owner's Songs of Conquest reference;
  do not restore the neon double-brightening or heavy fog/blur stack.
- The inventory diptych and responsive 40–54px item scale are restored. Treat
  the 48vw side-pane geometry and 12×7 grid as protected behavior.
- Combat/feedback polish, HUD legibility, and first-minute impressions have
  had no human-driven iteration since the merge.

Iterate: play in the browser (`npm run dev`, or build + `npm start`), change,
play again. Use the owner's feedback loop; do not declare feel-work done from
green tests.

## Former pending ports

The login-restyle ports listed by the original handoff have landed and their
specs were promoted. `tests/pending-port/` is intentionally empty; do not use
the old list as instructions to re-port or replace the current implementations.

## Known seams (do NOT "simplify" one side away)

- `player:login` dispatches TWO flows: payloads with `guestId`/`quickGuest`/
  `resumeScionId` → chronicle-auth flow (SQLite houses/scions, wagon-pitch
  spawn, world web); plain payloads → direct admission (JSON guest saves,
  `awaitChronicles` browser flow). The client socket wrapper decorates only
  interactive logins with `awaitChronicles`.
- The town is the 2.5D Delaford Village carrying the four world-web road
  gates one tile beside its own portals (tin 37,94 / salt 64,114 / chalk
  37,138 / copper 12,115) plus wagon pitches. The full Crossroads conversion
  (sanctuary truce, wagon/chart panes in the client shell) is planned
  follow-up; server systems + playtests for it are live.
- Two Chronicles persistence stacks coexist (JSON chronicles-store + SQLite
  chronicles-repository). Unification is open work.
- `inventory.add` is synchronous, returns `{ ok, added, remainder, ... }`;
  currency is a slot-less carried balance; overflow defaults to `'reject'`,
  reward paths pass `{ overflow: 'drop' }`.

## Hard-won verification lessons

- The playtest harness proves the PROTOCOL, not the DOM. The merge shipped
  five client breaks invisible to 31/31 playtests (dead store action,
  never-set socket-auth flag, missing Adventure menu, clipped inventory rows,
  missing `?play` hook). After ANY client change run `npm run test:e2e` and
  actually look at the game.
- Playtest scenarios are timing/order sensitive; fire-and-forget `dev:*`
  commands can be dropped by the rate bucket — re-request inside `waitFor`.
- Windows console tools mangle regex escapes through heredocs; prefer direct
  file edits for regex-bearing code.
