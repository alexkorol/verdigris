---
task: TASK-0082
title: Dual-server parity matrix runner
state: READY
packet: BOUNDED-DESIGN
topology: INDEPENDENT
priority: high (D-116 regression sweep layer 1)
lane: deepseek; Windows native build required
base_commit: 1f82623d9a3936513327cc43362703443e14b02a
owned_paths:
  - playtest/tools/dual-server-matrix.mjs
  - orchestration/tasks/TASK-0082-dual-server-matrix/**
forbidden_paths:
  - playtest/harness.mjs
  - playtest/run.mjs
  - playtest/scenarios/**
  - server/**
  - native/**
  - src/**
---

# Outcome

Add a self-terminating wrapper that runs the unchanged playtest scenarios
against a fresh JS server and a fresh C++ server, serially, then writes one
comparison JSON artifact. The wrapper owns process startup/shutdown, uses two
explicit loopback ports from the coordinator capsule, creates isolated save
paths, and always kills only the child processes it spawned.

Required CLI:

```text
node playtest/tools/dual-server-matrix.mjs \
  --native-exe native/build/verdigris_server.exe \
  --js-port <PORT> --native-port <PORT> \
  --scenarios quickstart,movement,zones \
  --out orchestration/tasks/TASK-0082-dual-server-matrix/captures/smoke.json
```

With no `--scenarios`, it runs all current scenarios alphabetically. JSON
records revision, executable path/hash, URLs, exact child commands, per-server
scenario pass/fail/duration, and final parity status. Any red or asymmetric
scenario exits non-zero. It may parse ordinary runner output but may not alter
the runner or assertions.

# Acceptance commands

Use two ports in 6540-6559, never 6500. Paste literal transcripts and exit
codes in `REPORT.md`:

```powershell
powershell -File native/build.ps1 -RunTests
node --check playtest/tools/dual-server-matrix.mjs
node playtest/tools/dual-server-matrix.mjs --native-exe native/build/verdigris_server.exe --js-port 6541 --native-port 6542 --scenarios quickstart,movement,zones --out orchestration/tasks/TASK-0082-dual-server-matrix/captures/smoke.json
node -e "const r=require('./orchestration/tasks/TASK-0082-dual-server-matrix/captures/smoke.json'); if(!r.parity) process.exit(1); console.log('dual-server smoke: PASS')"
git diff --check
```

Also demonstrate one authentic negative using a disposable copy or a nonexistent
scenario argument; restore before commit. Capture the non-zero exit literally.

# Stop conditions

STOP if the wrapper requires changing harness/scenario assertions, binding a
non-loopback address, touching port 6500, or killing a process it did not
spawn.
