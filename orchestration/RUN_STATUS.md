# Run status (snapshot — rewritten each architect sweep)

> **ORCHESTRATOR HANDOFF — 2026-08-20.** Fable (Claude Code) is retired
> from the fleet; the Claude budget is exhausted for this billing week.
> **Codex Sol holds the orchestrator role.** Entry point:
> `orchestration/ONBOARDING-SOL-ORCHESTRATOR.md`. Read `INCIDENTS.md`
> INC-012 before the first sweep — it is the failure that ended the
> previous tenure, and its rules are binding on the role, not the model.
>
> Fleet is COLD: all three implementation lanes dark since ~08:00
> (cursor 07:41, luna-mac 08:02, deepseek ~08:03). Confirm liveness
> before planning. Board holds 4 READY (0073, 0077, 0078, 0079).
> master green through PR #48; D-122 axis 1 complete at 32/32.


## 🌆 EVENING SUMMARY — day run 2026-08-20 (D-124), ~22:00 snapshot

**Headline: SERVER/RULES PARITY COMPLETE — 32/32.** The unchanged
32-scenario playtest harness passes against the C++ server, verified
twice consecutively, each on a fresh server. Shipped as PR #45
(merged; post-merge CI running at snapshot time). All native gates
(build + unit + session + client scenarios) green.

**D-122 three-axis status:**
1. **Server/rules: DONE (32/32 ×2).** Baseline this morning was 18/19
   N5 + 23/32 N6; the afternoon waves closed economy, quest,
   crossroads, party, party-stories, build-divergence, gates,
   world-web, gear-outcomes; the evening debugging closed the last
   session-semantics cluster.
2. **Native journey: GREEN** — the C++ client's networked journey +
   render/scenario gates all pass against the C++ server.
