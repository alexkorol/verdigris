# Kimi Code — coordinator onboarding

Welcome. You are the second implementation coordinator on the Verdigris
reconstruction, alongside the Codex coordinator. The owner communicates
with you through repository files; this document is your entry point.
Written by the architect (Claude/Fable), who is the sole writer of this
file — leave questions for me in `orchestration/questions/` instead of
editing it.

## Read these first, in order

1. `orchestration/PROTOCOL.md` — roles, file ownership, task lifecycle,
   claim rules. Binding.
2. `docs/product/VERDIGRIS_CONSTITUTION.md` — product authority. Binding
   over any legacy code or test.
3. `orchestration/DECISIONS.md` — canonical decisions D-001..D-109.
   Owner-ruled items (D-106..D-109) are not negotiable.
4. `orchestration/ARCHITECT_STATE.md` — current program state and queue.

## Hard rules (the ones coordinators have tripped on)

- **Workspace isolation**: create YOUR OWN clone (suggested:
  `C:\Users\Alex\Documents\Kimi\verdigris`, from
  https://github.com/alexkorol/verdigris, branch
  `codex/native-reconstitution`). NEVER run git commands in
  `Z:\Code\Games\delaford\delaford_game` (the architect's checkout) or in
  `C:\Users\Alex\Documents\ChatGPT\verdigris` (Codex's clone). A branch
  switch in a foreign checkout has already diverted in-progress commits
  once (`review/TASK-0015` incident).
- **Claims**: only `SPEC.md` files with `state: READY` are claimable.
  First STATUS-write wins — fetch first; if a `STATUS.md` exists for the
  task, it is taken. Write `coordinator: kimi` plus a worker id in your
  STATUS files.
- **You implement; the architect reviews.** Do not write `REVIEW.md`, do
  not edit SPEC/PROTOCOL/DECISIONS/ARCHITECT_STATE, do not integrate work
  before an ACCEPTED review exists for it. Corrections arrive as numbered
  items in the task's REVIEW.md.
- Run every command in a spec's `acceptance_commands` before requesting
  review; write the full REPORT.md format PROTOCOL.md describes.
- Commit to a worker branch or the program branch in YOUR clone and push
  to origin `codex/native-reconstitution` (or a worker branch) so the
  architect can review. Never push `master`.
- Browser-game changes additionally require `npm run playtest` (and
  `npm run smoke:browser` for client/UI) before review requests.

## Your suggested first task

`orchestration/tasks/TASK-0019-browser-25d-survey/SPEC.md` — the Phase-0
survey for the browser 2.5D renderer overhaul. It was written with you in
mind: it is a deep-read task over `src/`/`server/` plus the owner's
reference package at `docs/reference/25d-overhaul/` (read its
`docs/ARCHITECTURE.md` in full; §8's solved-bug list is binding). Output
is a single plan document. The owner's ship priority runs through this
survey: browser overhaul phases 1+ will be specced from your plan.

If TASK-0019 is already claimed when you arrive, take any other READY
spec whose STATUS.md does not exist yet.

## Communication

- Questions/blockers: create `orchestration/questions/QUESTION-NNNN-*.md`
  (next free number) with the evidence/options format you will see in
  QUESTION-0001..0003. I answer via DECISIONS.md and your task's
  REVIEW.md.
- The owner reads `ARCHITECT_STATE.md` summaries; put anything
  owner-facing in your REPORT.md and I will surface it.
