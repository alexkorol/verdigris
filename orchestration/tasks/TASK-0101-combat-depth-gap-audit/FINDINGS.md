# TASK-0101 — Combat depth and feel gap audit (FINDINGS)

- worker: ox-pc-ai (openrouter/stealth/ox-alpha), coordinator: codex
- base: `610a240e1e4bdfacfd77bec49e36be945a1ced13`, branch
  `codex/TASK-0101-combat-depth-gap-audit-ox-pc-ai`
- companion artifact: `captures/combat-matrix.json` (deterministic, same citations)
- scope: read-only analysis; no play server; port 6500 untouched

## Method

Compared `docs/product/VERDIGRIS_CONSTITUTION.md` (actors/combat 87–116,
magic 118–123) against both authoritative native surfaces — the D-114
world-unit `Simulation` (`native/include/verdigris/core.hpp:314`) and the
tile-space parity world `WorldSimulation` (`core.hpp:860`) — plus their tests,
client presentation (`native/client/`), and the audio cue seam
(`native/audio/`). Evidence was gathered by reading the cited regions at the
frozen base and by literal greps recorded in the matrix. No code outside the
task folder was modified.

## Frozen invariants observed

Actor symmetry, one damage pipeline per surface, D-114 coherence, and the
D-115 gate are treated as frozen (SPEC; `orchestration/DECISIONS.md:114–120`).
Two findings below are framed as *coherence gaps* precisely because they are
violations of spirit, not invitations to add a second pipeline: telegraphs and
damage resolution currently differ between surfaces that the constitution
treats as one game.

## Key findings

1. **The implemented action vocabulary is six verbs** — `{Melee, Dash, Wait,
   Thrust, Sweep, WarCry}` (`core.hpp:27`). Against the constitution's family
   list (swings, thrusts, slams, leaps, guarded actions, buffs, war cries,
   combos, ranged attacks, magic — constitution 99–101): swings/thrusts/war
   cries are present and test-locked; slams exist only as an enemy-only boss
   mechanic in the tile-space path (`core.cpp:1986–2011`); leaps, guarded
   actions, combos, and magic are absent (grep exit 1 each; combos is the
   designated negative control); buffs generalize to exactly one hard-coded
   self-buff (`core.cpp:347–353`); ranged attacks are advertised data with no
   behavior (`core.cpp:1742–1750` assigns `behaviour_type`, but resolution at
   `core.cpp:2012–2019` treats every non-boss monster identically).

2. **Enemy responses are thin on both surfaces.** D-114 elites schedule
   Thrust/Sweep through a real telegraph contract with cancel-on-death and
   replay determinism (`core.cpp:649–720`; locks at
   `core_tests.cpp:2088–2094`). Tile-space monsters are a uniform 2-tile /
   1500 ms melee blob regardless of authored role. No enemy uses equipment,
   support, or range. The constitution's "elite difficulty from level, build,
   equipment, actions, and support" (constitution 90–92) is realized only by
   the action term.

3. **Telegraph contracts diverged.** D-114: `kTelegraphTicks = 3` ticks via a
   typed event consumed from a typed catalog (`core.hpp:32`,
   `main.cpp:1594–1631`). Tile-space: boss slam is a file-local
   `1000 ms / radius 2` constant pair (`core.cpp:1473–1474`) absent from
   `PresentationCatalog` (`core.hpp:92–104`), and the remote client guesses
   windup units with `event.value > 20 ? value/50 : value`
   (`presentation_state.cpp:239`). This is a coherence gap, not a tuning one.

4. **Impact feedback is visually strong, aurally narrow.** Render ops cover
   swing arcs, telegraph cones/arcs with HUD-reserve avoidance, hit flashes,
   target tints, floating numbers, critical treatment, screen-edge pulse,
   death/dash/drop/spawn beats (`render_list.hpp:22–30`;
   `main.cpp:1519–2060`; scenario locks `main.cpp:4371–4449`). Audio maps
   exactly five beats — hit, crit, kill, scion-lost, warcry-expire — leaving
   AttackStarted, Telegraph, Dash, slam impact, and buff activation silent
   (`event_cues.cpp:9–63`). Hit-stop/knockback/stagger do not exist anywhere.

5. **Equipment effects are mostly decorative data.** Consumed in play: flat
   `attack_bonus` in the shared formula (`core.cpp:409–410`) and, tile-space
   only, crit/beastbane/goods-found mods (`core.cpp:1950–1959,3152`).
   Derived-but-never-consumed: `block_chance` (summed and capped at
   `core.cpp:2577,3037–3042`, rolled nowhere), `ActorStats.resistances`
   (schema field, never read by any pipeline), defense channel totals,
   `attack_style` as a wire label only. Nothing changes reach, cadence,
   attack form, or movement — the explicit constitution promise for equipment
   (constitution 48–51). Critical chance also does not exist on the D-114
   surface at all.

6. **Control vocabulary is nearly complete.** WASD/mouse aim/LMB/RMB/QER/
   Space/Z/X are all wired and HUD-advertised (`main.cpp:3786–3980`,
   quickbar `2396–2401`, hints `3508,3676`). The one missing control contract
   is **gold auto-pickup** (constitution 97–98): coins drop as manually
   collected ground items (`core.cpp:3152–3157`) and the D-114 surface has no
   currency at all.

7. **Test coverage is deep where mechanics exist and mute where they don't.**
   The core locks listed in the matrix pin dash shape, resource gates, cones,
   multi-target sweeps, telegraph timing/cancel/replay, non-elite cadence,
   war-cry expiry, and catalog stability; session tests prove hits and
   telegraphs reach clients and that Gate-B's slam is dodgeable from payload
   alone. There is deliberately no lock proving `block_chance`,
   `resistances`, or `behaviour_type` inert — those paths are simply
   unreachable today.

