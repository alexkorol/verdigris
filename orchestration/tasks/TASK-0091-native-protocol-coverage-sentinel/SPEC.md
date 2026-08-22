---
task: TASK-0091
title: Native client protocol coverage sentinel design
state: READY
packet: MECHANICAL
topology: INDEPENDENT
job: MECHANICAL
priority: P1
base_commit: d2423873c577d299b3b39c56024d1d840993c72b
owner_visible_contribution: prevents a green native journey from silently losing a required wire step
dependencies: []
owner_input_dependency: none
owned_paths: [orchestration/tasks/TASK-0091-native-protocol-coverage-sentinel/**]
forbidden_paths: [everything else]
resource_capsule: read-only; no ports or servers
---

# Outcome

Produce `FINDINGS.md` and `captures/coverage.json` mapping every row of
`NATIVE_CLIENT_PROTOCOL_MATRIX.md` to native handler, client reducer/model,
presentation op, and automated test. Classify rows COVERED, PARTIAL, or RED and
propose the exact interface for a later read-only sentinel; do not implement it.

# Frozen invariants and evidence

The `{event,data}` envelope, current scenario assertions, D-122 journey gates,
and authoritative server ownership are frozen. Cite file:line plus test label
for every green cell. Missing evidence stays red. Required report: base SHA,
literal transcripts, changed-file list, JSON, and a successor recommendation.

# Acceptance

Run and record exit codes:

```powershell
rg -n "Gate A|Gate B|Gate C|chronicles:|world:|player:" docs/rebuild/NATIVE_CLIENT_PROTOCOL_MATRIX.md
rg -n "RemoteProtocolSession|ClientSnapshot|PresentationEvent|render_list" native/client native/tests
node -e "JSON.parse(require('fs').readFileSync('orchestration/tasks/TASK-0091-native-protocol-coverage-sentinel/captures/coverage.json','utf8')); console.log('coverage JSON: PASS')"
git diff --check
git diff --name-only
```

Expected: commands succeed; only the owned task folder changed. Negative
control: include at least one RED/PARTIAL row and the literal no-match or
missing-test evidence that prevents a false green.

# Stop and fallback

Stop on an ambiguous envelope or source/test contradiction. Do not edit the
matrix or source. While blocked, complete all provable rows and record the
smallest owner path for each red seam.
