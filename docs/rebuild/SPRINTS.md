# Native reconstitution sprint map

## Milestone A — authority and guardrails

Constitution, open decisions, legacy matrix/denylist, canonical agent guide,
and Rust-versus-C++ ADR.

## Milestone B — native build and core

Buildable C++20 workspace, fixed-step deterministic simulation, command/event
boundary, actor symmetry, and green headless tests.

## Milestone C — House/item/campaign loop

House and Scion persistence, extraction/death risk, item history/relic
candidacy, instance graph ownership, optional branch, and seasonal extension.

## Milestone D — runnable client

Native window/console shell with WASD intent, mouse-facing/action hooks, one
enemy, one drop, pickup/equip, extraction, and compact House status.

## Milestone E — optional visual foundation

Only after the above is stable: adjustable billboard camera, sorting, contact
shadows, procedural attack effects, and supplied visual references if available.

Each milestone ends with tests/build passing, a commit, and an update to
`docs/rebuild/HANDOFF.md`.
