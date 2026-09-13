# TASK-0102 — Skill system and binding gap audit

Lane: ox-pc-bb · Model: openrouter/stealth/ox-alpha · Branch: `worker/verdigris/pc/ox-pc-bb`
Base: `d2423873c577d299b3b39c56024d1d840993c72b` (audit reflects the current integrated head of the branch)
Scope: mechanical mapping only. No skill name, effect, cost, or balance is invented. Magic content stays owner-blocked (OD-003).

Machine-readable mirror: [`captures/skill-matrix.json`](captures/skill-matrix.json).

## 0. Frozen invariants honored

| Invariant | Source | Status in code |
|---|---|---|
| D-007 control contract (LMB primary, RMB weapon skill, Space dodge/dash, Q/E/R skills — E is a skill slot; X/Z/F/I-Tab utilities) | `orchestration/DECISIONS.md:30-40`; constitution `docs/product/VERDIGRIS_CONSTITUTION.md:94-101` | Partially wired — see §2 and negative controls |
| Shared actor/stat schema (one microscopic schema for players and monsters) | constitution `VERDIGRIS_CONSTITUTION.md:87-92` | Honored: one `Actor`/`ActorStats` (`native/include/verdigris/core.hpp:106-161`), elite monsters reuse the same resolver (`native/src/core.cpp:336-421`, tests `native/tests/core_tests.cpp:1585,454`) |
| Server/simulation authority (presentation requests → simulation resolves → events → display) | constitution `VERDIGRIS_CONSTITUTION.md:157-168` | Honored on the local path; **violated in spirit on the protocol path** for skill requests (see §9) |
| OD-003 owner block on production magic/spell content | `docs/product/OPEN_DECISIONS.md:10` | Honored here: no magic content mapped as implementable; see §11 |

## 1. Two authority paths exist (the central structural fact)

- **Path A — local headless core (authoritative action pipeline).**
  `Simulation::dispatch(Command::action_use)` (`native/src/core.cpp:299-302`) →
  `resolve_action` (`core.cpp:330`) → `resolve_actor_action` (`core.cpp:336-421`).
  Players and monsters share it; elites schedule telegraphed actions through the
  same pipeline (`core.cpp:649-720`). Deterministic fixed-step tick:
  `advance_tick` regenerates resources (+2/tick), counts cooldowns down, expires
  buffs (`core.cpp:722-738`).
- **Path B — native protocol server (N-series browser-parity combat).**
  Wire event `player:skill:trigger` → handler at `native/src/networking.cpp:2507`
  → `WorldSimulation::start_player_attack` (`core.cpp:1833-1883`) +
  `advance_combat` (`core.cpp:1885-2022`) → wire events via
  `emit_combat_event` (`networking.cpp:2062-2078`). This path has **no skill
  resolution**: every trigger becomes a primary attack (§9).

Path A serves local play and client scenarios; Path B serves remote play.
Q/E/R end-to-end authority exists only on Path A.

## 2. Input slot matrix

