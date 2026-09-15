#include "verdigris/networking.hpp"
#include "verdigris/starter_layout.hpp"

#include <algorithm>
#include <chrono>
#include <filesystem>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

using verdigris::networking::Envelope;
using verdigris::networking::JsonValue;
using verdigris::networking::ProtocolSession;

namespace {
void check(bool value, const char* message) { if (!value) throw std::runtime_error(message); }
std::string text(const JsonValue& value) { return value.string() ? *value.string() : ""; }
JsonValue state(ProtocolSession& session) {
  JsonValue result;
  check(verdigris::networking::parse_json(session.state_payload("starter-test"), result), "state parses");
  return result["state"];
}
std::string phase(ProtocolSession& session) { return text(state(session)["starterSlice"]["phase"]); }
int item_count(const JsonValue& snapshot, const char* id) {
  int count = 0;
  if (const auto* items = snapshot["inventoryDetails"].array())
    for (const auto& item : *items) if (text(item["id"]) == id) ++count;
  if (const auto* seats = snapshot["wearDetails"].object())
    for (const auto& [seat, item] : *seats) if (text(item["id"]) == id) ++count;
  return count;
}
bool reachable(const verdigris::TileGrid& grid, verdigris::Vec2 start, verdigris::Vec2 goal) {
  std::vector<bool> seen(static_cast<std::size_t>(grid.width * grid.height));
  std::vector<verdigris::Vec2> frontier{start};
  const verdigris::Vec2 steps[] = {{1,0}, {-1,0}, {0,1}, {0,-1}};
  if (!grid.walkable_at(start.x, start.y)) return false;
  seen[start.y * grid.width + start.x] = true;
  for (std::size_t i = 0; i < frontier.size(); ++i) {
    const auto at = frontier[i];
    if (at.x == goal.x && at.y == goal.y) return true;
    for (const auto step : steps) {
      const verdigris::Vec2 next{at.x + step.x, at.y + step.y};
      if (!grid.walkable_at(next.x, next.y)) continue;
      const auto index = next.y * grid.width + next.x;
      if (!seen[index]) { seen[index] = true; frontier.push_back(next); }
    }
  }
  return false;
}
struct Fixture {
  ProtocolSession session{"starter-test", "starter-socket", 81, false};
  std::vector<Envelope> events;
  std::string house, scion;
  std::int64_t now = std::chrono::duration_cast<std::chrono::milliseconds>(
      std::chrono::system_clock::now().time_since_epoch()).count();
  Fixture() {
    session.set_direct_emit([this](const Envelope& event) { events.push_back(event); });
    send("chronicles:house:found", {{"name", "House of the Palisade"}});
    for (const auto& event : events)
      if (const auto* houses = event.data["chronicle"]["houses"].array(); houses && !houses->empty())
        house = text(houses->front()["id"]);
    check(!house.empty(), "real House creation succeeds");
    scion = create("First Defender");
    admit(scion);
  }
  void send(const char* event, JsonValue::Object data = {}) {
    session.handle({event, std::move(data)}, [this](const Envelope& e) { events.push_back(e); });
  }
  std::string create(const char* name) {
    events.clear(); send("chronicles:scion:create", {{"houseId", house}, {"name", name}});
    for (const auto& event : events) if (event.data["createdScionId"].string()) return text(event.data["createdScionId"]);
    throw std::runtime_error("real Scion creation succeeds");
  }
  void admit(const std::string& id) { send("chronicles:scion:set-out", {{"scionId", id}, {"starterSlice", true}}); }
  void action(const char* action) { send("starter:action", {{"action", action}}); }
  void at(int x, int y) { session.shared_world()->teleport(x, y, now); }
  void tick() { session.tick(now += 150); }
  void clear_wave() { session.shared_world()->kill_all_monsters(); tick(); }
  void start() { action("field_hand"); at(16,22); action("interact"); }
};

void test_admission_choice_and_tool() {
  Fixture f;
  check(f.session.shared_world()->scene_id() == "owner-demo-prologue" && phase(f.session) == "occupation", "new starter arrives at threatened village");
  const auto& grid = f.session.shared_world()->grid();
  const auto layout=verdigris::starter_layout::generate();
  const auto replay=verdigris::starter_layout::generate();
  const auto variation=verdigris::starter_layout::generate(42);
  check(layout.props.size()==replay.props.size() && layout.props.size()>60 && layout.props.size()<240,
        "procedural scenery is deterministic and bounded");
  bool changed=layout.props.size()!=variation.props.size();
  for(std::size_t i=0;i<layout.props.size();++i) {
    const auto& a=layout.props[i];const auto& b=replay.props[i];
    check(a.x==b.x && a.y==b.y && a.kind==b.kind,"same seed preserves every scenery placement");
    if(i<variation.props.size())changed|=a.x!=variation.props[i].x || a.y!=variation.props[i].y;
    const int x=int(std::round(a.x)),y=int(std::round(a.y));
    if(a.solid && x>=3 && x<29 && y>=3 && y<29)
      check(!grid.walkable_at(x,y),"tree trunks and rocks match the server obstacle map");
  }
  check(changed,"different layout seeds produce different scenery");
  int openFormerHouse=0;
  for(int y=11;y<=17;++y)for(int x=5;x<=9;++x)openFormerHouse+=grid.walkable_at(x,y);
  check(openFormerHouse>20,"removing the house also removes its invisible rectangular wall");
  for (const auto goal : {verdigris::Vec2{16,22}, verdigris::Vec2{16,20}, verdigris::Vec2{14,15}, verdigris::Vec2{16,8}})
    check(reachable(grid, {16,23}, goal), "authority collision map connects the start, tool, rally, enemies and exit");
  check(state(f.session)["passiveTree"]["points"]["skill"].number().value_or(-1) == 0,
        "fresh starter has no skill point before defending the village");
  JsonValue login;
  check(verdigris::networking::parse_json(f.session.login_payload(), login), "login parses");
  check(text(login["player"]["starterSlice"]["phase"]) == "occupation", "normal login exposes starter state");
  const auto baseline = state(f.session)["attributes"];
  f.action("interact"); f.action("invented-occupation");
  check(phase(f.session) == "occupation", "tool and invalid choice cannot bypass occupation");
  f.action("scout");
  auto selected = state(f.session);
  check(phase(f.session) == "tool" && selected["attributes"]["dexterity"].number().value_or(0) == baseline["dexterity"].number().value_or(0) + 1, "occupation grants its one attribute");
  f.action("scout"); f.action("scribe");
  check(state(f.session)["attributes"].stringify() == selected["attributes"].stringify(), "choice is immutable and cannot farm attributes");
  f.at(12,24); f.action("interact");
  check(phase(f.session) == "tool", "remote interaction cannot collect starter tool");
  f.at(16,22); f.action("interact");
  check(phase(f.session) == "wave" && f.session.shared_world()->monsters().size() == 2, "collecting tool begins first pack");
  check(item_count(state(f.session), "wooden-club") == 1 && item_count(state(f.session), "bronze-dagger") == 0, "exactly one appropriate starter weapon is granted");
  f.action("interact"); f.admit(f.scion);
  check(item_count(state(f.session), "wooden-club") == 1, "repeat interaction and admission cannot duplicate starter weapon");
  f.send("instance:extract"); f.send("instance:enterSolo", {{"template", "crypt"}, {"layout", "warren"}});
  f.send("world:zone:enter", {{"zoneId", "thornward"}});
  check(f.session.shared_world()->scene_id() == "owner-demo-prologue", "active prologue cannot be bypassed by exit or zone commands");
}

void test_combat_and_forgiving_retry() {
  Fixture f; f.start();
  const auto initial_xp = state(f.session)["xp"]["current"].number().value_or(-1);
  f.at(14,16);
  f.send("player:skill:trigger", {{"direction", "up"}});
  bool killed = false;
  for (int step = 0; step < 100 && !killed; ++step) {
    f.tick();
    for (const auto& m : f.session.shared_world()->monsters()) if (!m.alive) killed = true;
  }
  check(killed, "ordinary skill command and server combat kill a real first-wave opponent");
  check(state(f.session)["xp"]["current"].number().value_or(-2) == initial_xp, "individual prologue kills cannot farm experience");
  check(f.session.shared_world()->ground_items().empty(), "individual prologue kills cannot farm loot");
  // Exercise the real lethal-hit path, not the unrelated developer kill verb.
  f.send("dev:hurt", {{"amount", 100000}});
  for (const auto& m : f.session.shared_world()->monsters()) if (m.alive) { f.at(m.x, m.y + 1); break; }
  for (int step = 0; step < 150 && phase(f.session) != "tool"; ++step) f.tick();
  const auto retried = state(f.session);
  check(phase(f.session) == "tool" && text(retried["lifecycle"]) == "alive", "lethal encounter damage retries without creating a dead Scion");
  check(retried["hp"]["current"].number() == retried["hp"]["max"].number(), "retry restores full health");
  check(f.session.shared_world()->scene_id() == "owner-demo-prologue" && f.session.shared_world()->monsters().empty(), "retry resets encounters inside the village");
  check(retried["xp"]["current"].number().value_or(-2) == initial_xp, "retry has no earned-value exploit");
  f.at(16,22); f.action("interact");
  check(phase(f.session) == "wave" && item_count(state(f.session), "wooden-club") == 1, "retry is playable without duplicating the tool");
}

void test_victory_persistence_and_scion_isolation() {
  const auto path = std::filesystem::temp_directory_path() / ("verdigris-starter-" + std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()) + ".json");
  struct Cleanup { std::filesystem::path path; ~Cleanup() { std::error_code ignored; std::filesystem::remove(path, ignored); } } cleanup{path};
  Fixture f; f.session.attach_persistence(path); f.start();
  f.clear_wave();
  check(phase(f.session) == "rally", "first pack ends at safe rally checkpoint");
  f.session.persist(); f.session.reset_world_for_new_socket();
  check(phase(f.session) == "rally" && f.session.shared_world()->scene_id() == "owner-demo-prologue", "reconnect resumes the active checkpoint");
  f.at(16,20); f.action("interact");
  check(f.session.shared_world()->monsters().size() == 3, "second pack has three attackers");
  f.session.persist();
  {
    ProtocolSession resumed("starter-test", "mid-wave-restart", 81, false);
    resumed.attach_persistence(path); resumed.reset_world_for_new_socket();
    check(phase(resumed) == "wave" && state(resumed)["starterSlice"]["wave"].number().value_or(0) == 2,
          "process restart restores the second wave rather than skipping or restarting the story");
    check(resumed.shared_world()->monsters().size() == 3 && item_count(state(resumed), "wooden-club") == 1,
          "restored combat checkpoint has its opponents and original weapon");
    check(state(resumed)["attributes"].stringify() == state(f.session)["attributes"].stringify(),
          "occupation bonus survives restart without being applied twice");
  }
  f.clear_wave(); f.at(16,20); f.action("interact");
  check(f.session.shared_world()->monsters().size() == 1 && f.session.shared_world()->monsters().front().boss, "last encounter is one boss");
  f.clear_wave();
  const auto victory = state(f.session);
  check(phase(f.session) == "victory" && victory["level"].number().value_or(0) == 2, "village victory grants first level");
  check(victory["passiveTree"]["points"]["skill"].number().value_or(-1) == 1,
        "first victory grants exactly the first spendable skill point");
  const auto xp = victory["xp"]["current"].number();
  f.clear_wave(); f.action("interact");
  check(state(f.session)["xp"]["current"].number() == xp && phase(f.session) == "victory", "repeated clear or distant interaction cannot repeat reward or exit");
  const auto second = f.create("Second Defender"); f.admit(second);
  check(phase(f.session) == "occupation" && state(f.session)["level"].number().value_or(0) == 1, "new Scion has independent starter progress");
  f.admit(f.scion);
  check(phase(f.session) == "victory" && state(f.session)["xp"]["current"].number() == xp, "returning Scion restores its own earned victory");
  f.at(16,8); f.action("interact");
  check(phase(f.session) == "departed" && f.session.shared_world()->scene_type() == "town", "victory exit leads to Crossroads");
  f.session.persist();
  ProtocolSession restored("starter-test", "restarted-socket", 81, false);
  restored.attach_persistence(path); restored.reset_world_for_new_socket();
  restored.handle({"chronicles:scion:set-out", JsonValue::Object{{"scionId", f.scion}, {"starterSlice", true}}}, [](const Envelope&) {});
  check(phase(restored) == "departed" && restored.shared_world()->scene_type() == "town", "completed Scion stays beyond prologue after process restart");
  check(state(restored)["xp"]["current"].number() == xp && item_count(state(restored), "wooden-club") == 1, "restart preserves reward and exactly one tool");
}
} // namespace

int main() {
  try {
    test_admission_choice_and_tool();
    std::cout << "  admission, occupation, tool and early-exit authority passed\n";
    test_combat_and_forgiving_retry();
    std::cout << "  real combat and forgiving retry passed\n";
    test_victory_persistence_and_scion_isolation();
    std::cout << "Starter slice authority tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << "Starter slice test failed: " << error.what() << '\n';
    return 1;
  }
}
