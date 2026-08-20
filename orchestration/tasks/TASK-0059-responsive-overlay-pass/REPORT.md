---
task: TASK-0059
state: REVIEW_REQUESTED
coordinator: cursor
worker_branch: codex/TASK-0059-responsive-overlay-pass-cursor
base_commit: 0ce26b150c106449d5bab5cf44e7a4605612308c
architect_review_required: true
---

# TASK-0059 REPORT — Compact overlay pass (1366×768 and 1280×720)

## Executive summary

Laptop-compact first-session overlays now stack without covering the HUD
orbs, quickbar, or a playable left canvas. 1920×1080 layout is unchanged
(centered guide banner, 1240px inventory). The capture script starts and
owns a production server on 127.0.0.1:6582 (cursor range; never 6500).

## Audit (unfixed dist → `before-*` PNGs + `capture-0059-before-evidence.json`)

First-session combos at 1366×768, 1280×720, and 1920×1080: chat ticker,
guide banner, adventure panel, inventory, settings, skill tree, loot
toast, death overlay.

## Per-defect table

| Defect | Found | Disposition |
|---|---|---|
| Inventory pane `min(100vw-16px, 1240px)` covers HP/MP orbs and quickbar at both compact sizes (1366 inventory 1240×656 @ y=8 vs orbs @ y=516) | yes — worst two: `before-1366x768-inventory.png`, `before-1280x720-inventory.png` | **fixed** — at `width <= 1366px` inventory is `min(100vw-24px, 680px)` and `--pane-host-panel-bottom: calc(orb + 120px)` so the pane sits above the overflowing orbs. After: `after-*-inventory.png` |
| Adventure zone-menu `white-space: nowrap` grew to 442px (party column 300px) and its AABB overlapped the MP orb | yes — before JSON zoneMenu 442×392; after chrome still listed Marsh of Reeds but column is 280px with wrap + scroll | **fixed** — compact party overlay `align-items: stretch; max-width: 280px`; zone-menu `min-width: 0; width: 100%; max-height: calc(100dvh - orb - 340px); overflow-y: auto`; notes/objectives wrap |
| Settings overlay card taller than the inset overlay after the HUD reserve grew (y=−30, clipped) | yes — `after` first rebuild, `settings-in-viewport` failed | **fixed** — compact overlay card `max-height` tied to the pane-host inset; body scrolls |
| Guide banner vs party overlay AABB | not reproduced (20px gap at 1366: guide right 969, party left 989) | **wontfix as collision** — still left-anchored at compact (`left: 186px`, `max-width: calc(100% - 186px - 316px)`) so a longer Aldwyn line cannot walk into the party column. 1920 stays `left: 50%; transform: translateX(-50%)` |
| Loot toast vs guide banner | not reproduced at these two laptop sizes | **wontfix as collision** — compact `padding-top: clamp(96px, 18vh, 160px)` kept as slack; 1920 padding unchanged |
| Identity vs chat peek / HP orb | not overlapping | **wontfix** — 0055 chip already above the orb; compact chat `left` offset is extra slack (drag-dock may still pin the peek) |
| Death overlay Continue clipped | not reproduced (panel + Continue in viewport at both compact sizes) | **wontfix as collision** — `height <= 800px` padding shrink kept as slack |
| Skill tree search out of view | not reproduced | **wontfix as collision** — `height <= 800px` panel max-height kept as slack |

## Changed files (owned_paths only)

- `src/components/layout/GameContainer.vue` — compact laptop stack; capture hook `window.__verdigrisOverlayCapture` for guide/loot/death
- `src/components/ui/panes/PaneHost.vue` — compact inventory width, HUD clearance, overlay z-index 88, settings card max-height
- `src/components/ui/LootMoment.vue` — compact toast offset
- `src/components/ui/world/GuideBanner.vue` — `box-sizing: border-box`
- `src/components/ui/world/DeathOverlay.vue` — short-viewport padding
- `src/components/passives/GeometricSkillTreePane.vue` — short-viewport panel max-height
- `tests/unit/responsive-overlay-pass.spec.js` — source-level compact-query asserts
- `orchestration/tasks/TASK-0059-responsive-overlay-pass/captures/` — self-starting Playwright script, before/after PNGs, evidence JSON

No `server/**`, `native/**`, or `playtest/**` assertion changes. 1920 inventory
width and centered guide banner are asserted (`inventory-stays-wide`,
`guide-stays-centered`).

