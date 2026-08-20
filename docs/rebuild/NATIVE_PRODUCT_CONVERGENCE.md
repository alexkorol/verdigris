# Native product convergence (canon, D-122, 2026-08-20)

The native reconstitution succeeds only when THREE parities hold. A
green server matrix alone is not native parity — at the time this doc
was adopted, `verdigris_client` linked only `verdigris_core`: the exe
the owner launches ran a private local simulation and never spoke to
the C++ server the N-waves prove.

| Axis | Meaning | Proof |
|---|---|---|
| 1. Server/rules parity | C++ server passes the UNCHANGED browser client + playtest harness | N1–N6 attach waves |
| 2. Native journey parity | Native client completes real player journeys against the C++ server over WS | Gates A/B/C below |
| 3. Presentation/feel parity | Readable, coherent, responsive native presentation | Quality rubric below |

Vocabulary (use exactly): **server parity** (axis 1) · **networked
native slice** (a defined journey passes axis 2) · **native
presentation baseline** (slice passes the quality gate) · **native
product parity** (all three). N6 completes *full server parity*, never
"full parity".

## Sequencing (replaces "C3 after N5")

C3 starts NOW against the already-landed N4 surface, in parallel with
N5. N6 (TASK-0058) releases only after Gate A passes. Every later
server wave is consumed by the native client in the same or adjacent
wave — no more invisible backend banking.

## Session architecture (architect-owned interfaces)

```text
Input → ClientCommand → IClientSession
                          ├── LocalCoreSession      (existing in-process sim; keeps all current scenarios)
                          └── RemoteProtocolSession (WS to verdigris_server; the eventual owner default)
        → ClientSnapshot + PresentationEvent stream → ClientModel → renderer/HUD
```

Rules: client code never touches `Simulation` outside LocalCoreSession;
remote mode runs NO second authoritative sim; remote failure NEVER
silently falls back to local (visible connection states: connecting /
connected / ready / disconnected / retrying / rejected / protocol
mismatch); gameplay rules never live in transport, reducers, renderer,
HUD, or scenario drivers; reuse the existing `{event, data}` envelope —
no second protocol model.

**Single-writer rule:** until the client is split into modules,
`native/client/main.cpp` is single-writer (one lane at a time; the
architect assigns it). Extract modules only as the journey requires —
no wholesale rewrite.

## Gate A — networked guest expedition (unblocked now, on N4)

Fresh profile, one command: start server on a task-owned loopback port
→ start client → visibly connect → guest login → enter route → move +
aim with real controls → fight a normal pack and one telegraphing
enemy → see outgoing hit, incoming hit, kill, drop → pick up a named
item → equip it → see the authoritative stat/behavior change → extract
→ bank → both processes shut down cleanly. Forbidden in the accepted
run: dev grants, direct sim mutation, preloaded state, browser client,
silent local fallback, scenario-only shortcuts.

## Gate B — Chronicles + persistence (after N5)

Create/select House → create Scion (optional mortal oath) → expedition
→ die permanently → see consequences → successor → inspect
crypt/relic/recovery → quit → relaunch → reconnect → persisted House
state proven.

## Gate C — campaign decision (after N6)

Route choice made on concrete info: goal, boss/danger, expected
trophy/material/item family, depth, branch consequence,
extraction/return condition. A route name alone fails.

## Quality rubric (every manual gate; architect + owner verdicts)

Score 0–2 each: input response · combat legibility · reward clarity ·
navigation clarity · UI hierarchy · visual cohesion. Pass = no zeroes
AND ≥9/12. Tests prove emission; this gate proves the result is usable.

## Protocol coverage matrix

`docs/rebuild/NATIVE_CLIENT_PROTOCOL_MATRIX.md` (created with the C3
scaffold) maps journey step → input → outbound event → server handler
→ authoritative response → ClientModel update → presentation evidence
→ automated test, for: connect, login, initial state, zone entry,
movement, aiming, primary/secondary action, telegraph, damage, death,
drop, pickup, inventory, equip, extraction, disconnect, reconnect,
persistence (post-N5).

## CI (phase 3)

Native workflow gains: local client scenarios → start server on
ephemeral loopback → remote handshake → remote guest journey → clean
shutdown; failure artifacts (logs, protocol transcript, final render
list, exit codes). Post-N5: reconnect + persistence restore.

## Throughput metric

Complete player journeys moved red→green in the native executable —
not merged handlers, effects, panes, or task counts.
