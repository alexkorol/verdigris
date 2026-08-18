import { announceFirstFindDrop, newFirstFindDrop } from './loot-moment.js';

export default {
  /**
   * The world receives an updated dropped items list
   */
  'world:itemDropped': (data, context) => {
    const previousItems = context.game.map.droppedItems;
    context.game.map.droppedItems = data.data;
    const firstFind = newFirstFindDrop(previousItems, data.data);
    if (firstFind) {
      announceFirstFindDrop(firstFind);
    }
  },


  'world:foreground:update': (data, context) => {
    context.game.map.foreground = data.data;
  },

  'world:scene:transition': async (message, context) => {
    const payload = message && message.data ? message.data : {};
    if (!context || typeof context.handleWorldSceneTransition !== 'function') {
      return;
    }

    await context.handleWorldSceneTransition(
      payload.scene || null,
      payload.playerState || {},
      payload.portal || null,
    );
  },
};
