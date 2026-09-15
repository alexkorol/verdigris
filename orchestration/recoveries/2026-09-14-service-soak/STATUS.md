# Service multi-instance network soak

- coordinator: codex
- worker: transport
- branch: codex/native-service-transport-20260914
- status: IMPLEMENTED
- scope: new test-only relay/helper, PowerShell runner and evidence report; no production module changes.
- parent authorization: bounded independent four-client/two-instance soak, explicit delay/jitter/interruption, resource and gameplay round-trip measurements.
- current evidence: 10-second smoke and 121.569-second full soak passed; four living actors, two isolated instances, 44 observed movements, authenticated interruption recovery, 336 gameplay response samples.
- delivery: test-only helper and runner; package owner must rerun against final executable. No Internet, maximum-load, combat-soak or visual acceptance claim.
