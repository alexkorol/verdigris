# Native service and Two-House co-op

**Implemented and locally tested. Packaged and verified. Pushed and remote-verified.**
Deployment, separate-computer/external-network verification and owner acceptance remain outstanding.

Runtime commit `6bec25d058f937e812b2b78c51d89870f498b7e0` was pushed and verified with `git ls-remote` on the task branch.

Branch: `codex/native-service-coop-20260914`, based on consolidated
`bafd218855608914f4f531a101f8e01cab524408`. Owned checkout:
`C:/Users/Alex/Documents/ChatGPT/verdigris-service-coop-20260914`.
Remote: `https://github.com/alexkorol/verdigris.git`.

## Player result

The actual native application authenticates to an independently running Windows
service, admits an owned House and living Scion, displays other players, forms a
party through invitations and readiness controls, enters an isolated expedition,
fights a shared Warden encounter, preserves private possessions, awards each
eligible House, returns to town and reconnects using its protected credential.
The service remains healthy after both clients and their launchers exit.

Online admission extends the original illustrated gateway menu and its physical
controls and typography. The generic centered login panel was removed after the
owner's correction. Transient feedback now fits at most two compact lines, uses
an explicit ellipsis when necessary, expires after four seconds and clears on a
scene transition. Short notices size to their text. Existing game artwork is reused.

## Selected packages

| Artifact | Exact identity |
| --- | --- |
| Player entry | `C:/Users/Alex/Documents/Verdigris Service QA 2026-09-14-r4/Verdigris.exe` |
| Player source / tree | `6bec25d058f937e812b2b78c51d89870f498b7e0 clean` / `903f05bb4badf3206b2cbea94a02982c371c9f0e` |
| Player client SHA-256 | `75c1e9ee9fe3601b9a6495d2baac6570919412b52b4a16883036e7b6b6176ae7` |
| Player launcher SHA-256 | `13afedc232903d3b1624c650b9c3ef4b762515d3c05d03aa1ee8475c026cc9a5` |
| Player ZIP | `C:/Users/Alex/Documents/Verdigris Service QA 2026-09-14-r4.zip` |
| Player ZIP SHA-256 | `d6bdc3e641e0524d8913c0b6e00dd9217fb6061c8432a975af75a5d8cfddd851` |
| Host executable | `C:/Users/Alex/Documents/Verdigris Service Host QA 2026-09-14-r2/verdigris_server.exe` |
| Host source / tree | `9cfb4aab9e6edc16ba5dd919c263f6673ca63f98 clean` / `5963dbd17d9353595f018f185756f61871462dee` |
| Host executable SHA-256 | `f8787626130a27577ba56f390dabd07d229627788af5340779578ee667e62147` |
| Host ZIP | `C:/Users/Alex/Documents/Verdigris Service Host QA 2026-09-14-r2.zip` |
| Host ZIP SHA-256 | `6aee1a44c3671b56dcb27eee2f2300d554f6ea2d56ae9ebf477e404ef2a0dc0e` |

Player manifest: 1,286 files; host manifest: seven files. Full identity record:
`native/build/package-identities-final.json`.

The player and host use the same service protocol/content contract. Host r2 is
unchanged by the later client presentation, movement-revision and QA route fixes.
The player's bundled server is for the separately preserved local-review path.
Archives contain only manifested package resources, with their executable hashes
checked inside the ZIP; disposable profiles and enrollment files are excluded.
Previous candidates and failure evidence remain recoverable.

## Authority and persistence

- Shared world state has separate actor position, facing, combat recovery,
  health and build state. One authority clock advances each instance once.
- The service validates account ownership, living Scion selection, invitations,
  leader/readiness state, economy quantities/prices/proximity and private results.
  Foreign instance data and private inventory/bank state are not broadcast globally.
- WinHTTP provides cancellable background WSS with system certificate/hostname
  validation. Plain WS is permitted only for loopback. CNG credentials authenticate
  server-owned identities; SQLite stores hashes and DPAPI protects the client's
  endpoint-bound credential cache. Revocation, expiration and takeover are enforced.
- SQLite schema 3 uses WAL/FULL and an exclusive Store lock. Private account state,
  circulation journal and outcome markers commit atomically before publication.
  Eligible per-House Warden claims survive restart and redeem once.
- Physical relic UUIDs survive circulation and later Scion deaths. Replay history
  prevents duplication, while original-House recovery receipts survive offline
  reconciliation and another owner's subsequent death.
- Queues, connections, frame sizes and rates are bounded. Epoch/sequence and scene
  fences reject stale commands; slow readers time out and disconnected input freezes.

