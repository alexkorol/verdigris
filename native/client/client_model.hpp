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
  bool alive = true;
};

struct ClientItemSlot {
  std::string id;
  std::string uuid;
  std::string name;
  int slot = -1;  // -1 = unplaced
};

struct ClientScene {
  std::string id;
  std::string type;
  std::string name;
};

struct ClientModel {
  ClientPlayer player;
  std::vector<ClientItemSlot> inventory;
  ClientScene scene;
  std::string house_name;
  // Most recent server/system message, for HUD toasts.
  std::string last_message;
};

}  // namespace verdigris::client
