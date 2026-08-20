# TASK-0061 review — ACCEPTED (mechanics) / Gate A remains RED

Architect rerun 2026-08-20 ~04:00, candidate merged on tip (clean):

- build.ps1 -RunTests -RunClientScenarios: all suites green, incl. the
  full remote journey (handshake -> zone -> move -> fight -> kill ->
  named pickup -> equip -> telegraph -> incoming hit -> stairs extract
  -> clean dual shutdown), mid-session server-kill negative (visible
  Disconnected, no local revival), and session-replaced handling.
- Architect PLAY PASS: drove --remote against verdigris_server on
  architect port 6566 (drive script + PrintWindow captures). The
  journey is REAL: instance zone, kills logged, loot named, HP/strike
  tracked, coins in backpack.
- Server gaps filed as notes, not rule edits — correct discipline.
  No server/**, src/**, playtest/** changes in the diff.

Quality rubric (remote window): input 2 · combat legibility 1 ·
reward clarity 1 · navigation 1 · UI hierarchy 1 · **visual cohesion 0**
-> **Gate A NOT passed** (no-zeroes rule). The remote window is a
dedicated debug painter (dot + squares + text log), not the C1/C2
presentation. Integrating the protocol/session mechanics now (tested,
non-regressive, unblocks parallel work); Gate A stays RED until
TASK-0064 (remote presentation unification) passes the rubric.

Numbered correction carried into 0064: remote mode must render through
the SAME presentation pipeline as local play (render_list painter,
billboards, combat effects, HUD, camera) — one presentation, two
sessions, per the D-122 architecture.
