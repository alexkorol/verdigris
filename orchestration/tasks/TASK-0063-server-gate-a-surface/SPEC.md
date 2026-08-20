---
task: TASK-0063
title: Server-side Gate A protocol surface (drops, extract, equip totals)
state: READY
packet: BOUNDED-DESIGN
lane: any native C++ lane (deepseek preferred on revival; cursor eligible)
priority: high (Gate A dependency, from 0061 server-gap notes)
owned_paths:
  - native/src/**
  - native/tests/**
  - orchestration/tasks/TASK-0063-server-gate-a-surface/**
forbidden_paths:
  - native/client/** (0064 owns it)
  - playtest/** assertions (harness unchanged - it must still pass 32/32)
---

# Outcome

Close the four server gaps 0061 documented, so the native journey has
first-class envelopes instead of workarounds:

1. `item:ground` (or matching browser event name - check src/ client
   handlers for the JS server's name and MIRROR it) emitted on kill
   loot and floor treasure: uuid, id, name, x, y.
2. `player:extract` handler: banks carried items/trophies to House
   store, emits a bank summary envelope; walking stairs-up keeps
   working (both paths converge on the same resolution).
3. `item:equip` response includes resulting wear-slot state + derived
   stat line so the client shows the authoritative change.
4. Session snapshot (`dev:state` and login) includes ground items in
   the current instance.

Browser-harness attach MUST stay green: PLAYTEST_WS_URL attach of the
N1-N4 sets against your build, harness unchanged. Additive envelopes
only - mirror JS server names/shapes (grep server/ for the canonical
event names; if the JS server lacks one, file a note, do not invent a
name without checking).

# Acceptance

build.ps1 -RunTests green + new C++ unit coverage for each envelope +
attach run transcript + one authentic negative (equip of unknown uuid
-> error envelope, no state change). Architect reruns + extends the
0061 drive script to see the drop label appear live.
