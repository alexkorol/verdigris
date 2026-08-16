# Verdigris agent guide

This is the canonical cross-platform agent guide. Read it before editing and
follow the repository preflight before planning or implementation.

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
Build and test the core before polishing a client.

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
