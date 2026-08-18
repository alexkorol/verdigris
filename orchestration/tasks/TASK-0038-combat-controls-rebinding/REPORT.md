# TASK-0038 REPORT — LMB/RMB attacks + key/mouse rebinding UI

## Executive summary

LMB now fires the primary attack (Bronze Arc) and RMB casts the weapon
skill (Cinder Fan) in the world, aimed at the cursor per D-007; clicks on
UI still operate UI, and the legacy context menu stays reachable via
Shift+right-click. Every combat action — all six skills and both mouse
buttons — is remappable in a new Settings → Controls section with
click-to-rebind capture, conflict refusal, ESC cancel, reset-to-default,
localStorage persistence, and live apply (no reload needed). The quickbar
labels follow the live bindings.

Base commit: `9d4f666` (program tip at claim time).
Worker branch: `codex/TASK-0038-rebinding-kimiwork`.

All acceptance gates are green (transcripts below), and the captures show
the rebinding UI, a completed rebind surviving a full page reload, and
LMB/RMB attacks landing — with WebSocket frame evidence of both the
client `player:skill:trigger` dispatch and the server's
`world:skill:effect` broadcast for each click.

## Approach

1. **Binding map (`src/core/config/controls.js`, rewritten).**
   `ACTION_DEFINITIONS` is derived from the shared skill registry
   (`@shared/skills/index.js`), so action ids are the skill ids
   (`primary-attack`, `dash`, `ability-1..4`) and labels come from the
   same source the quickbar uses ("Bronze Arc", "Cinder Fan", …).
   D-007 defaults: primary `mouse0`/`1`, dash `Space`/`Shift`/`2`,
   ability-1 `mouse2`/`q`/`3`, ability-2 `e`/`4`, ability-3 `r`/`5`,
   ability-4 `f`/`6`. Mouse buttons are first-class binding values
   (`mouse0/1/2`). API: `getBindings`, `getActionBindings`,
   `skillBindings`, `bindingLookup`, `findBindingOwner`,
   `bindingConflicts`, `addActionBinding` (refuses a binding already held
   by another action — no two actions share one binding),
   `removeActionBinding`, `resetBindings`, `subscribeBindings`,
   `primaryBindingLabel`, `displayBinding` (mouse0→LMB etc.),
   `normaliseBinding`. Persisted under `verdigris:controls:v1`; corrupt
   or unknown payloads fall back to defaults.
2. **Live apply (`src/core/utilities/input-controller.js`).** The input
   controller rebuilds its skill lookup via `subscribeBindings` on every
   change, exposes `getMouseBinding(button)` for world clicks, and
   accepts an injected bindings snapshot for tests. Movement (WASD /
   arrows) is unchanged.
3. **World-click semantics (`src/components/GameCanvas.vue`).** Left
   click: a queued context action still wins; otherwise the binding held
   by `mouse0` fires toward the cursor. Right click: Shift held (or RMB
   unbound) keeps the legacy server-authored context menu; otherwise the
   `mouse2` binding casts at the cursor. Aim is an 8-way compass
   direction (`atan2`, screen-space y-down) from player to cursor world
   tile, falling back to facing when the cursor is on the player's tile.
   Resolution stays server-authoritative exactly as before — the client
   only sends `player:skill:trigger` with skill id + direction.
4. **Settings UI (`src/components/ui/SettingsBindings.vue`, new; mounted
   in a new Controls section of `src/components/slots/Settings.vue`).**
   One row per action with removable binding chips, "+ Rebind" capture
   (window-level capture-phase keydown/mousedown/contextmenu listeners,
   stopPropagation, ESC cancels, conflict error naming the owning
   action), and reset-all.
5. **Quickbar (`src/components/hud/Quickbar.vue`).** Slot labels show
   `primaryBindingLabel(skillId)` and re-render on every binding change.

## Changed files

Owned by the spec:

- `src/core/config/controls.js` — rebindable action map, persistence,
  conflict detection, live-apply pub/sub (rewritten)
- `src/core/utilities/input-controller.js` — live bindings + mouse
  buttons
- `src/components/ui/SettingsBindings.vue` — rebinding UI (new)
- `tests/unit/controls-bindings.spec.js` — 9 tests for the binding map
- `orchestration/tasks/TASK-0038-combat-controls-rebinding/captures/` —
  evidence (new; see below)

Not owned but not forbidden — edited because the goal lives there
(minimal edits, all logic kept in owned files; see Deviations):

- `src/components/GameCanvas.vue` — world LMB/RMB click semantics (the
  canvas click handlers live in this component)
- `src/components/slots/Settings.vue` — 7-line mount of the owned
  SettingsBindings component (the spec's `ui/Settings*.vue` glob does not
  cover the real settings pane, which lives in `slots/`)
- `src/components/hud/Quickbar.vue` — label source swap (spec §4
  requires the skill bar to reflect bindings)
- `tests/unit/input-controller.spec.js` — legacy alias expectations
  updated to the D-007 defaults (Space/Shift now bind dash, not primary)
