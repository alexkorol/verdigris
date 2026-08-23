# REPORT — TASK-0093: Native typography and text-rendering contract audit

- **Lane:** ox-pc-bc · **Model:** openrouter/stealth/ox-alpha
- **Branch/worktree:** `worker/verdigris/pc/ox-pc-bc` @ `Z:\Code\.worktrees\verdigris\ox-pc-bc`
- **Base SHA:** `d2423873c577d299b3b39c56024d1d840993c72b` (verified ancestor of HEAD at claim)
- **Claim commit:** `0a8aa40e` (STATUS CLAIMED, pushed to origin)
- **State:** audit complete → REVIEW_REQUESTED

## Executive summary

The native client's entire visible-text surface is GDI inside
`native/client/main.cpp` (`TextOutA` + `GetTextExtentPoint32A` + three ad-hoc
`CreateFontA` sites: Verdana damage numerals, Verdana 13px legend chips,
Georgia Chronicles lines); the majority of HUD/pane text selects no font and
rides the memory DC's stock font. The browser reference is tokenized
(GameFont/ChatFont/UIFont @font-face, fluid clamp() sizes, ellipsis truncation,
stroke-outlined centered world text). FINDINGS.md documents all of this with
line citations, measured WCAG contrast (two native pairs fail 4.5:1:
quickbar-unavailable 3.54, player-damage-over-terrain ~3.45 worst case),
resolution/DPI evidence (960x600/1366x768/1920x1080 exercised; zero DPI-awareness
calls), glyph-range requirements (UTF-8 boundary; today one em-dash survives on
ANSI luck), wrapping/alignment/clipping gaps (char-count truncation vs pixel
ellipsis; fixed −9px numeral offset vs measured centering), offscreen-capture
determinism (render-list JSON byte-equality already locked; pixels are not), and
Windows/macOS backend needs. It recommends a backend-neutral text contract
(roles, TextOp, measure/wrap/truncate surface, six invariants) plus eight
locking tests, a negative control (**persistent chat log** — no native
render-list equivalent), and an owner-only choices table. **No font was
downloaded, generated, licensed, or selected; no renderer was chosen;
render-list determinism and D-113 art authority untouched.**

## Approach

Read-only audit: constitution + DECISIONS (D-113/D-119) first; then exhaustive
grep/read of `native/client` GDI text sites, render-list op definitions,
D-119 scenario/capture harness, and the TASK-0159 top-HUD planner seam;
browser side via font declarations, canvas text draws, CSS tokens,
truncation/wrap rules. Contrast ratios computed from literal source RGB pairs
(script preserved at `C:\Users\Alex\AppData\Local\Temp\opencode\t0093-contrast.ps1`,
results in `captures/coverage.json`). Acceptance commands run literally at the
end, transcripts in `captures/`.

## Changed files (all inside owned path)

```
orchestration/tasks/TASK-0093-native-typography-contract-audit/
├── STATUS.md            (claim → REVIEW_REQUESTED)
├── FINDINGS.md          (deliverable: 12 sections incl. required tables)
├── REPORT.md            (this file)
└── captures/
    ├── coverage.json                        (machine-readable audit coverage)
    ├── acceptance-1-rg-text-surface.txt     (1635 lines + exit line)
    ├── acceptance-2-rg-resolution-typography.txt (13 lines + exit line)
    ├── acceptance-3-git-diff-check.txt      (exit=0)
    └── acceptance-4-git-diff-name-only.txt  (exit=0)
```

## Public interfaces added/changed

None. Audit-only capsule; no source, build, test, or asset changes anywhere.

## Test commands + outcomes (literal acceptance transcripts)

All four commands from SPEC.md §Acceptance were executed verbatim from the
worktree root. Exit codes captured inline in each transcript file.

**1) `rg -n "Text|Label|font|DrawText|text" native/client src/components src/assets src --glob "*.vue" --glob "*.css" --glob "*.cpp" --glob "*.hpp"`**
→ exit code 0, 1635 matched lines.
Full transcript: `captures/acceptance-1-rg-text-surface.txt`. Head:

```
src/components\util\ItemGrid.vue:18:        :aria-label="itemAriaLabel(i)"
src/components\util\ItemGrid.vue:23:        @mouseover="showContextMenu($event, i, true)"
src/components\util\ItemGrid.vue:24:        @contextmenu.prevent="showContextMenu($event, i)"
src/components\util\ItemGrid.vue:29:          v-text="getItemFromSlot(i).qty"
src/components\util\ItemGrid.vue:91:    bus.$on('game:context-menu:first-only', ClientUI.displayFirstAction);
src/components\util\ItemGrid.vue:94:    bus.$off('game:context-menu:first-only', ClientUI.displayFirstAction);
…
tail:
src/components\sub\ContextMenu.vue:346:        color: var(--color-text-secondary);
exit=0 lines=1635
```

Key native rows cited in FINDINGS §1 come from this sweep
(`native/client\main.cpp` TextOutA/CreateFontA/GetTextExtentPoint32A sites).

**2) `rg -n "1920x1080|1366x768|typography|panel" orchestration/benchmarks orchestration/tasks/TASK-0079-browser-panel-inventory`**
→ exit code 0, 13 lines. Full transcript:
`captures/acceptance-2-rg-resolution-typography.txt`. Verbatim:

