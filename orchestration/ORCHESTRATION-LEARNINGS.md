# Orchestration learnings (living document)

> **2026-08-18 migration note:** this file is now FROZEN SOURCE
> HISTORY plus a staging area for new OBSERVATIONS. The operating
> system it seeded lives in: ORCHESTRATION.md (constitution + rules),
> RUN_STATUS.md (current truth, rewritten), INCIDENTS.md (append-only
> ledger, items 1-9 migrated as INC-001..009), ACCEPTANCE.md (gate
> registry), MODEL_SCORECARD.md (calibration). New failures append to
> INCIDENTS.md; promotions to RULE happen in ORCHESTRATION.md with an
> enforcement mechanism. Owner's research synthesis:
> C:\Users\Alex\Downloads\multi_harness_orchestration_field_guide.md.

Maintained by the architect automatically after every notable review,
failure, or process change. Purpose: stop repeating mistakes, stop
wasting tokens/time, and tune task design to each model's capability.
(Owner directive, 2026-08-18.)

## Capability tiers and what each needs

- **Weak (Luna workers):** broad goals FAIL here â€” the 2.5D parallax
  bug shipped because "billboard projection" was left as an exercise.
  Weak models need: exact file paths, function signatures, the hard
  math/algorithms already written by the architect, a numbered
  step-by-step plan, and a mechanical self-check per step. They
  execute well; they do not design well.
- **Medium (DeepSeek, Kimi K3):** can design within one subsystem, but
  specs must pin interfaces, invariants, and evidence format. Leave
  local decisions to them; never leave cross-cutting design (camera
  math, protocol shapes, persistence seams) to them.
- **Architect (Fable):** owns all cross-cutting design and now WRITES
  SCAFFOLDING CODE (D-120): interfaces, the tricky math, unit-test
  skeletons that lock in correctness before delegation.

## Verified failure modes (and the fix that worked)

1. **False greens** (0035 twice, 0037 twice): claimed passes that
   architect reruns falsified. FIX: literal transcripts + hard-fail
   capture scripts + architect reruns every gate personally. Zero
   false greens since.
2. **Stale-base clobbers** (0037 reverting 0033): merges resolved
   against old tips. FIX: standing diff-vs-current-tip check.
3. **Broad specs to weak models** (2.5D projection): geometry/math
   left implicit. FIX: D-120 scaffolding â€” architect ships the math.
4. **Invisible progress** (N1-N3): all verification lived in the
   harness, none in the owner's hands. FIX: D-117 â€” every wave ships
   an owner-visible increment; architect plays the exe.
5. **Evidence mirroring conflicts** (3 merge rounds, one marker bake):
   coordinators copying each other's task files. FIX: single-writer
   rule enforced; evidence lives only on its own worker branch.
6. **Env-flag-only proof** (0043 rev0): ten green runs under a flag
   nobody uses by default. FIX: acceptance must exercise the default
   path the owner actually runs.
7. **Driver artifacts read as product bugs** (0046 "silent combat"):
   an evaluator that never reached melee produced a false blocker.
   FIX: arc drivers must verify target contact / preconditions before
   attributing failures to the game.
8. **Token waste patterns observed:** re-requesting review without
   addressing corrections (0046 rev1); duplicated full-suite runs when
   a targeted scenario run answers the question; coordinators
   rebuilding evidence the architect must rebuild anyway. FIX: reviews
   state the EXACT acceptance command; coordinators run that and only
   that plus their own dev loop.

## Process rules now in force

- Specs are tiered: weak-model tasks get step plans + scaffolding;
  medium-model tasks get pinned interfaces + freedom inside them.
- The architect pre-writes: headers/interfaces, the risky math, and a
  failing test that the implementation must turn green.
- Every acceptance updates this document if it taught anything.

## 9. Windows Firewall prompts stall unattended agents (2026-08-18)

Every NEW exe binding a non-loopback socket pops a consent dialog that
sits unanswered while the owner is away, silently blocking sockets
(collapsed a full playtest run once; stalled the fleet again today).
FIX enforced in repo: server/index.js now binds 127.0.0.1 by default
(loopback never prompts); VERDIGRIS_BIND_HOST=0.0.0.0 opt-in for LAN.
Native verdigris_server already loopback. Rule for all future servers/
harnesses: explicit 127.0.0.1 bind, no exceptions without an owner
ruling. External tools we cannot patch (node.exe running dsh web UIs)
need one-time owner-run firewall allow rules - the architect must not
change firewall settings itself.


## 10. Architect session budget is a first-class cost (2026-08-18) - RULE

A day of 25-min sweeps + full-capture eyeballing + unfiltered suite
outputs burned 96% of the architect session window by early afternoon.
Enforcement (loop prompt + review procedure): default sweep cadence
50 min (standing coordinator loops make throughput insensitive to it);
capture-script JSON assertions are primary UI evidence with 1-2
eyeballed captures max per review; tool outputs tail-filtered; dense
cadence only during active owner iteration. Accepted-outcome-per-cost
includes the architect.
