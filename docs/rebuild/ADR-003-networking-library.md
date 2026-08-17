# ADR-003 — N1 networking library

Status: draft for architect ratification

## Decision

N1 uses a small in-tree RFC6455 adapter in `native/networking/`, backed by
Winsock on Windows and POSIX sockets elsewhere. JSON parsing and the WebSocket
handshake/framing live at the transport edge. The deterministic core remains
dependency-free and has no socket, thread, or JSON dependency.

## Comparison

| Candidate | License / protocol | MSVC + CMake reality | N1 decision |
| --- | --- | --- | --- |
| IXWebSocket | BSD-style; WebSocket and HTTP | Strong API, but typically requires vendoring a larger dependency and TLS/backend choices | Good future option; not selected for the minimal N1 binary |
| uWebSockets | Apache-2.0; high-performance WS/HTTP | Fast, but depends on uSockets and a more involved build integration; API is optimized for a larger service | Deferred until throughput or HTTP routing is a requirement |
| Boost.Beast + standalone Asio | Boost license; mature WS/HTTP | Excellent protocol coverage, but the current native helper intentionally has no package-manager/Boost prerequisite; vendoring Boost is too broad for N1 | Deferred; preferred if native HTTP/TLS becomes first-class |
| In-tree adapter (selected) | Repository code; RFC6455 subset + JSON envelope | Direct MSVC/CMake compilation, no network fetch, no ABI/toolchain drift, easy alternate-port harness runs | Correct N1 scope; replace behind the same session boundary when requirements grow |

## Constraints and follow-up

The adapter supports the browser contract needed by the unchanged `quickstart`
and `single-session` scenarios: masked text frames, ping/pong, close, bounded
payloads, and `{event,data}` envelopes. It does not claim production HTTP,
TLS, compression, fragmentation, or multi-player support. Any future library
replacement must preserve `ProtocolSession` as the transport-independent
boundary and add a focused comparison of binary size, shutdown behavior,
licensing, and cross-platform CI before changing this decision.
