---
task: TASK-0149
verdict: REVISE
reviewed_head: 96f4ccbd1572add96e34ccb230b9935b743d7ff3
reviewed_at: 2026-08-22 03:47 -07:00
---

# TASK-0149 review — REVISE

The frozen pushed head is clean, owned-scope-only, and substantially improves
the real owner launch. Independent detached verification passed PowerShell
parsing, a fresh native build/test run, `-Rebuild -LifecycleSelfTest` with a
real `VerdigrisNativeClient` window (normal WM_CLOSE exit 0 plus forced client
exit), exact tracked-PID cleanup, all four fail-fast negative controls, and
`git diff --check`.

One release-blocking failure-path leak remains:

1. `Start-OwnerServer` starts the server process and then may throw when the
   process remains alive but prints no matching readiness line within 12
   seconds, or when the reported port differs from the requested port.
2. Both callers assign `$server = Start-OwnerServer ...`. PowerShell does not
   complete that assignment when the function throws, so the caller's
   `finally` block still sees `$server` as null. The already-started live server
   is therefore not stopped or included in `Assert-SessionClean`.
3. Make `Start-OwnerServer` own cleanup for every exception after
   `Start-Process` (or otherwise publish the PID before any throwable readiness
   check). Add a deterministic negative control that exercises a live
   post-spawn readiness failure and proves the exact server PID is gone.
4. Preserve the accepted real-window happy paths and all current negative
   controls. Rerun the full native tests, `-Rebuild -LifecycleSelfTest`, the new
   post-spawn failure control, `git diff --check`, and push a new clean review
   head without amend or force-push.

Do not broaden this revision into gameplay, client, server, build-system, or
capsule changes. The current head is preserved and must not be integrated.

