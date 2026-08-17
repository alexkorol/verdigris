---
task: TASK-0032
state: REVIEW_REQUESTED
branch: codex/TASK-0032-browser-d106-implementation
commits:
  - 749cc4a6c93733abe7e0a24da6e3c3161215ea3e
  - ac3a721699f624c9a3ff5fc7df58dd3785c356f8
base_commit: e818764b
---

## Executive summary

The browser Chronicle path now implements D-106/D-109 recovery semantics. Hard
death transfers every equipped and carried item plus standalone trophies into
recoverable pools; socketed trophies remain embedded in their item. SQLite and
JSON adapters preserve relic/trophy identity through migration and UUID
deduplication. Instance retirement requeues surfaced relics and trophies once,
including JSON candidates picked up underfoot. Failed disconnect saves queue the
complete snapshot before player removal.

## Implementation

- Added all-carried death transfer and durable relic/trophy claim paths.
- Added SQLite migration/deduplication and JSON old-save compatibility.
- Added explicit instance retirement membership and one-time requeue markers.
- Shared Chronicle recovery runs for click and underfoot pickup paths.
- Added a durable atomic disconnect-save queue for failed combat/teardown saves.
- Preserved both SQLite and JSON adapters; long-term authority remains an
  owner-facing question per the spec. No Vesselforge formulas changed.

## Changed files

Server changes are confined to `server/**`, including Chronicle repository/store
adapters, death/recovery services, world retirement, pickup handling, socket
cleanup, and the new disconnect queue. Tests are confined to `tests/unit/**`.

## Verification

- Focused revision suites: 46 passed.
- `npm run test:unit`: 118 files, 757 tests passed.
- `npm run playtest`: 31/31 passed.
- ESLint and `git diff --check`: passed.
- Independent bounded validator: **ACCEPT** (scope, diff, retirement, underfoot
  recovery, and D-109 queue coverage).
- `npm run smoke:browser`: could not complete because port 6500 was already
  occupied by an existing listener serving HTML for `/world/players`; no
  external process was stopped.

## Manual checks

The revision test exercises relic+trophy retirement and second-retirement
idempotence. Inventory and websocket tests directly exercise JSON underfoot
recovery and complete failed-save snapshot queuing before removal.

## Specification deviations

None in the implementation. Smoke remains an environmental verification note;
the existing listener was preserved.

## Risks and limitations

The product-authoritative long-term store (SQLite versus JSON) remains
intentionally unresolved; adapters preserve both representations and stable
UUIDs. Port 6500 must be released or isolated for a clean smoke wrapper run.

## Questions for Fable or the owner

None newly introduced. The store-authority choice remains the owner-facing
question already called out by the task stop condition.

## Integration notes

Requires Fable architect review before integration. Integrate the worker revision
after acceptance, then rerun the three acceptance gates in a dedicated
integration worktree and append the provenance to `orchestration/INTEGRATION_LOG.md`.
