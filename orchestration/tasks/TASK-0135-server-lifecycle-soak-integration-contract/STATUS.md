# TASK-0135 claim

- task: TASK-0135
- state: REVIEW_REQUESTED
- coordinator: ox-pc-d
- worker: ox-pc-d (isolated PC Ox Alpha implementation worker)
- worker branch: `codex/TASK-0135-server-lifecycle-soak-integration-contract-ox-pc-d`
- worktree path: `Z:\Code\.worktrees\verdigris\ox-pc-d`
- route/base SHA: `aab574b8037bbb1d944600066387705555bc36c8` (routed HEAD and immutable task base; SPEC `base_commit eef04840465f8b3e0400be182ff29506a520eb60` verified ancestor of HEAD)
- started-at: 2026-08-21 23:12 -07:00 session wall-clock; exact claim commit clock is this commit's author/committer time
- ports: 6680-6699 reserved for this lane (loopback only; port 6500 untouched)
- machine: DESKTOP-TVU7OR7 (user `Alex`, Windows)
- task family: ARCHITECTURE / INDEPENDENT packet

## Experimental-unit configuration provenance

- endpoint: local OpenCode CLI session in `Z:\Code\.worktrees\verdigris\ox-pc-d`
- provider: `openrouter` (harness-visible model id `openrouter/stealth/ox-alpha`)
- model: `stealth/ox-alpha`
- variant/reasoning: not observed in this session; omitted rather than guessed
- harness: OpenCode CLI 1.18.21
- agent persona: ox-alpha

## Routing provenance

- `START_HERE_OX_PC_D.md` launch packet at this worktree routes ox-pc-d to
  TASK-0135 from routed HEAD `aab574b8` (immutable task base `aab574b8`;
  SPEC promoted at base `eef04840` after the TASK-0129 ACCEPTED dependency
  event) per the RUN_STATUS.md effective READY table row "TASK-0135 soak
  integration policy".
- Preflight verified: clean state, branch
  `codex/TASK-0135-server-lifecycle-soak-integration-contract-ox-pc-d`,
  HEAD `aab574b8`, upstream in sync (0/0), SPEC `base_commit` an ancestor.
- Fresh fetch performed immediately before claim: no competing STATUS.md, no
  RELEASE.md in the task folder, and origin's
  `codex/TASK-0135-server-lifecycle-soak-integration-contract-ox-pc-d` sits at
  the same route HEAD with no claim commit.

## Transition log

- CLAIMED: commit `50cd286f` (STATUS.md only), pushed to origin.
- IMPLEMENTED: commit `f58cb814` (`soak-integration-policy.json` +
  `fixtures/negative-cases.json` + `VALIDATION.md`). All five literal SPEC
  gates green with exit code 0; extra fixture-to-policy rule-resolution
  self-check green (8/8 cases). Full gate-3 rg transcript (395 lines)
  preserved at `evidence/gate3-rg-transcript.txt`. Worker writes confined to
  the task folder; no CI/native/server/src/playtest mutation; port 6500
  untouched.
- REVIEW_REQUESTED: REPORT.md written with literal gate transcripts and exit
  codes; this commit pushed to this worker branch only. No merge, no
  force-push.