## Test commands and outcomes

`npm run test:unit`:

```
 Test Files  134 passed (134)
      Tests  841 passed (841)
```

`npm run playtest` (PLAYTEST_PORT=6580, bind 127.0.0.1):

```
32/32 scenarios passed
Timing diagnostics: {"loadMode":false,"p99EventLoopLagMs":32.210943,"maxEventLoopLagMs":104.595455}
```

Browser smoke (production dist + `start:e2e`-equivalent NODE_ENV=development
server so `/world/players` exists; 127.0.0.1:6581, never 6500 — cursor capsule):

```
  ok 1 tests\e2e\browser-critical-loop.spec.mjs:135:1 › the built game supports the browser-critical guest loop (17.9s)
  1 passed (19.8s)
```

Capture (script starts/owns production server on 127.0.0.1:6582):

```
node orchestration/tasks/TASK-0059-responsive-overlay-pass/captures/capture-0059.mjs
```

```
CAPTURES OK {"1366x768.guide-vs-party":true,"1366x768.guide-vs-minimap":true,"1366x768.zoneMenu-vs-quickbar":true,"1366x768.zoneMenu-vs-mpOrb":true,"1366x768.identity-vs-chatPeek":true,"1366x768.identity-vs-hpOrb":true,"1366x768.chatPeek-vs-hpOrb":true,"1366x768.inventory-vs-hpOrb":true,"1366x768.inventory-vs-mpOrb":true,"1366x768.inventory-vs-quickbar":true,"1366x768.loot-vs-guide":true,"1366x768.zoneMenu-in-viewport":true,"1366x768.party-in-viewport":true,"1366x768.guide-in-viewport":true,"1366x768.inventory-in-viewport":true,"1366x768.settings-in-viewport":true,"1366x768.skillSearch-in-viewport":true,"1366x768.loot-in-viewport":true,"1366x768.deathPanel-in-viewport":true,"1366x768.continue-in-viewport":true,"1366x768.skill-search-visible":true,"1366x768.death-continue-visible":true,"1366x768.zoneMenu-fits-party-column":true,"1366x768.inventory-leaves-canvas":true,"1280x720.guide-vs-party":true,"1280x720.guide-vs-minimap":true,"1280x720.zoneMenu-vs-quickbar":true,"1280x720.zoneMenu-vs-mpOrb":true,"1280x720.identity-vs-chatPeek":true,"1280x720.identity-vs-hpOrb":true,"1280x720.chatPeek-vs-hpOrb":true,"1280x720.inventory-vs-hpOrb":true,"1280x720.inventory-vs-mpOrb":true,"1280x720.inventory-vs-quickbar":true,"1280x720.loot-vs-guide":true,"1280x720.zoneMenu-in-viewport":true,"1280x720.party-in-viewport":true,"1280x720.guide-in-viewport":true,"1280x720.inventory-in-viewport":true,"1280x720.settings-in-viewport":true,"1280x720.skillSearch-in-viewport":true,"1280x720.loot-in-viewport":true,"1280x720.deathPanel-in-viewport":true,"1280x720.continue-in-viewport":true,"1280x720.skill-search-visible":true,"1280x720.death-continue-visible":true,"1280x720.zoneMenu-fits-party-column":true,"1280x720.inventory-leaves-canvas":true,"1920x1080.identity-vs-hpOrb":true,"1920x1080.zoneMenu-in-viewport":true,"1920x1080.guide-in-viewport":true,"1920x1080.skill-search-visible":true,"1920x1080.death-continue-visible":true,"1920x1080.guide-stays-centered":true,"1920x1080.inventory-stays-wide":true}
```

Rerun: `node orchestration/tasks/TASK-0059-responsive-overlay-pass/captures/capture-0059.mjs`
(optional `CAPTURE_PORT=6583` inside 6580–6599). `CAPTURE_PHASE=before` records
without hard-fail.

## Authentic negative

The first after-rebuild capture failed `inventory-vs-mpOrb` /
`zoneMenu-vs-mpOrb` because `--pane-host-panel-bottom: orb + 20px` is
measured from the *viewport* while the orbs sit on a vertically centered
world-shell (orb top ≈ y=516 on 768). Restored a larger compact bottom
reserve (`orb + 120px`) and a viewport-based zone-menu max-height; the
same script then printed `CAPTURES OK`.
