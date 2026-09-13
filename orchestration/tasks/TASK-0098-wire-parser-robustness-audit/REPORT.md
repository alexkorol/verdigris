# TASK-0098 — REPORT

- Lane: `ox-pc-bc` · Model: `openrouter/stealth/ox-alpha`
- Worktree / branch: `Z:\Code\.worktrees\verdigris\ox-pc-bc` on `worker/verdigris/pc/ox-pc-bc`
- Base commit (SPEC provenance): `d2423873c577d299b3b39c56024d1d840993c72b` (verified ancestor of the audited line)
- Claim commit: `5a369bc1` (pushed to origin before any audit work)
- Audited tree: program tip `c274dafe` sources (lane fast-forwarded `cc85786f → c274dafe` before editing; zero unique local commits were discarded)
- Deliverables: this report, `FINDINGS.md`, `captures/parser-cases.json`, `captures/acceptance-rg-transcript.txt`

## Executive summary

Completed the bounded wire-parser robustness audit of the native server's network boundary. The transport layer is well bounded (loopback-only bind, 8 KiB handshake cap, 16 KiB frame cap with mandatory masking, strict envelope shape). Against that baseline the audit surfaced **two crash-class red candidates sharing one root cause — unbounded client-controlled recursion**: (F-A) JSON nesting depth in `JsonParser`, and (F-B) road-node tier recursion via `web_tier_width`. Both have full static source-to-sink paths and **zero test coverage**; both are escalated privately per SPEC (no exploit payloads published; dynamic confirmation intentionally not attempted under the read-only/no-fuzzing capsule). Twelve red candidates total are inventoried with conservative severity + preconditions; 30 parser cases map every boundary to a test, an expected-pass gap, or a red candidate. The SPEC-required negative control is delivered (deep-nesting case lacking a test, not marked safe).

## Approach

1. AGENTS.md preflight: clean tree, branch verified, `git fetch --prune origin`, upstream reconciliation (fast-forward only), no competing STATUS/RELEASE for TASK-0098.
2. Full read of `native/src/networking.cpp` (2,992 lines) and `native/include/verdigris/networking.hpp`; targeted verification in `core.cpp` (`House::route_unlocked` 132–134, `resolve_enter` 540–541) to confirm ordering assumptions.
3. Coverage survey of `native/tests/networking_tests.cpp` (450 lines, full read) and `native/tests/session_tests.cpp` via targeted search — no malformed-input/nesting/rate coverage exists anywhere.
4. Static source-to-sink analysis of nine boundary classes; each mapped to a test or a red candidate in `captures/parser-cases.json`.
5. All four acceptance gates run literally, twice (pass 1 below; pass 2 over the final tree recorded in STATUS.md).

## Changed files (owned paths only)

All under `orchestration/tasks/TASK-0098-wire-parser-robustness-audit/`:

- `STATUS.md` — claim (rev 1) then REVIEW_REQUESTED flip (rev 2)
- `FINDINGS.md` — boundary inventory + findings F-A…F-K with citations
- `captures/parser-cases.json` — `verdigris.audit.parser-cases` v1, 30 cases
- `captures/acceptance-rg-transcript.txt` — byte-exact gate-1 transcript

No other path was touched. No source files modified; no security policy changed; no ports opened; port 6500 never touched.

## Acceptance gate transcripts (pass 1, literal)

Pass 1 ran with evidence files present via `git add -N` so untracked deliverables appear in the diff gates. Gate 1's stdout was captured verbatim to `captures/acceptance-rg-transcript.txt`.

### Gate 1 — surface scan

Command (verbatim from SPEC):

```
rg -n "parse|payload|event|rate|auth|limit|invalid|unknown|close|error" native/src/networking.cpp native/include/verdigris/networking.hpp native/tests/networking_tests.cpp native/tests/session_tests.cpp
```

- Exit code: `0`
- Matching lines: `457`
- Transcript sha256: `C1C385D6C62C8EA927588283EF789D84C617E904CBF6DF15F0633FE836A19444`
- Byte-exact transcript committed at `captures/acceptance-rg-transcript.txt`. Excerpts (first 4 / last 4 lines):

```text
native/tests/networking_tests.cpp:14:using verdigris::networking::parse_envelope;
native/tests/networking_tests.cpp:18:  if (!condition) throw std::runtime_error(message);
native/tests/networking_tests.cpp:25:  std::string error;
native/tests/networking_tests.cpp:26:  check(parse_envelope(wire, decoded, &error), error.c_str());
...
native/src/networking.cpp:2971:  if (envelope.event == "party:returnToTown") {
native/src/networking.cpp:2984:void WebSocketServer::handle_message(const std::shared_ptr<Connection>& connection,const std::string& text){ Envelope envelope; std::string error;if(!parse_envelope(text,envelope,&error))return; ...
native/src/networking.cpp:2988:const bool quick=as_bool(envelope.data.get("quickGuest")); ... [full line in transcript]
native/src/networking.cpp:2989:void WebSocketServer::broadcast(const Envelope& envelope){ ... }
```

### Gate 2 — machine-readable cases parse

Command (verbatim from SPEC):

