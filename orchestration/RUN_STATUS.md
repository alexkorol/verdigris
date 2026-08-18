# Run status (snapshot — rewritten each architect sweep, never a diary)

- Current integration tip: origin/codex/native-reconstitution @ e5c43672
- Master: 1244b5bf (PR #19)
- Owner-visible target: playable native client (0050) + first-session
  browser polish (0049/0042) + N4 items parity (0047)
- Last architect playthrough: native testbed 2026-08-18 morning (owner
  + architect; verdict drove D-117/D-118)
- Immutable objective: D-116 parity, D-110 playable-first

## RUNNING

| Task | Owner | Topology | Base | Last evidence | Notes |
|---|---|---|---|---|---|
| 0049 first-session UI wave | deepseek | INDEPENDENT | 32d7b6e-era tip | 19 dirty files, capture scripts iterating | watch server/** scope on review |
| 0042 first-loot moment | kimi | INDEPENDENT | 560fb265 claim | 10 dirty files, loot.js + item events | re-asserted after RELEASE |
| 0047 N4 items parity | kimi-work | PIPELINED (after N3) | 05c3f46 | 3 dirty files | 13/13 attach bar |

## READY (unclaimed)

| Task | Packet | Notes |
|---|---|---|
| 0050 native client C1 (CRITICAL) | MECHANICAL | camera2d.hpp scaffold + 6-step plan supplied; highest owner value |
| 0051 native client harness | BOUNDED-DESIGN | sequence after/with 0050; move-and-camera scenario first |

## VERIFY / MERGE QUEUE

(empty — next expected: 0049 REVIEW_REQUESTED)

## BLOCKED / WATCH

- WATCH: first-goal + house-treasury flake at >=150ms ambient lag
  (2 sightings; third spawns a harness task).
- Owner decision pending: GitHub branch protection on master;
  does KimiWork K3 share the Allegretto quota?

## Fleet + budget

- deepseek: $9.49 credits (~$1/task-session, 99% cache hit) — primary
  implementation lane; ports 6540–6559.
- kimi K3: monthly pool ~1/5 left (resets 08-23) — scoped MECHANICAL
  packets only; ports 9880–9899.
- kimi-work K3: active on 0047; ports 6510–6529.
- codex: out of tokens.

## Experiments in flight (see LEARNINGS for hypotheses)

- EXP-1 (natural A/B): 0049 = BOUNDED-DESIGN packet (interface-only)
  vs 0050 = MECHANICAL packet (full scaffold). Metric: architect
  repair time + revision count at review. Decides default packet type
  for medium models.

## Next architect actions

1. Review 0049 on REVIEW_REQUESTED (screenshots + scope diff + gates).
2. Route first free coordinator to 0050.
3. Ask owner for branch protection on master.
