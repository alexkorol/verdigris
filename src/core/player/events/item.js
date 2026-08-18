// Item event handler
import bus from '../../utilities/bus.js';
import { newFirstFindPickup, presentFirstFindPickup } from './loot-moment.js';

const inventoryQuantity = inventory => (Array.isArray(inventory) ? inventory : [])
  .reduce((total, item) => total + Math.max(1, Number(item?.qty) || 1), 0);

export default {
  /**
   * A player picks up or drops an item
   */
  'item:change': (data, context) => {
    context.game.map.droppedItems = data.data;
  },

  /**
   * A player recieves an item in their inventory
   */
  'core:refresh:inventory': (incoming, context) => {
    const nextInventory = incoming.data.data;
    const previousInventory = context.game.player.inventory;
    const previousQuantity = inventoryQuantity(previousInventory);
    context.game.player.inventory = nextInventory;
    if (inventoryQuantity(nextInventory) > previousQuantity) {
      bus.$emit('sound:loot');
    }
    const firstFind = newFirstFindPickup(previousInventory, nextInventory);
    if (firstFind) {
      presentFirstFindPickup({ item: firstFind, player: context.game.player });
    }
  },
};
