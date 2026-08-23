# TASK-0162 report — native passive-tree payload hardening

## Executive summary

The native client's passive-tree mirror (`ClientModel::progression`, shipped by
TASK-0156) is now fail-closed. `apply_passive_tree` in
`native/client/remote_session.cpp` only updates the mirror when the
`passiveTree` envelope is an object with `schemaVersion == 2` (number),
`points.skill` and `earned` present as sane nonnegative integral numbers, and
`nodes`/`conduits` arrays whose entry counts fit a documented transport bound.
Any other payload — missing fields, wrong types, fractional, negative,
non-finite/overflowing numbers, oversized arrays — preserves the last valid
authoritative snapshot untouched and surfaces exactly one deterministic
`ProtocolError` presentation event per rejected envelope
(`passiveTree rejected: <stable reason>`). Rejected data can no longer become
zero or absurd gear-pane counts through unchecked casts. No server, wire,
UI, save, gameplay, or content authority changed; the single numeric cap is a
transport bound (documented in code), not a product rule.

The literal acceptance sequence passed at EXIT=0 with the tree confined to the
owned paths. Focused session tests drive the REAL production parser end to end
(socket → RFC6455 reader → `parse_envelope` → `apply_envelope`) against a
test-only scripted loopback WebSocket server that replays payloads the real
server would never emit.

## Approach

- **Validation (`native/client/remote_session.cpp`).** The TASK-0156 helper
  `apply_passive_tree` gained a fail-closed gate and now takes the session's
  event queue to emit diagnostics. Validation order is fixed:
  object → `schemaVersion` must be the number 2 → `points.skill` → `earned`
  → `nodes` array → `conduits` array → transport entry bounds. A shared
  `sane_passive_tree_integer` rejects non-numbers (strings/bools/null are
  distinct JSON arms), NaN/negatives via `!(raw >= 0.0)`, fractional values
  via `floor(raw) != raw`, and values above `kPassiveTreeTransportBound`
  (65536). On rejection exactly one `PresentationEventType::ProtocolError`
  event is pushed with the stable text `passiveTree rejected: <reason>`;
  the model is never touched on the failure path. On success the mirror is
  replaced wholesale so stale fields cannot leak between snapshots.
- **Transport bound, not balance.** `kPassiveTreeTransportBound = 65536`
  exists only so a hostile frame cannot overflow an int cast or force
  pathological behavior; it encodes no tree design, cost, budget, or balance
  opinion (the comment block above the constant says so explicitly).
  65536 is orders of magnitude above authored content and far below the 1 MiB
  single-frame ceiling in `reader_loop`.
- **Call sites.** All three production callers (player:login admission,
  dev:state snapshot, player:skilltree:update refresh) pass their envelopes
  through the same hardened path; absence of the key remains legal and silent.
- **Tests (`native/tests/session_tests.cpp`).** New test-only
  `ScriptedEnvelopeServer` binds inside this suite's TASK-0163 capsule
  (7160–7179; port 6500 never touched), completes the upgrade with the accept
  value paired to the client's fixed loopback key, and replays scripted raw
  envelopes paced by explicit grants so each assertion block observes its own
  frame deterministically. It serves exactly one connection so client retries
  can never replay the script. Two suites were added:
  - `remote_passive_tree_absence_stays_absent`: an admission with no
    passiveTree key leaves the tri-state mirror absent forever (never
    rendered as zero) with no diagnostics.
  - `remote_passive_tree_payload_hardening`: valid absent login → valid zero
    tree (dev:state channel) → valid nonzero refresh (skilltree channel);
    then a 20-case invalid battery rotating across all three production call
    sites (missing fields, wrong types, fractional, negative, bare-Infinity
    token, `1e400` inf, int-cast overflow, future schemaVersion, non-object
    tree, oversized node/conduit arrays); then the exact fractional payload
    repeated to prove byte-stable diagnostics; finally a valid recovery
    refresh proving the session stays healthy after rejects. After EVERY
    invalid frame the full progression tuple is asserted unchanged from the
    last valid snapshot, and each diagnostic text is asserted verbatim.

