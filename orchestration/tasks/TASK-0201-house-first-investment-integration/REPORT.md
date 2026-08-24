# TASK-0201 house investment layout prep report

## Deliverable

`native/client/house_investment_layout.hpp` — plans countinghouse investment
dialog from town steward anchor and `house_progression` eligibility.

## Evidence

- `orchestration/tasks/TASK-0201-house-first-investment-integration/run-tests.ps1` — exit 0
- `python native/tools/check_legacy_denylist.py` — PASS

## Residual gaps

- `main.cpp` choice UI persistence (integrator lease: ox-alpha-pc)
- Full TASK-0201 blocked until TASK-0190 and TASK-0200 ACCEPTED

## Successor

Integrator: show dialog at steward interact rect after first clear.
