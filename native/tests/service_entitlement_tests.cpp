// Headless authority fixtures: real ProtocolSessions and shared WorldSimulation,
// ordinary NPC/admission/extraction commands, no service development endpoints.
#include "verdigris/networking.hpp"
#include <iostream>
#include <stdexcept>

using namespace verdigris;
using namespace verdigris::networking;
namespace {
int checks=0,failures=0;
void check(bool value,const char* label){++checks;if(!value)++failures;std::cout<<(value?"PASS ":"FAIL ")<<label<<'\n';}
void setup(bool value,const char* label){if(!value)throw std::runtime_error(label);}
std::string text(const JsonValue& value){return value.string()?*value.string():"";}
JsonValue saved(ProtocolSession& session){JsonValue data;setup(parse_json(session.durable_payload(),data),"durable parse");return data;}
void send(ProtocolSession& session,const char* event,JsonValue data=JsonValue::Object{}){session.handle({event,std::move(data)},[](const Envelope&){});}
struct Actor {
    ProtocolSession session;
    std::string house,scion;
    explicit Actor(const char* id):session(id,std::string("socket:")+id,42,false){
        session.enable_service();session.set_direct_emit([](const Envelope&){});
        send(session,"chronicles:house:found",JsonValue::Object{{"name",std::string("House ")+id}});
        auto data=saved(session);house=text(data["chronicle"]["houses"].array()->back()["id"]);
        send(session,"chronicles:scion:create",JsonValue::Object{{"houseId",house},{"name",id}});
        data=saved(session);scion=text(data["chronicle"]["houses"].array()->back()["scions"].array()->back()["id"]);
        send(session,"chronicles:scion:set-out",JsonValue::Object{{"scionId",scion}});setup(session.actor_alive(),"living actor admission");
    }
    JsonValue progress(){return saved(session)["houseProgress"][house];}
    JsonValue entitlement(){return progress()["entitlements"]["first-warden:"+house];}
    int points(){return static_cast<int>(saved(session)["scionLoadouts"][house+":"+scion]["treeQuestPoints"].number().value_or(0));}
    void accept(){session.shared_world()->teleport(35,115,0);send(session,"player:npc:talk",JsonValue::Object{{"item",JsonValue::Object{{"id",1}}}});
        setup(text(progress()["firstGoal"])=="clear-floor","ordinary Aldwyn first-goal acceptance");}
};
void guide(ProtocolSession& session){session.shared_world()->teleport(35,115,0);send(session,"player:npc:talk",JsonValue::Object{{"item",JsonValue::Object{{"id",1}}}});}
void return_safely(ProtocolSession& session){
    auto world=session.shared_world();setup(world->in_instance(),"extraction starts in expedition");
    const auto exit=world->metadata().stairs_up;bool placed=false;
    for(const auto delta:{Vec2{1,0},Vec2{-1,0},Vec2{0,1},Vec2{0,-1}}){const Vec2 at{exit.x+delta.x,exit.y+delta.y};
        if(!world->grid().walkable_at(at.x,at.y))continue;
        if(at.x==world->metadata().stairs_down.x&&at.y==world->metadata().stairs_down.y)continue;
        world->teleport(at.x,at.y,0);placed=world->in_instance();if(placed)break;
    }
    setup(placed,"walkable extraction-adjacent fixture");send(session,"player:extract");setup(!session.shared_world()->in_instance(),"ordinary extraction returned to town");
}
void run(){
    Actor a("entitlement-a"),b("entitlement-b"),dead("entitlement-dead"),absent("entitlement-absent"),unaccepted("entitlement-unaccepted");
    a.accept();b.accept();dead.accept();absent.accept();
    check(a.points()==0&&b.points()==0,"accepting the goal grants no skill point");
    const std::string instance="instance:entitlement:shared-warden";
    a.session.enter_shared_instance(instance,[](const Envelope&){});
    b.session.adopt_world(a.session.shared_world(),instance,[](const Envelope&){});
    dead.session.adopt_world(a.session.shared_world(),instance,[](const Envelope&){});
    unaccepted.session.adopt_world(a.session.shared_world(),instance,[](const Envelope&){});
    auto mortality=saved(dead.session);(*mortality.object())["life"]=0;(*mortality.object())["lifecycle"]="permadead";
    setup(dead.session.restore_durable(mortality.stringify()),"dead participant fixture");dead.session.shared_world()->set_actor_alive(false);
    check(a.session.shared_world()->instance_key()==b.session.shared_world()->instance_key()&&a.session.runtime_actor_id()!=b.session.runtime_actor_id(),"distinct living actors occupy one shared instance");
    check(!a.session.shared_first_warden()&&!b.session.shared_first_warden(),"living Warden grants no entitlement");
    std::string boss;for(const auto& monster:a.session.shared_world()->monsters())if(monster.boss)boss=monster.uuid;
    setup(!boss.empty()&&a.session.shared_world()->metadata().depth==1,"shared depth-one Warden fixture");
    // Only the fixture arranges the resolved encounter. This invokes the core
    // test seam, not a client command or a production administration endpoint.
    a.session.shared_world()->kill_all_monsters();
    check(a.session.shared_first_warden()&&b.session.shared_first_warden(),"one resolved Warden grants both eligible living Houses an entitlement");
    check(!dead.session.shared_first_warden()&&dead.entitlement().is_null(),"dead participant receives no Warden entitlement");
    check(!absent.session.shared_first_warden()&&absent.entitlement().is_null(),"nonpresent House receives no shared entitlement");
    check(!unaccepted.session.shared_first_warden()&&unaccepted.entitlement().is_null(),"present House without accepted goal receives no entitlement");
    const auto earned_a=a.entitlement(),earned_b=b.entitlement();
    check(text(earned_a["status"])=="earned"&&text(earned_b["status"])=="earned","entitlements are earned before safe return");
    check(text(earned_a["houseId"])==a.house&&text(earned_b["houseId"])==b.house&&a.house!=b.house,
        "durable entitlements identify distinct owning Houses");
    check(text(earned_a["scionId"])==a.scion&&text(earned_b["scionId"])==b.scion,"each entitlement records its own participating Scion");
    check(text(earned_a["instanceId"])==instance&&text(earned_b["instanceId"])==instance&&text(earned_a["bossId"])==boss&&text(earned_b["bossId"])==boss,
        "both independent entitlements retain the same source instance and Warden");
    check(saved(a.session)["houseProgress"][b.house].is_null()&&saved(b.session)["houseProgress"][a.house].is_null(),"one House's entitlement never enters another account snapshot");
    const auto checkpoint_a=a.session.durable_payload();
    check(!a.session.shared_first_warden()&&!b.session.shared_first_warden()&&a.points()==0&&b.points()==0,"repeated Warden notification neither duplicates entitlement nor grants early reward");
    return_safely(a.session);
    check(a.points()==1&&text(a.entitlement()["status"])=="redeemed"&&text(a.progress()["firstGoal"])=="complete","A safe return redeems its own entitlement once");
    check(b.points()==0&&text(b.entitlement()["status"])=="earned"&&b.session.shared_world()->in_instance(),"A return does not redeem B reward or reset B expedition");
    return_safely(b.session);
    check(b.points()==1&&text(b.entitlement()["status"])=="redeemed","B safe return independently redeems its own entitlement");
    check(a.session.gameplay_snapshot(false)["state"]["questPoints"].number()==std::optional<double>(1)&&
        b.session.gameplay_snapshot(false)["state"]["questPoints"].number()==std::optional<double>(1),"production snapshots expose each Scion's authoritative earned point");
    for(int repeat=0;repeat<5;++repeat){guide(a.session);guide(b.session);send(a.session,"player:extract");send(b.session,"player:extract");}
    check(a.points()==1&&b.points()==1,"repeated guide and extraction claims do not double reward");
    ProtocolSession redeemed(a.session.identity(),"socket:redeemed-restart",42,false);redeemed.enable_service();
    setup(redeemed.restore_durable(a.session.durable_payload()),"redeemed account restore");guide(redeemed);send(redeemed,"player:extract");
    auto restored=saved(redeemed);
    check(restored["scionLoadouts"][a.house+":"+a.scion]["treeQuestPoints"].number()==std::optional<double>(1)&&
        text(restored["houseProgress"][a.house]["entitlements"]["first-warden:"+a.house]["status"])=="redeemed","restored redeemed account cannot receive a second point");
    ProtocolSession earned(a.session.identity(),"socket:earned-restart",42,false);earned.enable_service();
    setup(earned.restore_durable(checkpoint_a),"earned account restore");guide(earned);restored=saved(earned);
    check(restored["scionLoadouts"][a.house+":"+a.scion]["treeQuestPoints"].number()==std::optional<double>(1)&&
        text(restored["houseProgress"][a.house]["entitlements"]["first-warden:"+a.house]["status"])=="redeemed","earned entitlement restored in town can be claimed at Aldwyn");
    guide(earned);restored=saved(earned);
    check(restored["scionLoadouts"][a.house+":"+a.scion]["treeQuestPoints"].number()==std::optional<double>(1),"recovered earned entitlement remains single use");
}
} // namespace
int main(){try{run();}catch(const std::exception& e){std::cerr<<"Entitlement fixture failed: "<<e.what()<<'\n';return 2;}
    std::cout<<"service_entitlement_tests: "<<checks<<" checks, "<<failures<<" failures\n";return failures?1:0;}
