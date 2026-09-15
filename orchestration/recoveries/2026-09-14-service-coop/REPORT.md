# Native service/co-op implementation evidence

## Scope and source

Owned branch: `codex/native-service-coop-20260914`.
Base: `bafd218855608914f4f531a101f8e01cab524408` on the consolidated native lineage.
Owned checkout: `C:/Users/Alex/Documents/ChatGPT/verdigris-service-coop-20260914`.
The existing installed `Verdigris Native 2026-09-13` package, profiles, and dirty
architect checkout are untouched. This branch is a private QA integration;
normal installation, default-branch merge, public hosting and release are separate.

## Implemented contract

- Separate actor state over a shared encounter: position, facing, input,
  attack/dash recovery, life and build are independent. Monster pursuit and boss
  mechanics advance once per instance authority time, not per participant/input.
- Maintained Windows WinHTTP WebSockets, cancellable background connect/send/read,
  system certificate/hostname validation, WSS for non-loopback endpoints. A
  maintained TLS gateway terminates the external endpoint on the service host.
- Cryptographic enrollment/recovery/session credentials; credential hashes in
  SQLite and a DPAPI-protected endpoint-bound native cache. The server derives
  account ownership; guest IDs and editable local saves cannot admit online users.
- Native account, party, peer sprites/animation, readiness, invitation acceptance
  and shared entry. Production snapshots replace online development-state polling.
  Peer hit attribution cannot alter the local player's health or outgoing-hit proof.
- One bounded service command queue and 50 ms authority clock; per-connection
  writer queues, rate/frame limits, sequence/epoch replay fences, actor/scene
  fences, expired/revoked credentials, finished-reader reclamation and disconnected
  account retirement. Legacy local-review input and transport timing are preserved.
- SQLite schema 3 atomically commits private accounts, circulation journal and
  outcome markers before gameplay publication. Per-House first-Warden entitlement
  records bind the source encounter to each eligible House/Scion. Claims survive
  restart and cannot be redeemed twice. Relics retain UUID and queued/released/
  claimed ownership; retired unclaimed releases requeue, claimed items do not.
- Independent service process, private operator files, readiness/stop controls,
  offline enrollment/recovery, consistent backup/fresh restore, separate player
  profile, and clean-source player/service packaging with embedded identities.

## Evidence obtained during integration

All records below are actual runs, not inferred from code or worker completion.
Final frozen-package runs and hashes are recorded separately when produced.

- Initial consolidated `verify-native.ps1`: passed before implementation.
- Actor regression initially failed because moving A changed B; fixed by actor/
  instance separation. Service authority baseline reproduced ten failures; fixed
  gates pass for mortal oath, crypt admission, forged purchases, nonpositive bank
  deposits and House bank/treasury separation.
- Integrated build 6 `native/build.ps1 -RunTests`: exit 0. Includes authority 10,
  actor 27, entitlement 21, protocol 286 (this run), relic/parser 23, online co-op
  28, Store 202, transport 148 assertions, plus all pre-existing native suites.
  Store coverage includes five real child crashes and joint account/world rollback.
- Integrated native `--scenario all`: 83 scenarios, exit 0. Existing particles,
  inventory, camera, terrain, UI, audio and fullscreen performance gates passed.
- Integrated build 7: exit 0, including 46 online co-op checks with four accounts
  in two simultaneous isolated parties, 287 protocol assertions and Store 202.
- Extended two-process native journey passed at
  `native/build/graphical-service-20260914-202804/clients`: ordinary account and
  owned House/Scion controls, separate movement, native party invitation/readiness,
  shared Warden victory, separate rewards/possessions, return and cached reconnect.
  A/B retained different exact inventory UUIDs. Their passive point totals moved
  2→6 and 2→5 respectively (individual combat progression plus the House award).
  Concurrent 1280x800 steady paint averaged 25.42/25.05 ms, p95 32.74/32.24 ms,
  peaks 46.48/50.65 ms; first 20 frames per scene and open panels excluded.
  These are renderer times, not server tick/RTT or a claim of 60 fps.
- Two distinct graphical native processes against an independently started service:
  initial real enrollment/House/Scion/movement/party/shared-combat run passed at
  `native/build/graphical-service-20260914-195247/clients`. Both accounts landed a
  real hit and retained separate identities. Parent inspected 1280x800 captures,
  the wide combat capture, and successive peer-movement frames. A town capture
  occluded actors behind a tree; the later driver frames the open arrival court.
- Extended graphical run `graphical-service-20260914-200927` correctly FAILED:
  combat routing crossed public exit stairs and returned to town. The driver now
  excludes stairs during combat and asserts instance continuity; it permits stairs
  during deliberate return. No gameplay balance or terrain was changed for it.
- Actual process operations and online/local launcher tests passed. Worker pressure
  evidence exercised 1,152 malformed handshakes, malformed/oversize frames, a slow
  reader, 20x96-message bursts and authenticated reconnects. Immediate admission
  under a saturated queue may fail; recovery after an explicit 300 ms interval was
  measured. Final package pressure measurements remain a distinct run.
- Review found and reproduced a retained-session callback cycle; the old build
  failed the new weak-reference shutdown check after four real account admissions.
  The callback now keeps a weak connection reference. A second review found that
  a claimed relic could not circulate after its later owner's permanent death;
  the journal now records distinct source Scion deaths and retains replay history.
  Final focused reruns passed: 51 co-op/lifetime checks, 31 relic/parser checks,
  285 protocol checks (this run), and 21 entitlement checks. Both identified
  regressions are corrected; package-level reruns follow the clean source freeze.

## Boundaries

`coop-v1` is explicitly QA-only pending an owner policy ruling. Party size defaults
4 and is configurable 2..8; disconnected input freezes with a 30-second reservation;
no late entry, retrospective disconnected credit or post-death credit; unfinished
instances retire on restart while committed carried items and earned claims survive.
No Internet or separate-computer result is inferred from two local processes.
No authorized host/domain/deployment endpoint was supplied. The Caddy example has
not been deployed. Installation promotion and owner visual acceptance remain open.
