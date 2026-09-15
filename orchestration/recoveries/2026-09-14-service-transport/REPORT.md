# Native service transport evidence

## Identity and result

- Coordinator: codex; worker: transport.
- Base: `bafd218855608914f4f531a101f8e01cab524408`.
- Implementation: `58f6adbcc769160ef653724add1e7d6bdf91217a`.
- Branch: `codex/native-service-transport-20260914`.
- Worktree: `C:/Users/Alex/Documents/ChatGPT/verdigris-service-transport-20260914`.
- **Implemented and locally tested. Pushed and remote-verified.** Normal push to origin succeeded; `git ls-remote origin refs/heads/codex/native-service-transport-20260914` matched the implementation SHA before this evidence-only follow-up.

The new `ServiceTransport` implements Windows WebSocket connections using the
maintained WinHTTP API. No existing transport, game, launcher, build, or shared
integration files were changed by this worker. Parent coordinator owns integration.

## API and behavior

`native/client/service_transport.hpp` exposes namespace `verdigris::client`, class
`ServiceTransport`, with constructor/destructor and these methods:

```cpp
bool connect(const std::string& endpoint, std::string* error = nullptr);
bool send(const std::string& text, std::string* error = nullptr);
bool receive(std::string& text, std::string* error = nullptr);
void close();
```

Call networking methods on worker threads. One send and one receive may run
concurrently. Overlapping calls on the same side fail explicitly. `close()`
cancels an active handshake, send, or receive; join callers before destroying
the object. Cancellation closes the transport directly, without waiting for a
peer close acknowledgement. Reconnect creates independent state, and callbacks
retain their own buffers until Windows reports final handle closure.

Bounds: 2048 endpoint characters; 1 MiB incoming/outgoing text messages; 16 KiB
read chunk; 10 seconds for the complete handshake; 5 seconds per send; 30 seconds
for the complete receive including fragments. A timeout closes its connection
and returns false with an error. There is no application message queue.

Endpoints use `wss://hostname[:port]/path` with TLS 1.2 and normal Windows
certificate/hostname validation. There is no certificate bypass. Explicit
`ws://` is restricted to `127.0.0.1`, `localhost` (mapped to literal loopback),
and `::1`. Ambiguous loopback aliases are rejected. Credentials, queries,
fragments, non-ASCII URL characters, malformed ports, and backslashes are
rejected. ASCII punycode hostnames and percent-encoded paths can be supplied.
Redirects, implicit Windows authentication, and cookies are disabled. The
initial implementation connects directly and does not discover system proxies.
The non-Windows branch returns an explicit unsupported-platform error.

## Verification

Run from the owned worktree after initializing the installed MSVC x64 developer
environment (`VC/Auxiliary/Build/vcvars64.bat`). Direct compile command:

```text
cl /nologo /std:c++20 /EHsc /W4 /Zi /Fdnative\build\ native\client\service_transport.cpp native\tests\service_transport_tests.cpp /Fonative\build\ /Fenative\build\service_transport_tests.exe
native\build\service_transport_tests.exe
python native/tools/check_legacy_denylist.py
git diff --check
```

Results: compiler exit 0 with no warnings; **148 service transport checks
passed**, test exit 0; denylist PASS, exit 0; whitespace check exit 0.
The local generated convenience wrapper is
`native/build/build-service-transport.cmd` (ignored build output).

The standalone C++ test runs disposable Winsock fixtures on dynamic loopback
ports and verifies:

- Disconnected operations, invalid/insecure endpoints, port overflow, outbound cap.
- `localhost` connection, preservation of `/game/socket`, concurrent send/receive echo.
- Fragment assembly, exact 1 MiB inbound success, limit-plus-one rejection with no partial output.
- Binary-message rejection, server close, HTTP 302 refusal.
- Twenty pending-handshake cancellations and twenty pending-receive cancellations;
  callers finish within one second of close.
