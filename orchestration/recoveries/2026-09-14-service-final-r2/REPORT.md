# Final r2 host operations and network acceptance

## Artifact and topology

Measured executable: `C:/Users/Alex/Documents/Verdigris Service Host QA 2026-09-14-r2/verdigris_server.exe`.

- Embedded source identity: `9cfb4aab9e6edc16ba5dd919c263f6673ca63f98 clean`.
- Executable SHA-256: `f8787626130a27577ba56f390dabd07d229627788af5340779578ee667e62147`.
- Scripts ran from the parent's integrated `native/tools/` using Windows PowerShell 5.1. No source edits were made for this validation.
- Evidence root: `C:/Users/Alex/Documents/ChatGPT/verdigris-service-transport-20260914/native/build/`.
- Tests used only private disposable Stores and the exact test-owned service process. The pressure and soak runners verified source/copy/source hashes before running their private EXE copies. No installed service, firewall, public endpoint or DNS was changed.
- Parent confirmed an 83-scenario graphical suite on this same machine: it began approximately 20:50:40 local time and was still running at 20:54:51, overlapping these checks and the soak that began around 20:53. These are **concurrent graphical-regression load measurements**, not quiet-machine timing measurements. The final two-client rendering journey was held until the capacity run completed. This lane performed no further heavy workload afterward.

These are local Windows service/protocol checks. They do not establish Internet or TLS delivery, maximum capacity, sustained combat, long-duration memory stability or visual acceptance.

## Operations — exit 0

`test-service-operations.ps1` passed private ACL enrollment without secret output, exclusive Store ownership, readiness, stdin-independent lifetime, graceful stop, backup, fresh restore, and operation of the restored Store.

- Evidence: `native/build/service-final-r2-operations/`.
- Script SHA-256: `a50f465f9fd4c34f24a3ff97c7d7322b17f4f54536b345b32d0ae7e9a0063615`.
- Shutdown stdout: `service_metrics ticks=8 authority_avg_us=376 authority_peak_us=2252 commits=1 commit_avg_us=1948 commit_peak_us=1948`.
- The service explicitly reported `verdigris_server stopped cleanly`; stderr was empty.

## Pressure — exit 0

`test-service-pressure.ps1` passed 1,152 sequential malformed handshakes, five malformed wire cases, an authenticated nonreader, 20 peers each sending 96 burst messages, and 33 authenticated reconnects. The existing explicit 300 ms post-flood settling interval remains part of this test.

- Evidence: `native/build/service-final-r2-pressure/`.
- Script SHA-256: `94631b97372f33f203e6a4e20ac6210fc649e54a81d594e684d34eecc7f08380`.
- `pressure-results.json` SHA-256: `aec0dba221b7f9d970547bd61dde99a245943ca95e031ed2a72f656f2561fd23`.
- Elapsed: 8,712.131 ms; malformed handshake phase: 1,450.716 ms.
- Nonreader disconnected after 3,191.929 ms and 36 requested snapshots.
- Healthy ping RTT: 22 samples, median 41.722 ms, maximum 101.734 ms, without injected delay.
- Private bytes: initial 1,425,408; final 1,949,696; sampled peak 3,555,328. Working-set peak: 8,966,144 bytes.
- Threads: sampled peak 27, final 6. Handles: peak 178, final 127. Process CPU: 2,500 ms.
- Shutdown stdout: `service_metrics ticks=194 authority_avg_us=11710 authority_peak_us=41158 commits=3 commit_avg_us=3516 commit_peak_us=4972`.
- The service explicitly reported `verdigris_server stopped cleanly`; stderr was empty.

## Soak — exit 0

The exercised initial QA capacity is **four authenticated clients in two independent two-person expeditions**, with seeded **20–35 ms one-way TCP chunk delays in both directions** and an interruption/reconnect midway. The nominal injected delay for one chunk in each direction is **40–70 ms round trip**. The detailed workload and interpretation remain documented in `../2026-09-14-service-soak/REPORT.md`.

- Evidence: `native/build/service-final-r2-soak/`.
- Duration: **120,016.836 ms**, 79 full four-client rounds, 40 snapshot-confirmed movement changes. All four ended at 100/100 HP.
- Gameplay snapshot round trips: 328 samples, median **200.864 ms**, p95 **231.651 ms**, maximum **263.431 ms**.
- Mean configured delay actually applied per forwarded chunk: forward 27.542 ms, reverse 27.542 ms; 1,044 forward and 10,396 reverse chunks.
- Interrupted interval: 2,555.404 ms; interruption through authenticated restored snapshot: 3,108.416 ms. Account, actor and instance persisted, and the command epoch rotated.
- Recorded actor-presence frames: 8,512, excluding the disposed pre-interruption connection history. Both instance rosters retained exactly their own two actors.
- Instance IDs: `instance:3488b240b27733a7d199f0020ab13280` and `instance:29a402463b3e246990b9bc3cb5d4651d`.
- Shutdown stdout: `service_metrics ticks=2549 authority_avg_us=42994 authority_peak_us=105778 commits=23 commit_avg_us=2370 commit_peak_us=6550`.
- The service explicitly reported `verdigris_server stopped cleanly`; stderr was empty. Post-soak health passed before shutdown.

| Process resource | Before admission | Soak start | Soak end | Sampled peak (setup + soak) |
| --- | ---: | ---: | ---: | ---: |
| Private bytes | 1,441,792 | 2,670,592 | 3,190,784 | 3,502,080 |
| Working-set bytes | 6,557,696 | 8,171,520 | 8,708,096 | 9,064,448 |
| Threads | 6 | 14 | 13 | 14 |
| Handles | 125 | 153 | 154 | 154 |

Server CPU grew from 4,406.250 to 106,359.375 ms across the timed soak: 101,953.125 CPU ms, approximately **84.9% of one logical CPU**. Resource peaks were sampled every 50 ms. These measurements include confirmed concurrent graphical-regression load on the machine. The observed authority processing average/peak were **42.994/105.778 ms**; SQLite commit average/peak were **2.370/6.550 ms**. Passing this workload does not establish spare timing capacity or a higher supported client count. Private bytes grew 520,192 from soak start to end; this duration cannot establish a long-term memory plateau.

| Evidence artifact | SHA-256 |
| --- | --- |
| Integrated `test-service-soak.ps1` | `f9d25ba2cfde654d3ef16330ecf65de7b3aee67544f84bfe112b1380da03c0ff` |
| Integrated `service-soak.cs` | `1a9e42f35d7f1e7b76c7717f0f84a15b8a7bef28e75fc984efc0869ff0782e8e` |
| `soak-results.json` | `4d32f8d03b0dbaed0000cc47010b0432640efd17ccf9e14871f4c2406ed980d2` |

The original packaged executable was hashed again after all three checks and still matched `f8787626130a27577ba56f390dabd07d229627788af5340779578ee667e62147`. The PowerShell runner's hash differs from the earlier transport-worktree run because the integrated checkout uses its own line-ending representation; each run records the exact executed file hash.
