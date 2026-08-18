---
task: TASK-0046
title: Playability re-evaluation on current master (post-friction-wave)
state: READY
priority: high (D-110/D-115 verification; owner-facing)
owned_paths:
  - orchestration/tasks/TASK-0046-playability-reevaluation/**
forbidden_paths:
  - everything else (READ-ONLY evaluation; zero product edits)
base: current master (>= 71b6b20) or program tip
architect_review_required: true
---

## Goal

Answer one question with evidence: after the friction wave (0033
daytime, 0036 inventory, 0037 movement, 0040 first encounter, 0041
death moment) and the 0043 harness work, is the browser game's first
ten minutes actually fun-adjacent — and what is the NEW ranked friction
list? This updates the D-110 definition of "playable" that 0034 set.

## Method (0034's method, environment fixed)

1. Fresh build of the CURRENT tip. Run your own game server on a free
   port. The client MUST be proven connected to it: capture
   `window.ws.url` in the first minute of each arc and include it in
   the log. A dev-mode client dials `ws://<hostname>:6500` regardless
   of page port (src/main.js:30) — build with
   `VITE_WS_URL=ws://localhost:<yourport>` or serve the prod build from
   the game server. NEVER evaluate against the owner's :6500.
2. Two arcs, ~10 minutes each: guest quickstart (`?play`) and
   Chronicles House/Scion (mortal oath on). Timestamped captures,
   beat-by-beat session log, headless-fidelity caveats where honest.
3. For each 0034 friction item (both codex's accepted list and kimi's
   edb75ff list): mark FIXED / SURVIVES / NOT-REPRODUCED with capture
   evidence. New findings ranked blocker/major/minor.
4. Read-only: `git status` scope proof in the REPORT.

## Acceptance evidence

REPORT.md with the verdict paragraph an owner can read in one minute,
the per-item disposition table, ranked new inventory, captures/ with
the two `window.ws.url` proofs, and the scope proof.
