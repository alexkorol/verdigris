#include "verdigris/networking.hpp"
#include <cmath>
#include <iostream>
#include <stdexcept>
using namespace verdigris;
using namespace verdigris::networking;
void require(bool ok, const char* text) { if (!ok) throw std::runtime_error(text); }
int main() {
  try {
    const auto ignore=[](const Envelope&){};
    ProtocolSession a("actor:a","socket:a",71,true), b("actor:b","socket:b",72,true);
    a.enter_shared_instance("instance:qa:1",ignore);
    b.adopt_world(a.shared_world(),"instance:qa:1",ignore);
    const auto before=b.shared_world()->position();
    a.shared_world()->teleport(10,10,0);
    require(b.shared_world()->position().x==before.x && b.shared_world()->position().y==before.y,
            "A movement overwrites B actor position");
    require(a.shared_world()->scene_id()==b.shared_world()->scene_id(),"actors share instance identity");
    const auto monsters=b.shared_world()->monsters().size();
    a.leave_to_town(ignore);
    require(b.shared_world()->in_instance() && b.shared_world()->monsters().size()==monsters,
            "leaving destroys ally encounter");
    std::cout << "native co-op actor regressions: PASS\n";
    return 0;
  } catch (const std::exception& e) { std::cerr << "native co-op actor regressions: FAIL: "<<e.what()<<'\n'; return 1; }
}
