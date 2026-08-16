# ADR-002 — Persistence seam for the native core

Status: **PROPOSED** (architect draft, 2026-08-16). The owner decides; no
implementation task may claim this until it reads ACCEPTED.

## Context

The native core is a deterministic, fixed-timestep, command/event
simulation with no I/O (D-002). Durable state that must survive process
death: House (name, standing-equivalents, routes/branches, stores,
relic pool, Legends record, lineage), the active Scion, and — eventually —
season boundaries. Two established facts shape the choice:

1. Determinism gives us a second persistence primitive for free: a seed +
   command log replays to an identical state (proven by the replay tests).
2. The browser game's Chronicles repository (SQLite tables for
   account/House/Scion/relic/world-progress, with a bounded JSON fallback,
   `server/core/repositories/chronicles-repository.js:93-160`) is working
   evidence of the *shape* of durable House data — reference only, not a
   port source (LEGACY_MATRIX addendum).

## Decision (proposed)

**Snapshot-based persistence with a versioned, explicitly-serialized
House/Scion state; command-log replay used for tests and debugging, not as
the durable format.**

- The core gains a serialization boundary (`persistence/` seam, already
  reserved): pure functions `snapshot(Simulation) -> bytes` and
  `restore(bytes) -> Simulation`, no I/O inside the core. The platform
  layer owns files/databases.
- Format: a small versioned binary or JSON structure (schemaVersion field
  mandatory; unknown-field tolerance mandatory) covering House, Scion,
  fallen Scions, Legends (bounded, so snapshots stay small), relic pool,
  and RNG state (`Rng::state` + `serial`) so a restored simulation
  continues deterministically.
- Mid-instance state is NOT durable in v1: extraction risk means a crash
  or quit mid-expedition resolves conservatively (Scion returns to the
  House with nothing banked; carried-but-unextracted value is lost). This
  matches the risk model rather than fighting it, and dodges the hardest
  snapshot problems (instances, ground items, in-flight windups).
- Storage backend in v1: one flat file per House (atomic write via
  temp+rename). SQLite is deliberately deferred until there is a concrete
  multi-entity query need; the Chronicles schema shows what tables would
  look like when that day comes.

## Consequences

- Restoring RNG state means save/load cannot be used to reroll drops —
  the determinism guarantee extends across restarts.
- The bounded Legends cap (TASK-0001) is what keeps snapshots O(small).
- Mid-instance non-durability must eventually be surfaced honestly in UX
  ("leaving now abandons the expedition") — client work, later.
- A future season mechanic gets a natural hook: a season reset is a
  snapshot transform, not a special code path.

## Rejected alternatives

- **Durable command-log replay**: elegant but replay time grows unbounded
  with House age, every core change risks old-log compatibility, and log
  compaction reinvents snapshots anyway. Kept for tests/debugging only.
- **SQLite-first**: schema migrations and a C dependency before any query
  need exists contradicts the dependency-free core discipline.

## Open for the owner

- Whether mid-instance abandonment-on-crash is acceptable product
  behavior for v1 (architect believes yes; it is the honest reading of
  extraction risk).
- Save-file tampering stance for a single-player-first game (v1 proposal:
  no obfuscation; it is the player's file).
