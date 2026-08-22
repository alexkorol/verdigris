# Supervisor succession and temporary retirement

The primary desktop Verdigris supervisor is the current PC Codex architect task
while the owner keeps it active. Supervisor capacity is not assumed permanent;
the owner may temporarily retire or replace it when Codex credit pressure
requires. The supervisor cannot read the owner's account quota reliably, so an
owner warning or switch directive is the authoritative budget trigger.

Leader-token austerity is active under `LEADER_POLICY.md`. Continuous
reconciliation transfers to the portable `orchd` controller after its vertical
slice is accepted. Before then, any emergency monitor is short, read-only, and
at most hourly; the leader does not remain in a 15-minute reasoning loop.

## Retirement states

- `PRIMARY_ACTIVE`: normal architect/orchestrator loop.
- `RETIREMENT_WARNED`: owner signals likely credit-driven switch; avoid opening
  optional long investigations, finish the current atomic coordination action,
  and prepare the handoff ledger.
- `HANDOFF_READY`: all coordination changes are committed/pushed as authorized,
  current evidence is freshly fetched, and the ledger below is complete.
- `SUCCESSOR_ACKNOWLEDGED`: replacement proves authority read, exact checkout,
  program/remote tips, current claims/reviews/alerts, and automation ownership.
- `TEMP_RETIRED`: predecessor stops only after acknowledgement, unless the owner
  explicitly orders an immediate stop.

## Required handoff ledger

Record in `RUN_STATUS.md` or a dated handoff packet:

1. canonical repo/worktree paths, remotes, branches, local and remote SHAs, and
   dirty/divergence status;
2. active/parked/requested/claimed/active workers with task, base, branch,
   ports, last durable evidence, and alert deadlines;
3. reviews, revisions, integrations, CI, owner gates, incidents, and open
   activation/queue/host-sync alerts;
4. autonomous runway hours and confidence, trailing accepted throughput,
   READY/AUTO_RELEASE/reserve/graph counts and composition;
5. exact next sweep, critical path, automatic promotions, and owner actions;
6. automation names, cadence, destination, notification caveats, and the owner
   or successor action required to transfer them;
7. standalone `alexkorol/orchestration` PC/Mac sync proof and bootstrap state.

## Successor preflight

The successor reads `AGENTS.md`, `ONBOARDING-SOL-ORCHESTRATOR.md`, PROTOCOL,
ORCHESTRATION, ACCEPTANCE, DECISIONS, INCIDENTS, RUN_STATUS,
BACKLOG_FACTORY, CONTENT_SCALE_MATRIX, and this file; fetches before acting;
reconciles repository evidence; posts a durable acknowledgement; then resumes
scan/review/integrate/release/decompose/restock. It does not restart completed
work or implement worker tasks.

A failed handoff or missing acknowledgement is an owner-notification event. It
does not authorize the outgoing supervisor to burn remaining credits by taking
over implementation.

## Verified live PC checkpoint — 2026-08-22 05:41 PDT

State: `PRIMARY_ACTIVE`; this is a succession checkpoint, not a retirement
request.

- Program checkout: `Z:\Code\Games\delaford\delaford_game`, branch
  `codex/native-reconstitution`, clean local/remote program head
  `e812c20ade6c022b163a047e32f311a1119db8fb` before this checkpoint refresh
  commit. `origin/master` is not the worker base;
  workers use the exact routed program heads recorded in `RUN_STATUS.md`.
- Standalone broadcast checkout:
  `Z:\Code\.worktrees\orchestration\pc-overnight-game-wave`, clean branch
  `codex/pc-overnight-game-wave`, local/remote head `98ab162`; observed
  standalone `origin/main` is `59a70b6`. PC has no shared-main authority.
