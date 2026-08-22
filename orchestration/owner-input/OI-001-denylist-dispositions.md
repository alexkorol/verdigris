# OI-001 — live denylist dispositions

**State:** WAITING_EVIDENCE on TASK-0085. **Deadline:** before compatibility
cleanup; not on the current Gate B/C critical path.

Decision required: choose separate dispositions for wire key `legacyRelicId`
and item id `bronze-dagger` after the occurrence/breakage table is accepted.

Recommended choice: preserve `legacyRelicId` as a documented compatibility
wire key until a versioned migration exists; migrate `bronze-dagger` away from
player-visible/canonical content while retaining only the minimum compatible
read path proven necessary by evidence.

Viable alternatives:

1. Keep both as explicit, documented exceptions.
2. Version and migrate both together, accepting every named client/save/harness
   update in one compatibility wave.
3. Remove either immediately only if TASK-0085 proves no live consumer.

Acceptance rubric: no save loss, no unchanged-harness regression, no denied
starter kit restored to canon, and every surviving exception is wire/data-only
and documented. No assets are involved. While pending, all unrelated native,
presentation, and tooling work continues; workers do not rename either token.
