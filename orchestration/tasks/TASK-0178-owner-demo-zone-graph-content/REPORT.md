# TASK-0178 report

## Deliverable

`native/content/seeds/owner_demo_zones.json` — village-defense prologue, town hub,
two main-route combat zones with bosses, Glimmer Cave dead-end branch, gate labels,
instance metadata (lifetime/allow_fresh), and Cartographer-aligned seeds.

## Evidence

- `orchestration/tasks/TASK-0178-owner-demo-zone-graph-content/run-tests.ps1` — exit 0
- Zone validator + negative control (removed boss) PASS
- Town cross-ref validator PASS after zones landed
- `python native/tools/check_legacy_denylist.py` — PASS

## Residual gaps

No runtime Cartographer adapter (TASK-0191) or multi-zone runtime (TASK-0192).

## Successor

TASK-0188 gate integration, TASK-0191 Cartographer adapter, TASK-0192 multi-zone runtime.
