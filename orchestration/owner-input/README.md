# Owner-input queue

Architect-owned, batched decisions only. Workers never infer answers from this
queue. `WAITING_EVIDENCE` packets do not require owner attention yet; Sol
promotes them to `READY_FOR_OWNER` when their named evidence is accepted.

| Packet | State | Critical-path deadline | Decision |
|---|---|---|---|
| OI-001 | WAITING_EVIDENCE (TASK-0085) | before denylist compatibility cleanup | `legacyRelicId` and `bronze-dagger` disposition |
| OI-002 | WAITING_EVIDENCE (TASK-0114/0088) | before Stage-2 renderer implementation | production renderer dependency |
| OI-003 | WAITING_EVIDENCE (TASK-0093/0094) | before production asset/font promotion | asset, font, and provenance policy |
| OI-004 | WAITING_EVIDENCE (TASK-0105) | before TASK-0112 content implementation | authoritative passive-tree source |
| OI-005 | PARKED-NONCRITICAL | before production magic content | Arcane Lattice production direction |
| OI-006 | PARKED-NONCRITICAL | before House economy implementation | crafting/economy service boundary |
| OI-007 | PARKED-NONCRITICAL | before authored campaign/season/travel releases | campaign density, travel risk, season inheritance |

Unrelated tasks continue per each packet's fallback section. No asset
generation is currently requested from the owner.
