---
question: QUESTION-0003
related_task: release-tooling
state: OPEN
---

# Standalone browser smoke command does not boot its server

## Decision needed

Should `npm run smoke:browser` be repaired so it starts the production/dev
server before invoking Playwright, or should the repository document
`npm run test:e2e` as the sole browser smoke gate and retire the standalone
script?

## Evidence

- `npm run smoke:browser` completed its Vite build, then failed at
  `page.goto('/')` with `net::ERR_CONNECTION_REFUSED` for
  `http://127.0.0.1:6500/`.
- `package.json` defines `smoke:browser` as `npm run build && playwright test`,
  with no server bootstrap.
- `npm run test:e2e` uses `start-server-and-test start:e2e http://127.0.0.1:6500
  "playwright test"` and passed all 3 browser tests.

## Options

1. Repair `smoke:browser` to use the same server lifecycle as `test:e2e`
   (recommended), preserving the documented command name.
2. Retire or redirect the standalone script and document `npm run test:e2e` as
   the required browser gate; this is smaller but leaves a misleading script
   name in package metadata.

## Impact

This does not block the accepted native reconstitution tasks. It does make a
documented browser release command fail before any page is loaded, so it
should be resolved before claiming the all-gates `verify` command is healthy.
