# Verdigris agent guide

This is the canonical cross-platform agent guide. Read it before editing and
follow the repository preflight before planning or implementation.

**Implementation coordinators (Codex, Kimi Code):** your binding process
doc is `orchestration/PROTOCOL.md`. Kimi Code: start at
`orchestration/ONBOARDING-KIMI.md`.

## Required preflight

From the repository root, run:

```bash
git status --short
git remote -v
git fetch --prune origin
git status -sb
git rev-list --left-right --count HEAD...@{upstream}
```

Do not edit on a dirty, stale, or diverged branch. Preserve user work and
resolve remote configuration deliberately. Native reconstitution work belongs
on a `codex/` branch and must not overwrite the historical browser game.

## Product authority

Before gameplay, item, campaign, setting, House, or progression work, read:

```text
docs/product/VERDIGRIS_CONSTITUTION.md
```

The constitution outranks inherited Delaford behavior, obsolete tests, archived
plans, and current implementation accidents. Use
`docs/rebuild/LEGACY_MATRIX.md` and `config/legacy-denylist.json` when deciding
what may cross into native production code.

## Native boundary

The native workspace is `native/`. Keep simulation deterministic, fixed-step,
headless, and independent of windowing, GPU, sockets, SQLite, DOM, and assets.
Presentation requests commands; simulation resolves them and emits events.

The deterministic core is built and green. **The player-facing client is the
product bottleneck now.** "Build the core before polishing a client" served
its era and is retired: it taught a generation of lanes to ship provable
geometry and skeleton chrome while the owner repeatedly reported an
unplayable-feeling game. Do not use core priority to defer experience work.

## Native presentation gate (binding, owner-ruled 2026-08-30)

The repository's gates were all correctness-shaped (render-list ops,
determinism, denylists) and none were experience-shaped, which is why "1 FPS
under input", "invisible quest text", and "tiny window" each shipped behind a
fully green suite. These rules close that hole:

1. **See the game before claiming presentation work.** Launch it
   (`native/tools/play-native.ps1`), capture the live window
   (`native/tools/capture-window.ps1 -OutPath <png>`), and look at the
   capture. Agent-harness desktop screenshots are typically masked; the
   capture tool is the supported way for an agent to see pixels. A
   presentation claim without a viewed capture is an unverified claim.
2. **Frame budget is a machine gate.** `--scenario all` includes
   `frame-budget` (20 real 32bpp frames at 3440x1440 through the production
   paint path, <40 ms average). Never raise the bound to pass; find the
   cost. The F3 overlay shows live paint milliseconds.
3. **Input handlers must be trivial.** WM_MOUSEMOVE can arrive at 1000 Hz
   and WM_PAINT/WM_TIMER are Windows' lowest-priority messages: per-event
   simulation syncs or invalidations starve the frame loop into single-digit
   FPS. Store input; let the fixed tick consume it.
4. **All HUD chrome goes through `native/client/ui_skin.hpp`** (GDI+ panels,
   orbs, slots, chips, type ramp). Raw-GDI rectangles for UI surfaces are a
   regression to the skeleton era. Extend the skin; don't bypass it.
5. **Unbounded presentation state is a defect.** Anything that grows per
   input or per event (effects, logs, labels) needs a cap or rate limit at
   the point of growth.

## Historical browser reference

The Vue/Node game remains a playable reference and design laboratory. Do not
mechanically port it or restore Delaford defaults merely to satisfy a legacy
test. Browser changes still require the existing `npm run playtest` gate; native
changes require the commands documented in `native/README.md`.

Before claiming a browser gameplay change works, run the real protocol harness
(`npm run playtest`) and, for client/UI changes, the browser gate
(`npm run smoke:browser`). Unit tests alone do not prove canvas focus, context
menus, WASD after UI interaction, or rendered HUD state. Do not leave watch
servers running for probes. The browser server remains pinned to port 6500.

The existing WebSocket envelope is `{ event, data }`; server handlers receive
the payload at `data.data`. Preserve the current guest/Chronicles login seams,
server-authoritative progression, and graceful loading of stale persisted data
when touching the historical reference.

## Handoff discipline

Work in coherent milestones, commit each green milestone, and update
`docs/rebuild/HANDOFF.md` after each one. Never leave the only meaningful work
uncommitted.
