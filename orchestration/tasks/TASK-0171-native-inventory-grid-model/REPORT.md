# TASK-0171 report — Diablo-style inventory grid model

## References

- WIZARD `tools/rpg_inventory` — main backpack `12x6` horizontal layout (test.js contract)
- Owner Demo handoff — real grid backpack with item art (integration deferred to TASK-0184)

## Deliverable

`native/client/inventory_grid.hpp` — pure 12×6 grid with:

- Multi-cell footprints, bounds/overlap rejection, move/swap
- Stack count/max metadata preserved on move
- Hover cell → item id lookup
- Stable ids, deterministic `serialization_order()` by ascending id
- 48-item capacity bound

## Commands

```text
run-tests.ps1  -> exit 0, 92 checks passed
check_legacy_denylist -> PASS
git diff --check -> clean
```

## Residual gaps

- Not wired to gear overlay / `main.cpp`
- No rotation (D2-style fixed orientation per SPEC fallback)
- Item art adapter (TASK-0182) not connected

## Successors

- TASK-0184 grid inventory / paper-doll integration (depends 0171+0172+0180+0182)
