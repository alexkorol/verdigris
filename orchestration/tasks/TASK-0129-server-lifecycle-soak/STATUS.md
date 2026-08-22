# TASK-0129 claim

- task: TASK-0129
- state: CLAIMED
- coordinator: ox-pc-d
- worker: ox-pc-d (isolated PC Ox Alpha implementation worker)
- worker branch: `codex/TASK-0129-server-lifecycle-soak-ox-pc-d`
- worktree path: `Z:\Code\.worktrees\verdigris\ox-pc-d`
- route/base SHA: `0d40d79db80c53280bb7cfe6f42318b39dab6f4c` (routed HEAD; immutable SPEC base `88d9210bf2b27ab3a776974be23f54c6174c3fff` verified ancestor of HEAD)
- started-at: 2026-08-21 22:31 -07:00 session wall-clock; exact claim commit clock is this commit's author/committer time
- ports: 6680-6699 reserved for this lane (loopback only; port 6500 untouched)
- machine: DESKTOP-TVU7OR7 (user `Alex`, Windows)
- task family: BOUNDED-DESIGN / INDEPENDENT packet

## Experimental-unit configuration provenance

- endpoint: local OpenCode CLI session in `Z:\Code\.worktrees\verdigris\ox-pc-d`
- provider: `openrouter` (harness-visible model id `openrouter/stealth/ox-alpha`)
- model: `stealth/ox-alpha`
- variant/reasoning: not observed in this session; omitted rather than guessed
- harness: OpenCode CLI 1.18.21
- agent persona: ox-alpha

## Routing provenance

- `START_HERE_OX_PC_D.md` launch packet at this worktree routes ox-pc-d to
  TASK-0129 from routed HEAD `0d40d79d` (immutable base `88d9210b`) per the
  RUN_STATUS.md effective READY table row "TASK-0129 server lifecycle soak".
- Preflight verified: clean state, branch
  `codex/TASK-0129-server-lifecycle-soak-ox-pc-d`, HEAD `0d40d79d`, upstream in
  sync (0/0), SPEC base an ancestor.
- Fresh fetch performed immediately before claim: no competing STATUS.md, no
  RELEASE.md in the task folder, and origin's
  `codex/TASK-0129-server-lifecycle-soak-ox-pc-d` sits at the same route HEAD
  with no claim commit.
