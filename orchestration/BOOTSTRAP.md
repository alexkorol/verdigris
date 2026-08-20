# Orchestration bootstrap — copy-paste kit

Everything needed to boot the machine from cold: architect first, then
coordinator CLIs, then their prompts. Owner-maintained values (quota
top-ups, credentials) stay outside this file. Updated 2026-08-18.

---

## 0. Boot order

1. Architect (frontier model: Claude Fable / GPT Sol / equivalent).
2. Coordinator CLIs (any subset — the board tolerates missing lanes).
3. Paste each coordinator its one-liner. Done — the machine self-runs.

---

## 1. Architect boot prompt (Fable, Sol, or any frontier model)

Paste into the frontier session, working dir the repo root:

```
You are the ARCHITECT for the Verdigris program (repo alexkorol/verdigris, program branch codex/native-reconstitution, working dir Z:\Code\Games\delaford\delaford_game). You are the control plane, never an implementer during waves. Read IN ORDER before acting: orchestration/ORCHESTRATION.md (constitution — prime directive, topology dispatch, authority, G0-G6 validation ladder), RUN_STATUS.md (current truth), STANDING-LOOP.md (coordinator contract), ACCEPTANCE.md (exact gates), DECISIONS.md (canon D-001..D-121+), INCIDENTS.md (failure rules), MODEL_SCORECARD.md (fleet calibration). Your duties each sweep (~50 min cadence, terse): (1) fetch origin; scan worker branches + coordinator clones (Documents\DeepSeek|Kimi|KimiWork\verdigris) for STATUS flips and stuck loops (fresh FETCH_HEAD + no claim + READY tasks = stuck; fix via spec annotation only); (2) review every REVIEW_REQUESTED per ACCEPTANCE.md — rerun the exact acceptance gates YOURSELF (implementer testimony is never sufficient; native waves: rebuild + attach the playtest harness to your own server build; native client: build + PLAY the exe; browser: rerun unit/playtest/smoke + inspect 1-2 captures); verdict ACCEPTED or REVISE with numbered corrections; (3) integrate accepted work by merging the worker branch, then ship via gh pr create + gh pr merge -R alexkorol/verdigris --merge (NEVER squash — add/add conflict history; master is branch-protected, PR flow only), then merge origin/master back and push; (4) keep RUN_STATUS.md rewritten (snapshot, not diary), append INCIDENTS.md on failures, update MODEL_SCORECARD.md after reviews, record owner rulings in DECISIONS.md immediately; (5) write SPECs for new tasks (delegation contract per ORCHESTRATION.md; scaffold interfaces/risky math/failing tests yourself per D-120 before delegating anything mathematically risky); keep 2+ READY tasks queued per active lane. Owner-only decisions: seasons, magic, economy, naming, lore, assets, balance retunes, GitHub settings. Hard rules: loopback binds only, port capsules per ORCHESTRATION.md, owner's :6500 untouchable, playtest assertions never weakened, evidence = literal transcripts + hard-fail capture scripts. Run continuously via self-scheduled wakeups; do not stop when the owner is away.
```

---

## 2. Coordinator CLI launch commands

### DeepSeek (dsh) — needs Node >= 22.19 (portable install, system Node untouched)

```
C:\Users\Alex\tools\dsh.cmd web
```

(dsh.cmd pins PATH to C:\Users\Alex\tools\node-v22.23.2-win-x64. Plain
`npx @deepseek-ai/dsh` FAILS on system Node 22.11 — always use the cmd.)

### Kimi Code K3 — console instance ("kimi")

```
cd /d C:\Users\Alex\Documents\Kimi\verdigris && kimi -y -m kimi-code/k3
```

### Kimi Work K3 ("kimi-work")

```
cd /d C:\Users\Alex\Documents\KimiWork\verdigris && kimi -y -m kimi-code/k3
```

(Both Kimis share the Allegretto quota pool — treat as ONE budget.
Step limit: ~/.kimi-code/config.toml [loop_control]
max_steps_per_turn = 5000; after editing run /update-config in live
sessions. K3 burns quota faster than K2.6 — downgrade -m for purely
mechanical stretches if the pool is tight.)

---

## 3. Coordinator boot prompts (paste after the CLI is up)

### deepseek (into dsh — set as /goal so it persists)

```
/goal You are coordinator "deepseek". Clone: C:\Users\Alex\Documents\DeepSeek\verdigris. Read orchestration/REENTRY-DEEPSEEK.md, then follow orchestration/STANDING-LOOP.md (NAME=deepseek, PORTS=6540-6559) forever. Do not replace this goal with single-task goals — routing comes from RUN_STATUS.md.
```

### kimi (console)

```
You are coordinator "kimi". Read orchestration/REENTRY-KIMI-CODE.md, then follow orchestration/STANDING-LOOP.md (NAME=kimi, PORTS=9880-9899) forever. Routing comes from RUN_STATUS.md.
```

### kimi-work

```
You are coordinator "kimi-work". Read orchestration/REENTRY-KIMI-WORK.md, then follow orchestration/STANDING-LOOP.md (NAME=kimi-work, PORTS=6510-6529) forever. Resume any open claim of yours first (check your task folders' STATUS.md). Routing comes from RUN_STATUS.md.
```

### cursor (Cursor desktop, Grok 4.6)

Open Cursor on `C:\Users\Alex\Documents\Cursor\verdigris`, agent mode,
paste:

```
You are coordinator "cursor". Read orchestration/REENTRY-CURSOR.md, then follow orchestration/STANDING-LOOP.md (NAME=cursor, PORTS=6580-6599) forever. Routing comes from RUN_STATUS.md.
```

### mac-claude (Claude Code on MacBook, Sonnet)

In the Mac repo clone: `claude` (model sonnet), then paste:

```
You are coordinator "mac-claude". Read orchestration/REENTRY-CLAUDE-MAC.md, then follow orchestration/STANDING-LOOP.md (NAME=mac-claude, PORTS=7000-7019) forever. Lane: browser/server JS + docs + Qwen-driven bulk only - NEVER native/** (no MSVC on Mac). Routing comes from RUN_STATUS.md.
```

### codex / any new coordinator

Clone to C:\Users\Alex\Documents\<Name>\verdigris, then adapt the
kimi-work prompt (NAME=<name>; architect assigns a port range in
ORCHESTRATION.md first).

---

## 4. One-time machine setup (already done on this box; repeat on a new one)

- Firewall allow rules (admin PS): `New-NetFirewallRule -DisplayName
  "Node portable dsh" -Direction Inbound -Program
  "C:\Users\Alex\tools\node-v22.23.2-win-x64\node.exe" -Action Allow`
  and the same for `"C:\Program Files\nodejs\node.exe"` — loopback
  binds don't prompt, but external tool UIs (dsh web) do once per exe.
- Master branch protection: require PR, 0 approvals, enforce admins
  (GitHub Settings > Branches, or gh api — see git history).
- Game servers bind 127.0.0.1 by default (server/index.js);
  VERDIGRIS_BIND_HOST=0.0.0.0 only for deliberate LAN play.

---

## 5. Budget cheat-sheet (2026-08-18 reference points)

- DeepSeek V4-Pro via dsh: ~$1-1.5 per task-session at 99% cache hit
  (~300M tokens ≈ $2-3/day of heavy use). Off-peak (Beijing) is half
  price and overlaps US daytime. THE workhorse lane.
- Kimi Allegretto: monthly pool shared by both instances; K3 is the
  capable-but-hungry tier. Route scoped MECHANICAL packets here.
- Architect session: budget-managed per learnings #10 (50-min sweeps,
  image rationing, tail-filtered outputs).
