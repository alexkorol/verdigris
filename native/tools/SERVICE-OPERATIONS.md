# Verdigris private service operations

This Windows service package runs independently from the graphical client.
Use a dedicated operator account, a fixed local NTFS data directory, and the
Windows 10/Server 2016 or newer system libraries. The Store uses Windows CNG
and WinSQLite; their runtime/licensing is supplied by Windows. Gameplay content
is compiled into the executable. No renderer or client art is required.
`HISTORICAL-LICENSE.txt` preserves the repository's inherited license notice;
this unsigned private package is not a public release authorization.

## Start, readiness and shutdown

From Windows PowerShell 5.1 in the extracted service package:

```powershell
./start-service.ps1 -DataDirectory 'C:/VerdigrisService/data' -Port 6540 -QaPolicy coop-v1
./verdigris_server.exe --health --data 'C:/VerdigrisService/data'
./stop-service.ps1 -DataDirectory 'C:/VerdigrisService/data'
```

The start script gives a newly created data directory a private current-user
and SYSTEM ACL. Existing directories retain their permissions; protect an
existing/imported directory equivalently before use. Keep data outside the
package and outside network shares. The Store holds an exclusive ownership lock
and verifies SQLite integrity/schema before the listener can become ready.

Readiness means the Store opened and listener started; it is distinct from a
process merely existing. Runtime storage/listener failure clears readiness and
stops the service. `--health` and `--stop` use private Windows events and must
run as the operator in the same Windows session. For a Windows service manager,
invoke controls in its service session or forward CTRL_BREAK/SIGTERM. This is
a console service executable, not a registered Windows SCM service.

Service mode ignores stdin and exits through explicit stop/console signals.
A client launcher owns only its client process. The local-review launcher
retains its separate private-server mode and stdin shutdown contract.
Wait for process exit before backup, restore, enrollment or operator recovery.
Configure the chosen process manager with a bounded restart delay, finite log
retention (for example five 10 MiB files), and failure alerting. Do not log
credentials or gameplay message bodies. Native startup/shutdown messages contain
no secrets; the server does not provide an unbounded application event log.

## Explicit QA policy

`--qa-policy coop-v1` acknowledges the implementation's current private-QA
policy, not an owner ruling for a permanent public economy: four party
participants by default (`-PartyCapacity 2..8` / `--party-capacity 2..8`); 30-second disconnected reservation with frozen input; no late
entry or retrospective disconnected/post-death objective credit; unfinished instances retire to town on
restart while preserving already committed carried inventory. Accounts/Houses
remain independent. Final party size, permanent loss/recovery terms and public
loot policy require their own owner ruling before production promotion.

## Enrollment and account recovery

While the service is stopped:

```powershell
./verdigris_server.exe --enroll --data 'C:/VerdigrisService/data' --output 'C:/private/new-enrollment.txt'
./verdigris_server.exe --recover 'acct_verified-account-id' --data 'C:/VerdigrisService/data' --output 'C:/private/new-recovery.txt'
```

Output must be a new file. The executable gives it an explicit current-user
and SYSTEM ACL and writes the code directly without printing it. Codes expire
after 24 hours and are redeemed through the graphical client. Deliver them by
an already authorized private channel. Recovery is operator-only after verifying
account ownership out of band. Keep secrets out of command lines, logs, source
control, screenshots, public packages and gateway URLs. Service credentials
expire according to Store policy; logout/revocation and replacement are handled
by the authenticated service. Local-review identities never authenticate service
accounts.

## Backup, restore and rollback

Stop and wait for service exit, then:

```powershell
./verdigris_server.exe --backup 'C:/private/backups/new-backup' --data 'C:/VerdigrisService/data'
./verdigris_server.exe --restore 'C:/private/backups/new-backup' --data 'C:/VerdigrisService/restored-new'
```

Destinations must not exist. Backup uses SQLite's backup operation and completion
marker; restore rejects incomplete or unsupported data. Retain backup folders
under a private parent ACL and treat credential hashes and saves as private.
Validate readiness and disposable-account persistence from the restored
directory before changing the configured data path. Preserve the prior package
and data; never copy a database over a running service. Roll back by stopping,
restoring into a fresh directory if needed, and selecting the previous compatible
package/data pair. A newer database schema may reject an older executable.

