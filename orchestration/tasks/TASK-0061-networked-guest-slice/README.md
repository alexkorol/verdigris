# TASK-0061 Gate A launcher (cursor capsule)

Starts `verdigris_server` on a loopback port in 6580-6599, then the native
client in `--remote` mode. The client talks only through IClientSession;
there is no in-process simulation in this path.

Usage (from repo root, after `powershell -File native/build.ps1`):

```
powershell -File orchestration/tasks/TASK-0061-networked-guest-slice/run-gate-a.ps1
```

Keys once the window is up: E enter route, WASD move, mouse aim, click/space
fight, X take underfoot, 1-9 equip backpack slot, walk the gold stairs pad
to extract, Esc quit.
