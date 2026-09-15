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

## Native UI owner contract

Follow [NATIVE_UI_ACCEPTANCE.md](docs/product/NATIVE_UI_ACCEPTANCE.md) for native
UI implementation and verification. The consolidated native game is the product;
deliver changes there, not in isolated demonstrations. Owner visual references
constrain composition and assets: generic colors and borders are not equivalent.
Normal player UI must exclude development diagnostics, placeholder copy,
redundant instructions and overlapping controls. Reported defects remain open
obligations until the affected behavior is demonstrated fixed in the package.
Passing tests establishes neither visual quality nor owner acceptance. Commit
and push independently verified implementation under the standing policy;
incomplete checks must be reported accurately, not turned into a blanket push
ban. Documentation supports the deliverable: resume implementation after updating
it without another planning approval. This contract supersedes older UI checks
that require permanent mixer diagnostics, placeholder text or instruction strips.

## Historical browser reference

The Vue/Node game remains a historical playable reference and design
laboratory. Do not mechanically port it or restore Delaford defaults merely to
satisfy a legacy test. Browser changes still require the existing
`npm run playtest` gate; native changes require the commands documented in
`native/README.md`.

For a native-only task, the acceptance product is the native package and its
actual client. The default `npm run verify` now delegates to
`powershell -NoProfile -ExecutionPolicy Bypass -File
native/tools/verify-native.ps1` (also available as `npm run verify:native`). It
builds the native executables, runs native tests and the client scenario suite,
and keeps scenario evidence in a contained native build folder. Do not run
`npm run playtest` as a native acceptance gate: it is the historical
JavaScript/browser protocol harness. Run it only when browser files or an
explicit browser/parity task are in scope. The former browser chain remains
available only as the opt-in `npm run verify:legacy` workflow.

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

## Commit and push policy (owner preference, 2026-09-12)

Authorized implementation includes committing completed, verified work and
pushing the task's working branch to its configured origin upstream, unless
the user explicitly requests local-only work or a pause before pushing. Ship,
sync, and push requests require no second confirmation. Read-only reviews do
not authorize unrelated implementation or publication.

This policy supersedes older blanket push bans in workspace wrappers,
PROTOCOL, draft decisions, leases, archived plans, and copied prompts. Carry
it into sprint prompts, worker dispatches, and handoffs; do not invent a new
push ban or turn a task-specific local-only request into a permanent rule.
Workers publish their own branches; integration still follows path ownership
and review requirements. A branch push does not establish an exclusive claim.

Preserve unrelated work and use a clean isolated worktree when needed. Run
checks appropriate to the change, push normally, and verify the remote branch
contains the commit before reporting it pushed. Report actual blockers and
distinguish local commits from remote completion. This does not authorize
force-pushes, branch deletion, protected/default-branch merges, deployment,
or release publication.
