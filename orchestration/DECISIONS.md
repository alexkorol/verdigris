# Canonical decisions

Settled = binding until the owner reverses it. Provisional = architect's
call, cheap to reverse, flagged for owner attention. Owner-only = no agent
may decide it.

## Settled

- **D-001 Language**: native runtime is C++20 (ADR-001,
  `docs/rebuild/ADR-001-native-language.md`). MSVC on Windows now; CMake is
  the cross-platform path. No general-purpose engine (no Godot/Unreal/
  Unity/Bevy-as-engine).
- **D-002 Simulation boundary**: deterministic fixed-timestep headless core;
  commands in, events out; no windowing/GPU/sockets/DB/DOM/assets in the
  core. Presentation may only consume events and snapshots.
- **D-003 Actor symmetry**: one stat schema and one damage pipeline for
  players, monsters, future mercenaries. Elites differ by level, build,
  equipment, and abilities — never by a separate stat universe.
- **D-004 House/Scion model**: the House is the persistent identity
  (lineage/tribe, not a building); Scions are mortal individuals. Campaign
  progress (routes, branches, knowledge) is House-owned. Unextracted value
  is lost on death; significant carried items enter a House relic pool and
  can re-enter the loot stream with history.
- **D-005 Setting**: Bronze Age / pre-iron fantasy. The Delaford legacy
  denylist (`config/legacy-denylist.json`) governs what may enter native
  production code. No medieval defaults, no Delaford starter kits.
- **D-006 Track isolation**: `prototypes/founding-slice/` is a disposable
  feel lab. Its code and numbers carry no design authority; findings enter
  canon only through this file or the constitution.
- **D-007 Native client control contract** (unblocks the client task; the
  E-key conflict raised by Codex is resolved as follows):
  - WASD movement (continuous), mouse aiming.
  - LMB primary attack, RMB weapon skill.
  - Space dodge/dash.
  - Q / E / R: additional skills (E is a SKILL slot, not equip).
  - X: pick up nearest item; Z: toggle loot-name filter; gold-like
    currency (if/when it exists) auto-picks.
  - F: contextual interact (extraction standard, shrines, doors).
  - I or Tab: gear/inventory; no context-menu dependency; no piano bar.
  Equip moves to the inventory UI, not a world key.

- **D-106 Death recoverability (OWNER-RULED 2026-08-16)**: items are never
  destroyed by Scion death. Everything carried (equipped, pack, trophies)
  returns to a recoverable pool — significant items to the relic pool,
  the rest to the wider loot pool at minimum. Supersedes the baseline
  "unequipped carried items are lost forever" behavior (TASK-0018).
- **D-107 Camera direction (OWNER-RULED 2026-08-16, resolves D-102)**:
  ARPG preset is the primary camera (pitch ~62, moderate perspective,
  no tilt-shift). Miniature-style treatment applies when the player zooms
  in with the wheel (blend toward stronger perspective/tilt at close
  zoom). High Table is rejected. Both clients default to the ARPG values.
- **D-108 Look/feel acceptance target (OWNER-SUPPLIED 2026-08-16)**: the
  webchat demo vendored at `docs/reference/25d-overhaul/` (playable
  `dist/songs-of-the-mire.html`, math in `docs/ARCHITECTURE.md`) is the
  acceptance target for rendering look and feel. Its gameplay is
  throwaway; its projection/terrain/lighting design is authoritative
  reference. The phased integration brief in its HANDOFF.md targets the
  browser game as the near-term shippable product while the native
  rebuild continues.

- **D-109 Forgiving persistence (OWNER-RULED 2026-08-16)**: logout,
  disconnect, or crash never loses items or progress — the Scion keeps
  everything and returns to town (House) on next login; the instance is
  simply left. Death is the only loss event, and networked play must
  prevent disconnect-caused deaths (safe pull-out on connection loss).
  ADR-002 is ACCEPTED as amended by this ruling. If logout-as-escape
  proves abusable, the fix is an in-danger logout delay, never item loss.

- **D-110 Playable-first (OWNER-DIRECTED 2026-08-16 evening)**: the
  measure of progress is a real player having a good session — first
  minutes clear, combat satisfying, loot exciting, death fair,
  progression legible. All further feature/infra/renderer work queues
  BEHIND fixing actual play friction. Evidence of friction comes from
  played sessions (TASK-0034), not harness scenarios.

## Provisional (architect's call, owner may override)