- Silent-handshake failure within the 10-second total bound.
- Actual idle receive timeout at 30 seconds (asserted between 29 and 32 seconds).
- Nonreading peer with a small receive window forces the 5-second send timeout;
  failure observed before seven seconds.

Separate certificate check: an isolated Node HTTPS server used an ephemeral
OpenSSL self-signed localhost certificate in `native/build/`. No Windows trust
store was changed. The client test invocation was:

```text
native\build\service_transport_tests.exe --reject-tls wss://localhost:<ephemeral-port>/game/socket
```

It printed `TLS validation rejected untrusted server certificate`, exit 0.
The generated fixture driver is `native/build/verify-transport-tls.cjs`; it
starts the HTTPS fixture, runs the above test and shuts down its own server.
The fixture cert can be reproduced with `openssl req -x509 -newkey rsa:2048
-nodes -keyout <key.pem> -out <cert.pem> -days 1 -subj /CN=localhost -addext
subjectAltName=DNS:localhost`. These QA key/cert files are untracked build
outputs and are not client/service package resources.

## Design references

Microsoft documents separate send/receive concurrency and cancellation of
pending asynchronous requests by handle closure. Short API initiation calls
are serialized against closure; callbacks own buffers through final closure.
See [WinHTTP concurrency](https://learn.microsoft.com/en-us/windows/win32/winhttp/concurrency-in-winhttp).

The upgrade is completed only after HTTP 101, following
[WinHttpWebSocketCompleteUpgrade](https://learn.microsoft.com/en-us/windows/win32/api/winhttp/nf-winhttp-winhttpwebsocketcompleteupgrade).
Callback state is released on the final `HANDLE_CLOSING` notification described
in [WINHTTP_STATUS_CALLBACK](https://learn.microsoft.com/en-us/windows/win32/api/winhttp/nc-winhttp-winhttp_status_callback).

## Integration requirements and limits

- Add `native/client/service_transport.cpp` to client sources. MSVC automatically
  links `winhttp.lib` via pragma; other Windows toolchains must link WinHTTP.
- Register `native/tests/service_transport_tests.cpp` plus transport source as
  a standalone test executable. MSVC test pragmas link `ws2_32`, `advapi32`,
  and `crypt32`; other Windows toolchains need equivalent linkage.
- Parent must connect this transport to `RemoteSession`, graphical account
  flow, and build/package gates. This worker did not modify those owned paths.
- Full native gameplay/visual/package suites were not run for these isolated
  new files. No game integration, package, installation, visual acceptance,
  authorized public deployment, or separate-computer result is claimed.
- Positive trusted-certificate WSS exchange, explicit hostname-mismatch with
  an otherwise trusted certificate, IPv6 runtime connection, other Windows
  versions, system-proxy support, and an actual non-Windows build remain
  unverified. The verified local WSS rejection does not prove deployment.
- Owner installation, saves, settings, and running processes were not modified.
  No byte-identical preservation claim is made without corresponding hashes.

No unresolved gameplay policy was introduced by this transport-only subtask.

## Follow-up: service operations and launcher

Implementation commit: `5866a9c556a4fb4632d196e1264bf83a65ec163c`, normal-pushed
to the same origin branch; `git ls-remote` matched that exact commit. Worktree
was clean after the push. This subsequent edit only records that evidence.

Parent coordinator explicitly assigned additional disjoint paths after the
transport milestone: `native/src/server_main.cpp`, `native/tools/player-launcher.cs`,
additive `native/tools/package-native.ps1` edits, and new service operation/package
scripts and docs. No shared build, Store, router, client rendering or simulation
sources were edited in this worktree.

Server CLI now supports explicit `--service --data <dir> --port <port>
--qa-policy coop-v1`, private `--enroll` and `--recover <account-id>` output,
consistent `--backup` and fresh-directory `--restore`, `--health`, `--stop`, and
embedded `--build-info`. Service lifetime ignores stdin; readiness follows Store
open/listener success and is cleared on shutdown/failure. Windows control events
and credential output receive current-user/SYSTEM ACLs. The process polls router
`healthy()` and stops on failure. CTRL/termination handlers request shutdown;
console-close waits briefly for cleanup. Named-event controls require the same
operator and Windows session. Local review retains positional port/stdin quit.
Unicode Windows command-line paths are decoded from `wmain` to UTF-8 Store paths.

Launcher online mode requires only the graphical client, passes `--online
<endpoint>`, and sets `VERDIGRIS_SERVICE_PROFILE` plus isolated settings under a
per-endpoint profile. It removes local save-directory inheritance. Existing
local-review saves are refused as online profiles. Online startup owns no server;
the existing local private-server path and profile lock remain. The standard
native package includes online instructions; the service packager emits only
server executable, notices, operation scripts and gateway example, with hashes
and embedded source identity checks. No service/client package was created by
this worker: the integrated fresh build belongs to the parent gate.

### Follow-up test evidence

`native/build/build-service-operations.cmd` directly compiled this server entry
and the parent's Store/core sources, then linked against the parent's already
compiled `coop_networking.obj` and seasonal object. Parent checkout:
`C:/Users/Alex/Documents/ChatGPT/verdigris-service-coop-20260914`. MSVC C++20 `/W4`
compile and link exited 0 with no warnings using the normal CRT define. This
cross-worktree executable is a disposable integration fixture, not a clean
source-labelled deliverable. No parent sources or objects were modified.

```powershell
powershell.exe -NoProfile -ExecutionPolicy Bypass -File native/tools/test-service-operations.ps1 -ServerExecutable native/build/service_operations_server.exe
powershell.exe -NoProfile -ExecutionPolicy Bypass -File native/tools/test-service-launcher.ps1
python native/tools/check_legacy_denylist.py
```

All exited 0. Operations evidence:
`native/build/service-operations-95bd359f82dd46d3b6c1aee7424e3999/`.
The real service process proved private output ACL/no credential stdout, existing
output refusal, Store exclusion while running, readiness, stdin quit and EOF
independence, clean named-event shutdown, completed backup, fresh restore,
existing-destination refusal and restored Store enrollment. It did not prove
gameplay outcome recovery; that is covered by the Store/router integration lane.

Launcher evidence:
`native/build/online-launcher-52a85ec096a34764bc919b1034e93e05/`.
The production launcher was freshly compiled and invoked against labelled probe
executables. Online mode passed with no server executable present and correct
arguments/environment/profile/settings. A second real-process run proved the
local-review server readiness, remote client arguments and graceful stdin stop
contract. Endpoint rejection, endpoint-specific default profiles and locking
also passed. Probe executables are not graphical game acceptance.

PowerShell parser checks passed for all added/changed scripts. Exact package
creation/verification, configured gateway validation, Ctrl-close behavior,
separate-computer play and positive remote TLS remain parent integration or
external gates. The Caddy example follows its maintained
[WebSocket proxy documentation](https://caddyserver.com/docs/caddyfile/directives/reverse_proxy)
and [HTTPS setup](https://caddyserver.com/docs/quick-starts/https), but no Caddy
instance was deployed. No public network, DNS, firewall, installation or owner
profile changes occurred.

### Required parent build integration

Compile/link `service_store.cpp` and its Windows libraries into server/client
targets already sharing networking. Server compile must include the generated
build-header directory; `server_main.cpp` includes existing `build_identity.hpp`.
The entry point now uses `wmain` on Windows (console target; normal MSVC CRT
selection). It requires the agreed Store constructor and `WebSocketServer::healthy()`.
The parent's current build was inspected and already includes the generated
header path and Store linkage. Register the two added PowerShell real-process
tests in the integrated gates and run `package-service.ps1` from a clean
integrated commit, followed by exact package verification and gameplay tests.
