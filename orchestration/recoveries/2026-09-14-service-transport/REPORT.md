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