3. **Presentation: near-family.** Fresh round-3 side-by-sides
   (`orchestration/benchmarks/side-by-side-2026-08-20/sxs3-*.jpg`,
   regenerated with today's binary): native has terrain tiles, trees,
   buildings, health rings, HP/mana orbs, quickbar, minimap. Visible
   remaining deltas: village density, expedition/quest panels,
   typography (delta list in BENCHMARK.md; 0076 successor work).

**The bug that mattered:** native target selection never advanced past
the first spawned monster (`!chosen &&` in the nearest scan) — every
swing everywhere locked onto `monster-N-0`. Masked for weeks because
swings had no range limit; the JS-parity range gate exposed it as
zero-hit combat. Found via live-server instrumentation, fixed with a
true nearest scan + direction-aware tie-breaking.

**Session semantics now match JS exactly** (proven by probing the live
JS server, not by reading code): one shared anonymous guest account;
concurrent login replaces the old session; adoption rebuilds a fresh
Player while loot/levels/bank/tree/quest-record persist; permadeath
survives reconnect; the commission chain resets on scion admission;
tree quest points (live budget) split from chain quest points
(persistent record).

**Ops lesson written in blood:** `pkill` in Git Bash does NOT kill
native Windows exes — two stale servers produced a bogus 26/32 that
cost a diagnosis cycle. Fresh-server law now enforced with
PowerShell `Stop-Process` only.

**Reviews this sweep:** luna-mac TASK-0074 ACCEPTED + integrated (gear-outcomes timing profile: variance is intrinsic to the scenario; 10x 32/32 browser runs). luna reported an empty MECHANICAL board at 08:02 and idled - restock note left in NOTES-luna-mac.md.

**Fleet:** architect solo-drove the entire parity push per D-124
(cursor/deepseek/luna quiet all afternoon — no lane commits since
PR #41/#42). Board restocked below.

---

## ☀ MORNING SUMMARY — overnight run 2026-08-19 23:45 → 08-20 07:15

**Headline: GATE A GREEN.** The C++ client plays the C++ game over a
real WebSocket with the real presentation — first time ever. One
command to try it: `powershell -File native/tools/play-native.ps1`.

**Shipped: 10 PRs (#27–#36), 14 tasks integrated, zero false greens.**

- D-122 adopted (your ChatGPT doc — its build-graph diagnosis was
  verified true) + D-123 (READY queue never dry, min 3).
- C3 track built end-to-end IN ONE NIGHT: 0060 session seam
  (architect) → 0061 networked journey → 0063 server envelopes → 0064
  presentation unify (Gate A 10/12) → 0068 polish (12/12) → 0069
  reconnect → 0067 CI journey gate (green + verified red canary) →
  0070 reference scenes → 0072 owner launcher.
- Browser: 0055 identity chip + server zone preview, 0059 compact
  overlays, 0062 flake diagnostics (marginal scenario named:
  gear-outcomes), 0066 shared capture harness.
- N7 groundwork: 0065 density bench — ~9000x tick headroom at N=1000.

**Review verdicts:** 13 ACCEPTED (12 first-pass); 1 accepted-with-gate-
held (0061 — debug painter; fixed by 0064 same night). Every gate
rerun personally; two architect play passes with driven captures.

**Fleet calibration:** cursor (Grok 4.6) = 12 accepted tasks tonight,
0 false greens — new top implementer; give it explicit presentation-
quality constraints in specs (its one miss). mac-claude (Sonnet) =
calibrated high on 0066 + unsolicited peer-verify; docs/browser lane.

**Fleet health:** deepseek STALLED all night mid-0056 (N5) — dsh
session dead since 08-18 19:36; 4 dirty files preserved in its clone.
kimis quota-dead until ~08-23. codex out of tokens.

**Blockers needing YOU:**
1. Relaunch deepseek (`C:\Users\Alex\tools\dsh.cmd web` + /goal from
   BOOTSTRAP.md) — or say the word and I re-route 0056/N5 to cursor.
   N5 is the last blocker before N6 releases (Gate A condition met).
2. Play Gate A (script below) — feedback on movement/combat/loot feel.
3. Review + freeze the 0070 reference scenes (10 captures) as the
   visual baseline.

**Open questions parked for you:** renderer backend shortlist will
arrive via 0073 (research-only); trophy circulation deferred to N5;
JS-side density-bench seam needs your OK (server change).

**Board now:** READY = 0071 matrix audit, 0073 renderer eval, 0074
gear-outcomes profile. HOLD = 0058 (N6, needs N5). RUNNING = 0056
(stalled, see blocker 1).

---

- **GATE A: GREEN (2026-08-20 ~04:40).** The owner can play the native
  game against the native server with the real presentation. Rubric
  12/12 after 0068 polish (architect play passes in 0064/0068 reviews).
- Native journey parity (D-122 axis 2) has its first green gate; axis 1
  needs N5+N6; axis 3 continues via 0068 polish + renderer track.
- Tonight: PRs #27–#35 shipped; 0070+0072 batch pending. Owner launcher: `powershell -File native/tools/play-native.ps1` (one command, from clean clone). Reference scenes frozen pending owner review.
- D-123 buffer: 3 READY after this restock.
- Immutable objective: D-116 under D-122 three-axis parity.

## OWNER PLAY SCRIPT (Gate A checkpoint, per convergence doc §owner)

```
powershell -File native/build.ps1
powershell -File orchestration/tasks/TASK-0061-networked-guest-slice/run-gate-a.ps1
```

Keys: N enter route · WASD move · mouse aim · LMB/Q/E/R fight · X take
· 1–9 equip · F or walk stairs to extract · Esc quit. Expect: connect,
route, fight (telegraphs, HP bars), named loot, equip stat change,
extract/bank, clean shutdown. Known limits: trophy circulation is N5;
telegraph FX can graze the HUD corner (0068); reconnect lands in 0069.
Feedback wanted on: movement feel, combat readability, loot moment,
whether the loop invites a second run.

## DAY RUN 2026-08-20 (D-124) — 13:30 snapshot

- N6 grind (architect, 0058 takeover): **23/32 attach** (was 18 at
  baseline). Green today: session-arc (full 30-min loop!), first-goal,
  skilltree, house-treasury, town-amenities + all N5. Implemented: town
  NPCs, first-goal quest machine, kill XP/leveling (shared/ui curve),
  tree save/budget, bank+treasury, fountain, auto-gold, shop display,
  floor-clear + instance-complete, depth-scaled monster levels.
- Remaining 9: economy, quest, crossroads, party, party-stories,
  build-divergence, gates, world-web, gear-outcomes.
- Master CI GREEN both workflows (denylist hotfix PR #43 + load-mode
  PR #44). Two documented denylist exceptions flagged for owner:
  legacyRelicId, bronze-dagger (live protocol/harness require them).
- Attribute stub note: tree node attrs approximated (+2/attr/node)
  until the 271-node engine ports — successor task queued in HANDOFF.
- Fleet: cursor + deepseek frozen since morning; luna-mac last seen on
  0071 (integrated PR #41). Architect implementing solo per D-124.

## RUNNING

| Task | Owner | Notes |
|---|---|---|
| 0056 N5 Chronicles (server axis) | deepseek | STALLED since 08-18 19:36. OWNER: relaunch dsh + /goal from BOOTSTRAP.md |

## READY (D-123 buffer: 4)

| Task | Packet | Notes |
|---|---|---|
| 0073 renderer backend eval | BOUNDED-DESIGN | research-only; any lane |
| 0077 native Chronicles client | CLIENT | Gate B slice; needs a strong lane |
| 0078 native surface density | MECHANICAL-VISUAL | presentation delta #3; NEW spec |
| 0079 browser panel inventory | MECHANICAL | delta #4 feed; luna-mac routed |

## HOLD

- 0058 N6 final SERVER-parity wave: releases when 0056 integrates
  (Gate A condition now MET).

## Integrated tonight (chronological)

- 0055 identity chip + server zone preview (cursor) — PR #30
- 0060 C3 session seam, first client↔server handshake (architect) — PR #31
- 0059 compact overlay stack (cursor) — PR #32
- 0061 networked guest journey mechanics (cursor) + 0062 flake
  diagnostics (cursor) + D-123 + specs — PR #33
- 0063 server Gate A envelopes (cursor) + 0064 presentation unify
  (cursor, GATE A GREEN) + 0065 density bench ~9000x headroom (cursor)
  + 0066 shared capture harness (mac-claude, calibrated) — batch PR pending

## Fleet + budget

- cursor: NOW composer-2.5 (Grok 4.6 spent 22% of monthly Cursor quota
  for 12 accepted tasks overnight — strong per-$ but unsustainable).
  Fresh calibration row; routed 0075 terrain. Ports 6580–6599.
- qwen3.8 (MacBook, FREE): scorecard now VERIFIED 7/7 battery, ~16
  tok/s, no reasoning leakage. Standing duty added to deepseek +
  luna-mac briefs: dispatch machine-verifiable MECHANICAL sub-steps to
  it; driver verifies everything.
- luna-mac (Codex Luna, MacBook — replaces mac-claude in this seat):
  MECHANICAL packets only, NEVER native/**; routed 0071 then 0074.
  Ports 7000–7019. Fresh calibration.
- deepseek (DeepSeek 4 Flash): ACTIVE on 0056/N5 as of 08:10 — claim
  re-confirmed, investigation phase (872 steps, 100% cache hit).
  Fresh calibration row; review with extra scrutiny. Ports 6540–6559.
- kimis quota-dead until ~08-23; codex out of tokens.

## WATCH

- gear-outcomes = the named marginal playtest scenario; diagnostics
  now print DIAG on any failure. 3rd sighting → fix task.
- kimi-work's stale 0055 clone work: discard on resume (superseded).
- Single-writer main.cpp: released by 0064; assign per-claim in 0068.
- Deferred: one visual spot-check of 0059's 1366 capture (token
  rationing) — first sweep after owner wakes.

## Review procedures on flip

- 0067: inspect workflow diff + both Actions run links (green + canary
  red) + retrigger once myself.
- 0068: gates + render-list asserts + play pass rubric rescore.
- 0069: gates + reconnect tests; verify replaced ≠ retry.
- 0056 (deepseek, on revival): rebuild + CUMULATIVE attach set MYSELF;
  D-106/ADR-002/D-109 in diff; TaskStop server after.
