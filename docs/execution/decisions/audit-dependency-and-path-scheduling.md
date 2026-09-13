# VG-GOV-008 — Audit dependency and path scheduling

Draft evidence 2026-09-06. Not an owner VG-GOV-002 stamp. Planning IDs
stay DRAFT; this does not mint TASK numbers.

## Commands (from `docs/execution/pack/`)

```
python tools/roadmap.py validate
python -m unittest discover -s tools -p "test_*.py" -v
```

## Results

- `validate`: `valid=true`, `goals=200`, `edges=689`, `draft=200`.
  Warning: structural plan validation only; no game implementation implied.
- unittest: 20 tests OK, including `test_cycle`, `test_directory_overlap`,
  `test_glob_overlap`, `test_missing_dependency`, `test_shared_resource_conflict`,
  `test_uncertain_globs_conservative`.
- Negative control: wildcard overlap and shared integration hooks fail those
  fixtures; they are not treated as independent.

`wave` still returns no READY work (edition is all DRAFT). `--planning` is
NOT_CLAIMABLE.

VG-GOV-002 remains unstamped; this audit uses the pack validator as the
read-only graph tool named by the goal.
