---
task: TASK-0043
verdict: REVISE
reviewed_commits:
  - 7b81f874
  - 51c5253d
---

## What was reviewed

The full diff (architect-verified playtest-only: zero non-playtest commits
unique to the branch vs merge-base b29424f9; no server/src/native/package
changes), the ten-run loaded transcript
(`captures/full-load-ten-runs-2026-08-17.txt`, 10× 31/31, plausible
per-run variance), the authentic negative run (suppressed
`instance:enterSolo` → session-arc fails 0/1 exit 1), and an
architect-run full playtest at the merged tip.

## The problem — the proof only covers the flagged path

All ten green runs were executed with `PLAYTEST_LOAD_MODE=1`, which
unlocks the extended load-mode deadlines. The default path — plain
`npm run playtest`, which is what the owner and the architect actually
run, and what CLAUDE.md's playability gate invokes — was not part of the
proof. Architect's own full-suite run at the merged tip (32d7b6e +
7b81f874), load mode OFF, ambient machine contention only:

```
FAIL  session-arc (27135ms)
30/31 scenarios passed
Timing diagnostics: {"loadMode":false,"p99EventLoopLagMs":32.194559,"maxEventLoopLagMs":122.617855}
```

Immediately after, `npm run playtest --silent -- session-arc` solo
passed 1/1 (11.6s, max lag 32ms). This is the exact flake class the task
exists to kill, surviving in the default mode: real event-loop lag
reached 122ms (6× the 20ms baseline) with the env flag off, and the
adaptive guard did not protect the session-arc deadline.

## Required corrections (revision 1)

1. Make the default mode (no env flag) robust to measured ambient
   contention: the load-adaptive guard should key off actual observed
   event-loop lag continuously — not the `PLAYTEST_LOAD_MODE` flag — so
   a default-mode run under real contention gets the same bounded
   deadline extension. Authored deadlines remain the floor; assertions
   must not be loosened (unchanged constraint).
2. Proof for the revision, in addition to keeping the existing loaded
   evidence: THREE consecutive default-mode (`npm run playtest`, no
   flag) full runs at 31/31 while a moderate background load runs
   (document the load method), transcripts captured. The architect will
   rerun one default-mode full run personally before acceptance.

## Provisional integration note (deliberate, documented)

The branch's harness changes are scope-clean and strictly improve
stability, so the architect has merged 7b81f874 into the program branch
now (local merge at the review checkout) to avoid a revert-of-merge
hazard at re-integration. The TASK remains open in REVISE: revision
commits land on the same worker branch and will merge cleanly on top.
This does not change the acceptance bar above.

## What is correct (keep)

The load-method honesty, the per-run transcript format, the authentic
negative regression (real mutation, real failure, cleaned up), the
combat-target UUID pinning, and the strict no-assertion-loosening
discipline. The report's claims all checked out as stated — the gap is
coverage of the default mode, not truthfulness.
