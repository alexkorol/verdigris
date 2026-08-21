---
task: TASK-0081
title: Gate B Chronicles wire-contract freeze
state: READY
packet: MECHANICAL
topology: INDEPENDENT
priority: critical (unblocks TASK-0077 safely)
lane: deepseek or any exact-audit lane
base_commit: 1f82623d9a3936513327cc43362703443e14b02a
owned_paths:
  - docs/rebuild/NATIVE_CLIENT_PROTOCOL_MATRIX.md
  - orchestration/tasks/TASK-0081-gate-b-wire-contract/**
forbidden_paths:
  - native/**
  - server/**
  - src/**
  - playtest/**
  - orchestration/tasks/* except this task
---

# Outcome

Freeze the already-landed native-server Gate B wire surface before the client
implements it. Update the Gate B rows in
`docs/rebuild/NATIVE_CLIENT_PROTOCOL_MATRIX.md` and add
`captures/gate-b-wire-contract.json` containing one record per journey step:
House found/restore, Scion create/select/set-out, mortal-oath state, fatal
fall, crypt/relic state, successor creation, recovery, quit, and relaunch.

Each record must name the exact client event, payload keys, server response
event, response keys, current handler symbol/file line, and current automated
test label if one exists. A missing response or test remains explicitly red;
do not invent an envelope and do not change code.

# Non-goals

No client implementation, protocol redesign, source edit, gameplay decision,
or test weakening. This is a current-tip contract inventory consumed by
TASK-0077.

# Acceptance commands

Paste literal output and exit codes in `REPORT.md`:

```bash
node -e "JSON.parse(require('fs').readFileSync('orchestration/tasks/TASK-0081-gate-b-wire-contract/captures/gate-b-wire-contract.json','utf8')); console.log('gate-b contract JSON: PASS')"
rg -n 'chronicles:(house:found|scion:create|scion:set-out|state|scion-fallen)|player:chronicles:mutate' native/src/networking.cpp native/tests/networking_tests.cpp native/tests/session_tests.cpp
rg -n 'House lifecycle|Scion lifecycle|Death|Successor|Persistence' docs/rebuild/NATIVE_CLIENT_PROTOCOL_MATRIX.md
git diff --check
git diff --name-only
```

# Stop conditions

STOP and report a red row if the response shape cannot be proven from current
source/tests. Never fill a gap by guessing or by editing source.
