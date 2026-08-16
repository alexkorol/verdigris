# Verdigris orchestration protocol

Two systems work this repository concurrently:

- **Architect (Claude/Fable)** — product interpretation, architecture,
  decisions, task specs, reviews. Sole writer of `PROTOCOL.md`,
  `ARCHITECT_STATE.md`, `DECISIONS.md`, `tasks/*/SPEC.md`, `tasks/*/REVIEW.md`,
  and `docs/product/*`.
- **Codex coordinator (+ Luna workers)** — implementation, tests, commands,
  integration. Sole writer of `tasks/*/STATUS.md`, `tasks/*/REPORT.md`, and
  `INTEGRATION_LOG.md`.

Either side may create files under `questions/` (`QUESTION-NNNN.md`).
Never edit the other side's files. Product authority remains
`docs/product/VERDIGRIS_CONSTITUTION.md`; when a spec and the constitution
conflict, stop and file a question.

## Task lifecycle

```text
DRAFT → READY → CLAIMED → IMPLEMENTED → REVIEW_REQUESTED
      → (REVISE → CLAIMED …) → ACCEPTED → INTEGRATED
also: BLOCKED, SUPERSEDED
```

- Architect sets: DRAFT, READY, REVISE, ACCEPTED, SUPERSEDED (in `SPEC.md`
  frontmatter for DRAFT/READY/SUPERSEDED; via `REVIEW.md` verdicts for
  REVISE/ACCEPTED).
- Codex sets: CLAIMED, IMPLEMENTED, REVIEW_REQUESTED, INTEGRATED, BLOCKED
  (in `STATUS.md`).
- Only `READY` specs are executable. After READY, the spec is immutable;
  corrections arrive as numbered items in `REVIEW.md` or a replacement task.

## Codex obligations per task

1. Write `STATUS.md` on claim (state, worker, branch/worktree, started-at).
2. Work only inside `owned_paths`; never touch `forbidden_paths`.
3. Run every command in `acceptance_commands` before REVIEW_REQUESTED.
4. Write `REPORT.md`: executive summary, approach, changed files, public
   interfaces added/changed, test commands + outcomes, manual verification,
   commit SHAs, deviations, unresolved questions, risks, follow-ups.
5. Commit locally on the current program branch or a worker branch merged to
   it. NEVER push; the owner pushes.
6. On any stop condition in the spec: set BLOCKED, file a question, halt.

## Architect obligations per completed task

Read `REPORT.md`, inspect the named commits/diffs and load-bearing files,
run or spot-check verification, then write `REVIEW.md` with verdict
ACCEPTED / REVISE / BLOCKED / SUPERSEDED and numbered, testable corrections.

## Conventions

- Program branch: `codex/native-reconstitution`.
- Task IDs: `TASK-NNNN-short-slug`, monotonically increasing.
- At most three parallel READY implementation tasks; overlapping foundational
  files force sequential ordering.
- Browser-game changes still require the repo's `npm run playtest` gate;
  native changes require the commands in `native/README.md`.
