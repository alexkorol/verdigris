# TASK-0071 protocol matrix audit

Coordinator: `luna-mac`  
Worker branch: `codex/TASK-0071-protocol-matrix-audit-luna-mac`  
Audit base: `5d8d8c60837ea826851c0ceaef04d463b2060c25`  
Audit evidence commit: `b13dd7bef27579d093e899b1892fa34d78b96f70`  
Worker tip after merging current origin tip: `e30c943e4d6c4e3ff55c66a0bbb3d6d638b2c1de`

## Scope and result

Updated only `docs/rebuild/NATIVE_CLIENT_PROTOCOL_MATRIX.md`. Every ✅ row
now cites a current-tip source file, line label, and exact `check(...)` text.
Movement, telegraph, damage, death, drop, pickup, equip, extraction, and
reconnect are tied to the current session/networking tests. The drop row now
reflects the current `item:change` and `world:itemDropped` envelopes. The
formerly stale reconnect row is green. Persistence remains open with an
actionable TASK-0056/N5 gap, and the Gate B section lists the required House,
Scion, death, successor, and persistence envelopes as open rows.

No Windows build was run, per the Mac-lane SPEC instruction. No source,
`native/**`, `playtest/**`, or peer task files were edited.

## Literal verification transcripts

```text
$ git rev-parse HEAD
b13dd7bef27579d093e899b1892fa34d78b96f70
$ rg -n exact session labels native/tests/session_tests.cpp
106:        "remote-negative: state is rejected, not a silent local fallback");
123:          "remote: player:login acknowledged -> ready");
215:  check(session.model().player.x > start_x, "journey: movement echo updates x");
219:  check(session.model().player.facing == "right", "journey: aim updates facing");
243:  check(outgoing, "journey: outgoing combat:hit reached the client");
244:  check(kill, "journey: enemy death reached the client");
259:  check(gear != nullptr, "journey: named item entered inventory (pickup)");
294:  check(incoming, "journey: incoming combat:hit reached the client");
295:  check(telegraph, "journey: monster:telegraph reached the client");
317:  check(session.model().extracted, "journey: ExtractionCompleted from surface message");
366:  check(resumed, "reconnect: Retrying then Ready after server restart");
exit code: 0

$ rg -n exact networking labels native/tests/networking_tests.cpp
111:  check(movement && movement->event == "player:movement", "applied sample broadcasts player:movement");
134:  check(transition && transition->event == "party:scene:transition", "solo entry emits a scene transition");
263:        "floor treasure emits item:change and world:itemDropped");
402:  check(equipped.has_value(), "item:equip emits player:equippedAnItem");
exit code: 0

$ rg -n -F selected session labels native/tests/session_tests.cpp
106:        "remote-negative: state is rejected, not a silent local fallback");
121:    check(session.start(&error), "remote: connect + upgrade + login sent");
123:          "remote: player:login acknowledged -> ready");
353:  check(lost, "reconnect: ConnectionLost is visible (no silent local fallback)");
367:  check(session.model().player.uuid == guest, "reconnect: same guest identity re-logged in");
368:  check(!session.model().scene.id.empty(), "reconnect: login snapshot is authoritative");
405:  check(flushed, "replaced: first session is disconnected");
406:  check(lost, "replaced: ConnectionLost from player:session-replaced");
exit code: 0

$ rg -n -F selected networking labels native/tests/networking_tests.cpp
114:  check(state_axis(request_state(session, "m-3"), "x") > start_x, "right sample moves east");
144:        && in_zone["state"]["sceneMetadata"]["stairsDown"].is_object(), "both stairs exist");
266:  check(ground_item_has_fields((*floor)[0]), "ground envelope has uuid, id, name, x, y");
405:        "equip response includes wear-slot state");
412:        "snapshot wear matches the equip response");
exit code: 0

$ git diff --check
exit code: 0
$ git diff --name-only
docs/rebuild/NATIVE_CLIENT_PROTOCOL_MATRIX.md
exit code: 0
```

The `git diff --name-only` transcript above was captured before this report
was created; the task implementation commit contains the matrix, STATUS.md,
and this task report only.

## Current-tip synchronization transcript

```text
$ git merge --no-edit origin/codex/native-reconstitution
Merge made by the 'ort' strategy.
 orchestration/DECISIONS.md  | 14 ++++++++++++++
 orchestration/RUN_STATUS.md | 12 ++++++++++++
 2 files changed, 26 insertions(+)
exit code: 0
```

The synchronization changed coordination truth only; the audited session and
networking test sources were unchanged.
