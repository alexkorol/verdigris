---
task: TASK-0119
type: FINDINGS
worker: ox-pc-u
base_commit: 9fe673b66ffc082e865e0f0fb66f454ec1984949
captured_at: 2026-08-22
companion_evidence: captures/first-session.json
---

# TASK-0119 — Native onboarding and first-session journey audit

## Executive summary

The native owner journey is real and mostly legible: launch → Chronicles front
door → found House → name (auto-derived) Scion → oath choice → admission →
WASD/mouse combat with telegraphs and damage feedback → loot/equip → EXIT
extraction → banked counts → mortal fall/succession/crypt/relic recovery →
clean quit/relaunch with roster restore → honest connection states. The
accepted TASK-0145 front door and scenario harness are the load-bearing
achievements; this audit found **no contradiction between the client and the
constitution's durable loop** in what exists.

Two required player decisions are **not currently legible**, and they are the
audit's negative controls:

- **G-1 Goal choice (SPEC negative control).** The durable loop begins with
  "choose a specific expedition goal"
  (`docs/product/VERDIGRIS_CONSTITUTION.md:22-35`), but the client has no
  route/goal surface at all: the only zone entry is a hardcoded `N` key that
  enters `tin:1:0` (`native/client/main.cpp:3189-3190`), unlisted even in the
  F3 help text (`main.cpp:3001-3009`). The simulation already emits an
  expedition-phase signal (`SlayWardens → ExtractCarriedValue`,
  `native/include/verdigris/core.hpp:251,301`; `native/src/core.cpp:850-859`)
  that `event_label()` drops on the floor (`main.cpp:1673-1694`).
- **G-2 Progression beyond gear.** The server derives levels from combat XP
  (`native/src/networking.cpp:2045-2052`) and ships a `passiveTree` payload
  (`native/src/networking.cpp:586,817`), but no client code reads it
  (`native/client/remote_session.cpp:614-917` never touches `passiveTree`);
  level-ups are silent and the tree has no surface, so the constitution's
  player-facing progression trio (inventory ✓, stats ✓, passive tree ✗,
  `VERDIGRIS_CONSTITUTION.md:103-106`) is one-third invisible.

Everything else is friction, not blockers — detailed per step below.

## Method and evidence discipline

- Static audit of the routed base `9fe673b6` (spec base `9bd689b4`); read-only
  resource capsule honored: no ports opened, no play-server mutation, no
  runtime captures produced by this task.
- Every claim cites file:line on this base; behavioral claims that rest on more
  than reading cite the existing deterministic scenario harness results
  committed in source (`first-fight`, `loot-to-bank`, `telegraph-dodge`,
  `combat-juice`, `chronicles-gate-b` in `native/client/main.cpp:3385-4188`)
  or accepted reviews (`orchestration/tasks/TASK-0145-.../REVIEW.md`).
- Narrative wording, names, and lore remain owner-only (SPEC
  `owner_input_dependency`). Every proposed fix below is structural:
  surfacing existing authoritative state, correcting a contract mismatch, or
  adding an input seam. No copy is invented anywhere in this audit.

## Journey matrix (condensed; full version in captures/first-session.json)

| # | Phase | Player decision | Legible | Load-bearing evidence | Worst friction | Smallest non-lore fix |
|---|-------|-----------------|---------|----------------------|----------------|----------------------|
| 1 | launch | run owner launcher vs bare exe | yes | `play-native.ps1:392-402`; `main.cpp:4595-4646` | bare exe = anonymous local testbed, hardcoded House | make owner path the default; flags stay explicit |
| 2 | connect | wait for chronicle payload | yes | `remote_session.cpp:277-297,650-671`; `main.cpp:2483-2495` | none material | none |
| 3 | house/scion | found, create, oath, set out | yes | `main.cpp:2321-2369,2407-2426`; gate-b 38/38 | no text input anywhere; names auto-derived (`main.cpp:2292-2317`) | minimal name-input seam, fallback to derived |
| 4 | controls | adopt WASD/mouse/keys | yes | quickbar `main.cpp:2124-2203`; constitution :94-101 | full legend only in F3 overlay (:3001-3009); README:36-38 documents stale P/E/X bindings | always-available compact hint + fix README drift |
| 5 | goal choice | pick expedition goal | **NO** | `main.cpp:3189-3190`; phase events dropped :1673-1694 | loop's first decision invisible; world auto-assigned | render existing phase event; reserve route/goal choice slot on front door |
| 6 | combat | engage/dodge telegraphs | yes | scenarios :3535-3713; telegraphs :1433-1546 | dash undocumented outside F3 | same compact hint as #4 |
| 7 | loot | take drops | yes | nearest-scan :856-886; underfoot :424-426 | remote drops are client-synthesized "kill reward" placeholders (`remote_session.cpp:799-806`) | hide placeholder labels until authoritative name arrives |
| 8 | equip | choose gear | yes | pane :1924-2059; scenario :3574-3591 | only ATK shown though DEF/HP bonuses parsed (:74-93); remote U silently no-ops (:3232-3235) | surface parsed bonuses; disable U off-local |
| 9 | extraction | push farther or bank | yes | pad :2636-2684; objective strip :2917-2943; remote walk-on :437-442 | strip says "press F there" but remote F does not extract — contract mismatch | mode-aware strip text or route F to walk-on remotely |
| 10 | progression | invest in build | **NO** | XP/level `networking.cpp:2045-2052`; `passiveTree` payload :586,817 unread by client | silent level-ups; no tree surface | announce level deltas via existing hint channel; read-only tree summary chip |
| 11 | death/recovery | admit heir; understand loss | yes | fall flow :673-694; front-door record :2511-2530; succession select :2329-2356 | recorded red: heir admits with zero-life actor until heal (client-owned workaround documented at `main.cpp:4097-4108`; server fix = TASK-0148); soft death unexplained; loss rule unstated | hold front door until life>0; explain awaiting-respawn via hint channel |
| 12 | quit/relaunch | abandon instance or extract first | yes | Esc/close :3174-3177; lifecycle selftest `play-native.ps1:262-309`; roster restore :4143-4166 | instant quit while carrying value; snapshot loss rule never shown (`persistence/README.md:24-28`) | stake-naming confirm in hint channel when carrying |
| 13 | error/reconnect | recognize failure, recover | yes | chip+banner :2570-2616; retry 3×1/2/4s `remote_session.cpp:311-349`; session-replaced :695-700 | no manual retry after terminal failure | add `[R] reconnect` action to front door menu |

