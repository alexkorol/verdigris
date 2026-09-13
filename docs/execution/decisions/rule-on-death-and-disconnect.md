# VG-GOV-006 — Rule on death and disconnect

Draft 2026-09-06. Extends TASK-0018 (D-106 items recoverable on death),
TASK-0148 reconnect, TASK-0056. Does not mint TASK numbers. Does not
edit `native/src/core.cpp` (Kimi / TASK-0018).

## Negative control

Disconnect, crash, or quit **cannot** silently acknowledge uncommitted
extraction. HUD `extract:ok` is allowed only after the simulation has
already banked carried value.

## Transition table

| Event | Carry | Carried-value | House-value | Recovery | HUD |
|---|---|---|---|---|---|
| Death | uncommitted | relic pool (D-106), not destroyed | no new bank | re-entry roll; successor starts empty | `extract:uncommitted` |
| Death | already extracted | n/a (banked) | keeps banked | successor empty pack | `extract:ok` |
| Quit | uncommitted | left in instance | no silent bank | re-enter | `extract:uncommitted` |
| Disconnect / crash | uncommitted | not acknowledged | no new credit | reconnect; carry still uncommitted | `extract:uncommitted` + connection lost |
| Disconnect / crash | already extracted | already committed | keeps banked | reconnect roster | `extract:ok` + connection lost |

## Presentation lease

`native/client/rule-on-death-and-disconnect.hpp` and scenario
`death-disconnect`. Connection-lost chrome already exists; this packet
only forbids a fake extract ack.

## Status

**Drafted** for HUD. Core mortality/relic algebra remains Kimi.
