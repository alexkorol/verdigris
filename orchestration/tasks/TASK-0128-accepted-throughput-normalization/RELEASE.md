# TASK-0128 fresh-lane revision release — 2026-08-22 09:40 PDT

Released worker: `ox-pc-a` after pushed revision head
`d247638e34ea9f27de98d6fc316fd3361fb75427`.

This is not a dark-lane incident. The original worker completed its permitted
recovery and pushed a reviewable correction, then exhausted its automatic
recovery budget. Its branch and worktree remain preserved and are not active
capacity.

The architect's revision review accepts the collector architecture,
null/UNKNOWN discipline, exact experimental-unit key, source-revision fix, and
19-test foundation. Integration remains blocked only because a test run rewrites
six tracked fixture golden outputs (`skipped_folders` changes to the collector's
current canonical value).

Fresh lane `ox-pc-y` is authorized to branch from exact preserved head
`d247638e34ea9f27de98d6fc316fd3361fb75427` as
`codex/TASK-0128-runway-golden-revision-ox-pc-y`, replace the old STATUS claim,
and make only the review-requested revision:

1. regenerate and commit every affected fixture output using the existing
   corrected collector;
2. prove `node --test orchestration/throughput/*.test.mjs` exits zero and leaves
   `git status --short` empty;
3. retain the final-head/source-revision positive control and both relevant
   evidence-change/tamper negative controls;
4. rerun every literal SPEC gate and request review without guessing a runway
   rate or changing collector semantics.

Owned paths remain exactly those in SPEC. No game source, generated backlog
manifests, another task folder, ports, or port 6500 may be touched. The new lane
must push only its worker branch; this release does not authorize integration.
