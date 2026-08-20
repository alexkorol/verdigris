# TASK-0079 — Browser panel + typography inventory (delta #4 feed)

**Packet type:** MECHANICAL (browser/docs only — NO native/** changes)
**Lane fit:** luna-mac (or any MECHANICAL lane). Ports: per-lane capsule.

## Why

Presentation delta #4 (BENCHMARK.md): the native client lacks the
browser's panels (expedition list, quest tracker, guide banner, chat,
context menus) and its typography. Before that native work is specced,
we need a precise inventory of what the browser actually renders —
otherwise the native task guesses.

## Scope

Produce `FINDINGS.md` in this task folder cataloguing every persistent
and situational UI panel in the browser client:

1. For each panel: name, trigger (always-on / hotkey / NPC / event),
   screen anchor + approximate size at 1920x1080, and the Vue component
   path under `src/` that renders it.
2. Typography table: font family/size/weight/color for panel titles,
   body rows, tooltips, damage numbers, chat (read from the CSS/
   computed styles — cite the file).
3. One capture per panel (capture harness from 0066; disposable port
   from your capsule; owner :6500 untouchable) saved in the task
   folder, filenames `panel-<name>.png`.
4. Rank panels by gameplay load-bearing-ness (what a player cannot
   play well without) — that ordering becomes the native task's
   phasing.

## Non-goals

No code changes anywhere. No native work. No redesign opinions —
inventory only.

## Verification

- FINDINGS.md tables complete, every row citing a real `src/` path
  (spot-checkable by grep).
- Captures present and referenced from the tables.
- `npm run playtest` untouched (no code changed — state it).
