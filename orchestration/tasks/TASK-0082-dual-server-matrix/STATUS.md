# TASK-0082 status

```yaml
state: CLAIMED
task: TASK-0082-dual-server-matrix
lane: ox-pc-bb
model: openrouter/stealth/ox-alpha
coordinator: codex (worker lane)
base_commit: d2423873c577d299b3b39c56024d1d840993c72b
branch: worker/verdigris/pc/ox-pc-bb
worktree: Z:/Code/.worktrees/verdigris/ox-pc-bb
started_at: 2026-08-23T00:00:00Z
resource_capsule: loopback 6540-6559
```

## Plan

Implement `playtest/tools/dual-server-matrix.mjs` per SPEC: wrapper spawns a
fresh JS server (`node server/index.js`, hermetic save paths, capsule port) and
a fresh native server (`native/build/verdigris_server.exe <port>`), serially,
then drives the unchanged `playtest/run.mjs --attach` suite against each and
writes one parity JSON artifact. Kills only its own children.
