# TASK-0163 REPORT — Gate-B ordinary-play journey reliability

Worker: ox-pc-ac2 (coordinator `ox-pc-ac`), branch
`codex/TASK-0163-gate-b-ordinary-play-reliability-ox-pc-ac`,
worktree `Z:\Code\.worktrees\verdigris\ox-pc-ac2`, base
`7de9b31927e74448f07a26cc77e2f92e55a9a6a2`, ports 7160-7179 loopback only.

## Executive summary

The Gate-B ordinary-play journey in `native/tests/session_tests.cpp`
(House foundation → first Scion set-out under the mortal oath → ordinary
combat death → succession → heirloom recovery by exact UUID → same-guest
reconnect continuity) is now deterministic enough to serve as a release
gate. Both program-gate failure surfaces recorded against the combined
program are diagnosed causally inside the test source, the exploration
driver is replaced with a provably-covering state machine, five focused
deterministic controls pin that machine, and every literal acceptance
command passes from a clean build plus three consecutive exact session-test
runs. Zero runtime/gameplay files changed; zero assertions weakened;
zero timeouts inflated; no `dev:*`, teleport, direct-state mutation, or
hard-coded Warden coordinates were introduced.

## Causal diagnosis (both recorded failure surfaces)

Both failures shared one root class: the OLD driver's navigation decisions
were functions of wall-clock heuristics, so machine load redirected the
walk instead of merely slowing it. Replay evidence below comes from an
offline re-execution of the exact algorithms over this journey guest's
seeded warren floor ("ox-pc-r-gateb"; placement is identity-derived and
deterministic, so one replay is authoritative).

1. Seven-minute hunt, four kills, no named Warden. The old serpentine
   lattice joined full-height lane legs with GREEDY diagonal transits and a
   left-hand block rotation. That pair pins the walk against the warren's
   vertical wall ribs on the wrong side: the ideal (unperturbed) replay
   reached only lanes x=2 and x=7 in nine thousand steps and NEVER entered
   the reveal ring. Under load, late step echoes read as walls and the 25 s
   waypoint deadline skipped more legs. The Warden's seeded tile lies in a
   rib pocket whose only entries are authored gap corridors, so unless a
   lane leg physically passed within the two-tile telegraph reveal ring,
   `monster:telegraph` never fired and the driver wandered blind. The
   recorded four kills are the eastern trash packs traded en route - combat
   worked; coverage did not.
2. Retry missing the fatal-fall event. The pre-death sweep was pure
   right-hand wall following that treated ONE silent 400 ms movement window
   as a wall. A load-delayed echo therefore permanently rotated the walk
   onto a different maze cycle; the retry's cycle held no monster, so no
   incoming hit (hence no `chronicles:scion-fallen`) could occur inside the
   unchanged budget.
3. Third latent defect found while proving the repair: the driver derived
   tiles with `floor()`, but each accepted `player:move` is one sub-tile
   interpolation sample and the runtime's authoritative
   `occupied_tile()` ROUNDS positions. The take-relic leg demonstrably sent
   its Take from true tile (27,19) while believing it stood at (26,19):
   chebyshev distance 2, silent reach-gate rejection, "recovered" FAIL with
   no diagnostic. All driver tile math now goes through `gateb_tile_of()`
   (`lround`), matching the wire-agreed convention.

## Approach (test-driver only)

- Shared boustrophedon plan (`gateb_lane_plan`,
  `gateb_serpentine_waypoints`): fixed ascending lanes whose skipped columns
  are exactly the warren layout's static wall ribs and border walls -
  scene geometry, never spawn positions. Every walkable column lies within
  one tile of a lane, so full-height legs guarantee (a) passive-pack
  adjacency contact somewhere on the floor (the ordinary fall) and
  (b) entry into the two-tile boss reveal ring (the named Warden).
- Waypoint discipline (`GatebSweepState`, `gateb_waypoint_nudge`)
  replaces deadline-skipped greedy diagonals; primary-axis-first steps,
  bounded evidenced waypoint skips retained as degradation telemetry.
- Silent-step policy (`kGatebSilentRetries`, `gateb_declares_wall`): the
  same direction is re-issued before any wall rotation, so a slow echo now
  costs latency, never the path. Per-attempt echo window stays 400 ms.
