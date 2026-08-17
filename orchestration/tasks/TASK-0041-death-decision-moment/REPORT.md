---
task: TASK-0041
state: REVIEW_REQUESTED
worker_commits:
  - 0e23e3bf
  - 212a1e1c
  - 14a6ceea
integration_commits:
  - 5a493083
  - dfb41955
base_commit: b5f7b163
---

## Summary

Death is now an explicit, input-capturing decision moment. The server projects
the existing D-106 carried-value transfer into a `player:death-summary` frame;
the client renders a full-screen Bronze-Age overlay with loss/protection,
oath, destination, and one continue action. Oathed deaths return to the
existing Chronicles route; unoathed deaths dismiss the overlay and leave
respawn authoritative on the server.

## Scope

Changed paths are limited to `server/player/handlers/**`,
`src/core/player/events/**`, `src/components/**`, the focused death test, and
the two task captures. No native, prototype, combat-core, package, or
Chronicles persistence code changed. The overlay uses the existing bus/socket
seams and does not create a second succession or recovery implementation.

## Evidence

- `npx vitest run tests/unit/death-decision.spec.js` — 5/5 tests, 14 assertions.
- `npm run test:unit` — clean integration worktree: 122 files, 779/779 tests.
- `npm run build` — PASS.
- ESLint on changed JS/Vue/test files — PASS.
- Stylelint on changed Vue files — PASS.
- `git diff --check` — PASS.
- Worker `npm run playtest` — 31/31 scenarios PASS.
- Worker alternate-port browser gate — 1/1 PASS on port 6512.
- Default `npm run smoke:browser` was not claimed because the preserved owner
  listener/PID 10276 already owns port 6500; the alternate-port gate is the
  clean browser evidence.

## Captures

- `captures/death-oathed-1920x1080.jpg`: real rendered 1920×1080 oathed overlay,
  43,215 bytes (lossy JPEG).
- `captures/death-unoathed-1920x1080.jpg`: real rendered 1920×1080 unoathed
  overlay, 37,777 bytes (lossy JPEG).
- `captures/death-oathed.md`: mortal Scion loses carried values to the House
  pool, states the Chronicles successor destination, and offers one
  **Return to the Chronicles** action.
- `captures/death-unoathed.md`: soft-return Scion protects carried values,
  names the return destination, and offers one **Continue** action.

## Review notes

Fable revision 697e03ff required real rendered screenshots; revision commit
14a6ceea supplies both variants and adds private-player routing so the live
overlay receives the authoritative death summary. Architect D-115 hands-on
play remains required. The default smoke port conflict is environmental and
preserved rather than resolved by killing the owner listener. No new lore or
Chronicles fork was introduced.
