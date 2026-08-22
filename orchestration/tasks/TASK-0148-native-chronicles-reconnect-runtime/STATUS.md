# TASK-0148 claim

- task: TASK-0148
- state: CLAIMED
- coordinator: ox-pc-g
- worker: ox-pc-g (isolated PC Ox Alpha implementation worker)
- worker branch: `codex/TASK-0148-native-chronicles-reconnect-runtime-ox-pc-g`
- worktree path: `Z:\Code\.worktrees\verdigris\ox-pc-g`
- route/base SHA: `df851cead0dadcd96176b370ad132f8344c3c21d` (exact routed HEAD
  verified; immutable task base `060c11517d2ebb0aec0c4d4a38c5e3eb53141cb2` is
  HEAD~1, verified ancestor of HEAD)
- started-at: 2026-08-22 02:09 -07:00 session wall-clock; exact claim commit
  clock is this commit's author/committer time
- ports: 6740-6759 reserved for this lane (loopback only; port 6500 untouched)
- machine: DESKTOP-TVU7OR7 (user `Alex`, Windows)
- task family: IMPLEMENTATION / INDEPENDENT packet

## Experimental-unit configuration provenance

- endpoint: local OpenCode CLI session in `Z:\Code\.worktrees\verdigris\ox-pc-g`
- provider: `openrouter` (harness-visible model id `openrouter/stealth/ox-alpha`)
- model: `stealth/ox-alpha`
- variant/reasoning: not observed in this session; omitted rather than guessed
- harness: OpenCode CLI (headless CLI launch per RUN_STATUS fleet registration)
- agent persona: ox-alpha

## Routing provenance

- `START_HERE_OX_PC_G.md` launch packet at this worktree routes ox-pc-g to
  TASK-0148 at exact HEAD `df851cea` (verified with `git rev-parse HEAD`).
- `orchestration/tasks/TASK-0148-native-chronicles-reconnect-runtime/SPEC.md`
  is READY with owned paths `native/include/verdigris/networking.hpp`,
  `native/src/networking.cpp`, `native/src/server_main.cpp`,
  `native/tests/session_tests.cpp`, `native/tests/networking_tests.cpp`, and
  this task folder.
- Preflight per AGENTS.md run at claim time: clean tree, branch
  `codex/TASK-0148-native-chronicles-reconnect-runtime-ox-pc-g`, HEAD exactly
  `df851cea`, origin remote `https://github.com/alexkorol/verdigris`,
  fetch --prune done.
- Fresh fetch performed immediately before claim: no competing STATUS.md and
  no RELEASE.md in the task folder on any origin ref. First-STATUS-write-wins
  is exercised by this file.

## Scope discipline

- owned_paths only: the five native files above and
  `orchestration/tasks/TASK-0148-native-chronicles-reconnect-runtime/**`
- forbidden paths honored: no `native/client/**`,
  `native/include/verdigris/core.hpp`, `native/src/core.cpp`,
  `native/tests/core_tests.cpp`, `server/**`, `src/**`, `playtest/**`,
  `.github/**`, CI; no protocol invention, durable storage, merges,
  force-pushes, or port 6500.

## Transition log

- CLAIMED: commit `<claim-sha>` (STATUS.md only), pushed to origin worker
  branch. Implementation follows on this branch.
