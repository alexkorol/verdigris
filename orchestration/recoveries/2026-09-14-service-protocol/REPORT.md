# Production protocol test evidence

## Implementation

`native/tests/service_protocol_tests.cpp` exercises actual WinHTTP
`ServiceTransport` connections to a real `WebSocketServer` with a private,
disposable production `Store`. It sends ordinary version-1 envelopes only.
Operator enrollment-code issuance arranges accounts; no development command,
fake protocol handler, direct session mutation, or inventory injection is used.

Covered contracts:

- Unsupported protocol rejects before consuming an enrollment code; the same
  code then admits version 1. Subsequent code reuse is rejected.
- Unauthenticated guest-ID spoof and House creation fail without replacing the
  authenticated account. Authenticated guest-ID spoof cannot switch its actor.
- Repeating the exact House-founding epoch/sequence acknowledges the replay and
  leaves exactly one House.
- A foreign account cannot create a Scion in another House. Two accounts create
  and select independent owned Houses/Scions via production commands.
- Their ordinary starting possessions have independent physical item UUIDs.
- Token reconnect preserves account identity, rotates command epoch, and rejects
  old-epoch mutations without changing possessions or House roster.
- Raw supplied stale `sceneId` or foreign `actingActorId` rejects movement and
  preserves position/inventory. The field is intentionally `actingActorId`;
  party invite target `actorId` is a separate reference.
- Stopping/releasing the server and Store, reopening the same durable directory,
  and authenticating both accounts preserves the owned House, selected living
  Scion/positive life, and exact item UUID/data rows independently.

The receive helper uses the actual blocking transport on a joined reader thread,
a 256-event queue, and five-second expected-event deadlines. A production ping
barrier consumes older events before requesting fresh owner snapshots. Errors
and output contain no credentials.

## Verification identity

Baseline implementation commit `3957ae3a7263cd15792894a2e24514e7e5378e99` was
published and remote-verified after 260 checks passed against the parent's then
built objects. Historical test-binary SHA-256:
`24bd6fa923d43514276efe4bcced29aeae16a4260bc5c1bdb04856af337be291`.

The final run compiled fresh copies of the latest parent source/header files,
including its integrated Store global-state APIs and scene/actor fences:

- Parent checkout: `C:/Users/Alex/Documents/ChatGPT/verdigris-service-coop-20260914`.
- Parent commit locator: `44d450068afdc1231bc3d968172241891cd73338`, plus captured
  working-tree changes; this is not a clean package/source identity claim.
- Captured source directory: `native/build/protocol-source-final-1eda3fc7fd174f23abe260ebb18a35af/`.
- Per-file `source-manifest.json` SHA-256:
  `f47a6f005599678ab227fd5a372a0bc8e3b1e5b2d63a1cdb09d36825eefe3b78`.
- Each captured parent file was hashed before copy and checked against both its
  source and copy afterward. All compilation used the captured files.
- Final executable: `native/build/service_protocol_tests_final.exe`.
- Final executable SHA-256:
  `020e170579cdca015aa98bf33fe49f8ae6f49dd38a68f82e68ff802fed419981`.
- Disposable Store retained at
  `C:/Users/Alex/AppData/Local/Temp/verdigris-protocol-1816508617140700`.

MSVC C++20 `/EHsc /W4`, normal CRT/Winsock compatibility defines: compile/link
exit 0. **283 production service protocol checks passed**, test exit 0. Guard
assertions also count toward the printed check total; event timing may change
the total without changing the semantic cases. Existing captured-source warnings
were an unused `player_level` parameter in core and a constant-cast truncation
warning in networking. The new test module emitted no warnings.

Local reproducible wrapper: `native/build/build-service-protocol-final.cmd`.
It compiles the test plus `core.cpp`, `networking.cpp`, `seasonal.cpp`,
`service_store.cpp`, and `service_transport.cpp` with `native/include` from the
captured tree, linking Winsock and the production files' Windows pragma libraries.
Register the same standalone target in the parent's build/verification scripts.

## Diagnostic corrections and scope

Two initial test assumptions were corrected from source evidence: a new account
has a null Chronicle instead of an empty House array, and runtime actor IDs are
distinct from account IDs. Neither correction changed production code or relaxed
the ownership assertions. An intermediate source capture could not compile while
the router referenced the not-yet-integrated Store APIs; the final complete
capture above compiled and passed after those APIs arrived.

All server/Store restart operations occur inside this test process, with both
objects destroyed/reopened and the exclusive storage handle released. This
proves cold object/storage reload through the real production protocol. It is
not evidence of OS process termination, crash injection, packaged graphical
behavior, Internet deployment, or physical-machine separation. Those remain
separate parent/Store-lane gates. Re-run the target against the final integrated
source before packaging.
