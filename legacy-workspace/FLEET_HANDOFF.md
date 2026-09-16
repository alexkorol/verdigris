# Verdigris Owner Demo — fleet handoff

## Mission

Tonight's executable Owner Demo runway is the local commit `491f8f84` on
`codex/owner-demo-runway` at
`Z:\Code\.worktrees\verdigris\owner-demo-runway`. Its authoritative index is
`orchestration/OWNER_DEMO_RUNWAY.md`. It replaces the exhausted READY snapshot
in the dirty architect checkout for this run.

Deliver one integrated native C++ Owner Demo that makes the conversion feel
like Verdigris rather than a renderer or systems prototype. Read these first:

1. `Z:\Code\Games\delaford\VERDIGRIS_VISION.md`
2. `Z:\Code\Games\delaford\OWNER_DEMO_OVERNIGHT.md`
3. `Z:\Code\Games\delaford\delaford_game\AGENTS.md`

The failure and succession rules in
`Z:\Code\Games\delaford\FLEET_RESILIENCE_PROTOCOL.md` are binding. One active
coordinator is a renewable lease holder, not a single point of failure.

The Owner Demo is an internal confidence build for the sole developer. It is
not a public release and it is not a generic ARPG mockup.

## Non-negotiable visible outcome

The integrated journey must contain:

- a real splash/menu flow; Escape opens or closes menus and never exits;
- a town or other readable non-combat area;
- multiple combat zones connected by visible, hover-highlighted gates;
- persistent zone instances and Ctrl-click creation of a fresh instance;
- animated actors, visible weapons, attacks, arcs or trails, hit reactions,
  and readable deaths, even if the actor art remains vector-based;
- raster UI panes, frames, health/mana orbs, and inventory chrome based on the
  WIZARD work—no primitive circle or rectangle stand-ins;
- a Diablo-II-like grid backpack, paper doll, and real item art from
  `WIZARD/rpg_inventory`;
- active use of the WIZARD skill tree, inventory, map generation, spell
  lattice, splash/menu, Chronicles, orbs, and Framekit modules;
- a recognizable fifteen-minute path through combat, first level-up, the
  geometric skill tree, town, an economic choice, and another expedition.

## Product truth

Verdigris is a Bronze Age or ancient-world native C++ action RPG. The House is
a persistent lineage and economic unit; the Scion is mortal. Ordinary combat
should be low-friction, fast enough to feel good, and visibly satisfying. Most
strategic depth lives in build planning, equipment, Brands, Bonds, the skill
tree, and the spell lattice. Do not slow combat merely to manufacture tactical
complexity.

## Operating model

Use one coordinator and bounded, disjoint implementation lanes. The
coordinator owns the integrated build and the ready queue; workers own isolated
worktrees or clearly separated paths. Never allow multiple writers to improvise
inside the same client hotspot.

Recommended lanes:

1. Native integration, renderer, build stability, and performance.
2. Animated actors, combat readability, feedback, and sound.
3. WIZARD Framekit, orbs, menus, inventory, and item-art adoption.
4. Town, NPCs, combat zones, transitions, instances, and procedural layout.
5. Skill tree, spell lattice, items, Chronicles, Brands/Bonds, and House loop.
6. End-to-end playtesting, visual comparison, packaging, and evidence.

Planning and review agents may inspect broadly, but writers must have explicit
file ownership. Prefer several small integrations over one enormous merge at
the end.

## Definition of done

A task is complete only when it does at least one of the following:

- visibly improves the integrated Owner journey;
- produces verified evidence that rejects or confirms an implementation;
- removes a concrete blocker and releases a named successor task.

Code volume, token usage, contracts, or a standalone showcase are not evidence
of product progress. A WIZARD module is not adopted until it appears and works
inside the native integrated journey.

For gameplay claims, run the repository's required goal harness:

```powershell
Set-Location 'Z:\Code\Games\delaford\delaford_game'
npm run playtest
```

Do not claim a gameplay change works without a successful harness run. Also
capture screenshots or short recordings for visual changes and compare them
against the relevant JavaScript and WIZARD references.

## Coordinator cycle

At each checkpoint:

1. Observe the integrated build and collect worker evidence.
2. Reject work that only passes tests but misses the visual or playable target.
3. Integrate the smallest verified improvements.
4. Run the goal harness and capture the current Owner journey.
5. Split blocked work, replace failed approaches, and release successors.
6. Refill the ready queue from the vision and overnight brief.
7. Write a compact checkpoint report and continue unless a hard stop applies.

The coordinator may split, reorder, retry, or drop implementation tasks. It may
not silently change the product identity, waive the playtest gate, substitute
generic CSS or primitive shapes for supplied art, or declare the build done
without an end-to-end run.

## Suggested scheduled checkpoints

Do not keep the owner conversation in a continuous loop. Schedule bounded
fleet-side checkpoints instead:

- Launch: inventory the current native, JavaScript, and WIZARD states; assign
  collision-free lanes; record a baseline capture.
- After roughly two hours: integrate the first visible vertical slice and
  regenerate the ready queue.
- Mid-run: run the full Owner journey, compare visuals, and redirect stalled or
  low-fidelity lanes.
- Pre-morning: freeze risky expansion, repair integration, run the goal harness,
  and package the best coherent build.
- Morning handoff: provide the playable build, exact launch steps, harness
  result, screenshots or capture, completed and rejected work, known defects,
  and the next ten highest-value tasks.

If the scheduler cannot run timed checkpoints, the coordinator should create
these as dependency-triggered milestones: baseline complete, first integration
complete, mid-run journey complete, and release candidate complete.

## Stop conditions

Continue automatically while useful, collision-free work is available. Stop
and report instead of guessing when:

- the build or repository state risks destroying user work;
- two lanes require incompatible architecture decisions;
- required reference art or source is genuinely missing;
- a consequential product choice is not answered by the vision documents;
- integration repeatedly fails and no isolated fallback remains.

Ordinary task failure is not a stop condition: split it, retry with a different
approach, or release a prerequisite.

## Morning acceptance check

The owner should be able to launch the build and answer yes to these questions:

- Does this immediately look like the supplied Verdigris and WIZARD material?
- Can I see and feel attacks rather than infer them from disappearing enemies?
- Can I navigate menus without Escape killing the application?
- Is inventory a real grid and paper doll with real item art?
- Can I move from town through multiple readable gates into varied combat zones?
- Can I level, open the geometric skill tree, and see at least bounded spell and
  item progression?
- Is there a coherent path toward the House-versus-Scion economy?
- Did the required playtest harness pass on this exact integrated state?

If those answers are not mostly yes, report the build honestly as an interim
checkpoint and immediately seed the next ready tasks from the failed checks.
