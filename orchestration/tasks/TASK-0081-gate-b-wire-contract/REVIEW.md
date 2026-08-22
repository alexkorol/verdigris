# TASK-0081 review — REVISE

Architect review at worker head
`0302ea4c3126a574ad7248fceb9a2d7021c2e4c0` on 2026-08-21 20:43 PDT.
The owned-path boundary is clean, all five literal SPEC commands pass, the
capture parses, all 12 required journey records exist, and the broad Gate B
inventory is useful. Do not discard or rework the accepted rows.

The following exact corrections are required before this contract can safely
drive TASK-0077:

1. **Mortal-oath response shape:** record `mortal-oath-state` names
   `player:chronicles:select` as its client event but leaves `responses` empty.
   That request does produce `player:login` through `emit_login` at
   `native/src/networking.cpp:2632-2654`. Record that response and its exact
   relevant keys, or choose a genuinely response-less observation surface and
   mark the missing response explicitly RED. Reconcile the summary counts.
2. **Relaunch conditionals and proof boundary:** record `relaunch` currently
   gives `player:login {guestId}` -> `player:login` without its dispatch
   conditions. At `native/src/networking.cpp:2655-2681`, a non-quick,
   non-pending Chronicle admission with nonempty `guestId` emits
   `chronicles:state`; `emit_login` is only the fall-through path. Freeze the
   actual conditional alternatives and response keys. Keep the server-restart
   tests explicitly generic: they prove reconnection, identity, and a login
   snapshot, not Chronicle/account-state durability across server restart.
3. **Clock/throughput evidence:** `STATUS.md` records `transitioned-at` as
   20:55 PDT, but the pushed review commit's authoritative author/commit time
   is 20:42:44 PDT. The claim commit is 20:21:49 PDT, making durable
   claim-to-review-request commit latency **20m55s**, not ~35 minutes. Correct
   STATUS and REPORT so adaptive-runway telemetry is not poisoned. Preserve
   the imprecise 20:20 launch/start time separately if desired; do not derive
   an exact duration from it.
4. Rerun every literal SPEC acceptance command after corrections, update its
   transcript where output changed, run `git diff --check`, verify the complete
   base-to-revision path list includes only the two owned surfaces, update
   STATUS to `REVIEW_REQUESTED`, and push a revision commit on the same worker
   branch. Do not amend or force-push either existing commit.

No source, native test, gameplay, protocol redesign, or unrelated matrix work
is requested.
