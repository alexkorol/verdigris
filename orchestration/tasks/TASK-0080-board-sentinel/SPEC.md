---
task: TASK-0080
title: Effective-board sentinel and fleet sweep report
state: READY
packet: MECHANICAL
topology: INDEPENDENT
priority: critical (D-125 queue runway enforcement)
lane: luna-mac; Qwen drafting allowed with machine verification
base_commit: 42718fbc4340589e606fff94a6eaa3dfbd03ad1c
owned_paths:
  - orchestration/tools/board-sentinel.mjs
  - orchestration/tools/board-sentinel.test.mjs
  - orchestration/tasks/TASK-0080-board-sentinel/**
forbidden_paths:
  - orchestration/RUN_STATUS.md
  - orchestration/DECISIONS.md
  - orchestration/tasks/*/SPEC.md except this task
  - native/**
  - server/**
  - src/**
  - playtest/**
---

# Outcome

Create a read-only command that turns the repository's coordination files into
one deterministic sweep report. It prevents a short or imaginary queue from
looking healthy merely because old immutable SPEC headers still say READY.

CLI contract:

```text
node orchestration/tools/board-sentinel.mjs --repo . --min-ready 8 --json
```

The JSON must contain:

1. effective READY tasks listed in the current `RUN_STATUS.md` READY table;
2. CLAIMED, REVIEW_REQUESTED, REVISE, HOLD, and DRAFT tasks;
3. stale claims whose implementation has been integrated or superseded;
4. pairwise owned-path collisions among READY/CLAIMED work;
5. coordinator remote-branch last-commit timestamps when those refs exist;
6. queue-floor result and process exit status (`0` healthy, non-zero for a
   floor violation, unresolved collision, malformed task state, or duplicate
   task id).

Historical SPEC frontmatter alone is never sufficient to count a task READY.
The command is read-only: no commits, fetches, file edits, process kills, or
network calls.

# Tests and fixtures

`board-sentinel.test.mjs` builds temporary synthetic boards and proves at
minimum: integrated historical READY is excluded; a live READY row counts;
CLAIMED removes a task from READY; REVIEW_REQUESTED is surfaced; overlapping
owned paths fail; HOLD is excluded; stale claim is named; duplicate task id and
malformed state fail; healthy eight-task runway exits zero.

# Acceptance commands

Run from repository root and paste literal commands, output, and exit codes in
`REPORT.md`:

```bash
node --check orchestration/tools/board-sentinel.mjs
node --check orchestration/tools/board-sentinel.test.mjs
node --test orchestration/tools/board-sentinel.test.mjs
node orchestration/tools/board-sentinel.mjs --repo . --min-ready 8 --json
git diff --check
```

# Stop conditions

STOP rather than editing coordination truth to make the command pass. Report
any ambiguity in the current board grammar or any need to write outside the
owned paths.