## Verification

Native-only gates were used. The historical JavaScript `npm run playtest` was not
counted as native service evidence. Baseline native verification passed before
implementation; integrated `native/build.ps1 -RunTests` runs passed existing native
suites and new authority, actor, protocol, entitlement, party, Store and transport tests.

| Gate | Evidence and result |
| --- | --- |
| Store | 202 assertions, including contention, backup/fresh restore and five real child crashes at account/world/outcome transaction boundaries. |
| Final focused integration | 39 relic/parser, 62 pickup, 21 entitlement and 54 four-client party/lifetime checks; production protocol checks passed. `native/build/receipts-final.log`. |
| Transport | 148 checks, including rejection of an untrusted TLS endpoint without a validation bypass. |
| Movement revision | Full integrated session suite passed in `native/build/compile-ui-pose.log`; old-scene sequence 999 is rejected and new-scene sequence 1 is accepted. |
| Descent/return metadata | Nine route checks and 17 real-endpoint metadata checks; two actual clients then completed the local stairs reproducer. |
| Feedback | Presentation event suite and production-paint feedback scenario passed, including scene clearing, bounded long receipts, ellipsis and fitted short notices. |
| Exact final player | Fresh `package-native.ps1 -OutputDirectory <r4>` and `verify-player-package.ps1 -PackageDirectory <r4> -EvidenceDirectory <r4>/qa/verification` exited 0. All 84 scenarios passed; two launcher lifecycles and same-executable/same-profile settings restart passed. `native/build/package-player-r4.log`, `native/build/verify-player-r4.log`, package `qa/verification/scenario-exit.txt`. |
| Actual online launcher | `test-service-package-launch.ps1` exited 0. Actual top-level `Verdigris.exe` started the exact r4 client with the online endpoint, no local server; both owned processes closed and independent host health passed. `native/build/packaged-service-r4/launcher/summary.json`. |
| Actual two-client journey | `test-service-clients.ps1` exited 0. Both Scions landed their own hits, earned Warden credit, received separate rewards and reconnected. House points A: 2 → 6; B: 2 → 5. Distinct inventories/identities, matching shared instance `instance:82bfcbe4f797298f85c935a9659a07db`. `native/build/packaged-service-r4/clients/summary.json`. |

### Delivered visual and motion evidence

Viewed exact r4 launcher `online-account-ready.png` at 3440x1440 and A's real
`account-entry.png` at 1280x800. Viewed A's `shared-expedition.png` and B's successive
`warden-fight-0.png` / `warden-fight-11.png`: separate actors, movement, strikes,
damage attribution and native particles are visible; stale NPC feedback is absent.
Credential-page error/masking and revoked-session recovery captures are in the
packaged `qa/verification` suite. Wide images were produced at 3440x1440; the image
viewer scales their preview to 2048 pixels wide. The 1280x800 captures were viewed
without preview downscaling.

Two native processes and an independent host ran on one Windows computer using
disposable profiles and application-owned windows. Authenticated actor/House/Scion
identities and matching instance IDs accompany captures in `clients/summary.json`.
Sampled motion sequences show successive production frames; they are not continuous
real-time recordings. Visual inspection is distinct from owner acceptance.
Both `clients/A/warden-motion-sampled.mp4` and `clients/B/warden-motion-sampled.mp4`
contain 12 original 1280x800 production frames at 8 fps, verified with ffprobe.
The source frame sequences were inspected for movement and combat poses.

| Exact two-client rendering | Average | p95 | Peak | Samples |
| --- | ---: | ---: | ---: | ---: |
| A | 29.959 ms | 60.633 ms | 113.685 ms | 2,499 |
| B | 29.787 ms | 57.354 ms | 140.809 ms | 2,511 |

These steady samples exclude the first 20 gameplay frames per authoritative scene,
frontends and open party panels. Both clients run concurrently on one machine.
Tail frames exceed 40 ms; this is not a claim of 60 fps or uniformly smooth frame
delivery. The original separate `<40 ms average` full-resolution regression gates
remain unchanged. This journey's host reported authority average/peak
24.845/213.176 ms and commit average/peak 1.711/6.181 ms over 3,556 ticks/71 commits.
Exact package full-resolution regression averages: 258 particles 32.983 ms;
inventory drag 31.123 ms (42.112 ms peak); stationary scene 25.4 ms;
moving scene 35.5 ms (67.6 ms peak). All unchanged 40 ms average gates passed.

### Exact host r2 operations and network workload