Slot definitions live in the client only as *requests*; all gates re-checked
by the core (`native/client/main.cpp:931-933`: "Do not duplicate
target/range/cooldown rules in the client").

| Slot | Client binding | Local request seam | Core authority (Path A) | Protocol path (Path B) |
|---|---|---|---|---|
| LMB | `WM_LBUTTONDOWN` → `submit_action(Melee)` `native/client/main.cpp:4095-4103` | `Command::action_use(Melee)` / `"melee"` | `resolve_actor_action` nearest-target melee in `kMeleeRange` (`core.cpp:366-417`) | Primary swing loop (`core.cpp:1943-1982`) |
| RMB | `WM_RBUTTONDOWN` → `dispatch_dash()` `main.cpp:4104-4107` | `ActionType::Dash` / `"dash"` | Dash = movement burst of `kDashMovementTicks=10` ticks (`core.hpp:76`; dispatch `main.cpp:3948-3957`) | Same dash surface as Space; no separate weapon skill exists |
| Space | `VK_SPACE` → `dispatch_dash()` `main.cpp:4011` | same as RMB | same | same |
| Q | `kSkills {'Q',"Thrust",Thrust}` `main.cpp:909-913`, key handler `main.cpp:4012` | `Command::action_use(Thrust)` / `"thrust"` | Thrust resolves (`core.cpp:361-417`) | **Degrades to primary attack** (§9) |
| E | `{'E',"Sweep",Sweep}` `main.cpp:910-912` | `"sweep"` | Sweep resolves | **Degrades to primary attack** (§9) |
| R | `{'R',"WarCry",WarCry}` `main.cpp:911-912` | `"war-cry"` | War Cry resolves | **Degrades to primary attack** (§9); buff unreachable remotely |
| X / Z / F / I-Tab | pickup / loot-label toggle / extract / gear pane (`native/README.md:36-45`) | out of scope here | — | — |

Deviation from the D-007 letter: D-007 assigns **RMB = weapon skill**
(`orchestration/DECISIONS.md:33`), but the client binds RMB to dash,
identical to Space (`main.cpp:4104-4107`), and no second physical weapon-skill
action type exists anywhere. The quickbar labels confirm the shipped intent
(LMB Strike, Q Thrust, E Sweep, R WarCry — no RMB entry):
`main.cpp:2396-2404`.

## 3. Authoritative skill/action definitions (complete set)

The enum is closed; new values must keep recorded command streams stable
(`core.hpp:25-27`):

```cpp
enum class ActionType { Melee, Dash, Wait, Thrust, Sweep, WarCry };
```

| Action | Cost (resource) | Cooldown | Range/band | Targeting | Effect |
|---|---|---|---|---|---|
| Melee | 0 | `attack_speed_ticks` (=4 default, `core.hpp:118`) shared with Thrust (`core.cpp:399-400`) | `kMeleeRange` (`core.hpp:65`) | nearest opposite-kind alive actor (`core.cpp:371-388`) | base damage roll (`resolve_damage`), `core.cpp:408-418` |
| Thrust | `kThrustResourceCost=10` (`core.hpp:84`) | shares melee cooldown | `kThrustRange = 1.5×melee` (`core.hpp:66`) | forward cone predicate `is_forward(facing,Δ)` inside band (`core.cpp:374-381`) | damage ×13/10, min 1 (`core.cpp:22-23`, `core.cpp:411-412`) |
| Sweep | `kSweepResourceCost=15` (`core.hpp:85`) | ×3/2 attack speed (`core.cpp:26-27`, `core.cpp:395-397`) | melee range | all opposite-kind actors in range (`core.cpp:382-383`) | damage ×3/4 per target, min 1 (`core.cpp:24-25`, `core.cpp:413-414`) |
| WarCry | `kWarCryResourceCost=20` (`core.hpp:86`) | none beyond cost gating | self | self | `+kWarCryAttackBonus=4` attack for `kWarCryDurationTicks=20`, then `BuffExpired` (`core.hpp:88-89`, `core.cpp:348-353`, expiry `core.cpp:730-735`) |
| Dash | 0 | none in sim (movement-budget burst) | `kDashMovementTicks=10` movement ticks (`core.hpp:76`) | self/facing | short readable burst; scenery-blocked locally (`main.cpp:3948-3957`) |
| Wait | 0 | — | — | — | no-op filler (`core.hpp:27`) |

Elite monster variants use identical constants through the same resolver:
elite AI selects telegraphed Thrust (mid band + forward cone) or Sweep (close,
funded) with `kTelegraphTicks=3` windup (`core.hpp:32`, scheduling
`core.cpp:680-702`, resolution `core.cpp:658-671`), then re-checks
resource/cooldown gates at resolution ("deliberately checked again",
`core.cpp:684-686`).

**No magic action types, mana costs, or spell fields exist in native code.**
`resource_mana`/`bonus_mana` are item/stat vocabulary only (`core.hpp:511,619`;
emitted as item bonuses at `networking.cpp:373-375,447-449`) and never gate any
action.

## 4. Costs, cooldowns, resources

- Resource pool: `ActorStats.resource/resource_max` default 50/50
  (`core.hpp:113-114`); regen +2/tick for every actor (`kResourceRegenPerTick`,
  `core.hpp:87`; applied `core.cpp:725-728`).
- Single shared cooldown field `Actor.cooldown_ticks` (`core.hpp:146`);
  decremented once per tick (`core.cpp:729`). Gating order in the resolver:
  cooldown first, then affordability, before any target work
  (`core.cpp:364`); a failed gate consumes nothing (tests
  `core_tests.cpp:165,215`).
- Presentation reads the same numbers from the read-only
  `Simulation::presentation_catalog()` (`core.hpp:92-104`,
  `core.cpp:222-232`); the client mirrors catalog values instead of hardcoding
  (`skill_resource_cost`, `main.cpp:915-923`; quickbar paint `main.cpp:2441`).
  Catalog stability test: `test_presentation_catalog_is_authoritative_and_stable`
  (`core_tests.cpp:931`, catalog-vs-deduction checks at `946-948,971-973`).
- Path B cadence is wall-clock ms, not ticks: player swing interval
  `kN3PlayerAttackIntervalMs=350`, monster 1200–1500 ms, boss slam window
  1000 ms radius 2 damage 12 (`core.cpp:1468-1474`, `1913`, `1973`, `2019`,
  `1988-1991`). No resource/cooldown concept exists on this path.

## 5. Targeting model

- Path A player Melee/Sweep/Thrust: Manhattan distance bands + facing cone for
  Thrust; deterministic nearest-wins tie-break by scan order (`core.cpp:366-391`).
- Elite AI targeting: the player is the sole target; band selection per §3.
- Path B aim: boss > empowered > nearest-with-facing-tiebreak target selection
  with pack-focus retention (`core.cpp:1841-1877`); walking out of reach
  disengages (`core.cpp:1933-1940`). Direction comes from the trigger payload
  (`networking.cpp:2507`), populated client-side from last facing
  (`remote_session.cpp:430-443`). Aim itself is presentation-local on the
  protocol (no envelope): `remote_session.cpp:430-439`.

## 6. Effects inventory (all existing effects; nothing invented)

- Damage application + death handoff: `core.cpp:408-418`.
- BuffApplied/BuffExpired war-cry pair: `core.cpp:352`, `734`.
- AttackTelegraphed (elite thrust/sweep, 3-tick windup): `core.cpp:694,700`.
- AttackStarted naming (`"melee"/"thrust"/"sweep"`): `core.cpp:402-405`.
- Boss ground-slam telegraph/impact (Path B encounter contract,
  `boss:ground-slam`): `core.cpp:1986-2011`.
- Critical/beastbane parity fields (Path B hits): `core.cpp:1944-1971`.

## 7. Wire events

Envelope shape `{event,data}` throughout.

| Event | Direction | Producer | Consumer | Skill-relevant fields |
|---|---|---|---|---|
| `player:skill:trigger` | C→S | `remote_session.cpp:440-445` sends `{"skill","direction"}`; scenario harness sends `{"skillId":"primary-attack","direction"}` (`session_tests.cpp:1215-1220`) | `networking.cpp:2507` reads `payload->get("skillId")` | key mismatch — see §9 |
| `monster:telegraph` | S→C | `emit_combat_event` `networking.cpp:2063-2067` | `remote_session.cpp:756-768` | `attackerId/Name, skillId, x, y, radius, durationMs` |
| `combat:hit` | S→C | `networking.cpp:2070-2077` | `remote_session.cpp:770-814` | `skillId` (echoes `active_skill_id_` for player attacks, `networking.cpp:2071`), `amount, died, health{current,max}, baseAmount, beastbane*, critical, attackStyle` |
| `player:skilltree:save` | C→S | dev harness `main.cpp:5343-5350` (browser-wire compatible) | `handle_skilltree_save` `networking.cpp:1229-1238` | passive-tree snapshot |
| `player:skilltree:update` | S→C | `networking.cpp:1237` | model mirror `remote_session.cpp:913` | `player.socket_id`, `passiveTree` |
| passiveTree snapshot | S→C | `passive_tree_json` `networking.cpp:1192-1226` (schemaVersion 2, `points.skill`) | copy-only mirror `remote_session.cpp:97-117`, `client_model.hpp:70-81` | unspent/earned points, node/conduit counts |

Server-side power formula distinguishes legacy `"ability*"` ids as "mana
skills" (4+INT×0.5 vs 2+STR×0.45+weapon×1.5): `networking.cpp:2086-2093`.
No native wire id ever satisfies that prefix today; it is browser-parity
plumbing awaiting OD-003 content.

## 8. Client model & HUD presentation

- Named-action seam: `LocalCoreSession` maps `"thrust"/"sweep"/"war-cry"/"dash"/"wait"`
  back onto `ActionType` so seam consumers see real lifecycle beats
  (`local_session.cpp:125-136`, TASK-0122 note).
- Copy-only progression model (client never derives rules/costs/effects):
  `client_model.hpp:70-81`, mirrored at `remote_session.cpp:97-117`.
- HUD: quickbar strip paints exactly four slots (LMB/Q/E/R;
  `static_assert` against geometry helper, `main.cpp:2396-2404,261-268`),
  availability = alive ∧ affordable ∧ off-cooldown (`main.cpp:2442-2445`),
  cooldown sweep overlay normalized against a hardcoded 30-tick max
  (`main.cpp:2462-2471`), war-cry active highlight (`main.cpp:2446-2447`),
  render-list op `Op::Quickbar` (`render_list.hpp:35`) recorded next to the
  draw so scenarios catch suppression.
- State feed into HUD: `presentation_state.cpp:45-58` copies
  resource/cooldown/war-cry ticks from authoritative actors.
- Controls lines on HUD: `"WASD move | mouse aim | LMB attack | RMB/Space dash | Q E R skills | …"`
  (`main.cpp:3635`) and `"… Q Thrust | E Sweep | R WarCry"` (`main.cpp:3803`).
- Orbs: `Op::Orb` life/resource (`render_list.hpp:34`) — presentation-only;
  Vessels-of-Mana integration stays behind OD-003.

## 9. Negative control — Q/E/R have no end-to-end authority on the protocol path

Chain of evidence (each step cited):

1. The remote client sends the requested action under JSON key **`"skill"`**:
   `remote_session.cpp:440-445` (`{"skill": "thrust"|"sweep"|"war-cry", "direction": …}`).
2. The protocol handler reads key **`"skillId"`**, so a native remote client's
   request always falls back to the default `"primary-attack"`:
   `networking.cpp:2507` (`active_skill_id_=as_string(payload?payload->get("skillId"):nullptr,"primary-attack")`).
   Default field: `networking.hpp:207`; reset on login: `networking.cpp:643`.
3. Even when `active_skill_id_` *is* set, it never selects an action. It only
   labels outgoing hit events (`networking.cpp:2071`) and toggles a legacy
   `"ability*"` power formula (`networking.cpp:2090-2093`). The actual
   resolution, `WorldSimulation::start_player_attack`, ignores even the passed
   power (`(void)player_attack;`, `core.cpp:1881`) and merely locks a target;
   `advance_combat` stamps every player hit `skill_id="primary-attack"`
   (`core.cpp:1967`) on a flat 350 ms cadence (`core.cpp:1468-1474,1973`).
4. No feedback loop exists: protocol snapshots carry no resource/cooldown/war-cry
   state (grep for `resource` in `native/src/networking.cpp` yields only item
   bonus fields, `networking.cpp:373-375,447-449`), `ClientPlayer.resource`
   is a hardcoded 50/50 default never updated from the wire
   (`client_model.hpp:23-24`), and `ClientPlayer` has no cooldown field at all.
5. Test coverage confirms the asymmetry: the harness only ever triggers
   `{"skillId":"primary-attack"}` (`session_tests.cpp:1215-1220`,
   `networking_tests.cpp:315-327`); no test drives `"thrust"/"sweep"/"war-cry"`
   through `ProtocolSession` (repo-wide grep over `native/tests` returns zero
   such uses), while the local path is exhaustively covered (§10).

Conclusion: Q/E/R are fully authoritative only in local play. On the protocol
path the slots are presentational — presses resolve to plain primary attacks
with no cost, cooldown, cone, area, or buff semantics, and the HUD cannot show
true availability. Secondary negative finding: the **RMB slot** has no distinct
weapon-skill action anywhere (bound to dash, §2), so the D-007 letter is
unimplemented there on both paths.

Incidental observation (not repaired here — outside owned paths):
`fprintf(stderr,"[swing] …")` debug output ships in production combat,
`core.cpp:1942`.

## 10. Tests inventory (skill/action evidence)

Core (headless, deterministic):

| Test | Path | Proves |
|---|---|---|
| `test_skill_resource_gating_and_thrust` | `native/tests/core_tests.cpp:150` (checks 165,178-180,210-215) | unfunded/gated Thrust consumes nothing; pays named cost after one regen tick; shares melee cooldown; respects pre-existing cooldown |
| `test_sweep_hits_multiple_targets_and_gates_resource` | `core_tests.cpp:341` (375-378) | multi-target sweep, cost after regen tick, ×3/2 cooldown |
| `test_elite_thrust_telegraph_timing` | `core_tests.cpp:395` | 3-tick windup before elite thrust lands |
| `test_elite_skill_cone_gating` | `core_tests.cpp:425` | thrust band requires forward cone |
| `test_elite_skill_fizzles_when_resolution_gates_fail` | `core_tests.cpp:437` (472-473) | gates re-checked at windup end; shared sweep cooldown |
| `test_elite_sweep_uses_shared_pipeline` | `core_tests.cpp:454` | elites use the player resolver unchanged |
| `test_elite_telegraph_cancels_on_death` | `core_tests.cpp:476` (511) | telegraph never damages a dead target |
| `test_elite_skill_replay_is_deterministic` | `core_tests.cpp:514` (528) | telegraph+resolution replay byte-identically |
| `test_non_elite_melee_cadence_is_unchanged` | `core_tests.cpp:536` (552-553) | non-elite cadence untouched by skill work |
| `test_war_cry_buff_expiry_and_replay_determinism` | `core_tests.cpp:556` (575,594) | war cry cost/regen, expiry, deterministic under replay |
| `test_presentation_catalog_is_authoritative_and_stable` | `core_tests.cpp:931` (946-948,971-973) | catalog values match actual deductions |
| `test_elite_uses_same_universe` / `test_elite_kill_and_recorded_event` | `core_tests.cpp:1585,1617` | shared-actor symmetry for elites |

Protocol/session:

| Test | Path | Proves |
|---|---|---|
| N3 boss telegraph | `native/tests/networking_tests.cpp:204-212` | `monster:telegraph` `boss:ground-slam` with radius ≥2, duration ≥800 ms |
| Gate-B swing loop | `native/tests/session_tests.cpp:1215-1220` | `player:skill:trigger {"skillId":"primary-attack","direction"}` is the only exercised variant |
| Kill-loot loop | `networking_tests.cpp:315-327` | bare `{"direction":"left"}` trigger defaults cleanly to primary attack |

Client scenarios assert quickbar presence (`main.cpp:4265,4628` ≥4 Quickbar ops)
and war-cry HUD fade within the authoritative tick window
(`main.cpp:6061-6077`).

## 11. Proceed vs owner-blocked split

Content-neutral infrastructure that can proceed **without** OD-003 (all within
existing physical actions; no new names/effects/balance):

1. Unify the trigger wire contract: send/read one agreed key (`skill` vs
   `skillId`) and route `player:skill:trigger` on the protocol server into the
   existing `ActionType` resolver (or its explicit equivalent) so Q/E/R/RMB
   carry their current, already-shipped physical semantics end-to-end.
2. Emit authoritative resource/cooldown/buff state (e.g., extend snapshots or
   combat events) and remove the client's hardcoded 50/50 default
   (`client_model.hpp:23-24`); give `ClientPlayer` a cooldown field fed
   copy-only.
3. Decide/implement the RMB slot as a distinct binding per D-007 (currently
   dash-only, §2) using only existing `ActionType`s.
4. Add protocol-path tests driving `"thrust"/"sweep"/"war-cry"` names through
   `ProtocolSession` (today zero coverage, §9 step 5).
5. Replace the hardcoded 30-tick cooldown-sweep normalizer with catalog-fed
   values (`main.cpp:2462-2465`).

Owner-blocked until OD-003 (magic design pass) — required successors must
restate these blocks:

- Any **new** skill definition: names, elements, manifestations, Arcane Lattice
  integration, spell authoring format (`OPEN_DECISIONS.md:10`;
  `WIZARD_ARCANE_LATTICE_REFERENCE.md:34-36` — reference/design evidence only).
- Mana-as-a-second-resource semantics: production meaning for
  `resource_mana`/`bonus_mana` beyond item stat text
  (`core.hpp:511,619`), Vessels-of-Life-&-Mana gameplay semantics
  (`WIZARD_ARCANE_LATTICE_REFERENCE.md:45`).
- Repurposing the legacy `"ability*"` mana-formula branch
  (`networking.cpp:2090-2093`) for new content.
- Any generic wizard/starter-kit framing (constitution
  `VERDIGRIS_CONSTITUTION.md:118-124`, firewall at 175-180).

## 12. Evidence index

Acceptance-command transcripts (literal outputs + exit codes):
[`REPORT.md`](REPORT.md). Machine-readable matrix:
[`captures/skill-matrix.json`](captures/skill-matrix.json). All paths above are
relative to the repository root at audited head `33a381b8` (claim commit) on
`worker/verdigris/pc/ox-pc-bb`.
