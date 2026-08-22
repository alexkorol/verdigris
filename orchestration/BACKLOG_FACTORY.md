# Permanent full-product backlog factory

Owner-ruled by D-128. The old 24 READY + 12 successor target is retained only
as an emergency anti-stall floor. It is not adequate runway, planning
completion, or product progress by itself.

## Primary measure: autonomous runway hours

Autonomous runway is the amount of validated READY or automatically releasable
work the currently registered fleet can consume without new owner input,
architect prose, manual packet creation/promotion, branch preparation, or
acceptance design.

- target: at least 72 hours;
- warning: below 48 hours;
- critical queue incident: below 24 hours;
- current value: `UNKNOWN` until accepted-throughput telemetry and packet
  consumption weights are validated. Unknown is not green.

Required packet count is dynamic. Use trailing accepted throughput split by
model, harness and version, machine, task family, packet type, and complete
configuration provenance. Do not substitute human developer estimates.
Until enough comparable samples exist, use an aggressive provisional upper
capacity bound and overstock.

## Three maintained layers

1. **Terminal product graph** -- initial floor 2,000 concrete non-filler nodes,
   each with outcome, parent objective, dependencies, owner-visible relevance,
   likely surface, acceptance class, owner dependency, and category. It ends
   only at proven full-product gates.
2. **Detailed reserve** -- initial floor 500 substantial DRAFT/AUTO_RELEASE
   packets with outcome, topology/job type, dependencies, likely owned and
   forbidden paths, interface, acceptance/evidence, owner dependency, fallback,
   generation provenance, and successor rule. Distant mutable packets carry no
   stale immutable base.
3. **Executable runway** -- enough exact READY/AUTO_RELEASE packets for at least
   72 measured hours. READY packets carry exact base, paths, resource capsule,
   commands, evidence, and stops. AUTO_RELEASE packets carry exact release and
   refresh predicates so owner prose is not required.

These are rolling initial floors, never completion conditions.

## Sweep order

Every architect sweep performs:

1. worker, activation, review, and host-sync scan;
2. verdict and integration work;
3. accepted-throughput and runway calculation;
4. automatic release of valid unblocked packets;
5. successor analysis and generation;
6. terminal-graph decomposition;
7. packet validation and stale-draft refresh;
8. lane-specific queue restocking;
9. owner-input batching;
10. scorecard, coverage, and throughput update.

Below 48 hours, backlog production is P0. Below 24 hours, stop noncritical
architect activity and replenish. Never repair runway by implementing a worker
task in the architect checkout.

## Successor fan-out

Every accepted packet triggers documented successor analysis. Expected useful
ranges, not filler quotas:

- audit/gap analysis: 8-30 implementation, test, content, or decision packets;
- architecture/interface: 5-20 implementation, migration, negative-test, and
  integration packets;
- implementation: 3-10 integration, edge-case, hardening, performance,
  content-use, presentation, regression, or polish packets;
- owner-visible visual feature: implementation plus capture, usability,
  accessibility, performance, asset, and correction packets;
- content system: schema, tooling, validators, initial content, expansion,
  tuning, migration, UI, and regression packets.

Fewer is valid only with exhaustion evidence. An audit is not product progress
unless it immediately feeds executable successors.

## Queue composition

By expected fleet consumption, target at least 60% implementation,
integration, content, presentation, polish, or release work; no more than 25%
pure audit/research/inventory/evaluation unless discovery genuinely blocks
implementation. Testing, tooling, hardening, and acceptance fill the remainder.

## Required coverage

The terminal graph decomposes the complete ARPG across:

- core/architecture: deterministic simulation, command/event boundaries,
  actor model, persistence/migration/recovery, replay, network/protocol,
  reconnect, performance, platform, renderer/audio/input, packaging/update,
  and crash handling;
- player experience: profile, House/Scion lifecycle, selection, control,
  targeting, combat/skills, loot/inventory/equipment, extraction/death/recovery,
  progression/travel/endgame, save/relaunch, settings/accessibility/onboarding;
- combat, skills/magic, monsters/encounters, itemization, progression;
- world/campaign/endgame, presentation, content-production pipelines;
- quality/release: unit through clean-machine/soak/performance/migration/crash,
  device/resolution/accessibility, owner play, visual/audio regression,
  installer/signing/notarization/update/support.

`CONTENT_SCALE_MATRIX.md` provides the provisional scale envelope. Product
authority and owner-input gates remain binding; neutral schemas, tooling,
placeholders, tests, and unrelated work continue while taste decisions wait.

## Reporting contract

Report autonomous runway hours, trailing accepted throughput, claimable work by
lane, AUTO_RELEASE count, detailed reserve, concrete graph-node count, domain
coverage, implementation/content/polish share, audit share, owner-blocked share,
and unresolved critical-path decisions. Task counts alone are insufficient.

First substantial milestone: at least 2,000 graph nodes, 500 detailed reserve
packets, 72 hours validated runway, complete terminal/scale matrices, and an
end-to-end proof that workers continue without owner-authored prompts. Continue
the factory after that milestone until terminal product gates are proven.
