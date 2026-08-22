# TASK-0081 claim

- task: TASK-0081
- state: CLAIMED
- coordinator: ox-pc-a
- worker: ox-pc-a (only registered PC Ox Alpha implementation worker)
- worker branch: `codex/TASK-0081-gate-b-wire-contract-ox-pc-a`
- worktree path: `Z:\Code\.worktrees\verdigris\ox-pc-a`
- base SHA: `986264f44b6bd3e03633d05f8b3e69fad35d4688`
- started-at: 2026-08-21 20:20 PDT (-07:00)
- ports: 6620-6639 loopback only (this audit task is expected to need no server); port 6500 untouched
- machine: DESKTOP-TVU7OR7 (user `Alex`, Windows)
- task family: MECHANICAL / INDEPENDENT audit (Gate B Chronicles wire-contract freeze)

## Experimental-unit configuration provenance

- endpoint: local OpenCode TUI session in `Z:\Code\.worktrees\verdigris\ox-pc-a`
- provider: `opencode/x-preview-f-free` (model id exactly as served by the harness)
- model alias: `ox-alpha` agent persona running on exact model id `opencode/x-preview-f-free`
- harness: OpenCode CLI/TUI; version not exposed by `opencode --version` on this PATH
- configuration provenance: owner-launched OpenCode project rooted at
  `Z:\Code\.worktrees\verdigris\ox-pc-a`; system prompt + repository
  `AGENTS.md` + ignored launch packet `START_HERE_OX_PC_A.md`; identity proved
  before first write: root/branch/base/origin all matched the required truth.

## Reconciliation at claim time

- fetched origin with prune before claiming
- program tip `origin/codex/native-reconstitution` = `600e6432b03ba5ca063ef0cbdc9ad643c4a70308`
- coordination delta vs base (`600e6432..986264f4` inverse): tip only refreshes
  expected lane base to `986264f4` in `REENTRY-OX-ALPHA-PC.md` + `RUN_STATUS.md`;
  no source/contract change; required task base preserved, no rebase needed.
- no pre-existing `STATUS.md` / `RELEASE.md` / superseding route in the task folder
- tracked worktree clean before this claim
