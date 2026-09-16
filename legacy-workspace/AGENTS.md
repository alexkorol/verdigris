# Verdigris workspace

The game is the `delaford_game/` directory — work there. The canonical
agent guide is `delaford_game/AGENTS.md`. Its core rule:

**Never claim a gameplay change works without running the goal harness:**

```bash
cd delaford_game
npm run playtest    # boots a real server, plays the core loop; exit 0 = playable
```

`Z:\Code\WIZARD` is the prototype sandbox whose tools get ported into the
game. Authorized implementation includes committing verified work and pushing
the task's working branch unless the user explicitly requests local-only work.
Follow the commit/push policy in `delaford_game/AGENTS.md`; carry it into sprint
prompts and handoffs without adding push bans or a second approval checkpoint.
