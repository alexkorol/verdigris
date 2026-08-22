# TASK-0120 FINDINGS — release verification gap audit

- Worker: `ox-pc-e` · Branch: `codex/TASK-0120-release-verification-gap-audit-ox-pc-e`
- Immutable base: `42718fbc4340589e606fff94a6eaa3dfbd03ad1c` · Route head: `039dcfa7`
- Method: read-only static audit of committed evidence (workflows, build/test
  scripts, docs, orchestration records). No installer execution, no account
  mutation, no ports bound; port 6500 untouched. Machine-verifiable inventory:
  `captures/release-gates.json`. Literal gate transcripts: `REPORT.md` and
  `captures/gate-1-rg-release-surface.txt` (1,575 matching lines, exit 0).

## Verdict

No native build may be called **safely releasable** today. The engineering
gate layer (CI, build, protocol regression, native shutdown) is genuinely
proven; every distribution-adjacent surface — clean-machine harness, long
soak, performance budget, save migration, rollback, asset manifest, macOS
coverage, installer, signing — is partial, missing, or owner-only. Three
surfaces are proven, eight partial, five missing, two owner-only.

## Proven gates (committed, enforced, machine-checked)

1. **Current CI** — `ci.yml` (ubuntu, Node 22, `npm ci`, `npm run verify`
   = lint + lint:css + unit + build + full playtest + built e2e, failure
   artifacts) and `native.yml` (windows-latest, MSVC preset, denylist, ctest
   core/networking/session, camera2d, `--scenario all`, transcripts on
   failure). Latest master run green at `d2423873` (RUN_STATUS 2026-08-21).
2. **Protocol regression** — full `npm run playtest` 32/32 is a mandatory
   acceptance gate; the unchanged native attach suite passed 32/32 twice on
   fresh servers; native session tests cover login, journey,
   reconnect/session-replace inside CI.
3. **Server shutdown (native)** — `session_tests.cpp` asserts local,
   remote, and dual-side journey shutdown reach the disconnected state and
   run inside the CI ctest preset; `play-native.ps1` adds a no-orphan check.

## Partial gates (evidence exists; not release-enforcing)

4. **Clean clone/build** — both workflows build from fresh checkouts each
   run, which is a proxy, not a release harness: no retained build artifacts,
   no bare-machine → playable proof. TASK-0126 (DRAFT successor) owns this.
5. **Dependency/runtime packaging** — engines/volta pins + lockfile give
   reproducible `npm ci`; native is dependency-free. No SBOM, no bundled
   runtime, `security:audit` exists but is not wired into CI.
6. **Launcher** — Windows-only `play-native.ps1` (free 6520–6539 port, log
   tee, orphan check, desktop-shortcut recipe). No macOS/Linux launcher, no
   launcher smoke gate.
7. **Deterministic replay** — byte-equal replay determinism tests ship and
   stay green in the core suite; but there is no versioned replay record,
   stored replay artifact, or divergence runner (TASK-0100 READY audit +
   TASK-0106 successor own that layer).
8. **Crash/save recovery** — `write_atomic` + fail-closed restore, pending
   recovery queues, guest saves surviving restarts, pm2 autorestart with
   backoff/memory cap. No crash-injection matrix or corrupted-save corpus
   (TASK-0107 successor).
9. **Windows/macOS coverage** — Windows is exercised end-to-end in CI; the
   macOS/Linux CMake console path is documented but compiled by nobody: zero
   macOS automation exists.
10. **Logs/support bundle** — pm2 out/err logs, native `build/logs` tees, CI
    `ci-logs` artifacts on failure. No one-command support bundle; no
    retention/rotation policy.
11. **Release acceptance** — ACCEPTANCE.md's G0–G6 ladder with negative
    controls, default-path rule, and G5 personal rerun governs *task*
    acceptance; no release checklist binds all surfaces to a tagged,
    evidence-retained build.

## Missing gates (no committed proof; successors own them)

12. **Long soak** — `playtest/soak.mjs` exists but is invoked by no gate or
    workflow; the native lifecycle soak (TASK-0083) is READY and
    unimplemented. No duration-stability evidence is required today.
13. **Performance** — density bench explicitly omitted from CI
    (`ci-native.ps1`), `bench:instance` manual-only, playtest load mode is a
    fairness cap not a budget. No frame/memory/p95 gate (TASK-0099 READY).
14. **Save-version migration** — fail-closed only: `core.cpp` throws on
    unsupported/missing `schemaVersion`; unknown keys ignored; **no
    migration code path between schema versions exists**. The browser SQLite
    stores have no versioned migration either. TASK-0127 (DRAFT successor)
    owns the matrix.
15. **Upgrade/rollback** — `docs/deployment.md` documents the upgrade half
    (pull/ci/build/restart) and nothing else: no rollback procedure, no
    revert drill, no data-compat precheck for either server.
16. **Asset/license manifest** — root LICENSE plus intake-convention docs
    only; no generated provenance manifest, no lint (TASK-0094 READY).

## Owner-only gates (deliberately outside worker scope)

17. **Signing/notarization** — absent from the repository by policy (SPEC
    `owner_input_dependency`); requires owner certificates/accounts.
18. **Installer** — no packaging project exists at all; distribution
    decisions are owner authority, and executing installers is outside the
    worker resource capsule regardless. PROGRAM_GRAPH.md correctly orders
    clean-machine inventory (this audit) before installer/signing work.

External: none identified beyond standard CI runner infrastructure; GitHub
branch protection (ORCHESTRATION enforcement backlog item 1) is an owner
GitHub-settings action, counted here under owner-only rather than a new
category.

## Negative control (required by SPEC)

**Claim:** the `docs/deployment.md` update recipe (`git pull; npm ci;
npm run build; pm2 restart`) makes upgrading a live production server safe.

**Finding: unsupported.** No clean-machine or migration artifact backs it —
a save written by a different schema version hard-fails restore rather than
migrating (native) or has no versioned migration path at all (browser
SQLite), no rollback procedure exists anywhere in the repository, and no
pristine-host drill reproduces the update. The recipe silently assumes
cross-version data compatibility. Discharge: TASK-0126 + TASK-0127 artifacts
after this audit is accepted.

## Boundary respected

Every conclusion above comes from committed, inspectable evidence; no
narrow test was promoted to broad release proof (the strongest claim any
"proven" row makes is the exact command that enforces it). Staging stops
here: signing, notarization, installer execution, distribution, and account
actions remain owner-only, and this packet performs none of them. The
machine-verifiable companion inventory is `captures/release-gates.json`.
