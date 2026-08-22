# VALIDATION — Gate C decision envelope

Contract version: `1.0.0` (`supported_versions: ["1.0.0"]` in
`gate-c-decision-envelope.json`). This document is content-neutral: it defines
checks, codes, and ordering only. It never supplies campaign, reward, economy,
risk, or balance values.

## Inputs

- `envelope`: a JSON object intended to satisfy
  `gate-c-decision-envelope.json`.
- Evaluation is **deterministic and first-match-wins**: run the checks below in
  the listed order; the first failing check's code is THE error. If no check
  fails, the envelope is decision-ready.

## Field states

Each of the six decision fields (`concrete_goal`, `boss_or_danger`,
`expected_item_family`, `depth`, `branch_consequence`, `extraction_or_return`)
carries exactly one state:

- `AVAILABLE` — a concrete value/rule is on the current wire/source/test
  surface and is cited.
- `DERIVABLE-WITHOUT-GAMEPLAY-RULES` — computable from cited existing
  payloads/rules only within the field's documented scope (no invented value).
- `MISSING` — no source provides it; producing it requires an owner-only
  decision. A MISSING state inside an envelope is **honest and structurally
  valid**; it simply blocks decision readiness via the field's `MISSING_*`
  code. Envelopes must preserve MISSING honestly — never fill it to pass.

## Checks and error codes (in evaluation order)

| # | Code | Fails when |
|---|------|------------|
| 1 | `INVALID_JSON` | input does not parse as a JSON object |
| 2 | `UNSUPPORTED_VERSION` | `schema_version` is absent or not in `supported_versions` |
| 3 | `MISSING_ROUTE_IDENTITY` | `route_identity` absent, not an object, or missing road/node/tier identity sub-shape |
| 4 | `ROUTE_NAME_ONLY` | identity present but **none** of the six decision fields carries a state/value (input is a name/tier/blurb alone) |
| 5 | `MISSING_PROVENANCE` | `evidence_provenance` absent or lacking any required citation shape (authority source, audit reference with reviewed head, base commit) |
| 6 | `MISSING_CONCRETE_GOAL` | `concrete_goal` absent, `state: "MISSING"`, or null value |
| 7 | `MISSING_BOSS_OR_DANGER` | same pattern for `boss_or_danger` |
| 8 | `MISSING_EXPECTED_ITEM_FAMILY` | same pattern for `expected_item_family` |
| 9 | `MISSING_DEPTH` | same pattern for `depth` |
| 10 | `MISSING_BRANCH_CONSEQUENCE` | same pattern for `branch_consequence` |
| 11 | `MISSING_EXTRACTION_OR_RETURN` | same pattern for `extraction_or_return` |
| 12 | `CONTRADICTORY_DEPTH` | `depth` present but self-inconsistent: state AVAILABLE/DERIVABLE with null value, two differing depth/tier numbers inside `depth`, or `depth` tier conflicting with `route_identity` tier |
| 13 | `OWNER_PENDING_CONTENT` | readiness is claimed while owner-pending content remains: `completeness.ready === true` while any decision field is `MISSING`/`owner_pending`, or `completeness.missing_fields` is non-empty |

Checks 6–11 evaluate the six fields in the fixed order above (goal → boss →
family → depth → branch → extraction); that fixed order makes multi-failure
cases deterministic.

## Decision-readiness rule

An envelope is decision-ready if and only if:

1. checks 1–13 all pass;
2. every decision-field state is `AVAILABLE` or
   `DERIVABLE-WITHOUT-GAMEPLAY-RULES`;
3. no field carries `owner_pending: true`; and
4. `completeness.missing_fields` is empty.

`DERIVABLE-WITHOUT-GAMEPLAY-RULES` fields are decision-usable **only within
their documented scope** (currently: `branch_consequence`, immediate next
stage). A route name, tier, or blurb alone remains invalid under all
circumstances (check 3/4).

## Required failure coverage

The SPEC-required failures map to codes as follows:

- missing fields → checks 6–11 (`MISSING_CONCRETE_GOAL`, …,
  `MISSING_EXTRACTION_OR_RETURN`)
- route-name-only input → check 4 (`ROUTE_NAME_ONLY`)
- unsupported version → check 2 (`UNSUPPORTED_VERSION`)
- contradictory depth → check 12 (`CONTRADICTORY_DEPTH`)
- missing provenance → check 5 (`MISSING_PROVENANCE`)
- owner-pending content → check 13 (`OWNER_PENDING_CONTENT`)

Every documented code has at least one synthetic fixture in
`fixtures/negative-cases.json` carrying its `expected_error`.

## Fixtures

`fixtures/negative-cases.json` contains one negative case per code above
(13 codes; the SPEC-required nine plus `INVALID_JSON`,
`MISSING_ROUTE_IDENTITY`, `CONTRADICTORY_DEPTH`, `OWNER_PENDING_CONTENT` for
full deterministic coverage). All fixture values are obviously synthetic
(`synthetic-road-*`, placeholder states); none is a product value.
