---
task: TASK-0008
state: REVIEW_REQUESTED
branch: codex/TASK-0008-denylist-hardening
commits:
  - 56ac8f0e8b3b12007231db3b51a7387c8f54b1c2
base_commit: 74e58a0
---

## Executive summary

The native denylist now normalizes identifier variants, scans documented native
data/config extensions, supports explicit path/identifier/reason allowlists,
and has fixture-driven self-tests without changing the historical browser
exemption.

## Implementation

Matching is case-folded and token-aware across camelCase, PascalCase,
snake_case, kebab-case, punctuation, and joined identifiers, with boundary
handling so a denied token such as `ore` does not flag `score`. The scan covers
`.c`, `.cc`, `.cmake`, `.cpp`, `.cxx`, `.h`, `.hpp`, `.js`, `.json`, `.ps1`,
`.toml`, `.txt`, `.yaml`, and `.yml` under native paths. The denylist remains
sorted and category-noted; `crafting` was not added because the audit marks the
active House seam mixed.

Audit-backed additions: `anvil`, `barrow depths`, `bronze bar`,
`bronze pickaxe`, `defence`, `fenmire`, `hammer`, `legacy mode`,
`legacy relic id`, `legacy tile`, `old wood`, `ore`, `pickaxe`, `smith`, and
`woodcutting` (with retained/reordered `cooking`).

## Changed files

- `native/tools/check_legacy_denylist.py`
- `config/legacy-denylist.json`

## Interfaces

The checker adds `--self-test` and `--self-test --negative-fixture` modes and
retains the production checker invocation used by the native build gate.

## Verification

Worker evidence:

- Baseline checker at `74e58a0`: PASS, exit 0.
- Hardened self-test: PASS, exit 0.
- Hardened production checker: PASS, exit 0.
- Negative fixture: injected `bronzeDagger` is reported and exits 1 as
  intended.
- `powershell -NoProfile -File native/build.ps1 -RunTests`: denylist PASS,
  core tests PASS, exit 0.
- `git diff --check`: PASS.

## Manual checks

Scope review found exactly the two owned files changed. Browser `src/` and
`server/` remain outside the scan boundary.

## Specification deviations

None. The mixed-provenance `crafting` term is explicitly proposed-but-not-added
as required by the stop condition.

## Risks and limitations

The extension list intentionally includes native scripts/data/config files;
future legitimate exceptions must use the explicit allowlist with a reason.

## Questions for Fable or the owner

None.

## Integration notes

Independent validator `/root/validate_task_0008` returned **ACCEPT** after
checking the two-file scope, diff, baseline/hardened checker, normalization and
false-positive cases, allowlist behavior, self-test and negative fixture, and
native gate. The architect review is now **ACCEPTED**. It is disjoint from
TASK-0007 and the integrated build gate should be rerun after both tasks land.
