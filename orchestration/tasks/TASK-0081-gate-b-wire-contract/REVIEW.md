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

## Rev2 review at `52a7377b` — REVISE

Architect reran the five SPEC gates at
`52a7377b7654523044a2779a19ac2afaabdeda87`; all pass, the worktree is clean,
and corrections 1-2 above are substantially right. Preserve them. Three
evidence/precision corrections remain:

1. In `relaunch.responses[player:login].condition`, remove or qualify the
   sentence saying a same-identity relaunch takes the fall-through after
   admission. Same identity alone does not select that path. The dispatch at
   2655-2681 falls through only when both earlier predicates are false; for the
   native reconnect request (`guestId` + `quickGuest`, no `scionName`), this is
   normally because `quick_start_` is true. A non-quick same-identity request
   with empty `scionName` and `pending_chronicles_ == false` emits
   `chronicles:state`. State the actual boolean alternatives without a broader
   example.
2. STATUS says the rev2 commit clock is recorded in REPORT, but REPORT still
   has a placeholder. Add exact rev2 head
   `52a7377b7654523044a2779a19ac2afaabdeda87` and authoritative author/commit
   time `2026-08-21 21:03:34 PDT (-07:00)`. Also normalize the experimental
   identity now proven by saved OpenCode session metadata: harness-visible
   provider `opencode`, model id `x-preview-f-free`, variant `max`; upstream
   provider remains unknown. Do not label this run OpenRouter.
3. Append a literal rev2 acceptance section showing the post-correction output
   and exit code for every SPEC command, plus
   `git diff --name-only 986264f44b6bd3e03633d05f8b3e69fad35d4688...HEAD`
   (or the equivalent exact pinned rev2 head) proving the complete four-path
   owned range. The existing rev1 transcript may remain for provenance but is
   not a substitute for the requested post-correction rerun.

Push an additive evidence revision without amending or force-pushing. No
accepted matrix/capture content outside the one conditional sentence should
change.
