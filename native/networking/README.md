# Networking seam

`verdigris_server` is a deliberately small native protocol adapter. It keeps
the deterministic `Simulation` socket-free and translates the browser wire
envelope (`{"event", "data"}`) into session commands and snapshots.

Build and run it on the default port:

```powershell
powershell -File native/build.ps1 -RunTests
native/build/verdigris_server.exe 6500
```

The port is the first argument, or can be set with `VERDIGRIS_PORT`. To run
the unchanged scenarios against an alternate port while another server owns
6500, start `verdigris_server.exe 6511` and invoke:

```powershell
$env:PLAYTEST_WS_URL = 'ws://127.0.0.1:6511'
npm run playtest -- quickstart single-session
```

N1 implements guest `player:login`, quick guest `world:zone:enter`,
development `dev:give` and `dev:state`, session replacement, heartbeats
(ping/pong), a 16 KiB frame cap, and close handling. Static HTTP files are not
served: the playtest harness is a direct WebSocket client, and the native
client remains a separate presentation target.
