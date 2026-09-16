# Verdigris workspace — Claude Code

The game is `delaford_game/` — work there. The canonical agent guide
(shared with other coding agents like OpenAI Codex) is
`delaford_game/AGENTS.md`; the game's own `delaford_game/CLAUDE.md`
imports it plus Claude-specific preview notes.

The one rule that overrides everything: **never claim a gameplay change
works without running `npm run playtest` in `delaford_game/`** (boots a
real server and plays the core loop; exit 0 = playable). Green unit tests
are not sufficient — five shipped bugs hid behind a green suite.

Authorized implementation includes committing verified work and pushing the
task's working branch unless the user explicitly requests local-only work.
Follow the commit/push policy in `delaford_game/AGENTS.md`; carry it into sprint
prompts and handoffs without adding push bans or a second approval checkpoint.
Dev server ports are pinned (client :5173, WS :6500).
