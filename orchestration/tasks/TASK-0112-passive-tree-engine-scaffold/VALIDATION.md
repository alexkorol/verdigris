# VALIDATION — deterministic error codes and ordering

Contract: `verdigris.passive-tree-authority` schema_version 1.0.0.
Scope: validation performed by the future native passive-tree authority
(and, as reference behaviour, the current browser authority). This document
defines error families, detection rules, evaluation order, and tie-breaks. It
defines no topology, node content, effects, economy, or balance.

## Normative language

MUST / MUST NOT / NEVER are absolute requirements of any conforming authority.
OWNER_PENDING marks decisions reserved to the owner (routed via OI-004); a
conforming implementation MUST NOT invent them.

## Validation pipeline

The authority evaluates one input at a time and is deterministic: same input,
same context ⇒ same result. Two entry points share one pipeline:

- **Direct allocation claim** (`allocation@1` + budget context): run stages 1–8.
- **Persisted blob load** (`persistence@1`): if `graph_version` equals current,
  run stages 1–8 on the stored allocation; otherwise attempt migration first
  (stage 3) and re-run stages 1–8 on the migrated form.

Evaluation order — **first failing rank wins**; when collecting all errors,
sort ascending by `(rank, element)`:

| Rank | Code | Detection |
|---|---|---|
| 1 | `MALFORMED_ALLOCATION` | Input does not satisfy its envelope shape: missing/non-array required collections, wrong field types, non-object entries. Checked before anything else because nothing downstream is meaningful for a malformed envelope. |
| 2 | `UNKNOWN_GRAPH_VERSION` | Declared `graph_version` matches no version an approved content source provides to this authority. Fail closed; never substitute a default graph. |
| 3 | `UNSUPPORTED_MIGRATION` | A persisted blob requires migration but its `from_version` is below the declared migration floor or has no registered strategy (see contract `migration.rules`). Never guess an outcome. |
| 4 | `UNKNOWN_NODE` | An allocated node identity does not exist in the authoritative graph for the resolved version. |
| 5 | `DUPLICATE_NODE` | The same node identity occurs more than once in `allocated_nodes` (origin included); equivalently, the same `edge_id` occurs more than once in `edge_choices`. |
| 6 | `MALFORMED_EDGE` | An edge choice fails its envelope or the graph: unknown `edge_id`, unknown `variant`, structurally invalid entry. |
| 7 | `DISCONNECTED_ALLOCATION` | The allocated set is not connected to the origin through allocated nodes only: either a chosen edge has an endpoint outside the allocated set, or some allocated node is unreachable from the origin over chosen edges. Locked/invisible-node allocation (browser precedent) belongs to this family via visibility rules until owner content defines finer classes. |
| 8 | `OVERSPENT` | `spent > earned` under owner-approved cost rules. Report; NEVER clamp silently. |
| 9 | `COUNTER_CONFUSION` | Any payload, persistence record, API surface, or derivation that collapses, aliases, or mutually derives the persistent commission-chain counter (`quests.questPoints`) and the live tree-budget counter — including a single merged points ledger, one counter written under the other's key, or earned computed from the wrong ledger without an explicit designated-source declaration. |

Rationale for the order: container before version, version before structure,
identity before duplication, duplication before edge semantics, structure
before connectivity, connectivity before economy, economy before the
cross-cutting two-ledger invariant, migration applicability immediately after
version resolution because it gates which form enters stages 4–9.

## Determinism and tie-breaks

1. Elements are examined in lexicographic order of their stable identity
   (node_id, then edge_id), independent of input array order.
2. Each failing check reports the FIRST offending element only, as
   `{ code, element }`; batch mode reports every offence sorted by
   `(rank, element)`.
3. Error wording is presentation-layer (`message_key` lookups) and carries no
   normative weight; codes are the contract.
4. No randomness, wall-clock, locale, or floating-point arithmetic participates
   in validation.

## Stack anchors (provenance, not authority)

| Code | Current browser authority analogue | Current native status |
|---|---|---|
| MALFORMED_ALLOCATION | `'Malformed passive tree.'` — server/core/passives/verdigris-authority.js:29-31 | absent; raw snapshots accepted verbatim (networking.cpp:1157-1166) |
| UNKNOWN_GRAPH_VERSION | `'…obsolete tree version.'` — verdigris-authority.js:32-34 | absent; no schemaVersion check on save path |
| UNSUPPORTED_MIGRATION | partial: pre-current versions take a supported one-time full refund reset (verdigris-authority.js:121-128); no explicit unsupported-version error exists | absent |
| UNKNOWN_NODE | `'…unknown node.'` — verdigris-authority.js:40 | absent |
| DUPLICATE_NODE | `'…duplicate or malformed conduit.'` (edge ids) — verdigris-authority.js:44-47; node dedupe is silent ([...new Set], line 38) rather than an error | absent |
| MALFORMED_EDGE | `'…unknown conduit choice.'` — verdigris-authority.js:48-50 | absent |
| DISCONNECTED_ALLOCATION | `'Allocated conduits must join allocated nodes.'` :66-71; `'Every passive must connect to the origin.'` :84-85; locked outer circle :86-91 | absent |
| OVERSPENT | `'…spends more points than this scion has earned.'` — verdigris-authority.js:73-74 | absent; native clamps display instead of rejecting (networking.cpp:1150) |
| COUNTER_CONFUSION | prevented structurally by separate fields (quests.questPoints vs top-level live counter); no explicit error code today | divergent live-counter semantics recorded as QP-2 (networking.hpp:174-177); this code makes the collapse detectable |

## Negative controls (never accepted authority)

1. `native_plus_two_axis_walk` — networking.cpp:1092-1119 with STUB NOTE
   818-821: parses id text to score hex axes and awards a flat per-node bonus.
   Forbidden as authority by contract `node.forbidden_interpretations`.
2. `native_raw_snapshot_save` — networking.cpp:1157-1166 stores client blobs
   verbatim. Forbidden by contract `persistence.trust_boundary`; fixture
   NEG-010 encodes the gap.
3. `counter_collapse` — forbidden everywhere by contract
   `counter_separation`; fixture NEG-007.

## OWNER_PENDING register (validation-side)

- Migration strategy selection per version transition (which versions refund,
  which revalidate) — OI-004/owner.
- Canonical increment sources, caps, and reset rule for both counters (QP-2) —
  architect/owner ruling required before any implementation may pick one.
- Visibility-rule shapes and any validation classes finer than
  DISCONNECTED_ALLOCATION — OI-004 content source.
