---
id: TASK-0026
title: Native CI triggers on denylist and workflow changes
state: READY
track: tooling
priority: medium
base_commit: current program tip (coordinator records the SHA)
dependencies: []
parallel_safe: true
owned_paths:
  - .github/workflows/native.yml
forbidden_paths:
  - native/**
  - src/**
  - server/**
  - prototypes/**
  - config/**
acceptance_commands: []
---

## Goal

Close the PR #4 review's P2 tooling gap: `native.yml` triggers only on
`native/**`, so a PR changing only `config/legacy-denylist.json` (or the
workflow itself) skips the gate that enforces it.

## Scope

Add `config/legacy-denylist.json` and `.github/workflows/native.yml` to
both `push.paths` and `pull_request.paths` filters. Nothing else.

## Acceptance criteria

Workflow YAML parses (validate with any local YAML parse, e.g.
`node -e "require('js-yaml')..."` or a python one-liner; paste proof in
REPORT.md); diff touches only the workflow file.

## Review focus

Exact path list; no other workflow edits.
