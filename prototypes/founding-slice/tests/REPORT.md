# TASK-0003 worker report

## Summary

Added a self-contained verification command for the Founding of a House
prototype. It builds into a temporary directory, compares that output with the
committed `index.html` without overwriting it, serves the prototype from an
ephemeral Node HTTP server, and runs headless Playwright checks against the
browser-visible slice and its `window.__V` surface.

## Changed files

- `prototypes/founding-slice/run-checks.mjs`
- `prototypes/founding-slice/tests/slice-checks.mjs`
- `prototypes/founding-slice/tests/REPORT.md`

No game, shared configuration, native, server, or orchestration files were
changed.

## Verification command and outcome

Command:

```text
node prototypes/founding-slice/run-checks.mjs
```

Output:

```text
Founding slice verification
PASS  build freshness / drift guard — wrote index.html: 2226 KB, slots: boss, dwelling, raider, ruin, scion_dex, scion_int, scion_str, shrine, terrain1, terrain4, tree, wagon
PASS  load: boot with no console errors
PASS  fresh-house arc: all three directions have distinct stats — {"str":{"str":7,"dex":4,"int":3},"dex":{"str":4,"dex":7,"int":3},"int":{"str":3,"dex":4,"int":7}}
PASS  full loop: equip → crisis → LMB combat → clear → death/relic → successor → founding — relic it1; Continue — Tereth of House Cinderwatch

4/4 checks passed in 18.2s
```

The source files also pass:

```text
npx eslint prototypes/founding-slice/run-checks.mjs prototypes/founding-slice/tests/slice-checks.mjs
```

## Negative drift probe

An ephemeral copy of `index.html` was deliberately modified by appending a
marker. The byte comparison rejected the copy while the committed artifact
remained unchanged:

```text
NEGATIVE PASS: deliberate scratch index drift detected; committed index untouched
```

The scratch directory was removed after the probe.

## Commit

The final worker commit hash is supplied in the worker handoff. This report
avoids embedding a self-referential hash so it cannot become stale when the
revision is committed.

## Notes and follow-ups

The harness uses only the repository's Playwright package and no new
dependencies. It drives the standard via the real `E` interaction, performs a
real canvas `LMB` attack, and uses the debug panel only for deterministic wave
clearing. No unresolved questions or stop conditions remain.

## Revision 1

Validator findings:

- P1: the HTTP server containment check used a Windows-only backslash prefix,
  so POSIX paths were rejected incorrectly. The guard now computes
  `relative(resolve(root), file)` and rejects absolute results and parent
  traversals using both slash forms while allowing files under the root.
- P2: the report contained the historical implementation SHA. The commit
  section now refers to the handoff instead of hardcoding a revision SHA.

Verification commands and outcomes:

```text
node prototypes/founding-slice/run-checks.mjs
4/4 checks passed in 18.2s

npx eslint prototypes/founding-slice/run-checks.mjs prototypes/founding-slice/tests/slice-checks.mjs
passed

POSIX/Windows path-containment logic check
PASS: root/index allowed; ../escape and absolute paths rejected on POSIX and Windows path models

negative drift probe
NEGATIVE PASS: deliberate scratch index drift detected; committed index untouched
```
