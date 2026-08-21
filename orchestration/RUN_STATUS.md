# Run status — Codex Sol sweep

Snapshot: 2026-08-20 19:45 PDT

Sweep base: `1f82623d` (`codex/native-reconstitution`)

GitHub: PR #49 merged; latest program and master CI green; no open PRs.

Orchestrator: **Codex Sol**, active from the clean MacBook worktree
`/Users/alexkorol/Code/verdigris-sol`.

## Objective and proof state

Immutable objective: D-116 under D-122's three axes.

1. Server/rules parity: **DONE** — unchanged attach suite 32/32 twice on
   fresh native servers, plus post-hotfix verification.
2. Native journey parity: Gate A **GREEN**; Gate B and Gate C remain.
3. Presentation parity: terrain and HUD landed; surface density and
   panels/typography remain.

D-125 is now binding: before three coordinators are live, maintain at least 8
effective READY packets plus 4 sequenced successors. Effective means listed
below, unclaimed, dependency-free, and without owned-path collision.

## Fleet liveness

| Lane | State | Last remote evidence | Current instruction |
|---|---|---|---|
| cursor / composer-2.5 | PAUSED by owner | TASK-0076 at 2026-08-20 07:41 PDT | Resume on TASK-0083 calibration; do not claim native-client work concurrently |
| deepseek Flash | PAUSED by owner | TASK-0056 worker at 2026-08-18 19:36 PDT | Old 0056 claim is obsolete; resume on TASK-0081 |
| luna-mac | COLD | TASK-0074 at 2026-08-20 08:02 PDT | Separate coordinator process required; resume on TASK-0079 |
| qwen3.8 local | HEALTHY | `/v1/models` returned `qwen3.8` at this sweep | Executor only; a live coordinator must machine-verify its output |

INC-012 remains binding: Sol does not absorb the implementation backlog while
two or more lanes are dark. The owner launches/revives coordinators.

## Interrupts

- REVIEW_REQUESTED: **none**.
- REVISE: **none**.
- TASK-0056's committed CLAIMED status is stale provenance; the architect
  takeover and full server parity supersede it. No coordinator may resume it.
- Port 6500 remains owner-only. Port 7000 is occupied by a macOS system
  service; luna uses 7001-7019.

## Effective READY — 9 packets

These paths are disjoint at the board level. First committed claim wins.

| Priority | Task | Packet | Route | Contribution |
|---|---|---|---|---|
| P0 | TASK-0081 Gate B wire-contract freeze | MECHANICAL | deepseek | freezes the N5/N6 envelopes before client work |
| P0 | TASK-0080 board sentinel | MECHANICAL | luna-mac + Qwen | machine-enforces D-125 and collision/liveness truth |
| P1 | TASK-0079 browser panel inventory | MECHANICAL | luna-mac | supplies presentation delta #4 contract |
| P1 | TASK-0083 server lifecycle soak | BOUNDED-DESIGN | cursor calibration | guards the PR #46 reader-thread lifetime fix |
| P1 | TASK-0082 dual-server matrix runner | BOUNDED-DESIGN | deepseek after 0081 | automates D-116 dual-server regression evidence |
| P1 | TASK-0086 Gate C contract audit | MECHANICAL | deepseek/docs lane | maps current N6 information before Gate C UI |
| P2 | TASK-0084 reference-capture manifest | MECHANICAL | luna-mac + Qwen | detects stale/missing presentation evidence |
| P2 | TASK-0085 denylist exception audit | MECHANICAL | luna-mac + Qwen | prepares owner ruling without changing canon |
| P2 | TASK-0073 renderer backend evaluation | BOUNDED-DESIGN | any research lane | feeds Stage 2 renderer ADR |

## HOLD despite historical READY headers

`RUN_STATUS.md` is current routing truth; the immutable SPEC headers remain as
issued history.

| Task | Release condition | Reason |
|---|---|---|
| TASK-0077 native Chronicles client | TASK-0081 ACCEPTED and architect freezes exact client interfaces | Gate B event names/payloads must be pinned before implementation |
| TASK-0078 native surface density | TASK-0077 ACCEPTED/integrated | both own `native/client/**`; single-writer rule |

## Sequenced successors — 4 DRAFT

| Task | Dependency |
|---|---|
| TASK-0087 native pane shell | 0079 + 0078 accepted; architect freezes pane model |
| TASK-0088 renderer ADR | 0073 accepted; owner approves any new dependency |
| TASK-0089 Gate C native journey | 0077 + 0078 + 0086 accepted; missing contract fields resolved |
| TASK-0090 native progression panes | 0087 accepted; authoritative payload audit complete |

## Restart order

1. Unpause DeepSeek with its refreshed standing goal; it claims TASK-0081.
2. Unpause Cursor with its refreshed standing goal; it claims TASK-0083.
3. Start a separate luna-mac coordinator; it claims TASK-0079, then uses Qwen
   only for verified mechanical substeps.
4. Sol sweeps immediately after each claim, reviews every flip, restocks in
   the same sweep, and promotes pipeline work only when dependencies are
   ACCEPTED/integrated.

WIP remains architect + 3 workers maximum. A task count never justifies an
owned-path collision or an owner-only product decision.

## Owner decisions still parked

- `legacyRelicId` and `bronze-dagger` denylist dispositions (TASK-0085 gathers
  evidence only).
- Renderer production dependency after TASK-0073/TASK-0088.
- OD-001 and OD-007 through OD-013 remain owner-only where applicable.