All three runners exited 0 against the selected host hash, checked again afterward:
`test-service-operations.ps1`, `test-service-pressure.ps1`, `test-service-soak.ps1`.
See `../2026-09-14-service-final-r2/REPORT.md` for commands, script hashes and raw evidence.

| Check | Observation |
| --- | --- |
| Operations | Private ACL enrollment without secret stdout; exclusive Store; readiness; stdin-independent lifetime; clean stop; backup and fresh restore. |
| Pressure | 1,152 malformed handshakes, five malformed wire cases, authenticated nonreader, 20 peers × 96 burst messages and 33 authenticated reconnects; explicit 300 ms settling interval after flood. |
| Soak | 120.017 seconds, four authenticated clients in two isolated two-person expeditions; 40 confirmed movements and interruption/reconnect. |
| Injected delay | Seeded 20–35 ms per forwarded TCP chunk in each direction, ordered delivery. This is not arbitrary packet reordering or a fixed RTT. |
| Snapshot RTT | 328 samples: median 200.864 ms, p95 231.651 ms, maximum 263.431 ms. |
| Reconnect | 3,108.416 ms from interruption through authenticated restored snapshot; actor/instance retained and command epoch rotated. |
| Authority / SQLite | Authority average/peak 42.994/105.778 ms; commit average/peak 2.370/6.550 ms. |
| Resources | Private bytes 2,670,592 → 3,190,784, peak 3,502,080; approximately 84.9% of one logical CPU. |

This bounded soak overlapped a graphical regression workload. It establishes neither
quiet-machine capacity nor spare timing capacity, maximum players or a long-term
memory plateau. Rendering, snapshot RTT, authority work and SQLite latency are
separate measurements. All owned service runs stopped cleanly.

## Failures found and corrected

Independent-actor and callback-lifetime regressions failed before their corrections.
Relic tests exposed loss after a later owner's death; recirculation and durable
original-House receipts now pass both-death/restart checks.

The r2 graphical journey exposed stale own movement after party entry: A saw B at
`(7,14.6)` while B still displayed `(7,20)`. A route-margin-only explanation was
incomplete. The server resets movement sequencing on new instance entry; the client
retained its town high-water mark. The correction resets only on an explicit scene
transition and rejects old-scene movement. After that fix, a separate QA route failure
crossed descent stairs and treated any scene change as extraction. Existing descent
metadata now guides routes and only an intentional return to town satisfies the check.
Neither correction changes terrain or combat balance. Failure captures remain under
`native/build/local-{margin,pose}-verification`; corrected local evidence is under
`native/build/local-stairs-verification`.

R3 passed all 84 scenarios and the actual co-op journey, but the owner rejected its
generic login presentation. It is superseded by r4. The earlier long notification
also led to an explicit fitting regression; technical validity alone did not settle
the visual defects.

## Operations, policy and remaining dependencies

The host is an independent Windows console service process, not an installed SCM
service. `native/tools/SERVICE-OPERATIONS.md` documents enrollment/recovery/revocation,
private Store paths, readiness/stop, version compatibility, backup/fresh restore and
rollback. The player entry is `Verdigris.exe --online "wss://<host>/game"`; credentials
are entered in the native menu and never put in command-line arguments.

`coop-v1` is explicitly private-QA policy: party capacity defaults to four, configurable
2–8; disconnected actors freeze with a 30-second reservation; no late entry or
retroactive disconnected/post-death objective credit. Unfinished instances retire on
restart; committed carried inventory and earned claims survive. Permanent public
party/loot/loss/recovery policy still requires the owner's ruling. The tested initial
workload is four clients; configurable capacity is not a capacity certification.

No authorized host/domain/certificate/access was supplied. The same-host Caddy TLS
gateway example is not deployed. Separate-computer, external-network and trusted
external native WSS delivery remain unverified. First-Warden shared entry/combat/return
is the graphical acceptance slice; full-campaign/deep-floor co-op is not certified.
There was no normal-installation promotion, default-branch merge, public release or
deployment. These boundaries do not withhold verified task-branch commits.

## Preservation

The normal `C:/Users/Alex/Documents/Verdigris Native 2026-09-13` installation remains
at `30ed52c842cc0500db2b3cf0a6397454c52410c8 clean`. All 1,285 manifested resources
were rehashed with zero mismatches (`native/build/installed-preservation.json`).
Owner saves/settings were not targeted; no pre/post user-data hash comparison is
claimed. Only test-owned processes were stopped. Dirty architect and consolidated
checkouts remain untouched; new packages, Stores and profiles use separate paths.
