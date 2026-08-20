// Adventure zone objective preview. Display fields come from the server
// adventure/zone payload (player:login / party:update `adventureZones`) —
// bossDisplayName, treasureItemLevel, depth — sourced from THEME_MONSTERS
// and instanceItemLevelForDepth. There is no client-side mirror.

import { shallowRef } from 'vue';

export const adventureZoneTick = shallowRef(0);

let payloadZones = [];

const previewFrom = (zone = {}) => {
  const warden = zone.bossDisplayName || zone.warden || null;
  const itemLevel = Number(zone.treasureItemLevel ?? zone.itemLevel);
  const depth = Number(zone.depth);
  return {
    warden,
    itemLevel: Number.isFinite(itemLevel) && itemLevel > 0 ? itemLevel : null,
    depth: Number.isFinite(depth) && depth > 0 ? depth : null,
  };
};

const findPayloadZone = (zone = {}) => {
  if (!payloadZones.length) {
    return null;
  }
  if (zone.id) {
    const byId = payloadZones.find((entry) => entry.id === zone.id);
    if (byId) {
      return byId;
    }
  }
  if (zone.template) {
    return payloadZones.find((entry) => entry.template === zone.template) || null;
  }
  return null;
};

export const ingestAdventureZones = (zones) => {
  payloadZones = Array.isArray(zones) ? zones.slice() : [];
  adventureZoneTick.value += 1;
  return payloadZones;
};

export const resetAdventureZonePayload = () => {
  payloadZones = [];
  adventureZoneTick.value += 1;
};

export const getAdventureZonePayload = () => payloadZones.slice();

export const zoneObjective = (zone = {}) => {
  const sourced = zone.bossDisplayName || zone.treasureItemLevel != null || zone.depth != null
    ? zone
    : (findPayloadZone(zone) || {});
  const { warden, itemLevel, depth } = previewFrom(sourced);
  const parts = [
    warden,
    itemLevel ? `item-level ${itemLevel} gear` : null,
    depth ? `depth ${depth}` : null,
  ].filter(Boolean);

  return {
    warden,
    itemLevel,
    depth,
    line: parts.join(' · '),
  };
};

const ingestFromSocketMessage = (event) => {
  try {
    const message = JSON.parse(event.data);
    const data = message && message.data;
    if (!data || typeof data !== 'object') {
      return;
    }
    const zones = data.adventureZones
      || (data.party && data.party.adventureZones)
      || null;
    if (Array.isArray(zones) && zones.length) {
      ingestAdventureZones(zones);
    }
  } catch {
    // Non-JSON frames are ignored.
  }
};

export const bindAdventureZoneSocket = (socket) => {
  if (!socket || socket.__task0055AdventureZones) {
    return socket;
  }
  socket.__task0055AdventureZones = true;
  socket.addEventListener('message', ingestFromSocketMessage);
  return socket;
};

if (typeof window !== 'undefined') {
  const poll = () => {
    if (window.ws) {
      bindAdventureZoneSocket(window.ws);
    }
  };
  poll();
  const timer = setInterval(poll, 50);
  if (typeof window.addEventListener === 'function') {
    window.addEventListener('beforeunload', () => clearInterval(timer));
  }
}

export default { zoneObjective, ingestAdventureZones };
