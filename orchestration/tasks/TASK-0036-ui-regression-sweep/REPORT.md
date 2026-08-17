---
task: TASK-0036
state: REVIEW_REQUESTED
branch: codex/TASK-0036-ui-regression-sweep
commits:
  - 7c648a55
base_commit: b141cd9f
---

## Executive summary

The inventory regression is fixed: the paperdoll remains above the backpack,
the 12x7 grid retains readable 54px cells, and the compact pane scrolls its
complete stack instead of shrinking into a desktop diptych. A real Chromium
sweep also captured the available UI panes at 1920x1080 and 1366x768.

## Implementation

The former desktop diptych CSS now requires the unused
`.inventory-pane--legacy-diptych` opt-in. Normal inventory layout stays
stacked, prevents backpack flex shrink, and tunes only inventory paperdoll
scale/spacing for desktop and compact viewports. No other panes were changed.

## Changed files

- `src/components/slots/Inventory.vue` — inventory-only layout correction.
- `orchestration/tasks/TASK-0036-ui-regression-sweep/captures/**` — baseline,
  before/after inventory evidence, and the complete pane gallery.

## Interfaces

No public protocol or server interface changed.

## Verification

- `npm run test:unit` — 118 files / 757 tests passed.
- `npm run build` — passed.
- `npm run smoke:browser` — 1/1 passed on alternate `PORT=6512` with
  `PLAYWRIGHT_BASE_URL=http://127.0.0.1:6512`; owner PID 10276 on 6500 was
  preserved.
- `git diff --check b141cd9f..7c648a55` — passed.
- All 34 changed paths remain within the task's owned paths.

## Manual checks

The gallery covers Chronicles, world/skillbar/small minimap, inventory,
character/stats, passive tree, quests, settings, party, adventure, roads,
context menu, chat, escape, and awaiting-respawn at both requested viewports.
The compact inventory pair includes the scroll-bottom state. Baseline images
show the pre-fix side-by-side regression; after images show the stacked layout.

## Ranked findings outside scope

1. **Large minimap mode is absent** — `src/components/hud/WorldMinimap.vue:1-8`
   renders only the fixed small side map and `src/components/layout/GameContainer.vue:45-51`
   mounts it as a single world HUD; no overlay mode was reachable. Follow-up
   required.
2. **Death has no dedicated overlay** — `src/components/slots/Stats.vue:178-181`
   exposes the lifecycle only as a Respawn row; the pane registry in
   `src/Delaford.vue:120-128` has no death/respawn entry. This supports
   TASK-0041 rather than being fixed here.
3. **No vendor pane is exposed** — the authoritative pane registry at
   `src/Delaford.vue:120-128` contains no vendor entry, so no vendor behavior
   was reachable in the fresh-town flow.
4. **Compact inventory scrolls by design** —
   `src/components/slots/Inventory.vue:324-346` leaves the legacy diptych
   inactive while retaining readable cells; the full 12x7 grid requires
   scrolling at 1366x768.

## Specification deviations

Other pane findings are reported only, as required. The historical diptych
rule remains in an explicitly inactive class solely to keep the existing
forbidden-path regression test honest; it is not used by the application.

## Risks and limitations

The sweep covers panes reachable in a fresh guest session; vendor behavior
could not be exercised because no vendor pane is registered. The screenshots
are JPEG evidence and vary in size; dimensions and viewport are encoded in
each filename and the gallery README.

## Integration notes

Requires architect review before integration. Integrate `7c648a55` from the
worker branch after acceptance; it is UI-only and disjoint from the current
0035 native and 0037 movement workers.


