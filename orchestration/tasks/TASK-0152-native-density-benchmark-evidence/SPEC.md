---
task: TASK-0152
title: Native density benchmark evidence
state: READY
packet: IMPLEMENTATION
topology: INDEPENDENT
job: IMPLEMENTATION
priority: P1
dependencies: []
base_commit: 060c11517d2ebb0aec0c4d4a38c5e3eb53141cb2
owned_paths: [native/tools/entity_density_bench.cpp, orchestration/tasks/TASK-0152-native-density-benchmark-evidence/**]
forbidden_paths: [native/client/**, native/src/**, native/include/**, native/tests/**, native/tools/play-native.ps1, server/**, src/**, CI]
---

# Outcome

Turn the existing density bench into reproducible evidence for the owner-
visible encounter and presentation waves: fixed scenario IDs, hardware/build
provenance, percentile frame/update timings, and a nonzero exit for invalid or
incomplete samples. Do not optimize or change gameplay in this packet.

# Acceptance

Two identical seeded runs agree on counts/checksums, emit complete provenance,
and pass the documented threshold contract; malformed/incomplete evidence
fails; `git diff --check` passes.
