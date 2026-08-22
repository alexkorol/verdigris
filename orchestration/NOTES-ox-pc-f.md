# Notes — ox-pc-f

## 2026-08-21 ~23:55 PDT — TASK-0138 duplicate-dispatch collision (session B account)

Two live dispatches of TASK-0138 ran concurrently in the single routed
worktree `Z:\Code\.worktrees\verdigris\ox-pc-f` on branch
`codex/TASK-0138-release-proof-validator-ox-pc-f`. This note is the account
of session B (this session; OpenCode CLI 1.18.21, `openrouter/stealth/
ox-alpha`, ports 6720-6739). Session A authored the claim and the pushed
implementation; its own account is in the task REPORT.md Deviations.

Timeline (PDT, 2026-08-21):

- 23:31-23:33 — session A stages and pushes claim `f9458f4e`. Session B
  starts ~23:33, runs the AGENTS preflight, observes the staged claim, then
  the claim committed/pushed under it; verifies branch/base/origin.
- 23:37-23:39 — session A writes fixtures and runs its suite 32/32.
- 23:40-23:43 — session B (unaware of A's untracked work; its earlier folder
  listing showed only SPEC/STATUS) writes its own
  `validate-release-proof.mjs`, `fixtures/false-green.json`,
  `validator.test.mjs` over A's untracked files and runs its suite (16/21).
- 23:43-23:45 — session B detects its three fix-up edits no longer match the
  on-disk file; before it can react, session A snapshots B's versions to
  `%TEMP%\opencode\TASK-0138-collision\`, restores its implementation,
  re-runs its gates green, writes REPORT/STATUS, and pushes REVIEW_REQUESTED
  head `1992609b` at 23:48:05.
- 23:48-23:55 — session B stands down per `ORCHESTRATION.md` (unexpected
  competing writes = STOP, preserve state, never defeat a peer). No session-B
  implementation text is committed anywhere; B's versions survive only in A's
  temp snapshot. B then independently re-verified A's pushed head.

Session B verification of pushed head `1992609b` (all five SPEC acceptance
commands run literally from the repo root):

1. `node --test …/validator.test.mjs` → exit 0, 32/32 pass, 0 fail.
2. TASK-0131 manifest vs `b3599c80…` `--json` → exit 1, `NOT_PROVEN`,
   0 integrity errors, 11 evidence gaps (4 platforms, rollback, 6 owner
   actions) — expected non-release-ready with gaps, as SPEC requires.
3. `fixtures/false-green.json` vs `be6d5556…` `--json` → exit 1 (nonzero),
   12 precise integrity errors including `HASH_MISMATCH` naming bound
   `eeee…` (64 hex) vs actual `1cfdf18d91ac96361a8efed51c367172b1508afd4a6ac
   3dadde4ac60da3e82fc`, plus STALE_HEAD, EXIT_STATUS_CONTRADICTION,
   SIZE_MISMATCH, MISSING_ARTIFACT, VERDICT_CONTRADICTION — SPEC satisfied.
4. `git diff --check` → exit 0.
5. `git diff --name-only be6d5556…..HEAD` → exit 0; only `orchestration/`
   files (routing docs, sibling SPECs, this task folder) — owned-path
   discipline holds.
- Positive control (not in SPEC, per ACCEPTANCE.md negative-control
  discipline): `fixtures/ready-minimal.json` vs `be6d5556…` → exit 0,
  `RELEASE_READY`, zero findings — the validator is not an always-fail stub.

Assessment: the pushed REVIEW_REQUESTED state is honest, SPEC-conformant,
and confined to owned paths; claim first-write-wins (`f9458f4e`) is intact;
no peer evidence was edited by session B. REPORT.md "Unresolved questions"
item 1 (who was the other writer) is answered by this note.

Asks for owner/architect:

1. Confirm one dispatch per lane; a duplicate dispatch guard (worktree lease
   in orchd) would prevent recurrence. Both sessions behaved per protocol
   from their own viewpoints; only the shared-worktree assumption failed.
2. A's collision snapshot lives under the user temp directory and expires
   with it; preserve if the architect wants both implementations for study.
3. No code or lifecycle action is requested; TASK-0138 remains
   REVIEW_REQUESTED at `1992609b` for architect review.
