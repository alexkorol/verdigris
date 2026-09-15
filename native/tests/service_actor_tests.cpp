#include "verdigris/networking.hpp"
#include <chrono>
#include <cmath>
#include <iostream>
#include <stdexcept>

using namespace verdigris;
using namespace verdigris::networking;
namespace {
int checks=0,failures=0;
void check(bool ok,const char* label){++checks;if(!ok)++failures;std::cout<<(ok?"PASS ":"FAIL ")<<label<<'\n';}
void setup(bool ok,const char* label){if(!ok)throw std::runtime_error(label);}
bool same(WorldPosition a,WorldPosition b){return std::hypot(a.x-b.x,a.y-b.y)<1e-9;}
std::vector<WorldCombatEvent> hits(WorldSimulation& actor,int power,int& life,std::int64_t now){
    std::vector<WorldCombatEvent> result;
    for(auto event:actor.advance_combat(1,power,life,10000,now))
        if(event.type=="hit" && event.attacker_id==actor.actor_id())result.push_back(std::move(event));
    return result;
}
int monster_life(const WorldSimulation& world,const std::string& id){
    for(const auto& monster:world.monsters())if(monster.uuid==id)return monster.life;
    throw std::runtime_error("missing target");
}
struct CombatPair {
    WorldSimulation a{42,"actor:combat:a"},b{43,"actor:combat:b"};
    std::vector<WorldMonster> targets;
    int life_a=10000,life_b=10000;
    CombatPair(){
        a.enter_solo_instance("crypt","gauntlet");
        for(const auto& monster:a.monsters())if(!monster.boss&&!monster.empowered) {
            targets.push_back(monster);if(targets.size()==2)break;
        }
        setup(targets.size()==2,"two ordinary targets required");a.kill_all_monsters();
        for(const auto& target:targets)setup(a.reset_monster(target.uuid,10000),"reset combat target");
        b.join_instance(a);a.teleport(targets[0].x,targets[0].y,0);b.teleport(targets[1].x,targets[1].y,0);
    }
};
void independent_combat(){
    CombatPair pair;
    check(pair.a.instance_key()==pair.b.instance_key()&&pair.a.participant_count()==2,"two actors share one encounter state");
    pair.a.player_combat_mods().force_critical=true;
    check(!pair.b.player_combat_mods().force_critical,"actor combat modifiers do not alias");
    pair.a.start_player_attack(1,11,1000,"right");pair.b.start_player_attack(1,29,1125,"left");
    const auto a_hit=hits(pair.a,11,pair.life_a,1100);
    check(a_hit.size()==1&&a_hit.front().target_id==pair.targets[0].uuid,"B target selection preserves A pending swing and attribution");
    const auto b_early=hits(pair.b,29,pair.life_b,1224),b_hit=hits(pair.b,29,pair.life_b,1225);
    check(b_early.empty()&&b_hit.size()==1&&b_hit.front().target_id==pair.targets[1].uuid,"B has independent windup and target attribution");
    check(!a_hit.empty()&&a_hit.front().critical&&!b_hit.empty()&&!b_hit.front().critical,"forced critical remains on the owning actor");
    check(hits(pair.a,11,pair.life_a,1449).empty()&&hits(pair.a,11,pair.life_a,1450).size()==1,"A recovery remains based on A contact");
    check(hits(pair.b,29,pair.life_b,1574).empty()&&hits(pair.b,29,pair.life_b,1575).size()==1,"A attack does not consume B recovery");
    const auto saved_life=monster_life(pair.a,pair.targets[0].uuid);
    for(int repeat=0;repeat<1000;++repeat){pair.a.start_player_attack(1,11,1450,"right");hits(pair.a,11,pair.life_a,1450);}
    check(monster_life(pair.a,pair.targets[0].uuid)==saved_life,"1000 duplicate attack samples cannot duplicate same-time damage");
    pair.a.teleport(pair.targets[0].x+10,pair.targets[0].y,1700);
    check(hits(pair.a,11,pair.life_a,1800).empty()&&hits(pair.b,29,pair.life_b,1925).size()==1,"A disengagement preserves B held attack");

    CombatPair shared;
    shared.b.teleport(shared.targets[0].x,shared.targets[0].y,0);
    shared.a.start_player_attack(1,11,1000,"right");shared.b.start_player_attack(1,29,1000,"right");
    const auto first=hits(shared.a,11,shared.life_a,1100),second=hits(shared.b,29,shared.life_b,1100);
    const int damage=(first.empty()?0:first.front().amount)+(second.empty()?0:second.front().amount);
    check(first.size()==1&&second.size()==1&&monster_life(shared.a,shared.targets[0].uuid)==10000-damage&&
        monster_life(shared.b,shared.targets[0].uuid)==10000-damage,"both actors damage the same authoritative enemy exactly once");
    const auto key=shared.b.instance_key();const auto state=shared.b.monsters();const auto at=shared.b.position();
    shared.a.return_to_surface();
    check(shared.b.instance_key()==key&&shared.b.in_instance()&&same(shared.b.position(),at)&&
        shared.b.monsters().size()==state.size()&&monster_life(shared.b,shared.targets[0].uuid)==10000-damage,
        "one actor leaves without replacing ally world or rewinding enemy damage");
}
void shared_boss_clock(){
    WorldSimulation a(42,"actor:boss:a"),b(43,"actor:boss:b");a.enter_solo_instance("crypt","gauntlet");
    WorldMonster boss;for(const auto& monster:a.monsters())if(monster.boss){boss=monster;break;}
    setup(!boss.uuid.empty(),"boss fixture required");a.kill_all_monsters();setup(a.reset_monster(boss.uuid,10000),"boss reset");
    b.join_instance(a);a.teleport(boss.x,boss.y,0);b.teleport(boss.x,boss.y,0);
    int life_a=10000,life_b=10000,duration=0,warnings=0;
    for(const auto& event:a.advance_combat(1,0,life_a,10000,1000))if(event.type=="telegraph"){duration=event.duration_ms;++warnings;}
    for(const auto& event:b.advance_combat(1,0,life_b,10000,1000))if(event.type=="telegraph")++warnings;
    check(warnings==1&&duration>0,"two actors schedule one shared boss telegraph");
    int a_impacts=0,b_impacts=0;
    for(const auto& event:a.advance_combat(1,0,life_a,10000,1000+duration))if(event.type=="hit"&&event.target_id==a.actor_id())++a_impacts;
    for(const auto& event:b.advance_combat(1,0,life_b,10000,1000+duration))if(event.type=="hit"&&event.target_id==b.actor_id())++b_impacts;
    check(a_impacts==1&&b_impacts==1&&life_a==life_b&&life_a<10000,"shared boss impact damages each in-range actor once with correct identity");
    const auto after_a=life_a,after_b=life_b;
    for(int repeat=0;repeat<1000;++repeat){a.advance_combat(1,0,life_a,10000,1000+duration);b.advance_combat(1,0,life_b,10000,999+duration);}
    check(life_a==after_a&&life_b==after_b,"repeated and stale shared boss samples cannot replay impacts");
}
void pursuit_and_dash(){
    WorldSimulation control(42,"actor:control"),a(42,"actor:pursuit:a"),b(43,"actor:pursuit:b");
    control.enter_solo_instance("crypt","gauntlet");a.enter_solo_instance("crypt","gauntlet");b.join_instance(a);
    for(auto* world:{&control,&a,&b})world->teleport(23,28,0);
    control.advance_monster_movement(0);a.advance_monster_movement(0);b.advance_monster_movement(0);
    const auto initial=a.monsters();
    for(int now=50;now<=1000;now+=50){control.advance_monster_movement(now);a.advance_monster_movement(now);b.advance_monster_movement(now);
        for(int duplicate=0;duplicate<10;++duplicate){a.advance_monster_movement(now);b.advance_monster_movement(now-1);}}
    bool equal=control.monsters().size()==a.monsters().size(),moved=false;
    for(std::size_t i=0;i<a.monsters().size()&&equal;++i){const auto& c=control.monsters()[i];const auto& m=a.monsters()[i];
        equal=equal&&same(c.world_position(),m.world_position())&&c.movement_sequence==m.movement_sequence;
        moved=moved||!same(m.world_position(),initial[i].world_position());}
    check(moved&&equal,"extra actor and repeated samples do not accelerate the authoritative pursuit clock");

    WorldSimulation dash_a(42,"actor:dash:a"),dash_b(43,"actor:dash:b");dash_a.set_spawn_suppressed(true);
    dash_a.enter_solo_instance("forest","clearings");dash_b.join_instance(dash_a);
    dash_a.teleport(7,7,0);dash_b.teleport(7,9,0);const auto origin_b=dash_b.position();
    check(dash_a.dash("right",1000)&&same(dash_b.position(),origin_b),"A dash moves only A");
    check(dash_b.dash("right",1001),"A dash does not consume B dash recovery");
    check(!dash_a.dash("right",1499)&&dash_a.dash("down",1500),"A keeps its own exact dash recovery boundary");
    check(!dash_b.dash("right",1500)&&dash_b.dash("down",1501),"B keeps its independent dash recovery boundary");
}
void send(ProtocolSession& session,const char* event,JsonValue data){session.handle({event,std::move(data)},[](const Envelope&){});}
std::string string(const JsonValue& value){return value.string()?*value.string():"";}
JsonValue durable(ProtocolSession& session){JsonValue value;setup(parse_json(session.durable_payload(),value),"durable fixture parse");return value;}
void admit(ProtocolSession& session){
    session.enable_service();session.set_direct_emit([](const Envelope&){});session.schedule_inputs(true);
    send(session,"chronicles:house:found",JsonValue::Object{{"name","Clock QA"}});
    auto value=durable(session);const auto house=string(value["chronicle"]["houses"].array()->back()["id"]);
    send(session,"chronicles:scion:create",JsonValue::Object{{"houseId",house},{"name","Clock QA"}});
    value=durable(session);const auto scion=string(value["chronicle"]["houses"].array()->back()["scions"].array()->back()["id"]);
    send(session,"chronicles:scion:set-out",JsonValue::Object{{"scionId",scion}});setup(session.actor_alive(),"clock actor admission");
}
void scheduled_packets(){
    ProtocolSession a("clock-a","socket-a",81,false),b("clock-b","socket-b",82,false);admit(a);admit(b);
    const auto before_a=a.shared_world()->position(),before_b=b.shared_world()->position();
    for(int n=1;n<=1000;++n)send(a,"player:move",JsonValue::Object{{"direction","right"},{"sequence",n}});
    check(same(a.shared_world()->position(),before_a),"1000 movement packets update intent without advancing an actor");
    const auto now=std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    a.tick(now,true);b.tick(now,true);const auto once=a.shared_world()->position();
    check(std::abs(once.x-before_a.x-tile_movement::kMoveDistance)<1e-9&&once.y==before_a.y&&same(b.shared_world()->position(),before_b),
        "one authority tick consumes one A movement sample and leaves B stationary");
    for(int n=0;n<1000;++n)a.tick(now,true);
    a.tick(now-1,true);check(same(a.shared_world()->position(),once),"duplicate and stale session ticks cannot move again");
    send(a,"player:move",JsonValue::Object{{"direction","left"},{"sequence",1000}});a.tick(now+50,true);
    check(a.shared_world()->position().x>once.x,"stale movement sequence cannot replace current direction");
    send(a,"player:move",JsonValue::Object{{"direction",""},{"sequence",1001}});const auto stopped=a.shared_world()->position();a.tick(now+100,true);
    check(same(a.shared_world()->position(),stopped),"fresh stop intent prevents further authority movement");
    // The socket layer fences old connections. A fresh client starts its own
    // sequence at one and must not inherit the superseded connection's counter.
    a.replace_socket("socket-a-reconnected");a.clear_input();
    send(a,"player:move",JsonValue::Object{{"direction","right"},{"sequence",1}});a.tick(now+125,true);
    check(a.shared_world()->position().x>stopped.x,"reconnected client sequence one can move the existing actor");

    ProtocolSession fighter("clock-fighter","socket-fighter",42,false),ally("clock-ally","socket-ally",43,false);admit(fighter);admit(ally);
    fighter.enter_shared_instance("instance:clock-packets",[](const Envelope&){});
    ally.adopt_world(fighter.shared_world(),"instance:clock-packets",[](const Envelope&){});
    auto world=fighter.shared_world();WorldMonster target;
    for(const auto& monster:world->monsters())if(!monster.boss&&!monster.empowered){target=monster;break;}
    setup(!target.uuid.empty(),"scheduled combat target");world->kill_all_monsters();setup(world->reset_monster(target.uuid,10000),"scheduled target reset");
    world->teleport(target.x,target.y,0);
    const auto ally_before=durable(ally)["life"].number();
    for(int repeat=0;repeat<1000;++repeat)send(fighter,"player:skill:trigger",JsonValue::Object{{"skillId","primary-attack"},{"direction","right"}});
    check(monster_life(*world,target.uuid)==10000&&durable(ally)["life"].number()==ally_before,
        "1000 attack packets cannot resolve shared damage before the authority tick");
    const auto contact=std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count()+100;
    fighter.tick(contact,true);ally.tick(contact,false);
    check(monster_life(*world,target.uuid)<10000,"scheduled attack resolves through the authority tick");
}
} // namespace
int main(){
    try{independent_combat();shared_boss_clock();pursuit_and_dash();scheduled_packets();}
    catch(const std::exception& e){std::cerr<<"Actor fixture failed: "<<e.what()<<'\n';return 2;}
    std::cout<<"service_actor_tests: "<<checks<<" checks, "<<failures<<" failures\n";return failures?1:0;
}
