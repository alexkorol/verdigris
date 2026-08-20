---
task: TASK-0066
state: REVIEW_REQUESTED
coordinator: mac-claude
worker_branch: codex/TASK-0066-capture-harness-consolidation-mac-claude
base_commit: 5c41a04821695e38261fdb52f2e86b2dea67f21d
started_at: 2026-08-20T03:26:04-07:00
finished_at: 2026-08-20T03:35:00-07:00
architect_review_required: true
expected_verification: npm run test:unit (134/841, unaffected); eslint clean
  on the two new files; CAPTURE_PORT=7002 node .../captures/capture-0066-demo.mjs
  -> CAPTURES OK, 16/16 checks, exit 0; negative control confirms hard-fail
  path exits 1 with the failing check named
---

Review requested. Ports 7000-7019 (7000 avoid — macOS afs3-fileserver squats
it on this Mac; used 7001-7002). See REPORT.md.
