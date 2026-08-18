// Resources event handler

import bus from '../../utilities/bus.js';
import { normaliseChatMessage } from '../../chat-message.js';
import { formatManaRejection } from '../../mana-directive.js';
import { isGuideMessage } from '../../tutorial-beats.js';

export default {
  /**
   * Server-authored chat message. Two client-side presentation rewrites live
   * here (TASK-0049): the bare mana rejection becomes a directive line, and
   * Aldwyn's first-session beats are surfaced for a transient banner.
   */
  'game:send:message': (data, context) => {
    const message = normaliseChatMessage(data);
    if (!message || !message.text) {
      return;
    }

    let text = message.text;
    if (text === 'Not enough mana.') {
      const player = context && context.game ? context.game.player : null;
      text = player ? formatManaRejection(player) : text;
    }

    bus.$emit('game:send:message', {
      type: message.type || 'normal',
      text,
      color: message.color,
    });

    if (isGuideMessage(text)) {
      bus.$emit('tutorial:beat', { text });
    }
  },

  /**
   * Update skills
   */
  'resource:skills:update': (incoming, context) => {
    if (!context || !context.game || !context.game.player) {
      return;
    }

    const skills = incoming && incoming.data ? incoming.data.data : null;
    if (!skills || typeof skills !== 'object') {
      return;
    }

    context.game.player.skills = skills;
  },
};
