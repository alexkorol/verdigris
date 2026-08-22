# Content seam

Deterministic, versioned, content-neutral authoring schema and validator for
native zones, encounters, and visual-role references. Data lives apart from
simulation algorithms; nothing in this directory is loaded by the C++ core yet.
The seam is the future attachment point for versioned actors, items, routes,
and seasonal definitions.

WIZARD Cartographer is a candidate seeded map-content adapter. WIZARD Brands &
Bonds/inventory data is a candidate item-content source. Both must map into
versioned native schemas and pass deterministic connectivity/identity tests
before production adoption.

## Layout

```text
native/content/
  schema.json                  versioned schema: enums, entities, composite types
  seeds/zones.json             synthetic example zones (content-neutral)
  seeds/encounters.json        synthetic example encounters (content-neutral)
  validate_content.py          dependency-free validator CLI (Python 3 stdlib)
  tests/run_negative_tests.py  positive control + determinism + negative suite
```

## Commands

All commands run from the repository root with Python 3 only; no third-party
packages, network, or build step is involved.

Positive gate (must accept the committed seed):

```text
python native/content/validate_content.py
```

Expected final line: `OK schema=1 errors=0 warnings=0 encounter=3 zone=5`,
exit code `0`.

Negative gate (must reject every seeded defect deterministically):

```text
python native/content/tests/run_negative_tests.py
```

Expected final lines include one `PASS` per case and end with
`NEGATIVE SUITE PASS`, exit code `0`. The runner also proves the validator's
diagnostics are byte-identical across repeated runs for every case.

`--quiet` limits validator output to the final summary line. Exit codes:
`0` valid, `1` validation failures, `2` usage errors (argparse).

## Schema model

- `schema_version`: integer, currently `1`. Seeds and schema must match it
  exactly; any other value is rejected (`E_SCHEMA_VERSION`).
- Seed files are envelopes `{ "schema_version", "kind", "items" }`. The kind
  must match the file declared in `schema.json#seed_files`
  (`E_FILE_KIND`). Unknown envelope or item fields are rejected
  (`E_UNKNOWN_FIELD`); missing required fields are rejected
  (`E_MISSING_FIELD`).
- Identifiers follow `^[a-z][a-z0-9]*(-[a-z0-9]+)*$` with a maximum length of
  64 (`E_ID_FORMAT`). IDs are unique across all collections
  (`E_DUPLICATE_ID`).
- Zones carry an accepted zone template and layout (the enums mirror the
  identifiers accepted by `verdigris::is_zone_template`/`is_zone_layout`),
  an exit list whose targets must exist in the committed zone set
  (`E_UNKNOWN_ZONE_REF`) without duplicate edges within one zone
  (`E_DUPLICATE_EXIT`), and a visual-role map from presentation slots to
  registered visual roles (`E_UNKNOWN_SLOT`, `E_UNKNOWN_ROLE`).
- Encounters reference exactly one zone (`E_UNKNOWN_ZONE_REF`), declare a
  structural family enum member (`E_UNKNOWN_FAMILY`), and carry their own
  visual-role map.
- Zones unreachable from the lexicographically smallest zone id raise a
  non-fatal deterministic warning (`W_UNREACHABLE_ZONE`); warnings do not fail
  validation.

Diagnostics print as `<ERROR|WARNING> <file>:<json path> <CODE>: <message>`,
sorted by file, path, code, then message, so output is byte-stable for a given
input. Codes are stable public contract; messages are human-readable and may
be reworded in minor schema versions.

## Versioning policy

Schema-breaking changes increment `schema_version` and ship a validator that
still accepts the previous committed seed shape only during a migration task;
the committed seed always validates against the committed schema at any head.
Enum membership grows by adding sorted members; removals are breaking.

## Content neutrality

This directory carries no production lore, no balance values, and no numeric
tuning. Seed entries use `example-*` identifiers and generic display names.
Zone templates/layouts and structural encounter families reuse only existing
accepted identifier families from the simulation; everything else stays
synthetic until the owner approves real content through the approval matrix.
