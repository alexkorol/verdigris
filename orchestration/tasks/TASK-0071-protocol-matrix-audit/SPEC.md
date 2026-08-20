---
task: TASK-0071
title: Protocol matrix completion audit (every row → a named test)
state: READY
packet: MECHANICAL
lane: mac-claude suggested (docs/audit; no native build needed)
priority: medium (coordination truth for Gate B planning)
owned_paths:
  - docs/rebuild/NATIVE_CLIENT_PROTOCOL_MATRIX.md
  - orchestration/tasks/TASK-0071-protocol-matrix-audit/**
forbidden_paths:
  - everything else (read-only audit outside the matrix)
---

# Outcome

The matrix reflects post-0063/0064/0068/0069 reality: every ✅ row
names its exact test (file + check label) in the Automated-test column;
rows now covered (movement echo, telegraph, damage, death, drop,
pickup, equip, extraction, reconnect) get upgraded from 🧩/⬜ with
their test names; rows still open get a one-line gap note (what's
missing and which task family owns it). Add a Gate B section listing
the N5 envelopes the client will need (from the 0056 SPEC surface:
House/Scion lifecycle, death, successor, persistence) as ⬜ rows.

Verification is by reading the session/networking test sources on the
current tip — cite line labels, do not run Windows builds (Mac lane).

# Acceptance

Matrix accurate against tip (architect spot-checks 5 rows), no
non-matrix files touched, gap list actionable.
