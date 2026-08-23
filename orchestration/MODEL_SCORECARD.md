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
| Qwen3.8-27B-8bit local (LM Studio, MacBook) | verified MECHANICAL battery | 7 tasks | 7/7 (2026-08-20: json-normalize x3, code-edit node-checked, test-gen executed, md-table, throughput) | 0 | ~16 tok/s sustained, 1.8-9.3s per small task, zero reasoning leakage at temp 0; telemetry orchestration/telemetry/qwen-2026-08-20.jsonl; DRIVERS: deepseek (Tailscale URL) + luna-mac (localhost:1234); dispatch only with a machine verifier (parse/node --check/run tests); never interactive loops |
| Cursor Grok 4.6 (Cursor desktop) | browser features + infra | 3 | 100% (0055, 0059, 0062) | 0 | first-pass accept on calibration; hard-fail captures + authentic negative + disclosed deviations w/ repro; strong evidence discipline; BOUNDED-DESIGN works |
| Cursor Grok 4.6 (Cursor desktop) | native C++ features + CI | 9 | 0061 rubric miss then 8 straight first-pass (Gate A 12/12, reference scenes, launcher) | 0 | protocol/session/render/bench all strong; honest gap notes; give explicit presentation-quality constraints in SPECs |
| Claude Sonnet (Claude Code, MacBook) | browser infra (calibration) | 1 | 100% (0066) | 0 | first-pass; unsolicited peer-verify on 0062 first (standing-loop duty); clean lane discipline; browser/JS + docs lane |
| Codex Luna (MacBook) | docs/audit MECHANICAL (calibration) | 1 | 100% (0071) | 0 | precise file:line citations, honest gap tables; minor notes-file naming nit; MECHANICAL-only lane holds |
| Fable (architect) | scaffolding/reviews/integration | â€” | â€” | 0 | control plane + D-120 scaffolds |
| Fable (architect) | solo implementation (D-124 takeover) | 1 day | delivered 32/32 attach parity + 4 PRs | 0 | ANTI-PATTERN, see INC-012: correct output, wrong actor. Cost the owner a full billing week of Claude budget for work three cheaper lanes could have done. Retired from the fleet 2026-08-20. Do not repeat: a dark fleet is a stop condition, not a takeover trigger |
| Ox Alpha fleet (OpenCode CLI 1.18.21 / openrouter/stealth/ox-alpha, variant max) | native/browser implementation + validator waves (0080-0164) | ~45 routed lanes | all 45 landed ACCEPTED + integrated; 0 false greens remain on the board | 0 open | strong frozen-head evidence discipline (literal gates, committed captures, negative controls); a minority needed a numbered revision cycle before acceptance (0081, 0101, 0128, 0149, 0152, 0157, 0164) — the independent review gate caught each; a few claims needed activation-controller format repairs. Fleet currently provisioned/dormant awaiting owner launch |

## Calibration notes

- kimi-work K3 is the strongest verified implementer so far (2/2
  first-pass on substantial tasks, self-imposed evidence rigor).
- Luna's "weak" label is scoped: weak at unscaffolded design, fine at
  mechanical execution and honest evaluation. Harness may contribute
  (codex CLI context handling) â€” do not treat as pure model quality.
- DeepSeek economics (~$1/session) make it the default lane if quality
  lands; first review (0049) calibrates.
- EXP-1 in RUN_STATUS compares packet types across 0049 vs 0050.

