# Model scorecard (empirical; the unit is model × harness × task family)

Small samples — treat every row as calibration-in-progress, not
identity. Recalibrate after model/harness/prompt/acceptance changes.

| Model+harness | Task family | N | First-pass accept | False greens | Notes / best packet |
|---|---|---|---|---|---|
| Luna workers via codex CLI | native C++ features | ~8 | ~50% | 4 claims falsified (0035×2, 0037×2) | MECHANICAL only; needs scaffold + literal acceptance; drove D-120 |
| Luna workers via codex CLI | browser features/fixes | ~10 | ~70% | 1 (0043 rev0 flag-only) | good under exact specs; evidence discipline improved post-INC-001 |
| Luna workers via codex CLI | evaluation/playtesting | 3 | 33% | 0 (honest incompletes) | honest but driver-error-prone (INC-007); needs G0 preconditions |
| codex coordinator (Sol) | investigation/disproof | 1 | 100% (0048) | 0 | strong evidence-based diagnosis |
| Kimi Work K3 (kimi CLI) | native C++ parity waves | 1 | 100% (0044 N2) | 0 | BOUNDED-DESIGN works; honest stub inventories |
| Kimi Work K3 (kimi CLI) | browser features | 1 | 100% (0038) | 0 | delivered hard-fail captures unprompted; high trust |
| Kimi K3 console | browser features | 1 in flight (0042) | — | — | stalled once (quota); re-asserted |
| DeepSeek V4-Pro (dsh) | browser UI wave | 1 in flight (0049) | — | — | methodical; caught own flaky capture; ~$1/session, 99% cache hit |
| Fable (architect) | scaffolding/reviews/integration | — | — | 0 | control plane + D-120 scaffolds |

## Calibration notes

- kimi-work K3 is the strongest verified implementer so far (2/2
  first-pass on substantial tasks, self-imposed evidence rigor).
- Luna's "weak" label is scoped: weak at unscaffolded design, fine at
  mechanical execution and honest evaluation. Harness may contribute
  (codex CLI context handling) — do not treat as pure model quality.
- DeepSeek economics (~$1/session) make it the default lane if quality
  lands; first review (0049) calibrates.
- EXP-1 in RUN_STATUS compares packet types across 0049 vs 0050.