## Classification

- **Missing mechanics (routable now):** ranged/support behaviour realization;
  telegraph catalog unification; effect-slot generalization; passive
  consumption of block/defense/resist channels; gold auto-pickup.
- **Feel tuning (mechanism exists, owner-judged values; D-114/D-115 gated):
  ** impact-feedback breadth (audio rows, stagger/hit-stop if the owner
  wants them), telegraph window readability.
- **Owner-only design (audit stops):** leaps, combos, player slams, magic,
  new skills/names, every numeric retune.

## Recommended successors (ranked)

Each names exact paths a future task would own, dependencies, negative
controls, locking tests, and the owner-visible outcome. None proposes values.

### W1 — Realize ranged behaviour in tile-space combat (rank 1)
- Own: `native/src/core.cpp` (advance_combat monster branch ~2012–2022),
  `native/tests/core_tests.cpp`.
- Dep: none; wire already carries `behaviour.type` (`networking.cpp:930`).
- Negative control: a `behaviour_type == "melee"` monster must produce a
  byte-identical event stream before/after the wave (existing N2/N3 suites
  stay green unchanged).
- Locking tests: seeded world where a `ranged` monster damages the player
  from beyond 2-tile Chebyshev contact while a `melee` twin does not; replay
  determinism of the ranged stream; `buffer` stays inert unless routed.
- Owner-visible outcome: expedition packs visibly mix contact and pressure
  roles using already-broadcast data; values reuse authored constants until
  the owner retunes.

### W2 — One telegraph catalog row per telegraphed action (rank 2)
- Own: `native/include/verdigris/core.hpp` (PresentationCatalog),
  `native/src/core.cpp` (catalog builder + slam constants), 
  `native/client/presentation_state.cpp` (delete the unit-guess heuristic at
  line 239), `native/client/main.cpp`, `native/tests/core_tests.cpp`.
- Dep: none; prerequisite for any future telegraphed slam/leap-style wave.
- Negative control: catalog equality test extended to fail when a surface's
  window is edited without the catalog (extends
  `test_presentation_catalog_is_authoritative_and_stable`).
- Locking tests: remote-path windup renders from catalog values only (no
  arithmetic on `event.value`); boss slam window/radius read from catalog.
- Owner-visible outcome: identical telegraph readability on both surfaces and
  both client paths; heuristics gone.

### W3 — Per-actor effect slots behind BuffApplied/BuffExpired (rank 3)
- Own: `native/include/verdigris/core.hpp` (Actor), `native/src/core.cpp`,
  persistence adapter seam, `native/tests/core_tests.cpp`,
  `native/tests/session_tests.cpp`.
- Dep: foundational for future buff/debuff/status and magic waves; must keep
  stale-snapshot restore working (AGENTS.md grace rule).
- Negative control: removing the war-cry special case must fail
  `test_war_cry_buff_expiry_and_replay_determinism` unless the slot list
  reproduces it byte-for-byte.
- Locking tests: two simultaneous effects tick down independently and expire
  in deterministic order; snapshot round-trip preserves slots; old snapshots
  load.
- Owner-visible outcome: adding the next buff/status stops requiring Actor +
  serializer surgery; mechanism only, no new effects designed here.

### W4 — Consume defensive equip channels passively in the one pipeline (rank 4)
- Own: `native/src/core.cpp` (resolve_damage defender side), 
  `native/include/verdigris/core.hpp`, `native/tests/core_tests.cpp`.
- Dep: standalone; composes with W3 if blocks become visible beats.
- Negative control: zero-block/zero-defense-modifier actors produce
  bit-identical damage streams to today (guards the frozen pipeline).
- Locking tests: block_chance rolls consume the deterministic sim RNG and cap
  at the authored 75; resistances reduce matching damage only when nonzero;
  symmetry test extends to defenders (`resolve_damage` used for both kinds).
- Owner-visible outcome: defensive loot changes survivability; active guard
  stances remain owner design.

### W5 — Gold auto-pickup (rank 5)
- Own: `native/src/core.cpp` (WorldSimulation pickup-on-proximity),
  `native/tests/core_tests.cpp` (N4 suite).
- Dep: none (tile-space path; D-114 has no currency to extend).
- Negative control: non-currency ground items are never auto-collected.
- Locking tests: stepping onto/adjacent to a coin stack removes it and
  increments `coin_total()` deterministically; gear/trophies require X.
- Owner-visible outcome: constitution control contract (97–98) honored;
  coin showers stop demanding X-presses.

### W6 — Audio cue rows for existing beats (rank 6)
- Own: `native/audio/event_cues.{hpp,cpp}`, `native/audio/cue_spec.*`,
  `native/tests/presentation_events_tests.cpp` or audio mixer tests.
- Dep: none.
- Negative control: unknown events remain silent (existing table contract,
  `event_cues.hpp:15–18`).
- Locking tests: AttackStarted→swing cue, Telegraph→warning cue, dash→dust
  cue map deterministically; placeholder params flagged owner-final per
  TASK-0157 ownership.
- Owner-visible outcome: swings and enemy windups become audible; zero
  authority change.

## Honest gaps in this audit

- Presentation coverage was read from code and scenario assertions; no build
  or capture was executed under this read-only capsule, so visual claims rest
  on the shipped scenario/test evidence rather than fresh captures.
- The browser reference was not consulted beyond what native comments cite;
  where the constitution and native comments disagree, the constitution was
  treated as authoritative.
- W1/W4 need owner-supplied values eventually; successors are routable with
  placeholder reuse of authored constants, but retunes themselves stay
  owner-only.
