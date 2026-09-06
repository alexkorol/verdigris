# Read-only planning tools

Python 3.10+ standard library; no packages, network access or credentials required. Run from the pack root:

```sh
python tools/roadmap.py validate
python tools/roadmap.py show VG-MOVE-001
python tools/roadmap.py wave
python tools/roadmap.py wave --planning --assume-integrated VG-GOV-001
python -m unittest discover -s tools -p 'test_*.py' -v
```

`wave` returns no actual READY work in this edition: every goal is DRAFT. `--planning` is a dependency/path what-if, labeled NOT_CLAIMABLE. Assumed integrated sets must include their prerequisite closure. Exact owned paths, logical resources and current authority must be stamped before executable states pass validation.

The suggester checks file/directory/glob overlap conservatively and shared logical locks. It does not read live Git, acquire an exclusive claim, verify owner rulings, infer semantic contract conflicts or reserve external resources. Shared integration hooks still need the coordinator's separate integration slot. Eight is a proposed initial cap, not a hardware limit or permission to start eight processes.

Content expansion:

```sh
python tools/expand_content_lot.py templates/CONTENT_LOT_EXAMPLE.json backlog/EXAMPLE_CONTENT_CHILDREN.json
```

The command refuses to overwrite an existing file. The supplied example is not approved content: it generates 18 DRAFT child proposals for illustration. Each new lot still requires a manifest ruling and exact task ownership/acceptance. Children use LOT identifiers and remain in a separate registry until the coordinator maps them into real repository TASK packets. A parent is not complete until its non-N/A children are integrated and accepted.

Validation is structural planning QA, not game test evidence. Never edit a passing validation report to claim a gameplay result.
