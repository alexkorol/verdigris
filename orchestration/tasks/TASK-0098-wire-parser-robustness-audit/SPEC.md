---
task: TASK-0098
title: Native wire parser robustness and abuse-boundary audit
state: SUPERSEDED
superseded_by: integrated (reviewed head 48a9d487, 2026-08-23)
packet: MECHANICAL
topology: INDEPENDENT
job: MECHANICAL
priority: P1
base_commit: d2423873c577d299b3b39c56024d1d840993c72b
owner_visible_contribution: keeps malformed or hostile clients from crashing or corrupting the playable native server
dependencies: []
owner_input_dependency: none
owned_paths: [orchestration/tasks/TASK-0098-wire-parser-robustness-audit/**]
forbidden_paths: [everything else]
resource_capsule: read-only; no live fuzzing and no ports
---

# Outcome

Produce `FINDINGS.md` and `captures/parser-cases.json` inventorying envelope
parsing, type/range/size checks, authentication/rate gates, unknown events,
malformed JSON, disconnect cleanup, and deterministic error behavior. Map each
boundary to a current test or a red candidate case; no vulnerability claims
without a reachable source-to-sink path.

# Frozen invariants and evidence

Protocol compatibility and harness assertions are frozen. Cite exact parser,
handler, and test lines. Do not send traffic, publish exploit payloads, or
change security policy. Report severity conservatively with preconditions.

# Acceptance

```powershell
rg -n "parse|payload|event|rate|auth|limit|invalid|unknown|close|error" native/src/networking.cpp native/include/verdigris/networking.hpp native/tests/networking_tests.cpp native/tests/session_tests.cpp
node -e "JSON.parse(require('fs').readFileSync('orchestration/tasks/TASK-0098-wire-parser-robustness-audit/captures/parser-cases.json','utf8')); console.log('parser cases: PASS')"
git diff --check
git diff --name-only
```

Expected: only task evidence changes. Negative control: include one malformed
case lacking a test and do not mark it safe. Stop and privately escalate a
credible high-impact reachable flaw; otherwise continue the bounded audit.
