# TASK-0103 — REPORT

## Executive summary

Completed the monster/pack/rarity/encounter gap audit for the native
reconstitution at base `d2423873`. Evidence: `FINDINGS.md` (ranked ENGINE vs
CONTENT gaps with file:line citations, successor scaffolding S1–S6, negative-test
definitions N1–N8) and `captures/encounter-matrix.json` (13-dimension matrix:
spawning, pack composition, roles, aggro, rarity, equipment, unique/boss seams,
telegraphs, rewards, deterministic generation, network snapshots, presentation,
tests). Required negative control recorded: the `"uncommon"` rarity tier has a
consumer branch (`drop_monster_loot`, native/src/core.cpp:3092-3095) but no
producer anywhere in the codebase or coverage in any test/scenario. Frozen
invariants respected: stat symmetry (native/src/core.cpp:86-99 derives enemy
stats from `player_stats()`) and deterministic seeds (fnv1a instance seeds,
native/src/core.cpp:1734,3154) are cited, not challenged; scarcity/reward tables
remain documented unknowns. No roster/lore/balance choices were made.

## Approach

1. Preflight + claim commit on the lane worktree.
2. Read both encounter engines end-to-end: actor-space `Simulation`
   (spawn seam, elite telegraphs, death/rewards) and tile-space `WorldSimulation`
   (generation scatter, pack recipes, boss slam, loot), plus the protocol bridge,
   native client parse/render path, persistence boundary, seasonal hook, density
   bench, three native test binaries' encounter suites, and the browser encounter
   scenarios.
3. Classified every gap as content-neutral ENGINE or owner-dependent CONTENT;
   ranked ENGINE gaps by risk × blast radius; defined scaffolding seams and
   failing-today negative tests without authoring monsters/content.
4. Encoded the full map as the machine-readable encounter matrix; validated it
   with the SPEC's node command.
5. Ran every acceptance command verbatim; negative control documented in both
   FINDINGS.md and the JSON; committed per milestone.

## Changed files

- `orchestration/tasks/TASK-0103-monster-encounter-gap-audit/STATUS.md` (claim → review states)
- `orchestration/tasks/TASK-0103-monster-encounter-gap-audit/FINDINGS.md`
- `orchestration/tasks/TASK-0103-monster-encounter-gap-audit/captures/encounter-matrix.json`
- `orchestration/tasks/TASK-0103-monster-encounter-gap-audit/REPORT.md`

Nothing outside `orchestration/tasks/TASK-0103-monster-encounter-gap-audit/**`
was created or modified.

## Test commands + outcomes (verbatim from worktree root, PowerShell)

| Command | Exit code | Outcome |
|---|---|---|
| `rg -n "monster\|pack\|spawn\|rarity\|unique\|warden\|boss\|aggro\|telegraph\|elite\|role" native/include native/src native/client native/tests playtest/scenarios` | 0 | 1003 matching lines across all five scopes; audit corpus confirmed |
| `node -e "JSON.parse(require('fs').readFileSync('orchestration/tasks/TASK-0103-monster-encounter-gap-audit/captures/encounter-matrix.json','utf8')); console.log('encounter matrix: PASS')"` | 0 | `encounter matrix: PASS` |
| `git diff --check` | 0 | no whitespace/conflict-marker problems |
| `git diff --name-only` | 0 | empty output — working tree clean; all changes live in task-folder commits |

Expected result met: only task evidence changes exist between base and head.

## Key findings (top level; full ranked list in FINDINGS.md)

1. **Rarity vocabulary mismatch**: server default `"common"`
   (native/include/verdigris/core.hpp:760) vs client elite test
   `rarity != "normal"` (native/client/remote_session.cpp:666-668) renders every
   common monster as an elite/boss billboard at 1.85x
   (native/client/main.cpp:2076-2079). Untested seam.
2. **Telegraph geometry dropped on the wire→render path**: authored
   x/y/radius/durationMs for `monster:telegraph`
   (native/src/networking.cpp:1991-1994) discarded by the client
   (remote_session.cpp:571-584); boss ground-slam circle rendered as a
   player-facing cone (native/client/presentation_state.cpp:168-178).
3. **Roles and aggro are cosmetic/proximity-only**: behaviour_type has zero
   mechanics (ranged strike from Chebyshev≤1 like melee,
   native/src/core.cpp:1841-1845); monsters never move (static scatter
   core.cpp:1636-1716; facing-only pursuit core.cpp:661-665); disengage at range
   4 lets any fight reset (core.cpp:1865-1872). Elite telegraphs exist only in
   the actor sim, unreachable from networked encounters (core.cpp:1836-1859).

Also flagged: boss identity absent from dev:state wire
(networking.cpp:853-863), loot RNG decoupled from world seed
(core.hpp:984), `[swing]` fprintf debug in the swing path (core.cpp:1874),
boss placed by LCG slot with no arena/reachability guarantee
(core.cpp:1650-1716). No unique-tier concept exists in native code.

## Negative control

`"uncommon"` rarity: consumer branch exists (base gear chance 0.10,
native/src/core.cpp:3092-3095) but **no producer** — generator emits only
common/rare/elite (core.hpp:760; core.cpp:1684-1695) — and no test/scenario
covers it. Recorded in FINDINGS.md and `captures/encounter-matrix.json →
negative_control`, with the required generator-exhaustiveness negative test
defined (matrix N1 / FINDINGS N1).

## Commit SHAs (this lane)

| SHA | Message |
|---|---|
| `078640f6` | chore(TASK-0103): claim lane ox-sw-h |
| `56d45035` | docs(TASK-0103): encounter gap findings and encounter matrix evidence |
| *(final)* | docs(TASK-0103): report and review-requested status (see HEAD below) |

Final head SHA is reported in the worker handoff message.

## Deviations

- The SPEC file itself is absent at base `d2423873` inside this worktree; it was
  read from the coordinator checkout
  (`Z:\Code\Games\delaford\delaford_game\orchestration\tasks\TASK-0103-monster-encounter-gap-audit\SPEC.md`,
  read-only) and followed verbatim, including its acceptance block. No other
  deviation.
- `git diff --name-only` lists nothing because evidence was committed per
  milestone; `git status --short` before each commit showed only task-folder
  paths, satisfying the "ONLY task-folder changes" gate.

## Risks

- Findings cite line numbers at the audit head; line drift will occur as successor
  waves land — the companion JSON pins symbols + semantics alongside lines.
- The client rarity mapping (#1) and telegraph geometry (#2) are user-visible
  bugs that successor waves may mistake for content decisions; both now carry
  failing-today negative tests (N2/N5) to prevent that.
- Engine-level fixes touching draw order (S3/S6 scaffolding) can invalidate
  recorded seeds if drawn outside the documented order; the determinism negative
  tests pin this.

## Follow-ups (for the coordinator; no work performed here)

1. Route ENGINE #1/#2 to a presentation-seam packet (small, test-backed).
2. Adopt S1–S6 scaffolding packets before any roster wave; keep owner-dependent
   CONTENT items out of engine lanes.
3. Consider retiring the `[swing]` fprintf (core.cpp:1874) in the next native
   protocol slice touching combat.
4. Successor waves implement negative tests N1–N8 as they land the matching
   scaffolding; N1 and N2 are immediately actionable.