```
node -e "JSON.parse(require('fs').readFileSync('orchestration/tasks/TASK-0098-wire-parser-robustness-audit/captures/parser-cases.json','utf8')); console.log('parser cases: PASS')"
```

Output (literal):

```text
parser cases: PASS
```

- Exit code: `0`

### Gate 3 — whitespace hygiene

Command (verbatim from SPEC):

```
git diff --check
```

Output: empty (no whitespace errors).

- Exit code: `0`

### Gate 4 — scope check

Command (verbatim from SPEC):

```
git diff --name-only
```

Output (literal):

```text
orchestration/tasks/TASK-0098-wire-parser-robustness-audit/FINDINGS.md
orchestration/tasks/TASK-0098-wire-parser-robustness-audit/captures/acceptance-rg-transcript.txt
orchestration/tasks/TASK-0098-wire-parser-robustness-audit/captures/parser-cases.json
```

- Exit code: `0`
- Expected "only task evidence changes": satisfied — all three paths are inside owned_paths. (REPORT.md did not exist yet at pass 1; pass 2 in STATUS.md covers the final tree.)

## Findings digest (details in FINDINGS.md)

| ID | Case | Severity | One-line |
|---|---|---|---|
| F-A | PC-014 | high-if-confirmed (ESCALATED) | JSON nesting depth unbudgeted in JsonParser vs ~1 MiB reader-thread stack; parse is pre-auth |
| F-B | PC-015 | high-if-confirmed (ESCALATED) | `world:zone:enter` node tier drives recursion depth = tier in `web_tier_width`; core leg safely rejects first (core.cpp:540-541) |
| F-C | PC-018 | medium | shop buy trusts client-echoed price incl. ≤0 (free purchases) |
| F-D | PC-019 | medium | bank withdraw/deposit accept negative quantities → stack corruption candidate |
| F-E | PC-017 | medium | dev:give qty unclamped above 1 → near-int-max mint loop from one frame |
| F-F | PC-020 | medium | no rate/flood gates; sessions_ never pruned; tick thread advances dead sessions forever |
| F-G | PC-016 | medium | strtod inf/nan/out-of-range doubles cast to int unchecked (UB); dev:setlevel overflow math |
| F-H | PC-013 | medium (accepted-risk) | guestId replay takes over any session; documented JS-parity design behind loopback bind |
| F-I | PC-005/006 | low | malformed JSON + unknown events dropped silently; zero telemetry, unpinned by tests |
| F-J | PC-011 | low | pong length header truncates >125-byte ping echoes (self-desync only) |
| F-K | PC-025 | low | duplicate keys first-wins vs JS last-wins parity divergence |

Negative control delivered: **PC-014** — a malformed deep-nesting frame case that currently lacks any test, explicitly not marked safe. Per SPEC ("stop and privately escalate a credible high-impact reachable flaw"), F-A and F-B are flagged as escalations in FINDINGS/STATUS rather than treated as ordinary backlog candidates; remediation sketches are included but nothing was implemented (capsule forbids code changes outside owned paths).

## Manual verification

Static-only by design: the resource capsule forbids live traffic/fuzzing/ports, so crash-class claims are explicitly labeled *pending owner-approved dynamic confirmation*. Every red candidate cites a complete file:line path from source to sink; severity wording carries its preconditions (loopback reach, post-login, etc.).

## Public interfaces added/changed

None. Read-only audit; no API, protocol, policy, or behavior change.

## Deviations and process notes

- Branch reconciliation: lane fast-forwarded from `cc85786f` to program tip `c274dafe` before editing (pure fast-forward; local had zero unique commits; remote worker tip `0c373d2f` was an ancestor, so pushes fast-forward without force). SPEC `base_commit d2423873` remains the immutable provenance anchor exactly as in accepted sibling TASK-0100 (same pinned base).
- Pre-commit hooks ran normally (node_modules present; lint-staged matched nothing staged of ours). No `--no-verify` used anywhere.
- Pass-1 gate 4 could not list REPORT.md because it is assembled from pass-1 transcripts; pass 2 over the final tree (recorded in STATUS.md) covers it.

## Unresolved questions / follow-ups for the owner

1. Approve (or decline) a dynamic confirmation harness for F-A/F-B; if confirmed, smallest fixes are a JsonParser depth budget and a tier clamp + iterative `web_tier_width`.
2. Decide whether silent-drop telemetry (F-I) should exist even as counters.
3. Policy decision on identity takeover (F-H): keep documented anonymous-guest parity or add a gate.
4. Backlog candidates if desired: price authority (F-C), bank qty sign checks (F-D), give clamps (F-E), session eviction + rate budget (F-F).

## Risks

- Crash-class candidates remain unconfirmed dynamically; if real they kill the whole server process from one frame, but exposure is limited to hosts that can already reach the loopback bind.
- None of this changes runtime behavior today; risk of this audit itself is nil (evidence-only commit).

## Commit SHAs

- Claim: `5a369bc1` (pushed)
- Content head (this evidence set): see STATUS.md `content_head` (the commit adding FINDINGS/captures/REPORT)
- Frozen pushed head: branch tip of `worker/verdigris/pc/ox-pc-bc` after pushing the REVIEW_REQUESTED flip (recorded in STATUS.md and reported to the operator)