```
orchestration/tasks/TASK-0079-browser-panel-inventory\SPEC.md:1:# TASK-0079 — Browser panel + typography inventory (delta #4 feed)
orchestration/tasks/TASK-0079-browser-panel-inventory\SPEC.md:9:browser's panels (expedition list, quest tracker, guide banner, chat,
orchestration/tasks/TASK-0079-browser-panel-inventory\SPEC.md:10:context menus) and its typography. Before that native work is specced,
orchestration/tasks/TASK-0079-browser-panel-inventory\SPEC.md:17:and situational UI panel in the browser client:
orchestration/tasks/TASK-0079-browser-panel-inventory\SPEC.md:19:1. For each panel: name, trigger (always-on / hotkey / NPC / event),
orchestration/tasks/TASK-0079-browser-panel-inventory\SPEC.md:20:   screen anchor + approximate size at 1920x1080, and the Vue component
orchestration/tasks/TASK-0079-browser-panel-inventory\SPEC.md:22:2. Typography table: font family/size/weight/color for panel titles,
orchestration/tasks/TASK-0079-browser-panel-inventory\SPEC.md:25:3. One capture per panel (capture harness from 0066; disposable port
orchestration/tasks/TASK-0079-browser-panel-inventory\SPEC.md:27:   folder, filenames `panel-<name>.png`.
orchestration/tasks/TASK-0079-browser-panel-inventory\SPEC.md:28:4. Rank panels by gameplay load-bearing-ness (what a player cannot
orchestration/benchmarks\side-by-side-2026-08-20\browser-scenes.mjs:16:  page.screenshot({ path: path.join(outDir, `browser-${name}-1920x1080.png`) });
orchestration/benchmarks\side-by-side-2026-08-20\BENCHMARK.md:12:2. HUD: browser has ornate orbs + iconed quickbar + minimap + panels;
orchestration/benchmarks\side-by-side-2026-08-20\BENCHMARK.md:16:4. Panels/typography: expedition panel, guide banner, chat — later.
exit=0 lines=13
```

Confirms TASK-0079 remains SPEC-only (no FINDINGS/captures exist upstream);
FINDINGS §2 states this instead of citing phantom artifacts.

**3) `git diff --check`**
→ exit code 0, no output (no whitespace/conflict-marker problems).
Transcript: `captures/acceptance-3-git-diff-check.txt` (`exit=0`).

**4) `git diff --name-only`**
→ exit code 0, empty output. All changes in this task are NEW files under the
owned folder; untracked files do not appear in `git diff`, so confinement is
proven by status instead. Supplementary evidence, run immediately after:

```
$ git status --short
?? orchestration/tasks/TASK-0093-native-typography-contract-audit/FINDINGS.md
?? orchestration/tasks/TASK-0093-native-typography-contract-audit/captures/
```

(STATUS.md was tracked by the earlier claim commit.) No file outside
`orchestration/tasks/TASK-0093-native-typography-contract-audit/**` was created,
modified, or deleted.

## Manual verification

- Every citation in FINDINGS.md was read directly from source this session
  (main.cpp regions 1719-1766, 2094-2320, 2428-2502, 2975-3202, 3470-3570,
  3636-3835, 3880-3896, 4131-4260, 5740-5846, 6250-6340; render_list.hpp;
  Chatbox.vue; perspective-renderer.js; map.js; fonts.scss; _tokens.scss).
- Contrast ratios recomputed with WCAG 2.1 relative-luminance formula
  (results reproduced in `captures/coverage.json`).
- Negative control verified both ways: browser chat exists
  (Chatbox.vue:439-441,468,494); `render::Op` enum has no chat member and
  ClientState has no chat seam.
- No servers started; port 6500 untouched; resource capsule respected
  (read-only).

## Commit SHAs

| Commit | Content |
| --- | --- |
| `0a8aa40e` | claim: STATUS CLAIMED (pushed to origin) |
| content commit | FINDINGS.md + captures/ + REPORT.md (see STATUS `implementation_commit`) |
| flip commit | STATUS → REVIEW_REQUESTED with frozen pushed head |

## Deviations

- None from scope. Interpretive note recorded above for acceptance command 4
  (`git diff --name-only` cannot list untracked new files; confinement shown
  via `git status --short`). Transcripts preserved unedited either way.

## Unresolved questions (owner input pending per spec)

1. Final font family selection + license approval (pixelmix /
   PxPlus_IBM_VGA8 / Px437 provenance included in that decision).
2. Renderer/backend choice (GDI continuation vs DirectWrite/CoreText-backed
   implementation of the same contract).
3. Unicode scope beyond Latin-1 + typographic punctuation.
4. Whether quickbar-unavailable state gains a non-color affordance and which
   contrast target (AA 4.5:1 blanket vs documented exceptions) applies.
5. Native chat: build now or defer (negative control shows zero parity).

## Risks & follow-ups

- Successor implementation tasks should take the contract in FINDINGS §9 +
  locking tests §10 as their acceptance skeleton; T5/T6 can land before any
  font decision because they test tokens and metrics, not faces.
- TASK-0079 panel inventory remains open upstream; its typography table will
  overlap §2 here and should cite back rather than re-measure.
- High-DPI displays blur all current text (no DPI awareness anywhere in
  `native/client`); worth folding into the next client platform task.

*Status flipped to REVIEW_REQUESTED with the frozen pushed head recorded in
STATUS.md.*
