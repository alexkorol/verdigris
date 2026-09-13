# TASK-0115 REPORT — Browser panel and typography inventory

- **Task:** TASK-0115 (packet MECHANICAL, topology INDEPENDENT, supersedes TASK-0079)
- **Lane / model:** ox-pc-bc / openrouter/stealth/ox-alpha
- **Base commit:** `d2423873c577d299b3b39c56024d1d840993c72b` (verified ancestor of work HEAD before starting)
- **Branch:** `worker/verdigris/pc/ox-pc-bc` (worktree `Z:\Code\.worktrees\verdigris\ox-pc-bc`)
- **Claim commit:** bd3bde95 (`status(TASK-0115): claim browser panel and typography inventory`)
- **Scope discipline:** every changed/new path is inside
  `orchestration/tasks/TASK-0115-browser-panel-typography-inventory/**` —
  proven below via literal `git status --short`. No src/, server/, native/,
  config, or test changes. No font selection. No native work.

## Outcome

19/19 browser panels inventoried and captured GREEN at 1920x1080 AND
1366x768 (38 hard-fail PNGs), each with a driver assertion proving the named
panel visible before its PNG counts, measured bounding boxes + computed
typography per viewport, trigger/anchor/mounted-path/gameplay-load/phasing-rank
rows in `captures/panels.json`, full analysis + CSS citations in FINDINGS.md,
and a passing negative control proving the capture driver's hard-fail path.

No panel required the RED fallback; nothing required port 6500 or any source
or assertion change to reach.

## Preflight (AGENTS.md), verbatim

```
$ git status --short            -> (clean)
$ git fetch --prune origin      -> (ok)
$ git status -sb                -> ## worker/verdigris/pc/ox-pc-bc...origin/codex/native-reconstitution
$ git rev-list --left-right --count 'HEAD...@{upstream}' -> 0    0   (in sync)
$ git merge-base --is-ancestor d2423873… HEAD -> ok ("base is ancestor of HEAD")
```

## Environment fix (node_modules only, not tracked)

First server start failed: `better-sqlite3` native binding absent in this
worktree's node_modules (Node v22.23.2 / ABI 127). Fixed without touching any
tracked file:

```
$ npm rebuild better-sqlite3        -> exit 0 ("rebuilt dependencies successfully")
$ node -e "require('better-sqlite3')" -> "better-sqlite3 loads OK"
```

## Capture runs, verbatim transcripts

Main run (production build from this branch; capsule port 6620 loopback):

```
$ CAPTURE_PORT=6620 node orchestration/tasks/TASK-0115-browser-panel-typography-inventory/captures/capture-0115.mjs
[capture-harness] production build (PORT 6620)
[capture-0115] 1920x1080
[capture-0115] context menu hit at 966,540
[capture-0115] 1366x768
[capture-0115] context menu hit at 689,384
CAPTURES OK {"1920x1080.hud-chrome-visible":true,…,"1366x768.context-menu-visible":true}
driver exit=0
```

Full 38-check JSON line stored in `captures/capture-0115-run.log`; checks
mirror into `captures/capture-0115-checks.json`, measurements into
`captures/capture-0115-evidence.json`.

Negative control (disposable output path, injected false assertion):

```
$ NEGATIVE_CONTROL=1 SKIP_BUILD=1 CAPTURE_PORT=6621 node …/capture-0115.mjs
[negative-control run reaches all panels, then]
Error: CAPTURE FAILED: negative-control.false-visibility-inventory
negative-control exit=1
```

Exit code 1 recorded; disposable `captures/.tmp-negative-control/` removed
afterwards (`Remove-Item -Recurse -Force` → removed=True). Transcript:
`captures/capture-0115-negative-control.log`.

## Acceptance commands, verbatim with exit codes

```
$ rg -n "font-family|font-size|font-weight|color" src --glob "*.vue" --glob "*.css"
  exit=0  (827 matching lines; full output: captures/rg-typography-sweep.txt)

$ rg -n "component|panel|pane|chat|quest|expedition|guide|context" src/components --glob "*.vue"
  exit=0  (626 matching lines; full output: captures/rg-components-sweep.txt)

$ node -e "const p=JSON.parse(require('fs').readFileSync('orchestration/tasks/TASK-0115-browser-panel-typography-inventory/captures/panels.json','utf8')); if(!Array.isArray(p.panels)||!p.panels.length) process.exit(1); console.log('panel inventory: PASS')"
panel inventory: PASS
  cmd exit=0

$ git diff --check
  exit=0  (no whitespace errors)

$ git diff --name-only
  exit=0  (empty — all deliverables are new untracked files under the owned folder)
```