- Active implementation/revision evidence:
  - TASK-0145 head `78dcac60` and TASK-0149 revision head `a88d307d` are
    independently ACCEPTED and integrated at `2df5eac5` and through
    `8677f021`; combined program binaries passed all native suites, all eight
    client scenarios, and the real-window lifecycle selftest;
  - ox-pc-l, ports 6840-6859, TASK-0146 revision head `086ac07b` independently
    ACCEPTED and integrated as the four-commit program chain through `5324f13e`;
    combined program gates passed and pushed head is `08e8776e`;
  - ox-pc-p, ports 6920-6939, TASK-0147 claim `3ee9f928`; first process exited
    with owned-path edits preserved after its real GDI capture exposed broken
    scenery geometry. The supervisor independently rejected the worker's fresh
    05:40 stale probes; the actually regenerated 05:41 motif set is coherent.
    Its 06:00 native capture then used a stale 04:54 executable predating the
    05:39 header. One retry failed from a supervisor argument-parsing fault and
    the exact session retry then wedged without an event; final fresh-context
    dirty-lane salvage PID 25328 must rebuild, prove executable freshness, and
    recapture. Its next stop without handoff quarantines the lane;
  - ox-pc-q, ports 6940-6959, TASK-0148 claim `815a359b` released and clean
    preserved after its recovery proposed `mutate/select` for the first Scion,
    bypassing the required accepted `create/set-out` chain. It is exhausted,
    not capacity;
  - ox-pc-r, ports 6960-6979, TASK-0148 fresh r5 route from release head
    `c1acd4ec`, independent claim `837a412f` pushed inside the activation SLA,
    clean pre-write exit resumed in the exact session at PID 7324. Its start
    contract freezes the accepted first-Scion chain and bans `dev:*`,
    `player:chronicles:mutate`, and direct state shortcuts.
- Preserved/non-capacity: ox-pc-d/e/g/m are dirty P0 quarantines after exhausting
  one recovery; ox-pc-n is a clean activation failure after its launch and one
  recovery both stopped before claim; a/b/c/h are preserved historical or
  exhausted routes as detailed in `RUN_STATUS.md`; ox-pc-o is clean but
  exhausted after its claim plus one stopped recovery. Never infer capacity
  from their old logs or pushed claim heads.
- Queue/factory: board sentinel healthy, 27 effective READY, zero REVISE, 17
  sequenced DRAFT successors, zero owned-path collisions; deterministic factory
  verified at 2,000 nodes / 500 packets. Runway remains honestly `UNKNOWN`.
- Local supervision: human dashboard `http://127.0.0.1:4737/` health 200,
  Node PID 12400; visible PowerShell monitor PID 18912. The monitor's alert set
  is restricted to current completed/active routes `i/j/k/l/p/r`, so preserved
  historical/quarantined lanes cannot produce false toasts; an unattended
  REVISE route now raises a distinct P1 alert. Same-task automation
  `verdigris-surge-supervisor` is ACTIVE every five minutes and derives lanes
  dynamically from current routing/config. Chat-spawning cron
  `pc-fleet-emergency-monitor` remains PAUSED.
- Shared native session tests use the 6580-6599 capsule across worktrees. One
  combined run overlapped ox-pc-l's test process and failed different timing
  checks; after its listener exited, the exact integrated binaries passed
  core/networking/camera/session, all eight client scenarios, and lifecycle.
  Do not classify a concurrent-capsule failure without listener/parent-process
  evidence; route a deterministic isolation hardening packet.
- A stale local TASK-0141 cherry-pick tool chain was found trying to write the
  program worktree during TASK-0146 integration. App thread inventory proved
  no second live coordinator existed and TASK-0141 was already integrated;
  the exact orphan process tree was terminated before it altered the index.
  Never permit two coordinator commands to share this checkout.
- Exact next sweep: verify p/r PIDs, logs, git changes, and pushed heads;
  freeze any clean REVIEW_REQUESTED head; run every literal SPEC gate independently and
  visually inspect TASK-0147 captures; reject any TASK-0148 journey containing
  `dev:*` setup or direct state mutation; integrate only ACCEPTED commits; rerun
  combined native gates and the owner launch after each product integration.
