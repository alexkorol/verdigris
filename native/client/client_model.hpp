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
  int attack = 12;
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
  ClientItemSlot equipped;
  ClientScene scene;
  std::string house_name;
  // Most recent server/system message, for HUD toasts.
  std::string last_message;
  int last_outgoing_hit = 0;
  int last_incoming_hit = 0;
  int kills = 0;
  bool extracted = false;
};

}  // namespace verdigris::client
