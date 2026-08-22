# OI-004 — authoritative passive-tree source

**State:** WAITING_EVIDENCE on TASK-0105. **Deadline:** before TASK-0112 moves
from schema/scaffold to node-content implementation.

Decision required: provide/approve the authoritative topology and node-content
source that replaces the current hex-axis/+2 approximation.

Recommended choice: a versioned machine-readable graph with stable node IDs,
edges, start positions, attribute/effect references, migration version, and a
separate owner-controlled balance table.

Viable alternatives: approve a smaller authored production tree for the first
campaign; or keep the approximation explicitly prototype-only while UI and
persistence contracts are built. Do not silently promote the approximation.

Acceptance rubric: deterministic parse, connectedness checks, stable IDs,
migration behavior, two-counter quest-point semantics preserved, and no balance
embedded in renderer/UI code. No assets are requested. Fallback: audit,
versioned schema, parser tests, persistence seams, and pane layout continue.