- Owner action required now: none. On credit-driven replacement, the successor
  must fetch both repositories, reproduce this checkpoint against live evidence,
  acknowledge the current alerts/reviews/automations, and only then assume the
  loop.

### Live routing delta — 2026-08-22 06:31 PDT

- Program head before this coordination refresh is `9fe673b6`; standalone PC
  broadcast head is `fffb70c` on `codex/pc-overnight-game-wave`. Shared
  orchestration `main` remains Mac-owned and untouched.
- TASK-0147 fresh captures now come from an executable built after the corrected
  generated header and are coherent at 1366x768 and 1920x1080. The worker is
  still producing committed default-resolution/composite evidence; no handoff
  has been accepted yet.
- TASK-0148 now proves ordinary-combat death, successor creation, and reconnect
  continuity. Its heirloom-recovery leg is still running and remains an
  acceptance blocker.
- Three additional read-only prerequisite lanes are active from exact routed
  base `9fe673b6`: ox-pc-s / TASK-0116 claim `297b4e9d`, ox-pc-t / TASK-0117
  claim `763228e1`, and ox-pc-u / TASK-0119 claim `fad856c3`. Their worker
  branches are pushed. They may write only their task audit folders; their
  purpose is immediate promotion of product-facing VFX, synthetic-placeholder
  audio, and first-session implementation successors after independent review.
- Dashboard JSON and monitor script parse successfully. Human dashboard remains
  HTTP 200; visible monitor PID 18184 watches the three new lanes as well as the
  current p/r routes. No new Codex task was created.
- Owner action required now: none.

### First-session audit checkpoint — 2026-08-22 06:57 PDT

- TASK-0119 worker head `4104e0c8` is independently ACCEPTED and integrated at
  `a5f4133e`. Its 13-step contract identifies the remote extraction
  instruction mismatch and the dropped expedition-phase/progression surfaces
  as the highest-value non-lore onboarding fixes.
- The next lane may implement those bounded interaction fixes on the current
  program base. House/Scion naming remains owner-gated and must not be silently
  added. Owner action required before the unblocked interaction successor:
  none.

### Product integration checkpoint — 2026-08-22 06:50 PDT

- TASK-0147 worker head `974ccab6` is independently ACCEPTED (review
  `6575f330`) and integrated at program commit `19be98db`. The coordinator
  reran the full native build/test/client-scenario suite and direct
  `first-fight`; all passed on the combined program head.
- Fresh visual evidence proves the executable postdates the corrected generated
  header. Five wide scenes, five compact scenes, the default-window scene, and
  representative before/after composites were visually inspected. The native
  presentation is materially richer and coherent, while remaining honestly
  labeled as procedural placeholder art rather than final terrain quality.
- TASK-0148 on ox-pc-r remains active and blocked on a normal-play heirloom
  recovery proof after succession; no shortcut or direct-state workaround may
  be accepted. ox-pc-s/t/u remain active on read-only VFX, audio, and onboarding
  contracts that must produce executable implementation successors after
  independent review.
- Owner action required now: none.

### Morning ship and owner-play checkpoint — 2026-08-22 07:12 PDT

- PR #51 merged the verified program head `eb65c08b` to protected `master` at
  `db3fc0467ed0cc978f5152aeec558208825bd0af`. The local full native release
  gate passed before the PR; GitHub CI/Native workflows remained in progress
  at merge time and must be followed to terminal state.
- The owner live-played the release and judged it materially better but still
  far below a real-game presentation bar. Embedded vector assets are compiled
  and rendered, yet are visually dominated by the repetitive procedural tile
  field. Presence, deterministic captures, and coherent geometry are no longer
  sufficient visual-acceptance proxies; whole-frame owner play is binding.
- Owner found deterministic Esc behavior: `VK_ESCAPE` calls
  `PostQuitMessage(0)` before checking `gear_overlay`. TASK-0153 on ox-pc-v is
  the active hotfix/clarity lane; verified claim `8474ac51`. It must prove Esc
  closes gear first without quitting and a subsequent bare Esc exits.
