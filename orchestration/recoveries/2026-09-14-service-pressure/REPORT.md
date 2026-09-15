# External service pressure evidence

## Result and identity

The new `native/tools/test-service-pressure.ps1` passed against a copied real
native service process. It made no changes to production modules or the parent
checkout. The test creates a fresh evidence directory, copies and hashes the
selected executable, generates two disposable enrollments/private Store, launches
only that copy, and closes only its own process. Copying prevents the test from
locking the parent's build output during subsequent builds.

- Input executable: `C:/Users/Alex/Documents/ChatGPT/verdigris-service-coop-20260914/native/build/verdigris_server.exe`.
- Copied SHA-256: `55dae80b178899c1fbc7f4cff516c8fce18e993378dc97efe3349278936758b9`.
- Embedded identity: `31059e1a385ea3f5e6822340b7ce8bcf5551d3f4 dirty`.
- Exact evidence: `native/build/service-pressure-0570a5e27588451187ca27d1b42be26b/pressure-results.json`.
- Topology: one Windows machine, loopback; no injected latency or jitter.
- The executable is a development snapshot. Its hash identifies the tested bits;
  the dirty source label does not establish a clean release/package identity.

Command, from this worktree, exit **0**:

```powershell
powershell.exe -NoProfile -ExecutionPolicy Bypass -File native/tools/test-service-pressure.ps1 -ServerExecutable 'C:/Users/Alex/Documents/ChatGPT/verdigris-service-coop-20260914/native/build/verdigris_server.exe'
```

Use Windows PowerShell 5.1; the managed test probe is compiled in-process with
the installed .NET Framework. This fixture uses maintained .NET ClientWebSocket
for healthy/account connections and deliberately raw loopback frames only to
arrange malformed/pressure inputs. It is not a new production transport.

## Observations

| Check | Observed result |
|---|---|
| Sequential malformed HTTP handshakes | 1,152 refused, 1,197.3 ms total; periodic healthy pings succeeded |
| Malformed WebSocket cases | Five closed: 16,385-byte declaration, unmasked text, unsupported fragmentation, reserved bit, invalid JSON |
| Authenticated nonreader | 37 full-snapshot requests, then disconnected in 3,154.6 ms; input paced at no more than 14/s |
| Burst command pressure | 20 upgraded peers, 96 ping messages each; offending peers closed |
| Authenticated recovery | Passed after explicit 300 ms dispatcher settling interval |
| Reconnect churn | 33 authenticated reconnects completed after pressure |
| Healthy pings | 22 samples; median 39.93 ms, maximum 78.70 ms RTT |
| Test duration | 7,733.0 ms inside the managed probe |
| Service CPU | 1,968.75 ms accumulated process CPU during that interval |
| Private bytes | Baseline 1,372,160; final 3,764,224; sampled peak 3,878,912 |
| Working set | Sampled peak 9,158,656 bytes |
| Threads | Sampled peak 10; final 6 |
| Handles | Sampled peak 145; final 126 |

Resource sampling runs every 50 ms. Samples can miss shorter spikes; these
values are sampled maxima, not guaranteed absolute peaks. CPU time is aggregate
process time, not server tick latency. Snapshot pace is an application request
rate, not simulated network latency. The local TCP stream is ordered.

The gate requires healthy RTT below two seconds, final private-byte growth below
128 MiB, final thread count below 80, valid reconnect, and final service readiness.
These guards detect failures in this bounded test; they do not establish capacity,
an unlimited-player claim, a long soak, or production latency targets.

## Counterevidence and diagnostic check

The first two candidates attempted fresh authentication immediately after the
burst peers closed. The new connection upgraded, then closed before an auth
event. Stage/stack evidence isolated this to post-flood reconnect, after all
earlier checks passed. No service code was changed in response.

The smallest follow-up introduced an explicit 300 ms wait for the dispatcher to
retire queued intents from closed offenders. The same executable then recovered
and passed. This supports transient pending-command saturation as the explanation;
it does not claim admission is guaranteed at the instant the global queue is full.
The settling interval is visible in the script and result JSON. The parent was
notified of both failing and passing observations.

## Scope and remaining gates

Credentials stay in disposable private files/memory and are never included in
reported metrics or service command-line arguments. Startup/shutdown logs are
retained beside the results. On failure, the script records its stage and always
requests shutdown of its exact spawned process, using a force stop only if that
owned QA process fails to exit after the bounded graceful wait.

The test does not create Houses/Scions, measure combat simulation, inspect pixels,
exercise a public gateway, or test separate computers. The parent's real native
co-op/package/performance gates remain required. Re-run this script against the
final clean packaged service to establish final-delivery evidence.
