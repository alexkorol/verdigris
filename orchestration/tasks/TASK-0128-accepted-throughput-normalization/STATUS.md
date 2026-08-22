# TASK-0128 claim (fresh-lane golden-output revision)

- task: TASK-0128
- state: CLAIMED
- transitioned-at: 2026-08-22 09:44 PDT (-07:00)
- coordinator: ox-alpha
- worker: ox-pc-y
- provider: openrouter
- model: stealth/ox-alpha
- worker branch: `codex/TASK-0128-runway-golden-revision-ox-pc-y`
- worktree path: `Z:\Code\.worktrees\verdigris\ox-pc-y`
- preserved implementation base: `d247638e34ea9f27de98d6fc316fd3361fb75427`
- branch head at claim: `0dbfbefbc56a9809d153638d175ce23f2bc78afc` (architect RELEASE commits `9de3edda`, `0dbfbefb` on top of the preserved base)
- ports: none required (read-only Git/task evidence capsule; Node.js 22); port 6500 untouched
- machine: DESKTOP-TVU7OR7 (user `Alex`, Windows)
- task family: MECHANICAL / INDEPENDENT / IMPLEMENTATION packet
- claim basis: RELEASE.md in this task folder (2026-08-22 09:40 PDT) releasing stale
  ox-pc-a claim after pushed revision head `d247638e`; this fresh lane replaces the
  old STATUS per the release authorization and performs only the numbered
  golden-output revision

## Experimental-unit configuration provenance (normalized)

- endpoint: local OpenCode TUI session in `Z:\Code\.worktrees\verdigris\ox-pc-y`
- provider (harness-visible): `opencode`; upstream provider remains unknown
- model id (harness-visible): `stealth/ox-alpha` via OpenRouter; variant/agent alias `ox-alpha`
- harness: OpenCode CLI/TUI; version not exposed on this PATH (recorded unknown, never guessed)

## Scope of this lane (per RELEASE.md)

1. regenerate and commit every affected fixture output using the existing corrected collector;
2. prove `node --test orchestration/throughput/*.test.mjs` exits zero and leaves `git status --short` empty;
3. retain the final-head/source-revision positive control and both relevant evidence-change/tamper negative controls;
4. rerun every literal SPEC gate and request review without guessing a runway rate or changing collector semantics.
