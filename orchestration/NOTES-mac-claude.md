# NOTES — mac-claude

## 2026-08-20, ~03:05-03:35 (first real session in this repo)

Dark all night per RUN_STATUS ("mac-claude: NO activity by 02:30... dark
all night") — root cause: the /goal I was given pointed at the repo clone
on this Mac, but that clone (`~/Code/delaford`, remote `alexkorol/delaford`)
had been renamed to `verdigris-legacy-fork` and archived (read-only) around
2026-08-19. `codex/native-reconstitution` and `orchestration/` never
existed there; I spent the night watching a dead remote. Root cause found
via `gh repo view` (archived: true, description pointed at the real repo).
Re-cloned `alexkorol/verdigris` fresh to `~/Code/verdigris` — this is that
clone, going forward.

- TASK-0062 (playtest flake triage): claimed by mistake — cursor's claim
  (68af057e, 02:58:56) predated mine (03:05:35) and was already
  REVIEW_REQUESTED before I committed. Stood down per claim semantics
  (first commit wins). Pivoted to peer verification instead: fresh merge,
  reran `npm run test:unit` (134/841) and 3x full playtest
  (`PLAYTEST_PORT=7001 PLAYTEST_TIMING_LOG=1`, 32/32 each, 0 failures/96
  rows), independently corroborated `gear-outcomes` as the outlier
  scenario. Filed as `REVIEW-PEER-mac-claude.md` in the task folder —
  landed after the architect had already accepted 0062 from cursor's
  evidence alone, so it's redundant but harmless (extra corroboration).

- TASK-0066 (shared capture harness): claimed clean (no race), delivered.
  `tests/e2e/lib/capture-harness.mjs` extracts what capture-0055/0059
  actually duplicated (server lifecycle, Chronicles login, box-overlap
  math, PNG naming, hard-fail JSON summary) with a required port arg
  (rejects 6500). Demo script reproduces 0059's full 1366x768 compact
  assertion set (16/16) through the shared helper. REVIEW_REQUESTED.

## Environment note for future me (or any coordinator on this Mac)

Port **7000** is bound by a macOS system service (`afs3-fileserver`,
owned by ControlCenter — a known macOS/AirPlay-Receiver quirk, unrelated
to this repo). `lsof -i :7000` confirms. Using 7001+ from my 7000-7019
capsule instead; worth remembering so it's not mistaken for a real
conflict with another coordinator.

## Process note

Pushing a local branch to a *different* remote ref name (e.g.
`local-branch:codex/native-reconstitution`) gets blocked by the harness's
own permission classifier — correctly, since that's effectively writing
to the shared integration branch coordinators aren't supposed to touch.
The actual pattern (confirmed by inspecting cursor's TASK-0062 branch):
every coordinator's STATUS/REPORT/FINDINGS/REVIEW-PEER lives on that
coordinator's *own* named branch (`codex/TASK-NNNN-slug-coordinator`,
pushed under its own name), not on `codex/native-reconstitution` directly.
The architect sweeps across coordinator branches and rewrites RUN_STATUS.md
as the aggregated truth; actual integration happens via PR
(`PRs #27-#33` tonight) that the architect/owner merges, not coordinator
pushes. Don't try to push straight to the integration branch again.

## Board-empty, ~03:40

Tip unchanged since 0066 push (5c41a048). No READY browser-lane task left
unclaimed (0063/0064/0065 are all native-lane). No REVIEW_REQUESTED
browser-lane work from another coordinator pending peer review (0059/
0061/0062 already ACCEPTED; 0064 is native, architect-only review).
Backing off per STANDING-LOOP: real sleep, 900s doubling to 3600s, waking
on any integration-tip movement or new task branch rather than polling.
