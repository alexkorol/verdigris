# OI-007 — batched campaign, season, and travel choices

**State:** PARKED-NONCRITICAL. **Deadline:** before authored multi-act scale,
season reset release, or fast-travel implementation.

Decisions required: optional-branch density supporting the measured 6-30 hour
range; fast-travel/town-portal risk and destination rules; seasonal inheritance.
Recommended: wait for TASK-0096 measurements, then choose a minimum critical
route plus optional branches; make travel a visible risk-bearing command; keep
season history small and versioned until one playable season prototype exists.

Viable alternatives: campaign slice without seasons/travel; or travel seam with
no production destinations until the risk model is approved. Acceptance:
House-owned completion, optional branches matter, no checklist campaign,
disconnect cannot become travel, and reset/migration is explicit. No assets are
requested. Fallback: schema, graph validation, Gate C journey, content tooling,
and non-seasonal progression continue.