- INC-013 records ox-pc-v's initial activation-SLA breach and claim-first
  recovery. ox-pc-r was serialized around the release gate and resumed with
  every dirty owned-path edit preserved. Owner action required now: none.

### PC coordination checkpoint — 2026-08-22 09:58 PDT

- Protected Verdigris `master` is green at `230a8adf`. Program branch
  `codex/native-reconstitution` has TASK-0128 integrated through `882de214`
  before final coordination/capture binding. Standalone PC broadcast branch was
  last pushed at `11f9788`; shared orchestration `main` remains Mac-owned at
  `d068012a`.
- TASK-0128 fresh worker head `2b469e3d` is Tier-A ACCEPTED (review `f7260900`)
  and integrated. Both worker and program gates pass 19/19; the deterministic
  snapshot remains `hours:null` / `UNKNOWN`.
- Active implementation lanes: ox-pc-r / TASK-0148, ports 6960-6979, PID 17640,
  valid claim `837a412f`, remote workflow guard `3119a08e`, dirty owned paths
  preserved; ox-pc-x / TASK-0122, ports 7060-7079, PID 18144, valid claim
  `fdbbdca6`, one authorized same-session recovery after a clean pre-write stop.
  No other OpenCode CLI process counts as live Verdigris capacity.
- Queue proof is 24 effective READY, 2 valid CLAIMED, 16 sequenced successors,
  zero collisions. Factory target remains 2,000 nodes / 500 packets and runway
  hours remain UNKNOWN pending comparable accepted telemetry.
- Local monitor PID 17316 and dashboard PID 9588 were healthy with dashboard
  HTTP 200 before this checkpoint. The monitor now detects activation failure,
  malformed claims, duplicate worktree writers, and stale Native push triggers.
- Exact next sweep: verify x produced its first owned source write or release it;
  freeze any r/x handoff; reject TASK-0148 temporary probes or shortcut paths;
  run literal task gates, integrate only ACCEPTED work, re-prove board/factory,
  and refresh both program and standalone broadcast heads.
- Owner action required now: none. A replacement supervisor must fetch both
  repositories and reproduce this checkpoint before assuming authority.

TASK-0128's durable integration/capture head is
`b4c9ce9d44e0343e78e52947fe0083e63f153697`. Its captures are bound to source
`847067e40230869e0a21b4be5e2ec67037fd69e5` and independently pass `--check`
at that later capture commit. The ledger-refresh commit containing this note
changes no collector input or task evidence. After the checkpoint, ox-pc-x
produced owned client-source writes and remains live; it is no longer a
pre-write recovery risk.

### Accepted TASK-0122/TASK-0148 checkpoint — 2026-08-22 12:11 PDT

- Program implementation head before this ledger commit is `8d314d5c` on
  `codex/native-reconstitution`; protected `master` remains `230a8adf` pending
  the protected PR/check path.
- TASK-0122 worker head `d67129e4` is independently ACCEPTED and integrated as
  `032ae03e`; full native, focused VFX, browser 32/32, negative-control, scope,
  and two-resolution visual gates pass.
- TASK-0148 worker head `9f00e198` (implementation `5732367e`) is independently
  ACCEPTED and integrated as `127a540e` + `8d314d5c`; the architect proved the
  complete normal-envelope death/succession/exact-relic/reconnect chain.
- Combined native gates pass. Combined browser acceptance passed 32/32 after
  one retained timing diagnostic: `session-arc` first missed its 15 s death
  wait by 384 ms, then passed standalone in 11.7 s and in the full rerun at
  12.3 s.
- ox-pc-r and ox-pc-x are stopped, clean, and no longer active capacity. No
  other OpenCode Verdigris lane is active. Dashboard remains HTTP 200; monitor
  remains the live activation/stall/CI-signal safety net.
- Exact next step: re-prove board/factory invariants, push coordination truth,
  complete protected PR checks, refresh standalone PC broadcast without
  writing shared `main`, and then route only current-tip disjoint product work.
- Owner action required now: none.
