# TASK-0082-dual-server-matrix — STATUS

- state: CLAIMED
- lane: ox-pc-bf
- model: openrouter/stealth/ox-alpha
- base SHA: d2423873c577d299b3b39c56024d1d840993c72b
- claimed at (UTC): 2026-08-23T00:00:00Z
- branch: worker/verdigris/pc/ox-pc-bf
- ports capsule: 6540–6559 (acceptance run uses 6541 JS / 6542 native; never 6500)

## Plan

1. Implement `playtest/tools/dual-server-matrix.mjs` (owned path) that:
   - spawns a fresh JS server (`node server/index.js`) on `--js-port` and a
     fresh native server (`--native-exe`) on `--native-port`, loopback only;
   - gives each server isolated temp save paths;
   - runs the unchanged `playtest/run.mjs --attach` scenarios serially against
     each server (no runner/assertion changes);
   - writes one comparison JSON artifact to `--out` recording revision,
     executable path/hash, URLs, exact child commands, per-server scenario
     pass/fail/duration, and final parity status;
   - exits non-zero on any red or asymmetric scenario;
   - kills only the children it spawned, always.
2. Run acceptance commands literally; paste transcripts in this file.
3. Demonstrate one authentic negative (nonexistent scenario argument), capture
   non-zero exit, restore before commit.