Owned-path proof:

```
$ git status --short
?? orchestration/tasks/TASK-0115-browser-panel-typography-inventory/FINDINGS.md
?? orchestration/tasks/TASK-0115-browser-panel-typography-inventory/captures/
```

Lint of committed driver:

```
$ npx eslint orchestration/tasks/TASK-0115-browser-panel-typography-inventory/captures/capture-0115.mjs
  exit=0 (no output)
```

## Capture summary

| Panel | 1920x1080 | 1366x768 | Visible check |
| --- | --- | --- | --- |
| hud-chrome | panel-1920x1080-hud-chrome.png | panel-1366x768-hud-chrome.png | true/true |
| world-minimap | …-world-minimap.png | …-world-minimap.png | true/true |
| chat-peek | …-chat-peek.png | …-chat-peek.png | true/true |
| panel-nav | …-panel-nav.png | …-panel-nav.png | true/true |
| guide-banner | …-guide-banner.png | …-guide-banner.png | true/true |
| party-panel | …-party-panel.png | …-party-panel.png | true/true |
| zone-menu | …-zone-menu.png | …-zone-menu.png | true/true |
| roads-chart | …-roads-chart.png | …-roads-chart.png | true/true |
| chatbox-expanded | …-chatbox-expanded.png | …-chatbox-expanded.png | true/true |
| character-pane | …-character-pane.png | …-character-pane.png | true/true |
| inventory-pane | …-inventory-pane.png | …-inventory-pane.png | true/true |
| quests-pane | …-quests-pane.png | …-quests-pane.png | true/true |
| skill-tree-overlay | …-skill-tree-overlay.png | …-skill-tree-overlay.png | true/true |
| settings-overlay | …-settings-overlay.png | …-settings-overlay.png | true/true |
| logout-overlay | …-logout-overlay.png | …-logout-overlay.png | true/true |
| loot-moment | …-loot-moment.png | …-loot-moment.png | true/true |
| death-overlay | …-death-overlay.png | …-death-overlay.png | true/true |
| escape-menu | …-escape-menu.png | …-escape-menu.png | true/true |
| context-menu | …-context-menu.png | …-context-menu.png | true/true |

All PNGs live in `captures/` (~50 MB total; precedent TASK-0059 shipped ~36 MB).

## Method notes / deviations

- Context menu: plain RMB is bound to ability-1 (`controls.js:45-52`,
  D-007) so the menu requires Shift+RMB (`GameCanvas.vue:345-350`). CDP
  synthetic Shift+RMB did not reach the Vue handler (two probe scripts,
  temp dir outside the repo, deleted), so the driver dispatches the same
  trusted-shape DOM `MouseEvent('contextmenu', {shiftKey:true})` the browser
  produces; the real server round-trip answered `Walk here` + Cancel. No
  source/assertion change was needed — SPEC stop-line respected.
- `npm rebuild better-sqlite3`: environment repair inside node_modules only.
- Two earlier failed driver attempts (missing sqlite binding; a locator bug
  fixed in the driver itself) are part of the honest history; final state is
  the green run above.

## Files delivered (all under the owned folder)

```
FINDINGS.md
REPORT.md
STATUS.md                       (CLAIMED → REVIEW_REQUESTED flip)
captures/panels.json            machine registry, 19 panels × both viewports
captures/capture-0115.mjs       hard-fail capture driver (shared-helper based)
captures/capture-0115-evidence.json  measured boxes + computed fonts
captures/capture-0115-checks.json    38 visibility checks (all true)
captures/capture-0115-run.log         main run transcript
captures/capture-0115-negative-control.log
captures/rg-typography-sweep.txt     acceptance sweep #1 full output
captures/rg-components-sweep.txt     acceptance sweep #2 full output
captures/rg-fontface-sweep.txt       @font-face usage map
captures/rg-color-citations.txt      accent-color citations
captures/panel-1920x1080-*.png  (19)
captures/panel-1366x768-*.png   (19)
```

Commits: claim bd3bde95 → evidence/report commit (see git log; listed as
implementation_commits in STATUS.md) → STATUS flip to REVIEW_REQUESTED.
Frozen review head: pushed tip of `worker/verdigris/pc/ox-pc-bc` at request
time; no force-push, no rebase after this line.