## Key findings

### F-1 The loop's opening decision is missing (negative control G-1)
The constitution makes goal selection the first act of the durable loop.
Nothing in the front door, HUD, or help surfaces a choice; the client silently
enters `tin:1:0`. Ironically the core already computes and emits the goal
state (`ExpeditionPhaseChanged`, slay-wardens → extract-carried-value) — the
presentation just never subscribes. Surfacing an existing event is the
smallest possible move and cannot invent lore because the event text comes
from the core.

### F-2 Progression is earned but never shown (negative control G-2)
Kills grant XP and derive levels server-side; the wire carries `passiveTree`;
the native client renders neither. A first-session owner who kills monsters
sees no growth moment and cannot spend anything. Read-only surfacing
(level-delta line + points chip) requires no new rules and no new server work.

### F-3 One contract contradiction misleads at the extraction moment (F-fix class)
`objective: … press F there` (`main.cpp:2933`) versus remote Extract being a
deliberate no-op hint ("Reach the exit stairs…", `remote_session.cpp:437-442`).
Locally F extracts; remotely walking onto stairs-up does. The owner-facing
instruction is therefore wrong exactly once per successful run — at the moment
of maximum investment. This is the highest-value single-line fix.

### F-4 Control knowledge is debug-gated; docs contradict the binary
All non-quickbar bindings (dash Space/RMB, X, Z, F, I, wheel/Home) appear only
in the F3 overlay (`main.cpp:3001-3009`), and `native/README.md:36-38` still
describes P/E/X bindings that do not exist in the client. New owners learn
dash — the answer to every telegraph — by accident.

### F-5 Identity is chosen by derivation, not by the owner
No text input exists in the native client; House/Scion names derive from the
guest id (`main.cpp:2292-2317`). SPEC keeps final naming owner-only, so the
audit does not propose copy — it proposes the *input seam* so an owner can
name their own lineage when the owner approves doing so.

### F-6 Death consequences are honest but unexplained at their edges
Mortal fall → front door with fall record, crypt, heir admission, relic toast:
all present and scenario-proven. Missing: any statement of what happens to
carried/floor value (implemented in the persistence boundary,
`persistence/README.md:21-28` but never surfaced), any explanation of
`awaiting-respawn` soft death on the expedition screen, and the known
zero-life-heir red whose server fix belongs to TASK-0148 (client-side guard
documented at `main.cpp:4097-4108`).

### F-7 Positive findings worth protecting
- Honest-state discipline is consistently good: art chips never claim loaded
  PNGs that are not loaded (`main.cpp:2968-2986`), connection failures never
  fall back to local play (`session_tests.cpp:1-5`), the front door renders
  only authoritative chronicle data.
- Loose-guidance style is already right: the objective strip guides without
  checklist tutorial design (`main.cpp:2914-2943`). Any successor onboarding
  implementation should extend *this pattern*, not add quest-log checklists —
  per SPEC and constitution (`VERDIGRIS_CONSTITUTION.md:103-106`).

## Prioritized smallest fixes for the successor implementation task

Ranked by (owner-visible comprehension gain)/(implementation risk):

1. **Fix the extraction instruction mismatch** (#9/F-3) — mode-aware objective
   strip or remote F→walk-on. Smallest diff, removes an active lie.
2. **Render ExpeditionPhaseChanged** (#5/F-1a) — add the event_label case +
   objective-strip hook; core text is authoritative.
3. **Surface level-ups + passive-tree points chip (read-only)** (#10/G-2).
4. **Compact always-available controls hint + README binding correction**
   (#4/#6/F-4).
5. **Front-door `[R] reconnect` action** (#13).
6. **Stake-naming quit confirm when carrying value** (#12/F-6c).
7. **Awaiting-respawn explanation line; hold front door until heir life>0**
   (#11/F-6a,b) — complements, does not replace, TASK-0148.
8. **Hide remote placeholder loot labels until named** (#7).
9. **Gear pane: show parsed bonuses; disable remote U** (#8).
10. **Name-input seam with derived fallback** (#3/F-5) — pending owner approval
    of owner-facing naming at all.
11. **Owner path as default launch shape** (#1).

Items 1–2 plausibly land inside one small client-only task; 3–9 are each
≤ a day-class change against existing seams; 10–11 need an owner decision
first and should be questions, not silent choices.

## Deviations, risks, open questions

- No runtime session was driven (resource capsule: read-only, no ports);
  behavior claims lean on the committed deterministic scenarios and accepted
  reviews instead of fresh captures. Risk is low: every cited behavior is
  asserted by an automated scenario in-tree.
- `native/client/main.cpp:3036` contains a stray namespace-scope
  `render::List rl;` between `paint_scene` and the collision helpers. It
  compiles and appears unused by the paint path (each painter builds its own
  local list); flagged here as an observation for the owning lane, not fixed
  (forbidden path for this audit).
- Open question for the architect/owner: should the native client expose
  owner-facing naming at all before final narrative wording exists? (Blocks
  fix #10.)
