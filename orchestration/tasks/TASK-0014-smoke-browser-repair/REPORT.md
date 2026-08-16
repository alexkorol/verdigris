# TASK-0014 report — repair the `smoke:browser` lifecycle

## Executive summary

Repaired the documented `npm run smoke:browser` gate so it boots the same
`start:e2e` server lifecycle used by the existing built end-to-end gate before
running the critical browser loop. The original build step, command name, test
selection, and port 6500 remain unchanged.

## Approach

Changed only the `smoke:browser` script in `package.json`:

```text
npm run build && start-server-and-test start:e2e http://127.0.0.1:6500 "playwright test tests/e2e/browser-critical-loop.spec.mjs"
```

No dependency, Playwright configuration, server, test, or CI changes were
needed.

## Changed files

- `package.json` — one script line, adding `start-server-and-test` around the
  existing `start:e2e` server and critical-loop Playwright command.

## Verification

- `npm run smoke:browser` — PASS; Vite build completed and the critical browser
  loop reported `1 passed (28.7s)`.
- `npm run test:e2e` — PASS; the unchanged built gate reported `3 passed
  (26.6s)`.
- Port cleanup — PASS after each command; the post-run
  `Get-NetTCPConnection -LocalPort 6500 -State Listen` check returned no
  listener (`port 6500 released`).
- `git diff --check` — PASS.

Build output retains existing non-fatal warnings about stale browser data,
legacy Sass API usage, and chunk size; none caused a gate failure.

## Commit

The implementation commit is recorded in `STATUS.md` after local commit.

## Deviations, unresolved questions, risks

None. The lifecycle now intentionally matches `test:e2e:built` and leaves no
server running after the gate.
