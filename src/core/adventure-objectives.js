// Adventure zone objective preview (TASK-0049 deliverable 4).
//
// Each solo Adventure zone is a generated instance whose stairs are guarded
// by a theme boss named in `server/core/map.js` THEME_MONSTERS. Solo zones
// always open at depth 1 (party.js startSoloInstance -> enterFloor(1)), and
// every floor's treasure room guarantees a gear item at
// `instanceItemLevelForDepth(depth)` = 10 for the first delve. These tables
// mirror those two server facts BY VALUE so the Adventure panel can state a
// concrete draw (named Warden + guaranteed item-level treasure + depth)
// without a server change — the same documented display-constants pattern as
// the quickbar cost labels. No new item or economy data is invented here.
import { TEMPLATE_BOSS_NAMES, instanceItemLevelForDepth, SOLO_DELVE_DEPTH } from './adventure-objective-data.js';

export const zoneObjective = (zone = {}) => {
  const warden = TEMPLATE_BOSS_NAMES[zone.template] || null;
  const depth = SOLO_DELVE_DEPTH;
  const itemLevel = instanceItemLevelForDepth(depth);
  const parts = [
    warden,
    itemLevel > 0 ? `item-level ${itemLevel} gear` : null,
    depth > 0 ? `depth ${depth}` : null,
  ].filter(Boolean);

  return {
    warden,
    itemLevel,
    depth,
    line: parts.join(' · '),
  };
};

export default { zoneObjective };
