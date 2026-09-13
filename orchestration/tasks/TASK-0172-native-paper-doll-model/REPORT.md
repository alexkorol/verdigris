# TASK-0172 report — paper-doll equipment model

## References

- WIZARD `rpg_inventory/index.html` `DEFAULT_EQUIPMENT` slot set
- Kind→slot mapping from `taxonomySlotsForKind`

## Deliverable

`native/client/paper_doll.hpp` — 14 slots, kind compatibility, equip/replace/unequip,
two-handed main-hand vs off-hand conflicts, ring auto-slot, stable serialization order.

## Evidence

```text
run-tests.ps1 -> 39 checks PASS
check_legacy_denylist -> PASS
```

## Residual

Not wired to gear overlay. Integration: TASK-0184.

## Successor

TASK-0184 inventory/paper-doll integration (0171+0172+0180+0182).
