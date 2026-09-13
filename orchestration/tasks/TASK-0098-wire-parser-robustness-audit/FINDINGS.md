# TASK-0098 — FINDINGS: native wire parser robustness and abuse-boundary audit

- Lane: `ox-pc-bc` · Model: `openrouter/stealth/ox-alpha`
- Base commit (SPEC provenance): `d2423873c577d299b3b39c56024d1d840993c72b` (verified ancestor of the audited line)
- Audited head: `5a369bc1` (claim commit; source tree identical to program tip `c274dafe`, which this lane fast-forwarded to before editing)
- Machine-readable companion: `captures/parser-cases.json` (`verdigris.audit.parser-cases` v1, 30 cases)
- Method and capsule compliance: **static source-to-sink review only.** No traffic was sent, no live fuzzing performed, no ports opened, port 6500 untouched. No exploit payload strings are recorded; input shapes are described abstractly by policy. Severity is reported conservatively with explicit preconditions.

## Executive summary

The wire surface is bounded well at the transport layer — loopback-only bind, an 8 KiB handshake cap, a 16 KiB frame cap with mandatory masking, and a strict envelope shape check (`event` non-empty string, `data` object). Handler bodies are pervasively defensive: every `JsonValue` read goes through null-safe accessors, and both `std::stoi` sites are exception-guarded. Malformed JSON and unknown events are dropped deterministically without mutating state.

Against that baseline, the audit found one root-cause class that defeats those bounds: **unbounded recursion driven by client-controlled counts**. Two crash-class red candidates share it:

1. **PC-014 / F-A — JSON nesting depth.** `JsonParser` descends recursively with no depth budget; nesting is limited only by the 16 KiB frame cap, and parsing happens pre-authentication on a default-stack reader thread.
2. **PC-015 / F-B — road-node tier recursion.** A valid envelope carrying a large integer in a road node id reaches `web_tier_width`, whose recursion depth equals that wire-supplied integer.

Both lack any test coverage anywhere in `native/tests`. Dynamic confirmation was intentionally not attempted (read-only capsule forbids live probing); per SPEC these are escalated privately here as credible high-impact reachable flaws pending an owner-approved confirmation harness.

Below crash class, the audit found medium integrity/abuse candidates (client-authoritative shop price, negative bank quantities, unclamped mint loops, absent rate gates, unbounded session registry) and low observability/parity gaps (silent drops with no telemetry, duplicate-key first-wins divergence, oversized pong header bug). The negative control required by SPEC is delivered as PC-014: a malformed case that currently has no test and is explicitly not marked safe.

## Boundary inventory

| # | Boundary | Verdict | Primary citations |
|---|---|---|---|
| B1 | Envelope parsing | Strong shape checks; recursion depth unbudgeted | networking.cpp:94-201, 566-572 |
| B2 | WebSocket framing + size | Cap/mask/opcode handling sound; pong length-header defect for >125-byte pings | networking.cpp:2828 |
| B3 | Auth / identity | None beyond loopback bind + guest convention; takeover by guestId replay is documented JS-parity design | networking.cpp:2984-2988, 2811 |
| B4 | Rate / resource gates | Absent entirely | rg evidence; 2811, 2822, 2990 |
| B5 | Type/range/size in handlers | Silent-fallback helpers everywhere (safe); several range gaps (price, qty signs, numeric extremes, node tier) | networking.cpp:204-212 and sinks per case |
| B6 | Unknown events | Deterministic silent fall-through, unpinned by wire tests | networking.cpp:2379-2780 |
| B7 | Malformed JSON | Deterministic silent drop pre-auth, unpinned and unlogged | networking.cpp:2984, 98-104 |
| B8 | Disconnect cleanup | Correct join-not-detach shutdown; session registry never pruned by design | 2782-2803, 2805-2826, 2990; hpp 278-281 |
| B9 | Deterministic errors | Same input → same behavior on every path reviewed; no exception escapes the dispatcher | 811-814, 1177, 1116-1120 |

## Findings

### F-A (PC-014) — ESCALATION, crash-class: JSON nesting depth vs reader-thread stack

- Path: reader thread `handle_connection` → `handle_message` (networking.cpp:2828→2984) → `parse_envelope` → `JsonParser::parse` → mutual recursion `value()` ⇄ `object()`/`array()` (networking.cpp:118-197).
- Bound: only the frame cap `length>16384` (networking.cpp:2828). There is no depth counter anywhere in JsonParser.
- Why it matters: parsing occurs before any session/authentication state, so any connected client can reach it. MSVC default thread stack (~1 MiB) against up to ~16 K nesting levels makes stack exhaustion plausible; an access violation on a reader thread terminates the process.
- Coverage: none. `rg` across `native/tests` shows no depth/nesting case.
- Conservative claim: *credible, statically reachable crash candidate; dynamic confirmation intentionally not performed under this capsule.* Remediation sketch: hard depth budget in JsonParser (+ iterative descent or early reject), regression test for deep nesting.
- This case doubles as the SPEC-required **negative control**: malformed input lacking a test, not marked safe.

