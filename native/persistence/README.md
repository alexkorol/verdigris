# Persistence seam

The native core exposes pure `verdigris::snapshot(const Simulation&)` and
`verdigris::restore(bytes)` functions. The v1 bytes are a canonical,
line-oriented text format:

```text
schemaVersion=1
rng.state=...
rng.serial=...
...
```

Numbers and booleans are decimal (`0`/`1`); strings are UTF-8 bytes encoded as
lower-case hexadecimal. Collections carry a `.count` field followed by stable
numeric indexes. Fields are emitted in a fixed order, so equivalent durable
states produce byte-identical snapshots. `schemaVersion` is mandatory and
unknown keys are ignored on restore. Missing required values or malformed
known values fail restore rather than silently changing state.

The snapshot contains House routes/branches, stores, relic and lost-trophy
pools, pending recovery queues, seasonal rewards, bounded Legends, campaign
completion, the current and fallen Scions, and RNG `state` plus `serial`.
Events, actors, ground drops, monster state, windups, and all other live
instance state are intentionally absent. Under D-109, an active instance is
retired at the snapshot boundary: carried items/trophies remain on the Scion,
ordinary floor value disappears, and already surfaced relic/trophy candidates
return to the pending recovery queues for a later seeded entry.

`adapter.hpp` is a deliberately thin file layer for callers that need one
House save file. `persistence::write_atomic(path, bytes)` writes a temporary
file, flushes it, and atomically replaces the target (`MoveFileEx` on Windows,
`rename` elsewhere). `persistence::read(path)` only reads bytes; neither
function is linked into the simulation core unless a caller includes the
header.
