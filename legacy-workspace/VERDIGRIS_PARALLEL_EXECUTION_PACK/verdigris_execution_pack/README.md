# Verdigris native design and multi-agent execution pack

**Edition 1.0 | September 4, 2026 | Proposal, not dispatch authority**

Start with `VERDIGRIS_DESIGN_AND_EXECUTION.docx` or `docs/MASTER_DESIGN_AND_EXECUTION.md`.

This pack translates the D2R / Path of Exile 1 and 2 ambition into Verdigris-specific quality, systems and production goals. It contains 200 DRAFT backbone goals across 25 workstreams, 689 dependency edges, granular acceptance/negative controls, existing-task crosswalks, role prompts, packet/evidence templates, read-only planning tools and a content-lot expansion example.

## Contents

| Path | Purpose |
|---|---|
| VERDIGRIS_DESIGN_AND_EXECUTION.docx | Editable master design, execution model, 200-goal review index and worker prompt |
| docs/MASTER_DESIGN_AND_EXECUTION.md | Repository-friendly source of the master plan |
| docs/SOURCES.json | Pinned repository and primary external references |
| backlog/ATOMIC_GOALS.md | Complete checklist with outcome, acceptance, negative control, dependencies, candidate ownership, evidence and integration |
| backlog/tasks.json | Canonical structured planning registry; all goals DRAFT |
| backlog/tasks.csv | Import-friendly task projection, not a separate live board |
| backlog/dependencies.dot | Dependency graph source |
| prompts/ | Coordinator, worker, reviewer, integrator and player-experience prompts |
| templates/ | Task packet, claim/evidence records and content-lot example |
| tools/ | Read-only DAG/path validator, candidate-wave suggester, tests and content expansion |
| validation/ | Actual planning-tool test results; not game test evidence |

## Adoption sequence

First reconcile the current repository head and existing TASK packets. Obtain a recorded decision for the conflicting push/claim/review policies and adopt a genuinely exclusive shared claim mechanism. Then freeze product/technical decisions, refresh each READY packet to exact paths/base/commands, and dispatch only the collision-free integrated-dependency frontier. No VG planning ID reserves a repository TASK number.

Use the worker prompt only with an actual authorized packet and claim. The ZIP does not create agents, commit to the repo, push branches or perform background work.

## Validate locally

```sh
python tools/roadmap.py validate
python -m unittest discover -s tools -p 'test_*.py' -v
python tools/roadmap.py wave
python tools/roadmap.py wave --planning --assume-integrated VG-GOV-001
```

The normal `wave` output is empty because nothing is READY. Planning mode is explicitly hypothetical and cannot grant a claim. See `tools/README.md` for limits and content generation.

## Evidence and scope limits

The prior audit source baselines are `2d3e92a5` and `8597c654`; the integration ref was rechecked at the latter for this edition. No fresh game build, frame-time test, audio listening, platform playtest or exploit reproduction was performed. Source citations are ordinary reference keys listed in docs/SOURCES.json. Counts and gates are proposed scope, not achieved parity. The 200 goals are a backbone; additional content assets and later feature details require child task expansion.
