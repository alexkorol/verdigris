/**
 * Crash-safe handoff for the D-109 disconnect boundary.  If the normal
 * profile writer is unavailable, capture the exact durable guest-shaped
 * snapshot before removing the socket.  The record is intentionally explicit
 * and retryable; a failed save is never treated as a successful empty save.
 */
import fs from 'node:fs';
import path from 'node:path';
import { fileURLToPath } from 'node:url';
import { buildGuestSnapshot } from '#server/core/repositories/guest-save-store.js';
import { buildScionSnapshot } from '#server/core/services/chronicles.js';

const here = path.dirname(fileURLToPath(import.meta.url));
const DEFAULT_QUEUE_FILE = path.resolve(here, '..', '..', 'data', 'disconnect-save-queue.json');

const queueFile = () => path.resolve(process.env.DISCONNECT_SAVE_QUEUE_FILE || DEFAULT_QUEUE_FILE);

export const enqueueDisconnectSave = (player, error = null) => {
  if (!player?.uuid) return { ok: false, reason: 'Player has no durable identity.' };
  const snapshot = player.scionId && player.accountId
    ? buildScionSnapshot(player)
    : buildGuestSnapshot(player);
  const file = queueFile();
  try {
    let entries = [];
    if (fs.existsSync(file)) {
      const parsed = JSON.parse(fs.readFileSync(file, 'utf8'));
      entries = Array.isArray(parsed) ? parsed : [];
    }
    const record = {
      uuid: String(player.uuid),
      accountId: player.accountId || null,
      scionId: player.scionId || null,
      queuedAt: new Date().toISOString(),
      attempts: 1,
      error: error ? String(error.message || error) : 'profile save failed',
      snapshot,
    };
    const existing = entries.findIndex(entry => entry.uuid === record.uuid);
    if (existing >= 0) {
      entries[existing] = { ...entries[existing], ...record, attempts: (entries[existing].attempts || 0) + 1 };
    } else {
      entries.push(record);
    }
    fs.mkdirSync(path.dirname(file), { recursive: true });
    const temporary = `${file}.${process.pid}.tmp`;
    fs.writeFileSync(temporary, JSON.stringify(entries, null, 2), 'utf8');
    fs.renameSync(temporary, file);
    return { ok: true, file, record };
  } catch (queueError) {
    return { ok: false, reason: queueError.message };
  }
};

export const readDisconnectSaveQueue = () => {
  try {
    if (!fs.existsSync(queueFile())) return [];
    const parsed = JSON.parse(fs.readFileSync(queueFile(), 'utf8'));
    return Array.isArray(parsed) ? parsed : [];
  } catch {
    return [];
  }
};

export default { enqueueDisconnectSave, readDisconnectSaveQueue };
