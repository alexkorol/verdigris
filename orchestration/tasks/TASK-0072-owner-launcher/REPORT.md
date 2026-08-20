---
task: TASK-0072
state: REVIEW_REQUESTED
coordinator: cursor
worker_branch: codex/TASK-0072-owner-launcher-cursor
base_commit: 27d2be62038bba29abf68735288fd1d177b4c0aa
architect_review_required: true
---

# TASK-0072 REPORT — one-command owner launcher

## Executive summary

`powershell -File native/tools/play-native.ps1` builds when client/server
exes are missing or older than native sources, starts `verdigris_server`
on a free **6520–6539** port (never 6500), launches
`verdigris_client --remote` against it, tees server stdout/stderr under
`native/build/logs/`, and on client exit stops the server and prints a
no-orphan process check plus the log path. `-Local`, `-Port`, and
`-Rebuild` are supported. `native/README.md` documents the command and a
desktop-shortcut one-liner.

## Approach

- Stale = exe missing or older than the newest `.cpp/.hpp/.h/.ps1` under
  `native/src`, `native/client`, `native/include`, plus `native/build.ps1`.
  Build always delegates to `native/build.ps1`.
- Default port: first free loopback port in 6520–6539 via `TcpListener`.
  Explicit `-Port 6500` throws before bind.
- Wait up to 12s for a `listening` line in the server log, then run the
  client in-process so Esc/window-close unblocks the script. `finally`
  force-stops the server. Post-exit `Get-Process` of
  `verdigris_server`/`verdigris_client` fails the script if anything remains.

## Changed files

- `native/tools/play-native.ps1` (new)
- `native/README.md` — Owner play section only
- `orchestration/tasks/TASK-0072-owner-launcher/STATUS.md`
- `orchestration/tasks/TASK-0072-owner-launcher/REPORT.md`

## Verification

2026-08-20 (this clone, after a rebuild because sources were newer than
exes):

```text
play-native: starting server ws://127.0.0.1:6520
play-native: server log ...\native\build\logs\server-20260820-055536.log
play-native: starting client --remote 127.0.0.1 6520
```

Server log: `verdigris_server listening on ws://127.0.0.1:6520`.
Client process ran; after the client exited, the script printed:

```text
play-native: no orphan verdigris_server/client processes
play-native: server log ...\native\build\logs\server-20260820-055536.log
```

Exit 0. Architect acceptance: delete `native/build`, run the same
script, play, Esc, confirm the no-orphan line.

## Deviations

None. No `native/src` or `native/client` edits. Fresh-clone delete of
`native/build` is left for the architect rerun.