- Sweep and hunt phases both drive the same plan; hunt trade/dodge/
  withdraw/beeline machinery is untouched.
- Focused deterministic controls (`gateb_driver_state_machine_controls`),
  socket-free, registered in `main()`: frozen plan contract, walkable-
  column coverage property, silent-step policy semantics, full-height
  legs, strict boustrophedon ordering.
- All server binds moved into this lane's capsule 7160-7179 (scan upward);
  the dead-endpoint negative now uses never-bound 7159. Port 6500 is never
  touched.

## Changed files

- `native/tests/session_tests.cpp` (only file outside the task folder)
- `orchestration/tasks/TASK-0163-gate-b-ordinary-play-reliability/STATUS.md`
- `orchestration/tasks/TASK-0163-gate-b-ordinary-play-reliability/REPORT.md`

## Public interfaces added/changed

None. Test-only changes; no headers, sources, wire shapes, gameplay rules,
or commands changed.

## Acceptance commands and outcomes (literal)

From a clean build tree (`native/build` deleted first):

```text
$ powershell -NoProfile -ExecutionPolicy Bypass -File native/build.ps1 -RunTests
native legacy denylist: PASS
verdigris core tests: PASS
verdigris networking tests: PASS
camera2d tests: PASS
PASS gate-b-driver: serpentine plan matches its frozen contract
PASS gate-b-driver: lane plan sweeps within one tile of every walkable column
PASS gate-b-driver: silent steps re-issue before any wall rotation
PASS gate-b-driver: every lane leg spans the full floor height
PASS gate-b-driver: plan is strictly boustrophedon over its lanes
session tests passed
presentation events tests: PASS
GATE_EXIT=0
```

Three consecutive exact session-test executions, no source or fixture
changes between runs:

```text
RUN 1: exit=0   PASS=107 FAIL=0   last line: "session tests passed"
RUN 2: exit=0   PASS=107 FAIL=0   last line: "session tests passed"
RUN 3: exit=0   PASS=107 FAIL=0   last line: "session tests passed"
```

```text
$ git diff --check        -> exit 0
$ git diff --name-only    -> native/tests/session_tests.cpp
```

Full transcripts: `Z:\Code\.fleet\tmp\ox-pc-ac\gate-clean-runtests.log`,
`session-run1.log`, `session-run2.log`, `session-run3.log`.

## Manual verification

The Gate-B leg of run 1 shows the corrected machine working end to end:
sweep heartbeats advance monotonically across lanes ((4,8) → (10,6) →
(16,32) → (26,21)), the telegraph reveals the elite at its true tile
(`approach dist=3 ... elite=(25,18)`), the beeline kills it
(`hunt kill #11 (Warden of the Deep)`), the surfacing message and exact
UUID ground frame arrive, the context-menu Take refreshes the inventory
with that exact UUID, crypt status flips to recovered, and the reconnect
legs reproduce identical House/scion/relic records with oath and carried
heirloom intact. Every check name and count from the pre-change suite is
preserved; nothing was removed or loosened.

Negative controls honored: no `dev:*` envelopes, no direct simulation/state
mutation, no teleport, no seeded reward injection, no hard-coded Warden
coordinates (the only embedded geometry is the static warren rib/border
layout the control proves coverage against), no assertion deletion, no
timeout inflation (journey budgets 150000/420000/60000 ms unchanged),
no runtime or gameplay edits (`git diff --name-only` proves scope).

## Commit SHAs

- Claim: `d872687f` on this worker branch (pushed).
- Implementation + handoff: see `git log` head of the worker branch
  (this commit); pushed immediately after writing.

## Deviations

- Port bindings previously documented for other capsules (architect
  6572-6579, cursor 6580-6599, ox-pc-r 6960-6979) were retargeted into
  this task's assigned 7160-7179 capsule. Required by the routing packet's
  resource_capsule; loopback-only maintained; no other lane's range is
  touched.

## Unresolved questions

None.

## Risks and follow-ups

- The 25 s waypoint skip remains as a bounded, evidenced degradation path
  (policy inherited from the prior design); with the retry policy it should
  stay dormant except under extreme load.
- TASK-0162 (passive-tree payload hardening) is PIPELINED behind this task
  on the same owned file and can now proceed on a deterministic surface.
