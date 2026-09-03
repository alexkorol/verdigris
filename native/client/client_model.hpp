#pragma once

// C3 session seam (D-122): the presentation-facing state model. Plain
// structs only — no JSON values, sockets, or Simulation pointers may appear
// here. Both LocalCoreSession and RemoteProtocolSession publish this type;
// renderers and HUD read it and nothing else.

#include <cstdint>
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
  int cooldown_ticks = 0;
  int war_cry_ticks_remaining = 0;
  bool alive = true;
};

struct ClientMonster {
  std::string id;
  std::string name;
  std::string kind;       // stable monster kind id ("crypt-lurker", ...)
  std::string behaviour;  // "melee" / "ranged" / "buffer"
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
  bool expedition_map = false;
  int map_tier = 0;
  int map_goods_found_percent = 0;
  std::vector<std::string> map_modifiers;
};

// One purchasable row of a trader's stock (open:screen shop payload).
struct ClientShopRow {
  std::string id;
  std::string name;
  int price = 0;
  int qty = 0;
};

// The trader screen as the server last published it. `open` flips true on
// every open:screen and is cleared client-side by the CloseScreen command;
// rows and coins are authoritative payload mirrors only.
struct ClientShopScreen {
  bool open = false;
  std::string name;
  std::vector<ClientShopRow> rows;
  int carried_coins = 0;
};

// One stored item in the countinghouse (open:screen bank payload).
struct ClientBankItem {
  std::string uuid;
  std::string name;
  int qty = 0;
};

struct ClientBankScreen {
  bool open = false;
  int treasury = 0;
  int carried_coins = 0;
  std::vector<ClientBankItem> items;
};

// One node row of a road chart (open:screen chart payload).
struct ClientChartNode {
  std::string id;      // "tin:1:0" - feeds world:zone:enter verbatim
  std::string name;
  std::string warden;
  std::string status;  // "open" | "cleared" | "barred"
  int tier = 1;
};

struct ClientChartScreen {
  bool open = false;
  std::string road_id;
  std::string road_name;
  std::string blurb;
  std::vector<ClientChartNode> nodes;
};

struct ClientDialogueOption {
  std::string id;
  std::string label;
  std::string hint;
  std::string action;
  bool enabled = true;
};

struct ClientDialogueScreen {
  bool open = false;
  int npc_id = 0;
  std::string npc_key;
  std::string name;
  std::string role;
  std::string body;
  std::vector<ClientDialogueOption> options;
};

struct ClientHouseInvestment {
  bool first_clear_completed = false;
  bool eligible = false;
  std::string choice = "unchosen";
  bool reward_claimed = false;
  int scion_gear_tier = 0;
  int house_income_per_clear = 0;
};

// Town NPC roster entry, mirrored from the server's `npcs` snapshot array.
// Positions are protocol tile units like monsters; `actions` carries the
// server-authored verb list ("talk", "trade", "bank", "examine").
struct ClientNpc {
  int id = 0;
  std::string key;
  std::string name;
  std::string role;
  std::string examine;
  double x = 0.0;
  double y = 0.0;
  std::vector<std::string> services;
  std::vector<std::string> actions;
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
  // Verbatim string mirrors so the tree pane can render and extend the
  // authoritative allocation ("q,r" axial ids; root is "0,0").
  std::vector<std::string> nodes;
  std::vector<std::string> conduits;
  std::string selected_node;
};

struct ClientCompletedQuest {
  std::string id;
  std::string title;
  std::string deed;
};

// Authoritative campaign journal. All player-facing copy arrives from the
// server alongside the exact objective cursor; the client only lays it out.
struct ClientQuestState {
  bool present = false;
  bool campaign_complete = false;
  int quest_points = 0;
  int house_renown = 0;
  std::string active_id;
  std::string title;
  std::string giver;
  std::string summary;
  std::string objective;
  std::string reward;
  int objective_index = 0;
  int objective_count = 0;
  std::vector<ClientCompletedQuest> completed;
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
  bool campaign_complete = false;
  int endgame_maps_completed = 0;
  std::vector<ClientScionEntry> scions;
  std::vector<ClientCryptEntry> crypt;
};

struct ClientEndgameState {
  bool present = false;
  bool unlocked = false;
  bool active = false;
  bool cleared = false;
  int completed = 0;
  int tier = 0;
  int goods_found_percent = 0;
  std::string name;
  std::vector<std::string> modifiers;
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
  std::vector<ClientNpc> npcs;
  ClientShopScreen shop;
  ClientBankScreen bank;
  ClientChartScreen chart;
  ClientDialogueScreen dialogue;
  ClientHouseInvestment house_investment;
  // stats-manager attributes from the dev:state snapshot.
  int attr_strength = 10;
  int attr_dexterity = 10;
  int attr_intelligence = 10;
  // Walkable grid for the current scene (requested once per scene change).
  // Row-major, 1 = walkable; empty until the first map payload arrives.
  int map_width = 0;
  int map_height = 0;
  std::string map_scene_id;
  std::vector<std::uint8_t> map_walkable;
  // Authoritative scene theme ("town", "dungeon", "crypt", "wilds",
  // "marsh", "grove") from the dev:state snapshot.
  std::string theme = "town";
  // Combat experience from the authoritative snapshot. Older servers may
  // omit this block, so presence is explicit instead of fabricating a zero
  // progress bar from defaults.
  bool xp_present = false;
  double xp_current = 0.0;
  double xp_floor = 0.0;
  double xp_next = 0.0;
  ClientEndgameState endgame;
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
  ClientQuestState quests;
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
