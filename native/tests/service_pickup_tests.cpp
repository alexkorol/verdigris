// Service authority serializes requests: both possible pickup orderings are
// exercised with distinct accounts and actors sharing the same physical floor.
#include "verdigris/networking.hpp"
#include <iostream>
#include <stdexcept>

using namespace verdigris;
using namespace verdigris::networking;
namespace {
int checks=0,failures=0;
void check(bool ok,const char* label){++checks;if(!ok)++failures;std::cout<<(ok?"PASS ":"FAIL ")<<label<<'\n';}
void setup(bool ok,const char* label){if(!ok)throw std::runtime_error(label);}
std::string text(const JsonValue& value){return value.string()?*value.string():"";}
JsonValue saved(ProtocolSession& session){JsonValue out;setup(parse_json(session.durable_payload(),out),"parse durable fixture");return out;}
JsonValue inventory(ProtocolSession& session){auto data=saved(session);return data["scionLoadouts"][text(data["activeHouseId"])+":"+text(data["activeScionId"])]["inventory"];}
int owned(ProtocolSession& session,const std::string& uuid){int count=0;const auto rows=inventory(session);for(const auto& row:*rows.array())if(text(row["uuid"])==uuid)++count;return count;}
int ground(ProtocolSession& session,const std::string& uuid){int count=0;for(const auto& row:session.shared_world()->ground_items())if(row.item.uuid==uuid)++count;return count;}
struct Actor {
    ProtocolSession session;
    std::string house,scion;
    int confirmations=0;
    Actor(const char* id,const std::shared_ptr<ServiceRelicLedger>& ledger):session(id,std::string("socket:")+id,42,false){
        session.enable_service(ledger);session.set_direct_emit([](const Envelope&){});
        send("chronicles:house:found",JsonValue::Object{{"name",std::string("House ")+id}});
        auto data=saved(session);house=text(data["chronicle"]["houses"].array()->back()["id"]);
        send("chronicles:scion:create",JsonValue::Object{{"houseId",house},{"name",id}});
        data=saved(session);scion=text(data["chronicle"]["houses"].array()->back()["scions"].array()->back()["id"]);
        send("chronicles:scion:set-out",JsonValue::Object{{"scionId",scion}});setup(session.actor_alive(),"living service admission");
    }
    void send(const char* event,JsonValue data){session.handle({event,std::move(data)},[this](const Envelope& e){if(e.event=="player:pickup-confirmed")++confirmations;});}
    void take(const std::string& uuid){send("player:context-menu:action",JsonValue::Object{{"queueItem",JsonValue::Object{
        {"action",JsonValue::Object{{"actionId","player:take"}}},{"item",JsonValue::Object{{"uuid",uuid}}}}}});}
    void fill_pack(){
        // Arrange a valid server-owned restart snapshot, never a client dev verb.
        auto item=create_game_item("knife",CreateItemOptions{});setup(bool(item)&&item->size.width==1&&item->size.height==1,"one-cell fixture item");
        ServiceRelicLedger serializer;serializer.queue(*item,session.identity(),house,scion,"fixture");
        JsonValue journal;setup(parse_json(serializer.serialize(),journal),"fixture item serialization");
        const auto prototype=journal["relics"].array()->front()["item"];
        JsonValue::Array pack;
        for(int slot=0;slot<PlayerInventory::kSlotCount;++slot){auto row=prototype;
            (*row.object())["uuid"]="pickup-full:"+session.identity()+":"+std::to_string(slot);(*row.object())["slot"]=slot;pack.push_back(std::move(row));}
        auto data=saved(session);auto& loadout=data.object()->at("scionLoadouts").object()->at(house+":"+scion);
        (*loadout.object())["inventory"]=std::move(pack);setup(session.restore_durable(data.stringify()),"restore full backpack");
        setup(inventory(session).array()->size()==PlayerInventory::kSlotCount,"all backpack cells filled");
    }
};
void run_case(bool relic,bool reverse,bool full){
    auto ledger=std::make_shared<ServiceRelicLedger>();Actor a("pickup-a",ledger),b("pickup-b",ledger);
    Actor& first=reverse?b:a;Actor& second=reverse?a:b;
    if(full)first.fill_pack();
    const std::string instance="instance:pickup:shared";
    a.session.enter_shared_instance(instance,[](const Envelope&){});b.session.adopt_world(a.session.shared_world(),instance,[](const Envelope&){});
    const auto at=tile_movement::occupied_tile(a.session.shared_world()->position());b.session.shared_world()->teleport(at.x,at.y,0);
    setup(a.session.shared_world()->instance_key()==b.session.shared_world()->instance_key()&&a.session.runtime_actor_id()!=b.session.runtime_actor_id(),"two distinct actors on one shared floor");
    auto item=create_game_item("bronze-sword",CreateItemOptions{});setup(bool(item),"physical item fixture");
    if(relic){ledger->queue(*item,a.session.identity(),a.house,"fallen-scion","Fallen");auto record=ledger->release(a.session.identity(),instance);setup(bool(record),"released ledger fixture");
        a.session.shared_world()->add_relic_ground_item(record->item,at.x,at.y,record->item.uuid,record->scion,record->name);
    }else a.session.shared_world()->add_ground_item(*item,at.x,at.y);
    setup(ground(a.session,item->uuid)==1&&ground(b.session,item->uuid)==1,"shared exact UUID visible to both actors");
    const auto first_before=inventory(first.session).stringify(),ledger_before=ledger->serialize();
    first.take(item->uuid);
    if(full){
        check(inventory(first.session).stringify()==first_before&&owned(first.session,item->uuid)==0,"full-pack rejection preserves the original private inventory");
        check(ground(a.session,item->uuid)==1&&ledger->serialize()==ledger_before&&first.confirmations==0,"full-pack rejection preserves ground UUID, ledger and no pickup confirmation");
        first.take(item->uuid);check(ground(b.session,item->uuid)==1&&ledger->serialize()==ledger_before,"repeated full-pack rejection cannot claim or duplicate the item");
    }
    second.take(item->uuid);
    Actor& winner=full?second:first;Actor& loser=full?first:second;
    check(owned(winner.session,item->uuid)==1&&owned(loser.session,item->uuid)==0,"exactly the first admissible actor owns the original UUID");
    check(ground(a.session,item->uuid)==0&&ground(b.session,item->uuid)==0,"successful pickup removes the shared floor item for both actors");
    check(winner.confirmations==1&&loser.confirmations==0,"only the winning actor receives pickup confirmation");
    if(relic)check(ledger->records().size()==1&&ledger->records().at(item->uuid).state=="claimed","one successful physical pickup claims its durable ledger record");
    first.take(item->uuid);second.take(item->uuid);
    check(owned(a.session,item->uuid)+owned(b.session,item->uuid)==1&&a.confirmations+b.confirmations==1,"replayed requests cannot duplicate ownership or confirmation");
    ProtocolSession restored_a(a.session.identity(),"restart-a",42,false),restored_b(b.session.identity(),"restart-b",42,false);
    restored_a.enable_service();restored_b.enable_service();
    setup(restored_a.restore_durable(a.session.durable_payload())&&restored_b.restore_durable(b.session.durable_payload()),"restore independently saved private inventories");
    check(owned(restored_a,item->uuid)+owned(restored_b,item->uuid)==1,"saved account snapshots restore exactly one physical owner");
    if(relic){ServiceRelicLedger restored;setup(restored.restore(ledger->serialize()),"restore claimed ledger");
        check(!restored.release(a.session.identity(),"restart-instance"),"restored claimed relic cannot reappear on another floor");}
}
} // namespace
int main(){try{for(bool relic:{false,true})for(bool reverse:{false,true})for(bool full:{false,true})run_case(relic,reverse,full);}
catch(const std::exception& error){std::cerr<<"Pickup fixture failed: "<<error.what()<<'\n';return 2;}
std::cout<<"service_pickup_tests: "<<checks<<" checks, "<<failures<<" failures\n";return failures?1:0;}
