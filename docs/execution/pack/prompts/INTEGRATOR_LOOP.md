# Integrator loop — copy/paste prompt

You hold the currently authorized integration seat for Verdigris. Verify that permission before any program-branch change. This prompt does not grant it.

Read the current program head, policies, accepted frozen implementation heads, active claim tokens and reserved shared-file resources. Confirm that each candidate has independent review at the required tier, accepted dependencies, compatible content/schema versions and a rollback/forward-recovery plan.

Use your own integration worktree. Never switch another lane's checkout or force-push. Stage the smallest accepted candidate against the current program head. Reserve shared CMake, main.cpp, core.cpp, networking.cpp, public header and registry edits explicitly. An automatic conflict-free merge is not semantic validation.

Run relevant native tests, ordinary-play scenarios, persistence/fault cases, browser compatibility gates and package checks specified by the integration packet. Actual commands must exist at this head. Keep logs/artifacts in disposable, provenance-stamped locations. Do not use real owner saves.

If combined gates fail, preserve the failing head and repro; stop the train; revert only the candidate when safe or use approved forward recovery for schema/data transitions. Route a bounded correction. Do not mark ACCEPTED component tests as proof that the combined application works.

When gates pass, apply only the authorized commit/push policy. Record exact integrated head, source task heads, current claim tokens, commands/results, artifact hashes and affected gate state. Mark INTEGRATED only after the approved target contains the change. Release claims/resources through the shared authority. Reevaluate downstream readiness against the new head.

No production publication, account reset, credential action, force-push or irreversible operation is authorized here. End with the integration result, evidence and remaining risks.
