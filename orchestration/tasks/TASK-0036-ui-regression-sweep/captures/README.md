# TASK-0036 UI sweep gallery

All `pass-*` captures are real Chromium sessions against the built client on
2026-08-17. The suffix records the viewport used. JPEGs are the annotated
evidence gallery; the inventory pair includes an initial view and a scroll-to-
bottom view where the compact viewport needs the pane's native scroll.

## Pass gallery

For both `1920x1080` and `1366x768`:

- `chronicles` — Chronicles House/Scion onboarding.
- `world-skillbar-minimap` — world, six-slot skill bar, and small minimap.
- `inventory-after` / `inventory-after-scroll` — paperdoll above the 12x7
  backpack; the second view proves the complete grid at compact height.
- `character-stats` — character sheet and lifecycle summary.
- `passive-skill-tree` — fullscreen passive tree.
- `quests` — quest journal overlay.
- `settings` — settings overlay.
- `party` — party panel.
- `adventure-dialog` / `roads-dialog` — travel dialogs.
- `context-menu` — right-click world action menu.
- `chat-dialog` — expanded chat overlay.
- `escape-dialog` — escape menu.
- `death-awaiting-respawn` — soft-death state shown through Character and
  its `Awaiting Respawn` lifecycle/respawn countdown.

## Explicit fail / limitation evidence

- `before-inventory-1920x1080.jpg` and `before-inventory-1366x768.jpg` are
  baseline captures from the pre-fix build. They show the retired desktop
  diptych regression: paperdoll beside the backpack with reduced scale.
- No large minimap overlay exists in the current web client. The only
  implemented minimap state is the small side map shown in the `world-*`
  captures; this is reported as a follow-up defect, not changed by TASK-0036.
- No vendor pane was reachable in the fresh guest town session; the current
  pane registry exposes no vendor entry. This is reported as not present.
- `test-death*` files, if present from local diagnostics, are supplemental
  probes and are not acceptance evidence.