- **D-111 Day/night default**: owner is unsure the cycle belongs in the
  game. Until ruled: the game DEFAULTS to full daytime (most readable),
  with the cycle kept behind a settings toggle (off by default). Cheap to
  flip either way when the owner decides (TASK-0033).
- **D-116 THE MISSION (OWNER-RULED 2026-08-16 ~23:05)**: the point of
  this orchestration is the C++ conversion — the native version must
  reach the current web version's level OR BETTER, with multi-layer
  regression sweeps throughout. Strategy (architect): the C++ server
  speaks the EXISTING `{event,data}` WebSocket protocol so the current
  Vue client connects to it unchanged — which makes the existing
  31-scenario playtest harness the PARITY BAR and regression suite for
  the native server (run the same scenarios against JS and C++;
  divergence = regression). Parity lands endpoint-by-endpoint per
  `docs/rebuild/PARITY_ROADMAP.md`. Sweep layers: (1) protocol/playtest
  scenarios dual-run, (2) UI pane sweeps (0036 pattern), (3) core
  determinism replays, (4) D-115 play gate. The browser game stays the
  living reference and keeps improving (current playability wave
  continues) — every improvement raises the parity bar deliberately.
- **D-112 Two horizons, one product at a time (amended 2026-08-16 late
  after owner sustainability question; SUPERSEDED IN PART by D-116 —
  the native conversion is not deferred; it proceeds now via protocol
  parity)**: the BROWSER game is the
  near-term playable and fun-finding vehicle ONLY — not the launch
  platform (own measurement: ~43ms mean frame at 1440×1000; browsers
  fight a mature ARPG's density). The NATIVE build is the launch
  platform, per the constitution. Sequence: exe triage → browser reaches
  "actually fun" via the 0034 friction list → the native client then
  rebuilds the browser-VALIDATED design system by system under the
  D-115 play gate until parity, and becomes the shipped product. No
  parallel design invention across surfaces; browser perf work capped at
  "smooth enough to judge fun."
- **D-113 Art direction (from owner feedback)**: near-term aesthetic
  follows the webchat/slice approach — procedural/vector-first with
  minimal image inputs, consistent RELATIVE SCALE (a documented scale
  chart: player height as the unit; scenery sized against it). No return
  to Delaford-era assets. Owner supplies/approves any new image assets.
- **D-114 Feel-coherence rule (process)**: any change to movement,
  range, or speed constants must re-derive ALL related distance/time
  constants together against seconds-to-contact metrics, documented in
  one table in the diff. Reviews reject isolated constant changes.
- **D-115 Play gate (process)**: feel-affecting work is not ACCEPTED
  until the architect has driven a full session of the actual build and
  judged it as an experience, not per-feature evidence.

- **D-101 Player base-life offset**: the slice gives player-kind actors a
  modestly higher base-life constant within the shared formula. If adopted
  natively, express it as starting equipment/traits instead of a kind check.
- **D-102 Camera envelope**: 2.5D perspective-billboard, pitch ~52–62°,
  mild depth perspective, tilt-shift optional. The slice's "Miniature"
  preset is the current directional target; final projection stays an open
  experiment (camera lab preserved in both clients).
- **D-103 Slice banking**: the demo banks pack items at node completion for
  pacing. Native keeps true extraction risk per the constitution.

- **D-104 CMake presets schema v2**: `native/CMakePresets.json` targets
  presets schema version 2 (CMake ≥3.20, including the MSVC-bundled
  binary), not v3. Configure/build/test presets only; no v3-only fields.
  Revisit only when a demonstrated v3-only capability is needed
  (see TASK-0002 REVIEW, QUESTION-0001).

- **D-105 smoke:browser lifecycle**: QUESTION-0003 resolved with its
  option 1 — `smoke:browser` keeps its documented name and adopts the
  same `start-server-and-test` lifecycle as `test:e2e` (TASK-0014).
  Port 6500 stays pinned.

## Owner-only (do not decide by agent)

- **D-O1 Seasonal inheritance rule** — what survives a season reset
  (see `docs/product/OPEN_DECISIONS.md`).
- **D-O2 Asset pipeline/provenance** — vendoring full-resolution plates,
  packaging, and any generated-asset policy for the native game.
- **D-O3 Magic system** — production spell design waits on the actual
  WIZARD Spell/Arcane Lattice material
  (`docs/product/WIZARD_ARCANE_LATTICE_REFERENCE.md`); no generic mana
  wizard in the meantime.
- **D-O4 Monetization/distribution, final naming, lore canon.**
- **D-O5 Economy scope** — trade, passive income, currency exchange
  (flagged in the feature checklist; seams only until decided).
