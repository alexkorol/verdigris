# TASK-0143 claim

- task: TASK-0143
- state: REVIEW_REQUESTED
- coordinator: ox-pc-i
- worker: ox-pc-i (isolated PC Ox Alpha implementation worker)
- worker branch: `codex/TASK-0143-native-gameplay-runtime-slice-ox-pc-i`
- worktree path: `Z:\Code\.worktrees\verdigris\ox-pc-i`
- route/base SHA: `aaf89d3f7fe2fc47b5481c144883c6136b4d0ebf` (HEAD; immutable
  task base `d0f74af3d30f238479218f8be412a01d61e21df3` verified ancestor of HEAD)
- started-at: 2026-08-21 23:59 -07:00 session wall-clock; exact claim commit clock is this commit's author/committer time
- ports: 6780-6799 reserved for this lane (native slice is headless/offline; port 6500 untouched)
- machine: DESKTOP-TVU7OR7 (user `Alex`, Windows)
- task family: IMPLEMENTATION / INDEPENDENT packet

## Experimental-unit configuration provenance

- endpoint: local OpenCode CLI session in `Z:\Code\.worktrees\verdigris\ox-pc-i`
- provider: `openrouter` (harness-visible model id `openrouter/stealth/ox-alpha`)
- model: `stealth/ox-alpha`
- variant/reasoning: not observed in this session; omitted rather than guessed
- harness: OpenCode CLI 1.18.21
- agent persona: ox-alpha

## Routing provenance

- `START_HERE_OX_PC_I.md` launch packet at this worktree routes ox-pc-i to
  TASK-0143 from routed HEAD `aaf89d3f` with immutable base `d0f74af3`
  (`merge-base --is-ancestor d0f74af3 HEAD` → exit 0).
- `orchestration/RUN_STATUS.md` fleet table routes ox-pc-i (ports 6780-6799)
  to "TASK-0143 native gameplay/runtime slice" after TASK-0134 was accepted
  and integrated at `b7464d94`.
- Preflight per AGENTS.md run at claim time: clean tree, origin remote
  `https://github.com/alexkorol/verdigris`, fetch --prune done, branch in
  sync (0/0) with `origin/codex/TASK-0143-native-gameplay-runtime-slice-ox-pc-i`.
- Fresh fetch performed immediately before claim: no competing STATUS.md and
  no RELEASE.md in the task folder. First-STATUS-write-wins is exercised by
  this file.

## Scope discipline

- owned_paths only: `native/src/**`, `native/include/**`, `native/tests/**`,
  and `orchestration/tasks/TASK-0143-native-gameplay-runtime-slice/**`
- forbidden paths honored: no `native/client/**`, no `native/CMakeLists.txt`
  or `native/build.ps1`, no `server/**`, `src/**`, `playtest/**`, `.github/**`;
  no CI, merges, force-pushes, or port 6500.

## Transition log

- CLAIMED: commit `5147cbed` (STATUS.md only), pushed to origin worker branch.
- IMPLEMENTED (2026-08-22 00:10 -07:00): commit `ef35c911`. Authoritative
  expedition objective
  state added to the core — `ExpeditionPhase` (`SlayWardens` /
  `ExtractCarriedValue`) on `InstanceState`, transitioned by the simulation
  exactly when the last living warden dies, with one appended
  `EventType::ExpeditionPhaseChanged` telemetry event and a focused
  regression test pinning the transition, replay determinism, fresh-entry
  reset, and non-gating behavior. Mechanics, balance, recovery semantics,
  save format, and client files untouched. Native core gate green:
  denylist PASS, core PASS, networking PASS, camera2d PASS, session tests
  passed; exit 0. See REPORT.md for literal transcripts.
- REVIEW_REQUESTED: this commit; implementation carried by the IMPLEMENTED
  commit. Awaiting architect review per PROTOCOL.
