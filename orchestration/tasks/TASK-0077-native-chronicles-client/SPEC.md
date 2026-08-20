---
task: TASK-0077
title: Native Chronicles client slice (Gate B journey UI)
state: READY
packet: BOUNDED-DESIGN
lane: any client-capable lane (cursor when alive; architect fallback)
priority: CRITICAL (D-122 Gate B - N5 server surface is live as of PR #42)
owned_paths:
  - native/client/**
  - orchestration/tasks/TASK-0077-native-chronicles-client/**
forbidden_paths:
  - native/src/** (server gaps -> notes)
  - playtest/**
---

# Outcome (Gate B, docs/rebuild/NATIVE_PRODUCT_CONVERGENCE.md)

The remote native client walks the Chronicles lifecycle against the
N5 server surface: a pre-game House screen (found house / create scion
/ optional mortal oath / set out - drive the chronicles:* envelopes
the harness already proves), in-game death handling (permadeath ->
chronicles:scion-fallen presentation -> successor creation flow ->
relic recovery states), and relaunch persistence (reconnect shows the
same House state). Session tests extend with a full Gate B journey
scenario over the socket (create -> die -> successor -> recover relic
-> quit -> reconnect -> persisted). Protocol matrix Gate B rows get
test names.

# Acceptance

build.ps1 full gates green + new Gate B session scenario green +
architect play pass scoring the Gate B rubric (no zeroes, >=9/12).
