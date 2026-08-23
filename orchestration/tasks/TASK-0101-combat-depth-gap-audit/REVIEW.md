# REVIEW — TASK-0101 combat depth and feel gap audit

- verdict: ACCEPTED
- reviewed frozen worker head: `a742355d189966f0e344d0c4763e014f87ecb820`
- superseded prior head: `7794883eb98f69eb1203d22221774b75fbaebb41`
- reviewed base: `610a240e1e4bdfacfd77bec49e36be945a1ced13`
- answers program REVISE: `1a434371b281494d3f5aa6bdc3e50447e1814855`
- reviewer: Cursor successor architect/orchestrator
- reviewed_at: 2026-08-22T17:08:00-07:00

Revision 1 is accepted. The three numbered REVISE items are closed on the
frozen pushed head. Independent verification on
`codex/TASK-0101-combat-depth-gap-audit-ox-pc-ai` confirmed:

- local and remote heads both equal `a742355d`; worktree clean;
- `captures/combat-matrix.json` parses (`combat matrix: PASS`);
- `git diff --check 610a240e..HEAD` exits 0 over the committed artifact range;
- `git diff --name-only 610a240e..HEAD` is only the owned TASK-0101 folder;
- vocabulary sweep exits 0 (766 matching lines);
- designated negative control `rg -n -i combo` over native include/src/client/tests
  exits 1 (family absent, not generic-attack parity);
- W1 owned paths now include `core.hpp` WorldCombatEvent fields,
  `presentation_state.cpp` event→FX translation, `render_list.hpp`
  Telegraph/Damage/Impact ops, `main.cpp` painters, and `session_tests.cpp`;
- W1 `readable_lock` requires a deterministic session/client transcript: every
  ranged hit is preceded by a Telegraph render op and lands as a Damage/Impact
  op attributed to the ranged attacker; if readability is split out,
  GAP-TELEGRAPH-CATALOG routes first and invisible ranged damage must not route;
- cited ranges were spot-checked on the frozen tree (`core.hpp` 801–824,
  `core.cpp` 2012–2022 melee-only branch, `render_list.hpp` 22–30,
  `presentation_state.cpp` 196–260, `session_tests.cpp` 284–370).

This is a static audit. Integrate only the accepted worker evidence. Do not
promote the W1 implementation packet to READY until TASK-0161 releases
`native/client/main.cpp` (current frozen 0161 head still owns that path).
No numeric retune, projectile art, or new skill design is authorized.