### F-B (PC-015) — ESCALATION, crash-class: client-supplied node tier drives unbounded recursion

- Path: `world:zone:enter` handler (networking.cpp:2386) → `parse_node_id` accepts any `tier>=1 && index>=0` within int range (networking.cpp:805-816, stoi guarded) → `enter_road_node` (1517) calls `web_road_nodes(house, road, tier+1)` (1521) → per-tier `web_tier_width` recurses depth = tier (756-762); the node-building loop also iterates `max_tier` times (763-804).
- Ordering note: the core-simulation leg of the same event is safe — unknown routes are rejected by `House::route_unlocked` (core.cpp:132-134) inside `resolve_enter` (core.cpp:540-541) — so control always reaches the web path when the id parses.
- Impact claim (conservative): same process-wide crash family as F-A plus memory amplification from the loop; precondition is a single post-login frame from any local client.
- Coverage: none. Zone tests use small valid ids only (networking_tests.cpp:44-47). Remediation sketch: clamp tier to the chart frontier or a small constant in `parse_node_id`; make `web_tier_width` iterative/memoized; add hostile-id regression test.

### Medium findings (integrity/abuse; all with preconditions)

- **F-C (PC-018)** Shop buy trusts the client-echoed price: `player:shop:buy` reads `price` back from the payload (networking.cpp:1953) and only gates `carried_gold() >= price` (1954); zero/negative prices grant items with no deduction. Guest-economy trust boundary inherited from menu echo parity; server-authoritative pricing expectation violated.
- **F-D (PC-019)** Bank withdraw/deposit accept negative quantities: withdraw inserts a negative-qty item into inventory while inflating the bank stack (1985-1999); deposit mirrors it (2002-2009). Deterministic corruption candidate, no crash sink found; untested.
- **F-E (PC-017)** `dev:give` upper quantity unclamped for non-stackable items: per-unit create/add/spill loop (1063, 1074-1087) can run near-int-max iterations from one ≤16 KiB frame; ground-item memory grows unbounded.
- **F-F (PC-020)** No rate/flood control anywhere: every accepted frame runs handlers synchronously (combat events inline at 2507); tick thread advances every session forever including disconnected ones because `sessions_` is never pruned outside `stop()` (2811, 2822, 2990). Each identity allocates two simulations (577-580), so unique-id logins grow memory monotonically.
- **F-G (PC-016)** Numeric extremes: strtod unchecked (128-134; platform accepts inf/nan spellings), `as_int` casts double→int without range guard (204-206) — formally UB out-of-range; sinks include dev:setlevel (2390, whose `100+level*10` can overflow), dev:teleport (2389), itemLevel (1070).
- **F-H (PC-013)** Identity takeover by guestId replay evicts the prior socket and adopts its account state (2984-2988). Documented anonymous-guest JS parity; mitigated by loopback-only bind (2811). Listed as an accepted-risk boundary needing an explicit policy decision, not a code change.

### Low findings

- **F-I (PC-005, PC-006)** Malformed JSON and unknown events are silently dropped with no error envelope, counter, or log (2984; 2379-2780 fall-through). Behavior is deterministic and safe but gives operators zero hostile-client telemetry, and the silence itself is pinned by no wire-level test (the presentation-layer analogue at audio_mixer_tests.cpp:183-207 is a different layer).
- **F-J (PC-011)** Pong echoes pings larger than 125 bytes under a single-byte length header (2828): self-desync on the offending connection only.
- **F-K (PC-025)** Duplicate object keys resolve first-wins via `emplace` (180) vs JS `JSON.parse` last-wins: adversarial-input parity divergence only.

### Strengths verified (expected-pass boundaries)

Strict envelope shape enforcement incl. array-data rejection (567-572; tested at networking_tests.cpp:21-31); trailing-data rejection (100-103); frame cap before allocation + mandatory masking (2828); handshake key/header caps (2828); house-deposit positive-amount and overdraft guards (1317-1319); take-ground binding and chebyshev reach checks (1709, 1713); equip-miss error path with no mutation (1116-1120; tested 414-430); unknown movement direction no-op (tested 116-121); guarded stoi sites (811-814, 1177); join-not-detach shutdown with disconnect/reconnect coverage in session_tests.cpp:95, 192-195, 401-443.

## Coverage map summary (details in captures/parser-cases.json)

- 30 cases inventoried: 6 covered by current tests, 11 expected-pass gaps (correct code, no pinning test), 12 red candidates, 1 designated negative control (not marked safe).
- Every boundary B1–B9 maps to at least one test or red candidate as SPEC requires.
- Red candidates carry severity + preconditions + reachable path citations; none claims more than static analysis supports.