- `tests/unit/hud-orb-ui.spec.js` — Quickbar source assertion updated to
  the new label API
- `tests/e2e/browser-critical-loop.spec.mjs` — quickbar label
  expectations (`Bronze Arc [LMB / 1]`, `Cinder Fan [RMB / 3]`) and the
  canvas context-menu click now uses Shift+right-click

No forbidden paths touched (`server/**`, `native/**`, `prototypes/**`,
`package.json` all untouched).

## Interfaces

- `controls.js` exports the binding API listed above; storage key
  `verdigris:controls:v1`; binding values are normalised
  `KeyboardEvent.key` lower-case plus `mouse0/1/2`.
- `InputController.getMouseBinding(button)` → `{ id, label, type } |
  null`.
- No server protocol changes: attacks use the existing
  `player:skill:trigger` event with `{ skillId, direction, phase }`.

## Test commands and outcomes

`npm run test:unit` — green:

```
 Test Files  123 passed (123)
      Tests  788 passed (788)
```

`npm run smoke:browser` — green (includes build; the updated e2e spec
passes against the new labels and Shift+RMB menu):

```
  ok 1 tests\e2e\browser-critical-loop.spec.mjs:135:1 › the built game supports the browser-critical guest loop (18.3s)
  1 passed (20.6s)
```

`npm run playtest` — green:

```
31/31 scenarios passed
```

Targeted new specs:

```
 ✓ tests/unit/hud-orb-ui.spec.js (8 tests)
 ✓ tests/unit/controls-bindings.spec.js (9 tests)
 ✓ tests/unit/input-controller.spec.js (4 tests)
```

## Manual verification (captures/)

Captured with `captures/capture-0038.mjs` (Playwright, real Chromium,
real server on 127.0.0.1:6500; guest login → world):

- `01-quickbar-mouse-bindings.png` — quickbar/HUD with live binding
  labels
- `02-lmb-primary-attack.png` — LMB world click attack
- `03-rmb-weapon-skill.png` — plain RMB world click cast (no context
  menu)
- `04-settings-controls-bindings.png` — Settings → Controls rebinding UI
  with all six actions and default chips
- `05-rebind-cairn-ward-to-T.png` — completed rebind (Cairn Ward gains a
  `T` chip via the capture flow)
- `06-rebind-persists-after-reload.png` — the same `T` chip after a full
  page reload (localStorage persistence)
- `attack-frame-evidence.json` — WebSocket frame log. The LMB click
  produced a sent
  `player:skill:trigger {"skillId":"primary-attack","direction":"right","phase":"start"}`
  answered by the server's broadcast
  `world:skill:effect {"skillId":"primary-attack","direction":"right","fromX":42,"fromY":115,...}`;
  the RMB click produced the same pair for `"skillId":"ability-1"`.
  `rmbContextMenuOpened: false` confirms plain RMB no longer opens the
  menu. The script exits non-zero unless all three checks hold (it
  printed `CAPTURES OK {"primaryAttackSent":true,"weaponSkillSent":true}`).

## Commits

- `01a12d7` feat(controls): rebindable action map + live input
  controller (TASK-0038)
- `4a8983c` feat(controls): LMB/RMB world attacks + rebinding UI
  (TASK-0038)
- (this report commit) test + e2e expectation updates, captures,
  REPORT/STATUS

## Deviations

1. **Unowned-but-required file edits** (GameCanvas.vue,
   slots/Settings.vue, hud/Quickbar.vue, input-controller.spec.js,
   hud-orb-ui.spec.js, browser-critical-loop.spec.mjs): the spec's owned
   globs don't cover where click handlers, the real settings pane, or the
   quickbar labels live. Edits are minimal and all binding logic stays in
   owned files. `tests/e2e` spec changes are expectation updates forced
   by the intended behaviour change (labels + Shift+RMB menu).
2. **Context menu moved to Shift+right-click** — the choice the spec
   §2 asks us to document ("moves to a modifier … keep the existing
   context menu reachable somehow"). Shift+RMB was picked over
   right-click-hold because it needs no timing heuristic and is
   discoverable: the settings hint text says so. If RMB is unbound from
   every action, plain right-click falls back to the menu.
3. **ESC during rebind capture also closes the settings pane** — the
   pane's earlier-registered ESC listener still fires; the capture itself
   is cancelled first. Acceptable; documented here.
4. **Pane hotkeys (c/i/j/p/digits) pressed during a rebind capture may
   also trigger their pane** via the capture-phase hotkey listener; the
   binding is still recorded correctly. Cosmetic; documented here.
5. **Space moved from primary attack to dash** per the D-007 default
   shape the spec mandates; the number-row aliases keep every action
   reachable from the keyboard.

## Risks / follow-ups

- D-115 play-check: attacking should feel immediate — the click path adds
  no round trips beyond the pre-existing `player:skill:trigger` flow.
- Gamepad support and any server protocol changes are explicit stop
  conditions and were not attempted.
- If the architect wants right-click-hold instead of Shift+RMB for the
  menu, the change is isolated to `GameCanvas.rightClick`.
