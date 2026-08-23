# TASK-0162 REPORT — native passive-tree payload hardening (lane ox-sw-b)

- Branch: `codex/TASK-0162-native-passive-tree-payload-hardening-ox-sw-b`
- Base: `dc8df4399da4c2a0b8b92a4ea395cba79cc6ff17`
- Worktree: `Z:\Code\.worktrees\verdigris\ox-sw-b`

## Executive summary

The native client's `passiveTree` mirror (TASK-0156) now lands ONLY from an
envelope that validates against the frozen schemaVersion-2 wire contract
(schemaVersion == 2; object `points` with integral nonnegative in-range
`skill`; integral nonnegative in-range `earned`; array `nodes` and array
`conduits` within a documented transport entry bound). Malformed or incomplete
envelopes fail closed: the last valid authoritative snapshot is preserved
untouched and a deterministic `ProtocolError` presentation diagnostic
(`passiveTree rejected: <stable reason>`) surfaces. Valid absent / zero /
nonzero behavior is unchanged, proven by 60 new focused checks driving the
production envelope seam over a real socket. No server, wire, protocol-schema,
UI, save, gameplay, content, or balance change.

## Approach

- Production change confined to `native/client/remote_session.cpp`:
  - New pure validator `validate_passive_tree()` + numeric gate
    `sane_passive_tree_count()` (finite, integral, `[0, INT32_MAX]`) in the
    existing anonymous namespace. First violated expectation wins, so the
    diagnostic is deterministic for any given payload.
  - `apply_passive_tree()` now takes a diagnostics sink (the session's
    `pending_events_`); on validation failure it pushes exactly one
    `PresentationEventType::ProtocolError` event with text
    `"passiveTree rejected: <reason>"` and returns BEFORE any model mutation,
    preserving the last valid snapshot. On success it installs the validated
    mirror with `present = true`.
  - All three call sites (player:login admission, dev:state snapshot,
    player:skilltree:update) route through the same hardened seam; absence of
    the key still skips the call entirely (absence is not malformed).
  - Transport bounds documented in-code as transport/representation guards,
    NOT product rules: `kPassiveTreeMaxEntries = 10000` per array and an
    int-range ceiling on mirrored counts (`kPassiveTreeMaxPointValue =
    2147483647.0`). Both sit orders of magnitude above any real payload;
    nothing about tree design, costs, or progression is encoded. Values are
    rejected outright — never clamped, truncated, or zeroed.
- Test harness added to `native/tests/session_tests.cpp`:
  - `FakeEnvelopeServer`: minimal RFC6455 server (loopback capsule 6980-6999,
    reserved for this lane's parser harness) that completes the documented
    upgrade handshake, discards client frames, and delivers verbatim test-
    dictated envelope TEXT through a real TCP/WebSocket path into the
    production `parse_envelope -> apply_envelope -> apply_passive_tree` chain
    — including overflow literals like `1e400` no in-process JsonValue builder
    can express (the native JSON parser maps them to ±inf via strtod).
  - Three focused tests: absent-stays-absent; valid login/dev:state/
    skilltree:update mirroring including present-with-zeros; and a 24-case
    invalid matrix plus determinism repeat, invalid-login-seam, and
    post-rejection recovery proofs.

## Changed files

- `native/client/remote_session.cpp` (validation + diagnostics wiring)
- `native/tests/session_tests.cpp` (harness + 3 test functions + main() registration)

## Public interfaces added/changed

- None exported. `apply_passive_tree()` gained an internal diagnostics
  parameter; it is file-local (anonymous namespace). Wire format, envelope
  names, `ClientModel`, headers, and server behavior are untouched.

## Test commands + outcomes (exit codes)

Ran from worktree root, PowerShell:

| Command | Exit | Outcome |
|---|---|---|
| `powershell -NoProfile -ExecutionPolicy Bypass -File native/build.ps1 -RunTests` | 0 | MSVC build clean (pre-existing warnings only); denylist PASS; core/networking/camera2d/session/presentation-events suites ALL PASS; `session tests passed` |
| `native/build/verdigris_session_tests.exe` | 0 | `session tests passed` — includes the 60 new ptree checks (all PASS) |
| `git diff --check` | 0 | clean |
| `git diff --name-only` | 0 | empty working tree (everything committed) |

New-check highlights (60/60 PASS): exact stable diagnostics for missing
schemaVersion / points / points.skill / earned / nodes / conduits; wrong types
(schemaVersion string, points number, skill string, earned boolean, nodes
object, conduits string); fractional (2.5, 5.5); negative (-1, -4); non-finite
(1e400 -> inf, -1e400 -> -inf); overflow-like (2147483648,
999999999999999999999); oversized arrays (10001 entries vs bound 10000);
non-object passiveTree; identical-payload determinism; snapshot preservation in
every case; fresh valid payload accepted after rejections.

## Manual verification

- Attribution experiment during development: two interim runs showed varying
  failures in PRE-EXISTING long journey tests (`reconnect:*`, `gate-b:*` legs).
  A binary built from pristine base sources passed its sample, then my full
  binary was rerun under comparable conditions and passed the ENTIRE suite
  twice consecutively (direct run + verbatim acceptance run). The transient
  failures were load-sensitive flakes in timing-heavy loopback journeys
  (observed ~45 s scheduling gaps between swings during one run), not caused
  by this diff: those journeys never touch the passiveTree parser path and my
  tests execute after them. No owned-scope code was changed in response.
- One test-authoring bug was found and fixed before final green: the
  oversized-array helper initially emitted duplicate JSON object keys for the
  conduits variant, letting map last-wins semantics mask the violation; the
  helper now oversizes only the named key.

## Commit SHAs

- `6138f3dc` chore(TASK-0162): claim lane ox-sw-b
- `fd157c78` TASK-0162: fail-closed passiveTree envelope validation with deterministic diagnostics
- `<final>` docs(TASK-0162): report and REVIEW_REQUESTED (this commit)

## Deviations

- None from SPEC. Owned paths only. The transport bounds are documented as
  such in code comments and here; no balance/product rule was introduced.

## Unresolved questions

- None blocking. Should a future wire contract bump schemaVersion beyond 2,
  the validator will reject it fail-closed by design ("schemaVersion must be
  the number 2"); that bump will require a deliberate protocol task.

## Risks

- Strict `schemaVersion == 2` gating means an uncoordinated future server
  version bump disables progression display (fail-closed) rather than
  misparsing — intended per SPEC but worth remembering at integration time.
- The long loopback journeys in session_tests.cpp remain load-sensitive on a
  busy fleet machine (pre-existing; outside this task's scope).

## Follow-ups

- Consider a fleet-level serialization note for parallel lanes running native
  session suites simultaneously (the observed flake source).
- If the wire contract ever grows optional fields, decide explicitly whether
  they belong in the validator (currently only the frozen five expectations
  are enforced; extra unknown members are tolerated).