## Secure endpoint

The backend intentionally listens on loopback. Use a maintained TLS gateway
such as Caddy on the same authorized host. Adapt `Caddyfile.example` to the
authorized domain and backend port; the native endpoint is then
`wss://game.example.com/game`. [Caddy reverse_proxy](https://caddyserver.com/docs/caddyfile/directives/reverse_proxy)
supports the WebSocket upgrade and tunnel; its [HTTPS setup](https://caddyserver.com/docs/quick-starts/https)
requires a usable domain/certificate arrangement. Keep Caddy's administration
endpoint local. Never expose the plaintext backend publicly.

Validate the gateway configuration and certificate chain before external QA.
No host, domain, certificate, firewall changes or public deployment are performed
by packaging. Actual host access and explicit deployment authorization remain
external requirements. This example has not itself been deployed or externally
verified. Loopback tests do not establish separate-computer or Internet play.

## Native player package

Use the ordinary native package, then launch:

```powershell
./Verdigris.exe --online 'wss://game.example.com/game'
# Explicit local transport QA only:
./Verdigris.exe --online 'ws://127.0.0.1:6540/game' --profile 'C:/QA/online-player-a'
```

Online mode requires only the client executable, starts no server and never
owns the shared service process. Profiles default to
`%LOCALAPPDATA%/Verdigris/online/<endpoint-hash>`. Settings and the DPAPI-protected
credential cache use that profile. An explicit profile must be separate from
local-review saves. Normal double-click launch still uses the existing local
review flow. Do not import editable local saves into authoritative accounts.

## Package provenance

`package-service.ps1` requires a clean commit, performs a fresh native build,
checks embedded `--build-info`, copies only service resources into a fresh
directory and writes `service-manifest.json` with SHA-256 hashes. Run
`verify-service-package.ps1 -PackageDirectory <package>` to verify exact files
and source identity. Full gameplay, graphics, capacity, backup recovery and
separate-computer acceptance are additional gates, not claims made by hashes.

## Service contract and recovery evidence

Protocol version 1 is the private QA wire/content compatibility contract. A
version mismatch is rejected before enrollment is consumed. Production snapshots
and scene-scoped peer messages expose approved fields; development mutation and
editable-save surfaces are unavailable. Connection epochs, command sequences,
and native-client actor/scene fences reject replay and retired-context input.
On reconnect the owner receives a full snapshot instead of replaying uncertain
transactions. A repeated mutation sequence cannot grant a second result.

One authority clock schedules gameplay every 50 ms. The local review path keeps
its existing immediate input and 150 ms transport tick. Runtime bounds are 32
connections, 128 resident accounts, 512 queued messages (32 per connection),
16 KiB inbound frames, 160 messages/second per connection, and 2 MiB each for
staged and outgoing data per connection. Completed readers are reaped; committed
disconnected accounts retire after the QA grace. An admission during a saturated
queue can be rejected; clients reconnect after backoff. Initial integration evidence
covers four players in two instances and adversarial connection pressure, not a
public load target. See the implementation report for measured soak results.

SQLite schema 3 commits private accounts, the circulation journal, and outcome
markers in one transaction before gameplay output is published. Each eligible
House records its own first-Warden entitlement with the shared instance/boss and
own Scion IDs. Redemption is once per House, including an earned claim recovered
at Aldwyn after restart. Relics retain exact UUID and source ownership through
queued, released, and claimed journal states. Retired expeditions requeue only
unclaimed released relics; a claimed relic requeues only when its later owning
mortal Scion dies. Persisted source-death history rejects earlier death replays.
Ordinary ground
items in unfinished expeditions retire with that volatile instance under this QA
policy; already committed carried items and earned House claims persist.

Run `native/build.ps1 -RunTests` for actor, authority, entitlement, protocol,
relic, Store crash/recovery, transport, and existing native suites. Run
`test-service-clients.ps1` against two disposable accounts for the graphical
journey; `test-service-operations.ps1`, `test-service-launcher.ps1`, and
`test-service-pressure.ps1` cover their separate real-process boundaries.
