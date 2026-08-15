// Unified point economy: nodes and conduits each cost 1 point from a single
// pool, so travel is a real spend. The authored ten-ring tree has a 140-point
// lifetime budget: 117 from levels and 23 from quests.
export const VERDIGRIS_SKILL_TREE_POINTS = Object.freeze({
  skill: 140,
});

export const VERDIGRIS_SKILL_TREE_SOURCES = Object.freeze({
  levels: 117,
  quests: 23,
});

export const VERDIGRIS_SKILL_TREE_TOTALS = Object.freeze({
  layers: 10,
  nodes: 331,
  subtreeNodes: 34,
});

export const earnedVerdigrisPoints = (level, questPoints = 0) => Math.min(
  VERDIGRIS_SKILL_TREE_POINTS.skill,
  Math.min(
    Math.max(2, Math.floor(Number(level) || 1)),
    VERDIGRIS_SKILL_TREE_SOURCES.levels,
  ) + Math.min(
    Math.max(0, Math.floor(Number(questPoints) || 0)),
    VERDIGRIS_SKILL_TREE_SOURCES.quests,
  ),
);
