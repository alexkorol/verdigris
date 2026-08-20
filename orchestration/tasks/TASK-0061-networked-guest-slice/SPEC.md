# TASK-0061 — Networked native guest expedition slice (Gate A)

Packet: BOUNDED-DESIGN. Lane: strongest available native implementer
(deepseek/kimi-work). PIPELINED: releases only after TASK-0060
(session seam) integrates. Base: tip at claim.

## Outcome (owner-visible)

Gate A of docs/rebuild/NATIVE_PRODUCT_CONVERGENCE.md passes: one
command starts `verdigris_server` on a task-owned loopback port plus
the native client in remote mode; the owner completes connect → guest
login → enter route → move/aim → fight (telegraph, outgoing hit,
incoming hit, kill) → named item drop → pickup → equip → visible
authoritative stat change → extract → bank → clean dual shutdown.

## Frozen interfaces

`IClientSession` / `ClientModel` / `PresentationEvent` from 0060 —
extend via architect note, never fork. Remote mode runs no in-process
sim. No gameplay rules in transport/renderer/HUD. Envelope stays
`{event, data}` with payload at `data.data`.

## Owned / forbidden paths

Owned: `native/client/remote_session.*`, new client modules the
journey needs, remote scenario drivers, this task folder.
`native/client/main.cpp` edits are SINGLE-WRITER — claim it in STATUS
notes per session. Forbidden: browser harness, server rule changes
(file a note if a server gap blocks the journey), tests weakened.

## Forbidden in the accepted run

dev:give, direct sim mutation, preloaded inventory, browser client,
silent local fallback, scenario-only shortcuts.

## Acceptance

Full native build gates green + remote journey scenario green
(handshake, movement, fight, loot, equip, extract, shutdown) + one
authentic negative (server killed mid-session → visible disconnect
state, no fake offline play). Architect reruns everything and PLAYS
the exe against the server; quality rubric scored (no zeroes, ≥9/12).
