---
task: TASK-0057
state: REVIEW_REQUESTED
coordinator: deepseek
worker_branch: codex/TASK-0057-clustered-accents-deepseek
base_commit: 79723db35c4d2873d3c5af3fda13a4503337236d
started_at: 2026-08-18T16:15:00-07:00
architect_review_required: true
expected_verification: npm run test:unit (830/830); npm run playtest (32/32); browser smoke (1/1); hard-fail capture (village + zone, CAPTURES OK)
---

Implemented and pushed for architect review.

Floor accents now generate as coherent seeded blobs (blob growth from seed
cells) instead of one-cell checkerboard noise, on their own seed-derived rng
stream so the main generation stream (decor/water/monsters/items) stays
byte-identical. Density holds at the 12% budget (±10%).

Gates green: unit 830/830 (4 new tests), playtest 32/32, browser smoke 1/1,
hard-fail capture CAPTURES OK (village + Verdant Grove). Authentic negative:
growth=0 → clustering tests fail → restored green. Scope note in REPORT.md:
overworld village flowers live in world-layout.js (outside owned_paths) and
are unchanged — flagged for architect follow-up if wanted.

Commits:
- `32fdcdf8` — claim
- implementation commit — see git log after push
