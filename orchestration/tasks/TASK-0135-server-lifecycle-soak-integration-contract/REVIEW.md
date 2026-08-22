# REVIEW — TASK-0135 server lifecycle soak integration policy

Verdict: **ACCEPTED**

Reviewed worker head: `88092c97254d9a5836aca892deacd9b57d356f9a`

The worker stayed inside the task-folder boundary and produced the required
machine-readable policy, negative fixtures, validation record, report, and
evidence transcript. The policy keeps nightly hosting, release-proof
activation, long-term storage, and unsupported-platform execution explicitly
`OWNER_PENDING`; forbids port 6500; and makes deterministic failures, missing
artifacts, and timeouts impossible to retry into green.

Architect verification on the exact pushed head:

- required policy-key/forbidden-port command: exit 0,
  `soak integration policy: PASS`;
- required negative-case coverage command: exit 0,
  `soak policy negatives: PASS`;
- fixture-to-policy resolution plus owner-authority assertions: exit 0,
  `architect semantic checks: PASS`;
- `git diff --check aab574b8..88092c97`: exit 0;
- worker-authored path audit from routed head `aab574b8`: six paths, all under
  `orchestration/tasks/TASK-0135-server-lifecycle-soak-integration-contract/**`.

This is an orchestration contract only. Acceptance does not enable a nightly
job, spend runner minutes, declare release readiness, or authorize CI changes.
