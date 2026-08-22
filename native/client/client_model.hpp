#pragma once

// C3 session seam (D-122): the presentation-facing state model. Plain
// structs only — no JSON values, sockets, or Simulation pointers may appear
// here. Both LocalCoreSession and RemoteProtocolSession publish this type;
// renderers and HUD read it and nothing else.

#include <string>
#include <vector>

namespace verdigris::client {

struct ClientPlayer {
  std::string uuid;
  std::string scene_id;
  std::string display_name;  // server username / admitted scion name
  double x = 0.0;
  double y = 0.0;
  std::string facing = "down";
  int level = 1;
  int life = 100;
  int life_max = 100;
  int resource = 50;
  int resource_max = 50;
  int attack = 12;
  bool alive = true;
};

struct ClientMonster {
  std::string id;
  std::string name;
  double x = 0.0;
  double y = 0.0;
  int life = 1;
  int life_max = 1;
  bool elite = false;
  bool alive = true;
};

struct ClientItemSlot {
  std::string id;
  std::string uuid;
  std::string name;
  int slot = -1;  // -1 = unplaced
  int bonus_health = 0;
  int critical_chance = 0;
  int attack_rating = 0;
};

struct ClientGroundItem {
  std::string uuid;
  std::string name;
  double x = 0.0;
  double y = 0.0;
  // TASK-0145: set when the server marks the drop with a chroniclesRelic
  // record; `relic_of` carries the fallen scion's name for honest labeling.
  bool relic = false;
  std::string relic_of;
};

struct ClientScene {
  std::string id;
  std::string type;
  std::string name;
  double stairs_up_x = 0.0;
  double stairs_up_y = 0.0;
  bool has_stairs_up = false;
};

// TASK-0156: authoritative passive-tree progression, mirrored verbatim from
// the existing `passiveTree` wire envelope (schemaVersion 2). Plain counts
// copied from the payload only — no node semantics, costs, effects, or
// balance are derived here. `present` is the tri-state anchor: false until
// an authoritative payload arrives, so absence is never rendered as zero.
struct ClientPassiveProgression {
  bool present = false;
  int unspent_points = 0;  // passiveTree.points.skill
  int earned_points = 0;   // passiveTree.earned
  int node_count = 0;      // passiveTree.nodes entries
  int conduit_count = 0;   // passiveTree.conduits entries
};

// TASK-0145 Gate-B chronicle state, exactly as carried by the accepted wire
// contract (chronicles:state / player:chronicles:ready / scion-fallen /
// dev:state chroniclesRecord). Presentation renders this; it never invents
// Houses, Scions, oaths, or relics on its own.
struct ClientScionEntry {
  std::string id;
  std::string name;
  int level = 1;
  bool mortal = false;
};

struct ClientCryptEntry {
  std::string id;
  std::string name;
  int level = 1;
  // "" (no relic record) | "lost" | "queued" | "recovered"
  std::string relic_status;
  int relic_count = 0;
};

struct ClientHouseEntry {
  std::string id;
  std::string name;
  std::vector<ClientScionEntry> scions;
  std::vector<ClientCryptEntry> crypt;
};

struct ClientFallenScion {
  std::string scion_id;
  std::string name;
  int level = 1;
  int relic_count = 0;
};

struct ClientChronicle {
  bool present = false;  // any authoritative chronicle payload has arrived
  std::string account_name;
  std::vector<ClientHouseEntry> houses;
  std::string active_house_id;
  std::string active_scion_id;
  ClientFallenScion fallen;  // most recent fatal fall, empty until one occurs
};

const ClientHouseEntry* find_chronicle_house(const ClientChronicle& chronicle,
                                             const std::string& house_id);
const ClientScionEntry* find_chronicle_scion(const ClientChronicle& chronicle,
                                             const std::string& scion_id);

struct ClientModel {
  ClientPlayer player;
  std::vector<ClientItemSlot> inventory;
  std::vector<ClientGroundItem> ground;
  std::vector<ClientMonster> monsters;
  ClientItemSlot equipped;
  ClientScene scene;
  std::string house_name;
  // Most recent server/system message, for HUD toasts.
  std::string last_message;
  int last_outgoing_hit = 0;
  int last_incoming_hit = 0;
  int kills = 0;
  int stored_items = 0;
  int stored_trophies = 0;
  bool extracted = false;
  // TASK-0145: Chronicles front-door state. `chronicles_pending` is true from
  // the first authoritative chronicle payload until a scion admission
  // (player:login) lands — the owner is pre-game on this connection.
  ClientChronicle chronicle;
  bool chronicles_pending = false;
  std::string lifecycle;  // "alive" | "awaiting-respawn" | "permadead"
  // TASK-0156: mirrored passive-tree progression (absent until a payload
  // arrives on the wire).
  ClientPassiveProgression progression;
};

inline const ClientHouseEntry* find_chronicle_house(const ClientChronicle& chronicle,
                                                    const std::string& house_id) {
  for (const auto& house : chronicle.houses)
    if (house.id == house_id) return &house;
  return nullptr;
}

inline const ClientScionEntry* find_chronicle_scion(const ClientChronicle& chronicle,
                                                    const std::string& scion_id) {
  for (const auto& house : chronicle.houses)
    for (const auto& scion : house.scions)
      if (scion.id == scion_id) return &scion;
  return nullptr;
}

}  // namespace verdigris::client