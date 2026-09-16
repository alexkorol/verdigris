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
  // TASK-0145: set when the server tagged this ground item with
  // chroniclesRelic{relicId, scionId, scionName} (heirloom circulation).
  bool relic = false;
  std::string relic_scion_name;
};

// TASK-0145: one Scion row as the accepted chronicle payloads carry it —
// living roster entry or crypt record. Plain data only; no JSON types.
struct ClientChronicleScion {
  std::string id;
  std::string name;
  int level = 1;
  bool mortal = false;
  bool in_crypt = false;
  // Crypt-only heirloom state: "", "lost", "queued", or "recovered".
  std::string relic_status;
};

struct ClientChronicleHouse {
  std::string id;
  std::string name;
  std::vector<ClientChronicleScion> scions;
  std::vector<ClientChronicleScion> crypt;
};

// TASK-0145: presentation-facing snapshot of the accepted Gate-B envelopes
// (chronicles:state, player:chronicles:ready/update, chronicles:scion-fallen).
// The session is the only writer; renderers and HUD read it and nothing else.
struct ClientChronicles {
  bool exists = false;  // chroniclesExists / revision > 0
  int revision = 0;
  std::string account_id;    // chroniclesAccountId
  std::string account_name;  // accountName
  std::vector<ClientChronicleHouse> houses;
  std::string active_house_id;   // chronicle.activeHouseId
  std::string active_scion_id;   // session-authoritative when exposed
  std::string active_house_name; // derived from houses for display
  std::string active_scion_name; // tracked from create/select admissions
  // Fatal-fall report (chronicles:scion-fallen only — never inferred).
  bool fallen_report = false;
  std::string fallen_scion_name;
  int fallen_level = 0;
  int relic_count = 0;
};

struct ClientScene {
  std::string id;
  std::string type;
  std::string name;
  double stairs_up_x = 0.0;
  double stairs_up_y = 0.0;
  bool has_stairs_up = false;
};

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
};

}  // namespace verdigris::client
