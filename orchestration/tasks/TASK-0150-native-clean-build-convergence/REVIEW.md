---
task: TASK-0150
verdict: ACCEPTED
reviewed_head: 544175921eea5a999a2077c3d7c3a6c05d3dfb91
integrated_at: 10039385c0ea58acd18feec0532f6064df9d03d4
---

# TASK-0150 review — ACCEPTED

The four-line owned-path change closes the evidenced mismatch between the
canonical native helper and the CMake/CTest path: CTest now includes the
self-contained camera test and the built client's complete scenario suite.
No gameplay, client, server, launcher, packaging, signing, or owner-port
surface changed.

Independent architect verification at the frozen pushed head passed:

- exact-base ancestry, owned-file inventory, and `git diff --check`;
- `native/build.ps1 -RunTests -RunClientScenarios` with denylist, core,
  networking, camera, session, and all client scenarios green;
- a new disposable VS 2019 CMake/Ninja build (22/22), CTest 5/5, and direct
  `verdigris_client.exe --scenario all` with all seven scenarios green.

Accepted worker implementation `ae54f024`; frozen handoff `54417592`;
integrated implementation `10039385` with report/status at `ad51287b`.

