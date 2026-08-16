---
id: TASK-0014
title: Repair the smoke:browser gate's server lifecycle
state: READY
track: tooling
priority: medium
base_commit: current program tip (Codex records the actual SHA in STATUS.md on claim)
dependencies: []
parallel_safe: true
owned_paths:
  - package.json
forbidden_paths:
  - native/**
  - prototypes/**
  - src/**
  - server/**
  - tests/**
  - playwright.config.js
acceptance_commands:
  - npm run smoke:browser
---

## Goal

`npm run smoke:browser` boots the browser server exactly like
`npm run test:e2e` does, then runs Playwright — resolving QUESTION-0003
with its recommended option 1 (decision D-105).

## Why this task exists

AGENTS.md documents `smoke:browser` as the required browser gate for
client/UI changes, but the script runs `npm run build && playwright test`
with no server bootstrap and dies at `ERR_CONNECTION_REFUSED`. A
documented gate that cannot pass is worse than no gate.

## Product and architectural invariants

- Preserve the command name (documented in AGENTS.md); port 6500 stays
  pinned (repo rule).
- Reuse the existing `start-server-and-test` pattern from `test:e2e`; add
  no new dependencies.
- The script's build step must be preserved (smoke gate covers the built
  client, per its original intent) — keep `npm run build` before the
  server+test phase unless investigation shows `start:e2e` already serves
  the built output, in which case document that in REPORT.md.

## Scope

Adjust ONLY the `smoke:browser` script line in `package.json`.

## Non-goals

No Playwright config changes, no test changes, no CI wiring, no other
script edits.

## Deliverables

One-line-scale change, one commit.

## Acceptance criteria

- `npm run smoke:browser` exits 0 with all browser tests passing and no
  connection-refused failures.
- `npm run test:e2e` still exits 0 (unchanged behavior).
- No watch/dev servers left running afterward (repo rule) — verify port
  6500 is free post-run.

## Required verification

Both commands above; paste tails into REPORT.md; show `netstat`-style
proof that 6500 is released.

## File ownership

`package.json` only.

## Dependencies

None.

## Parallel-safety assessment

Trivially disjoint from everything.

## Review focus

That the lifecycle matches `test:e2e` exactly and the port is released.

## Stop conditions

If the fix genuinely cannot live in one script line (e.g. requires config
changes), stop and update QUESTION-0003 instead of expanding scope.