## Changed files

- `native/client/remote_session.cpp` — fail-closed validation + one stable
  ProtocolError diagnostic per rejected envelope (all three call sites).
- `native/tests/session_tests.cpp` — scripted-envelope harness plus the two
  focused suites (79 new PASS checks).
- `orchestration/tasks/TASK-0162-native-passive-tree-payload-hardening/**` —
  STATUS.md (claim → REVIEW_REQUESTED) and this REPORT.md.

## Acceptance evidence (literal commands)

### 1) `powershell -NoProfile -ExecutionPolicy Bypass -File native/build.ps1 -RunTests`

Full transcript: `C:\Users\Alex\AppData\Local\Temp\opencode\t0162-accept-build.log`

```text
native legacy denylist: PASS
camera2d tests: PASS
session tests passed
all audio mixer checks passed
EXIT=0
```

(core/networking/presentation-events suites ran green ahead of these; zero
FAIL lines in the transcript.)

### 2) `native/build/verdigris_session_tests.exe`

Full transcript: `C:\Users\Alex\AppData\Local\Temp\opencode\t0162-accept-session.log`

```text
PASS ptree-absent: no payload -> mirror stays absent, never rendered as zero
PASS ptree-absent: absent payload raises no diagnostic
PASS ptree: fresh admission starts absent (valid absent behavior)
PASS ptree: VALID zero tree makes the mirror present
PASS ptree: zero tree mirrors verbatim zeros (valid zero behavior)
PASS ptree: VALID nonzero update mirrors verbatim (valid nonzero behavior)
PASS ptree: missing points / missing earned / missing conduits: deterministic diagnostic text
PASS ptree: wrong-typed schemaVersion / points container / points.skill / earned / nodes / conduits on login: deterministic diagnostic text
PASS ptree: fractional points.skill / fractional earned: deterministic diagnostic text
PASS ptree: negative points.skill / negative earned: deterministic diagnostic text
PASS ptree: bare-Infinity points.skill / exponent-overflow earned (inf) / int-cast overflow earned: deterministic diagnostic text
PASS ptree: future schemaVersion on login / non-object passiveTree on login: deterministic diagnostic text
PASS ptree: oversized nodes array / oversized conduits array: deterministic diagnostic text
PASS ptree: repeated invalid payload surfaces again
PASS ptree: diagnostic text is byte-stable across repeats
PASS ptree: valid refresh applies after rejects (session healthy)
PASS ptree: valid recovery raises no diagnostic
(79 "PASS ptree" lines total; every invalid case also asserted
 "invalid update left the last valid snapshot untouched")
session tests passed
EXIT=0
```

### 3) `git diff --check`

```text
(no output)
EXIT=0
```

### 4) `git diff --name-only`

```text
native/client/remote_session.cpp
native/tests/session_tests.cpp
```

Exactly the owned source paths; no server, wire, UI, save, gameplay, or
content file touched.

## Negative controls honored

- No invented passive-tree limits that encode balance: the only cap is the
  documented transport entry bound; all well-typed values under it are
  mirrored verbatim (proven by the zero/nonzero/recovery checks).
- No server/protocol/UI/save/content change: acceptance command 4 lists only
  owned paths; the diagnostic reuses the existing `ProtocolError` event type;
  the envelope shape `{event, data}` is untouched.
- Port discipline: the scripted server binds loopback-only inside the suite's
  assigned 7160–7179 capsule; port 6500 was never touched.

## Deviations

None. `apply_passive_tree`'s signature grew an events parameter, but it is a
file-local anonymous-namespace helper in an owned file; no public interface
changed.

## Known risks

- The wire parser accepts bare `Infinity` tokens (strtod leniency); the new
  validator rejects them as malformed rather than letting them become counts
  (covered by the "bare-Infinity" case). Tightening the parser itself would
  touch `native/src/**`, which is outside this task's ownership.
