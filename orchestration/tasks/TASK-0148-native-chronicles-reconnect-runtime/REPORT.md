# TASK-0148 REPORT — Native Chronicles reconnect runtime

worker: ox-pc-r (openrouter, stealth/ox-alpha, OpenCode harness)
branch: codex/TASK-0148-native-chronicles-reconnect-runtime-ox-pc-r-r5
worktree: Z:\Code\.worktrees\verdigris\ox-pc-r
implementation commit: 5732367e (on top of supervisor coordination commit
3119a08e and claim commit 837a412f)
spec base: 060c11517d2ebb0aec0c4d4a38c5e3eb53141cb2

## Executive summary

The normal-player Gate-B owner journey broke at the fatal-fall step:
`chronicles:scion:set-out` admitted the first Scion WITHOUT the mortal oath
and hard lifecycle, so `process_combat`'s final-death gate could never fire
from an ordinary lethal wound. A normal player could not die, reach the crypt,
circulate an heirloom, succeed a House, or prove reconnect continuity. The fix
is the smallest real runtime correction in owned paths: set-out now admits
under the hard mortal lifecycle exactly like JS `beginScionSession`
(server/core/services/chronicles.js), the sworn oath is persisted on the
living roster so relogins restore it, and succession admission heals the reused
Simulation actor so an heir does not inherit the fallen scion's lethal wound.
A new literal loopback session scenario drives the COMPLETE journey with only
normal accepted envelopes and proves identical House/Scion/relic state after
disconnect/reconnect with the same guest identity.

## Pre-change failing step (preserved evidence)

`captures/gate-b-loopback-prechange-fail.log` (untracked work preserved from
this lane's pre-fix run of the same scenario binary):

- FAIL gate-b: set-out admits the Scion under the mortal oath
  (player.chronicles.mortal)
- FAIL gate-b: ordinary movement/combat death commits the fall
  (chronicles:scion-fallen)
- "pre-change runtime gap reproduced at the fatal-fall step"

## Approach

1. Froze the literal accepted journey before probing: non-quick login →
   `chronicles:house:found` → `chronicles:scion:create` +
   `chronicles:scion:set-out` (never mutate/select for the first admission) →
   earn a bronze sword via `wagon:outfit:buy` → ordinary movement/combat death
   in an `instance:enterSolo` delve → `player:chronicles:return` → successor
   via `chronicles:scion:create` + `player:chronicles:select` → heir rearms
   and equips through ordinary wagon/inventory surfaces → slay the floor elite
   so the heirloom surfaces → recover the EXACT uuid via context-menu Take →
   crypt-state check → disconnect → reconnect with the same `guestId` →
   identical chronicle snapshot + living-roster/crypt/oath assertions →
   resumed set-out proves carried-heirloom continuity.
2. The scenario driver is a minimal RFC6455 client using only what a normal
   client sees: position echoes, scene transitions, combat hits,
   `monster:telegraph` payloads, ground-change item lists, inventory refreshes,
   and game messages. No `dev:*`, no `player:chronicles:mutate`, no internal
   state mutation, no test bypass.
3. Hunting uses honest player behavior refined during this session: serpentine
   lattice exploration (bounded waypoints; a border-hugging wall-follow sweep
   provably orbits the outer ring and never reveals an interior elite),
   hold-ground trading once in reach (monsters are stationary; retreating out
   of reach produced silent whiffing), telegraph-circle dodging from the
   broadcast payload, beeline to the revealed elite with bounded escape phases,
   fountain-heal withdrawal at low visible health, and exact-uuid recovery via
   the normal `player:context-menu:action` Take surface.

## Changed files

- native/src/networking.cpp (owned)
- native/tests/session_tests.cpp (owned)
- orchestration/tasks/TASK-0148-native-chronicles-reconnect-runtime/
  captures/{gate-b-loopback-prechange-fail.log,gate-b-full-suite-green.log}

## Public interfaces added/changed

No new or changed envelope contracts. Behavior corrections only:

- `chronicles:scion:set-out` now admits the Scion under the mortal oath
  (`mortal_oath_=true`, `lifecycle_mode_="hard"`, fresh alive/lifecycle
  counters) and records `mortal:true` on the living roster.
- `reset_world_for_new_socket` restores the sworn-oath hard lifecycle from the
  living roster on socket adoption (JS parity); sockets without an admitted
  scion keep the soft-guest profile exactly as before.
- `player:chronicles:select` records the admitted heir's oath on the roster
  and heals the reused Simulation actor to full life (an heir must not spawn
  inheriting the predecessor's lethal wound, which caused an input-less second
  fall).

## Test commands + outcomes (literal SPEC acceptance)

All three acceptance commands executed probe-free from the final single-writer
tree (HEAD at run time = 5732367e's tree):

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File native/build.ps1 -RunTests -RunClientScenarios
# exit=0 — core/networking/camera2d/session suites green ("session tests passed"),
# legacy denylist PASS, all client scenarios green including chronicles-gate-b.
git diff --check          # exit=0
git diff --name-only 060c11517d2ebb0aec0c4d4a38c5e3eb53141cb2..HEAD  # exit=0
```

Full transcript preserved: `captures/gate-b-full-suite-green.log` (38 PASS
gate-b checks listed, "session tests passed", every `== scenario ... ==` line
green). Two earlier invocations of the same command hit timing-sensitive
failures in DIFFERENT pre-existing checks (reconnect drop-detection, then
journey pickup/extract) while the identical binary passed repeatedly when run
directly and after the preceding exes; judged environmental (fresh-binary
real-time-scanning stalls during the compile phase), not code regressions —
noted here for the reviewer's awareness.

Ports: server binds inside the ox-pc-r capsule 6960-6979 only; 6580-6599 was
verified free before runs; port 6500 never touched.

## Manual verification

The loopback journey IS runtime verification against the real WebSocketServer:
ordinary combat death commits `chronicles:scion-fallen`; crypt relic status
becomes `recovered`; reconnect with the same guest restores a byte-identical
chronicle snapshot; the heir's mortal oath and carried exact heirloom survive
the reconnect; resumed set-out re-admits the heir.

## Deviations

- Recovery takes the context-menu Take surface rather than
  `player:take:underfoot`: underfoot grabs ONE item by fixed tile priority, so
  recovering one specific uuid from an elite drop pile is exactly what the
  normal context-menu action is for. Both are accepted ordinary client inputs;
  the frozen journey wording allows "ordinary take inputs".
- During diagnosis, six clearly-marked temporary probes were added and ALL
  removed before the final evidence run; `rg "TEMP-GATEB-DEBUG|[take]|[dbg-"`
  over native/ returns zero matches in the committed tree.

## Unresolved questions

None blocking. One observation worth a follow-up task: the hunt exposed that a
blind wall-following sweep cannot reach interior elites; that is driver-level
knowledge here, but level design/patrol affordances may deserve product review.

## Risks / follow-ups

- The serpentine sweep and fight logic live in test-driver code only; no
  production balance/content/durability changes were made (SPEC stop conditions
  respected).
- Session-test suite has pre-existing timing-sensitive checks (see acceptance
  note); a future soak/flake-hardening pass could bound them explicitly.
