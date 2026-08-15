import { buildGuestSnapshot, saveGuest } from '#server/core/repositories/guest-save-store.js';
import identityRegistry from '#server/core/services/identity-registry.js';
import world from '#server/core/world.js';

const DEFAULT_COOLDOWN_MS = Number(process.env.PLAYER_SAVE_COOLDOWN_MS) || 60000;

const localAccountId = (player) => {
  const token = typeof player?.token === 'string' ? player.token : '';
  return token.startsWith('local:') ? token.slice('local:'.length) : null;
};

export const saveLocalAccountProfile = (player) => {
  const accountId = localAccountId(player);
  if (!accountId) return null;
  const snapshot = buildGuestSnapshot(player);
  return identityRegistry.updateLoginProfile(accountId, snapshot) ? snapshot : null;
};

export class PlayerPersistenceService {
  constructor({
    saveGuestPlayer = saveGuest,
    saveLocalProfile = saveLocalAccountProfile,
    saveChroniclePlayer = null,
    cooldownMs = DEFAULT_COOLDOWN_MS,
    logger = console,
  } = {}) {
    this.saveGuestPlayer = saveGuestPlayer;
    this.saveLocalProfile = saveLocalProfile;
    this.saveChroniclePlayer = saveChroniclePlayer;
    this.cooldownMs = cooldownMs;
    this.logger = logger;
    this.lastSuccessfulSave = new Map();
  }

  shouldThrottleSave(player, { force = false } = {}) {
    if (force) {
      return false;
    }

    const lastSavedAt = this.lastSuccessfulSave.get(player.uuid) || 0;
    const elapsed = Date.now() - lastSavedAt;

    return elapsed < this.cooldownMs;
  }

  async savePlayer(player, options = {}) {
    if (!player) {
      return null;
    }

    if (this.shouldThrottleSave(player, options)) {
      return null;
    }

    // Interactive browser Chronicles still use the durable JSON identity
    // record and guest/local character snapshot. Their House identity is also
    // mirrored into SQLite so wagons and the world web can share the same
    // lineage without making reconnects forget the selected Scion.
    if (player.legacyChroniclesStore) {
      try {
        const legacySnapshot = localAccountId(player)
          ? await this.saveLocalProfile(player)
          : await this.saveGuestPlayer(player);
        const ledgerSnapshot = await this.saveChronicleSnapshot(player);
        const result = legacySnapshot || ledgerSnapshot;
        if (result) this.lastSuccessfulSave.set(player.uuid, Date.now());
        return result;
      } catch (error) {
        if (this.logger && typeof this.logger.error === 'function') {
          this.logger.error(`[player-persistence] Failed to save ${player.username || player.uuid} locally`, error);
        }
        throw error;
      }
    }

    // Chronicle-auth scions are authoritative server-side characters. Keep
    // their complete snapshot with the House. Dynamic import avoids a
    // Player/service initialization cycle.
    if (player.scionId && player.accountId) {
      const snapshot = await this.saveChronicleSnapshot(player);
      if (snapshot) this.lastSuccessfulSave.set(player.uuid, Date.now());
      return snapshot;
    }

    try {
      // Local login accounts live in the same SQLite registry as credentials.
      // Any older non-local token falls back to a machine-local snapshot; the
      // archived website API is no longer part of Verdigris persistence.
      const result = localAccountId(player)
        ? await this.saveLocalProfile(player)
        : await this.saveGuestPlayer(player);
      if (!result) return null;
      this.lastSuccessfulSave.set(player.uuid, Date.now());
      return result;
    } catch (error) {
      if (this.logger && typeof this.logger.error === 'function') {
        this.logger.error(`[player-persistence] Failed to save ${player.username || player.uuid} locally`, error);
      }
      throw error;
    }
  }

  async saveChronicleSnapshot(player) {
    if (typeof this.saveChroniclePlayer === 'function') {
      return this.saveChroniclePlayer(player);
    }
    const { saveLivingScion } = await import('#server/core/services/chronicles.js');
    return saveLivingScion(player);
  }

  async flushAllPlayers(options = {}) {
    const players = [...world.players];
    if (!players.length) {
      return { saved: 0, total: 0 };
    }

    const results = await Promise.all(players.map(async (player) => {
      try {
        const saved = await this.savePlayer(player, options);
        return saved ? 1 : 0;
      } catch (error) {
        return 0;
      }
    }));

    const savedCount = results.reduce((sum, value) => sum + value, 0);
    return { saved: savedCount, total: players.length };
  }

  markDirty(player) {
    if (!player) {
      return;
    }

    this.lastSuccessfulSave.set(player.uuid, 0);
  }
}

const playerPersistenceService = new PlayerPersistenceService();

export default playerPersistenceService;
