# Four-client, two-instance delayed-network soak

## Scope and reproducibility

The initial measured QA capacity is **four authenticated clients in two simultaneous two-person expeditions** on one Windows host. This is the exercised capacity, not a maximum-capacity estimate. Actors remain near each expedition's entry, make bounded ordinary movement pulses, receive live actor presence, and request authoritative gameplay snapshots. This does not establish sustained combat, Internet performance, TLS delivery, native rendering or visual acceptance.

Run with Windows PowerShell 5.1:

```powershell
powershell.exe -NoProfile -ExecutionPolicy Bypass -File native/tools/test-service-soak.ps1 -ServerExecutable <exact-server.exe> -Seconds 120
```

Keep `native/tools/service-soak.cs` beside the runner. The runner copies the specified executable into a new evidence directory, checks source/copy/source SHA-256 agreement, records embedded identity and both test-file hashes, enrolls four disposable accounts, and owns only its spawned service. It stops that exact service in `finally`; private enrollment files stay within the ignored evidence directory and are not printed or committed. The server must provide the existing service enrollment, health and stop operations. No build registration is needed.

Each managed ClientWebSocket connects through its own test-only local TCP relay. Every ordered read chunk waits **20–35 ms in each direction**, using independent deterministic random streams derived from seed `20260914`. The nominal added delay for one chunk in each direction is **40–70 ms round trip**. This is a chunk-delay model: TCP coalescing, scheduling and multiple forwarded chunks can change measured application timing. It neither simulates packet-level loss/reordering nor traverses the Internet. The protocol client continuously drains incoming frames with bounded queues.

At the midpoint, the second client's connection is forcibly interrupted for at least two seconds. The other three continue requesting gameplay snapshots. Authentication with its existing token must retain account, actor and instance identity while rotating the command epoch. Both instance rosters must continue containing exactly their own two actors. All four must remain alive at completion.

Gameplay timing runs from ordinary movement-intent send through delivery of the following requested authoritative snapshot. It includes application processing and both relay directions; it is not a ping estimate or an acknowledgment that movement has already advanced a tick. Separate snapshots verify actual position changes. The response guard is 2 seconds. Process private memory, working set, thread and handle peaks are sampled every 50 ms, with admission/start/end snapshots; private growth is guarded at 128 MiB.

## Validation

The 10-second harness smoke passed after correcting the test serializer's collection cast (`ArrayList` through `IList`). No production change was made for that harness error. The full **121.569-second** soak then passed on September 14, 2026, with 81 complete four-client rounds and 44 snapshot-confirmed movement changes. All four actors ended at 100/100 HP. Service health still passed and the owned service stopped cleanly; stderr was empty.

Evidence directory: `native/build/service-soak-432e333cad67478fa194ddcfa52ab686/` in the transport worktree. `soak-results.json` records all measurements and final actor states. It contains no credentials. The copied EXE and disposable Store remain there for local reproduction; none is part of the commit.

| Exact artifact | SHA-256 / identity |
| --- | --- |
| Copied `verdigris_server.exe` | `9a9adfe77424f5abb8bf82fb4b56ca60f2ab00c0942d59d1f837f8b6356220c1` |
| Embedded source locator | `99a0af41a2a57c51fc305a5cc453915bc8d981fa dirty` |
| `test-service-soak.ps1` | `337dd3d1a504a1b775d91451328efaed056c694c3112384c3951af9dec11f9ad` |
| `service-soak.cs` | `1a9e42f35d7f1e7b76c7717f0f84a15b8a7bef28e75fc984efc0869ff0782e8e` |
| `soak-results.json` | `fc758376e73a5a630bd1fe6f1de91193252e8c15755c0d93af528e36d937e8f3` |

The dirty embedded locator is not a full source identity; the actual copied EXE hash identifies the measured artifact. The parent must rerun this script against its final clean package executable.

| Measurement | Result |
| --- | ---: |
| Gameplay snapshot round-trip samples | 336 |
| Median / p95 / maximum round trip | 199.839 / 217.331 / 255.022 ms |
| Mean injected forward / reverse chunk delay | 27.593 / 27.543 ms one way |
| Relay forward / reverse chunks | 1,092 / 10,580 |
| Actual interrupted interval | 2,420.667 ms |
| Interruption through authenticated restored snapshot | 2,951.762 ms |
| Counted actor-presence frames | 8,815 |

Presence count excludes the interrupted peer's disposed connection history, so it is a lower bound on delivered presence during setup and soak. Round-trip samples include the continuing peers' snapshots during the interruption and exclude setup/movement-verification snapshots.

| Process resource | Before admission | Soak start | Soak end | Sampled peak (setup + soak) |
| --- | ---: | ---: | ---: | ---: |
| Private bytes | 1,437,696 | 2,740,224 | 3,489,792 | 3,489,792 |
| Working-set bytes | 6,541,312 | 8,241,152 | 8,957,952 | 9,052,160 |
| Threads | 6 | 14 | 13 | 14 |
| Handles | 125 | 153 | 154 | 154 |

Server CPU increased from 2,828.125 to 88,656.250 ms across the timed soak: **85,828.125 CPU ms**, approximately **70.6% of one logical CPU** over 121.569 seconds. This is a measured cost for this workload and machine, not a capacity extrapolation. Private bytes grew 749,568 from soak start to end. A two-minute run cannot establish a long-duration memory plateau.

The independent instance IDs were `instance:017c2e8608c582a5071d93cde4147f58` and `instance:6c5b935844549438047a78bbee1a7e4c`. Each current roster remained exactly its two expected actors. Reconnect retained the second actor in the first instance with a new command epoch; both instances and the other three clients stayed usable through the outage.
