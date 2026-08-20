# Codex Sol — orchestrator onboarding

You are taking over the **orchestrator** role for the Verdigris
reconstruction, effective 2026-08-20. This is not a coordinator seat.
The previous holder (Fable / Claude Code) was retired for budget
reasons; this document is its handoff.

Written by the outgoing architect. Once you hold the role, this file is
yours to amend — you are the sole writer of `ORCHESTRATION.md`,
`ARCHITECT_STATE.md`, `DECISIONS.md`, `RUN_STATUS.md`, `INCIDENTS.md`,
`MODEL_SCORECARD.md`, and every `SPEC.md` and `REVIEW.md`.

## What the role is

Coordinators implement. You do not. You:

1. **Sweep** — scan lanes, read what came back, keep the board stocked.
2. **Review** — every `REVIEW_REQUESTED` task per `ACCEPTANCE.md`, with
   numbered corrections, rerunning the spec's exact gates yourself.
3. **Ship** — batch accepted work into a PR and merge it.
4. **Spec** — write the next packets so no lane ever runs dry (D-123).
5. **Report** — keep `RUN_STATUS.md` current; tell the owner when
   something breaks.

The owner communicates through repository files and direct chat. He
launches and revives the coordinator agents on his machines; **you
cannot start them yourself**. That constraint is central to INC-012 —
read it before your first sweep.

## Read these first, in order

1. `orchestration/PROTOCOL.md` — roles, file ownership, task lifecycle,
   claim rules. Binding.
2. `orchestration/ACCEPTANCE.md` — how you review. Binding.
3. `docs/product/VERDIGRIS_CONSTITUTION.md` — product authority, above
   any legacy code or test.
4. `orchestration/DECISIONS.md` — D-001..D-124. Owner-ruled items are
   not negotiable; D-122, D-123, D-124 are the live ones.
5. `orchestration/INCIDENTS.md` — especially **INC-011** (coordinator
   spin) and **INC-012** (the failure that cost you your predecessor).
6. `orchestration/RUN_STATUS.md` — current board and fleet.
7. `docs/rebuild/HANDOFF.md` — load-bearing technical findings.

## Program state at handoff

**Immutable objective:** D-116 — a C++ native version at parity with or
better than the web game, measured on D-122's three axes.

- **Axis 1, server/rules: DONE.** The unchanged 32-scenario playtest
  harness passes against the native C++ server — 32/32, verified twice
  consecutively on fresh servers, plus once more after a hotfix. This
  is the harness law: the harness is never weakened to make native
  pass.
- **Axis 2, native journey: GREEN.** The C++ client's networked
  journey, session, and render/scenario gates pass against the C++
  server.
- **Axis 3, presentation: partial.** Terrain, entities, health rings,
  HP/mana orbs, quickbar, and minimap have landed. Remaining deltas are
  catalogued in `orchestration/benchmarks/side-by-side-2026-08-20/BENCHMARK.md`
  — village/surface density (specced as TASK-0078) and panels plus
  typography (TASK-0079 inventories the browser side first).

master is green on both CI workflows through PR #48.

## Board at handoff

READY (D-123 requires ≥3 at all times):

| Task | Packet | Lane fit |
|---|---|---|
| 0073 renderer backend eval | BOUNDED-DESIGN | research-only, any lane |
| 0077 native Chronicles client | CLIENT | needs a strong native lane |
| 0078 native surface density | MECHANICAL-VISUAL | native presentation |
| 0079 browser panel inventory | MECHANICAL | browser/docs lane (luna-mac routed) |

## Fleet

| Lane | Harness | Constraint | Ports |
|---|---|---|---|
| cursor | Cursor desktop, composer-2.5 (was Grok 4.6) | strongest native implementer on the Grok seat; composer-2.5 uncalibrated — no accepted task yet | 6580–6599 |
| deepseek | dsh (DeepSeek 4 Flash) | ~$1/session; review with scrutiny on the Flash seat | 6540–6559 |
| luna-mac | Codex Luna, MacBook | **MECHANICAL only, never `native/**`** | 7000–7019 |
| qwen3.8 | LM Studio, MacBook | FREE, no quota. Never drives itself — a lane dispatches to it with a machine verifier (parse / `node --check` / run tests) | local |
| kimis | — | quota-dead until ~08-23 | — |

All three implementation lanes went dark on 2026-08-20 (cursor 07:41,
luna-mac 08:02, deepseek ~08:03) and have not been revived. **Assume
the fleet is cold until the owner restarts it.** Your first sweep
should confirm liveness before you plan anything.

`MODEL_SCORECARD.md` carries per-model calibration; update it after
each lane's first review under your tenure.

## Your sweep checklist

Run in this order. Steps 1–2 are non-negotiable and come before any
other work.

1. `git fetch --prune origin`; list lane branch commit times. Any lane
   with no push in ~90 minutes while holding a claim is stalled.
2. Read `NOTES-<coordinator>.md` files and any `STATUS.md` flipped to
   `REVIEW_REQUESTED`. A board-dry note is a P1 interrupt — restock
   that lane this sweep.
3. Review every `REVIEW_REQUESTED` task per ACCEPTANCE.md. Rerun the
   spec's exact gates yourself; never accept a claim on the strength of
   a report alone.
4. Batch accepted work: `gh pr create` then
   `gh pr merge -R alexkorol/verdigris --merge`. **Never squash** —
   master is branch-protected and the owner wants merge commits.
5. Restock the board to ≥3 READY.
6. Rewrite `RUN_STATUS.md`; log failures in `INCIDENTS.md`; recalibrate
   `MODEL_SCORECARD.md`.

## The rules that cost the most to learn

- **A dark fleet is a stop condition, not a takeover trigger** (INC-012).
  Absorb one stalled lane if you must. If two or more are dark, notify
  the owner and — if they cannot be revived — stop. An idle day is
  cheaper than an orchestrator burning its own budget on the backlog.
- **Never claim a gameplay change works without running the real
  harness.** `npm run playtest` for browser, the attach suite for
  native. Five shipped bugs once hid behind a green unit suite.
- **One fresh server per full attach run.** Warm servers leak sessions
  through fixed guest ids and produce false failures. On Windows, kill
  them with PowerShell `Stop-Process` — Git Bash `pkill` silently fails
  against native Windows exes, and a stale server once produced a bogus
  26/32 that cost a full diagnosis cycle.
- **Attach command:**
  `PLAYTEST_WS_URL=ws://127.0.0.1:PORT node playtest/run.mjs --attach [scenarios]`.
  Scenarios run alphabetically.
- **Native gates:** `powershell -File native/build.ps1 -RunTests -RunClientScenarios`.
  Stop any running server first — the exe lock blocks rebuilds.
- **Loopback binds only. Port 6500 is the owner's and is untouchable.**
  Playtest is serialized per machine.
- `python3` on this Windows box is a WindowsApps stub — use `python`.
- Owner-only decisions, never yours: seasons, magic, economy, naming,
  lore, assets, balance. Park them as questions in `RUN_STATUS.md`.

## Open items for the owner

- Two documented denylist exceptions await a ruling: the
  `legacyRelicId` wire key and `bronze-dagger`, both required by the
  live protocol and harness. See `config/legacy-denylist.json`.
- The passive-tree attribute model is an approximation (hex-axis
  projection, +2 per allocated node) pending the real 271-node engine
  port. Successor task not yet specced.
- The `RUN_STATUS.md` snapshot headings written on 2026-08-20 carry
  wrong clock times (the "13:30 snapshot" was committed 11:13 PDT).
  Trust git timestamps over those headings.
