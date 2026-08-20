# Model scorecard (empirical; the unit is model Ã— harness Ã— task family)

Small samples â€” treat every row as calibration-in-progress, not
identity. Recalibrate after model/harness/prompt/acceptance changes.

| Model+harness | Task family | N | First-pass accept | False greens | Notes / best packet |
|---|---|---|---|---|---|
| Luna workers via codex CLI | native C++ features | ~8 | ~50% | 4 claims falsified (0035Ã—2, 0037Ã—2) | MECHANICAL only; needs scaffold + literal acceptance; drove D-120 |
| Luna workers via codex CLI | browser features/fixes | ~10 | ~70% | 1 (0043 rev0 flag-only) | good under exact specs; evidence discipline improved post-INC-001 |
| Luna workers via codex CLI | evaluation/playtesting | 3 | 33% | 0 (honest incompletes) | honest but driver-error-prone (INC-007); needs G0 preconditions |
| codex coordinator (Sol) | investigation/disproof | 1 | 100% (0048) | 0 | strong evidence-based diagnosis |
| Kimi Work K3 (kimi CLI) | native C++ parity waves | 1 | 100% (0044 N2) | 0 | BOUNDED-DESIGN works; honest stub inventories |
| Kimi Work K3 (kimi CLI) | browser features | 1 | 100% (0038) | 0 | delivered hard-fail captures unprompted; high trust |
| Kimi K3 console | browser features | 1 | 100% (0042) | 0 | first-pass accept; self-run stale-base check + honest flake triage; additive renderer deviation disclosed properly | BOUNDED-DESIGN works |
| DeepSeek V4-Pro (dsh) | browser UI wave | 1 | 100% (0049) | 0 | first-pass accept, $1.47, 21 new tests, self-cleanup; one scope deviation (mirrored constants vs ask); BOUNDED-DESIGN works |
| Qwen3.8-27B-8bit local (LM Studio, MacBook via Tailscale) | probe battery only | 6 probes | DISPATCHABLE (2026-08-18 re-probe) | 0 | thinking OFF at template default (patched); old reasoning-runaway (P4 399-tok dump) GONE — 811ms clean; opt-in thinking at low effort emits no reasoning (inert?); lane = MECHANICAL packets + offline eval, NOT interactive loops; endpoint http://alexs-macbook-pro.tail4e0d34.ts.net:1234/v1 model qwen3.8, no chat_template_kwargs on default path, JIT other models disabled |
| Cursor Grok 4.6 (Cursor desktop) | browser features | 1 | 100% (0055) | 0 | first-pass accept on calibration; hard-fail captures + authentic negative + disclosed scope deviation w/ repro; strong evidence discipline; BOUNDED-DESIGN works |
| Fable (architect) | scaffolding/reviews/integration | â€” | â€” | 0 | control plane + D-120 scaffolds |

## Calibration notes

- kimi-work K3 is the strongest verified implementer so far (2/2
  first-pass on substantial tasks, self-imposed evidence rigor).
- Luna's "weak" label is scoped: weak at unscaffolded design, fine at
  mechanical execution and honest evaluation. Harness may contribute
  (codex CLI context handling) â€” do not treat as pure model quality.
- DeepSeek economics (~$1/session) make it the default lane if quality
  lands; first review (0049) calibrates.
- EXP-1 in RUN_STATUS compares packet types across 0049 vs 0050.

