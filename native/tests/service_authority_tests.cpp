// Integration regressions for the parent's service-enabled ProtocolSession.
// The storage worker commits this gate independently; failures against the
// integration baseline are intentional evidence, never a green acceptance claim.
#include "verdigris/networking.hpp"
#include <iostream>
#include <stdexcept>

using namespace verdigris::networking;
namespace {
int checks=0, failures=0;
void check(bool value,const char* label) {
    ++checks; if(!value)++failures;
    std::cout<<(value?"PASS ":"FAIL ")<<label<<'\n';
}
std::string text(const JsonValue& value) {return value.string()?*value.string():"";}
void send(ProtocolSession& session,const std::string& event,JsonValue data=JsonValue::Object{}) {
    session.handle({event,std::move(data)},[](const Envelope&){});
}
JsonValue saved(ProtocolSession& session) {
    JsonValue value;if(!parse_json(session.durable_payload(),value))throw std::runtime_error("invalid durable fixture");return value;
}
std::string found(ProtocolSession& session,const char* name) {
    send(session,"chronicles:house:found",JsonValue::Object{{"name",name}});
    const auto value=saved(session);
    const auto* houses=value["chronicle"]["houses"].array();
    if(!houses || houses->empty())throw std::runtime_error("House setup failed");
    return text(houses->back()["id"]);
}
std::string create(ProtocolSession& session,const std::string& house) {
    send(session,"chronicles:scion:create",JsonValue::Object{{"houseId",house},{"name","Authority QA"}});
    const auto value=saved(session);
    for(const auto& candidate:*value["chronicle"]["houses"].array())if(text(candidate["id"])==house) {
        const auto* scions=candidate["scions"].array();
        if(scions && !scions->empty())return text(scions->back()["id"]);
    }
    throw std::runtime_error("Scion setup failed");
}
struct Fixture {
    ProtocolSession session{"authority-qa","socket-authority",123,false};
    std::string house,scion;
    Fixture(){session.enable_service();house=found(session,"Authority House A");scion=create(session,house);
        send(session,"chronicles:scion:set-out",JsonValue::Object{{"scionId",scion}});
        if(!session.actor_alive())throw std::runtime_error("living admission setup failed");}
};
JsonValue inventory(ProtocolSession& session) {
    const auto value=saved(session);
    const auto key=text(value["activeHouseId"])+":"+text(value["activeScionId"]);
    return value["scionLoadouts"][key]["inventory"];
}
int count(ProtocolSession& session,const char* id) {
    int result=0;const auto rows=inventory(session);
    if(rows.array())for(const auto& item:*rows.array())if(text(item["id"])==id)result+=static_cast<int>(item["qty"].number().value_or(0));
    return result;
}
JsonValue item(ProtocolSession& session,const char* id) {
    const auto rows=inventory(session);
    if(rows.array())for(const auto& row:*rows.array())if(text(row["id"])==id)return row;
    throw std::runtime_error("missing fixture item");
}
void menu(ProtocolSession& session,const char* action,JsonValue ref) {
    send(session,"player:context-menu:action",JsonValue::Object{
        {"queueItem",JsonValue::Object{{"action",JsonValue::Object{{"actionId",action}}},{"item",std::move(ref)}}}});
}
void test_admission() {
    Fixture live;
    send(live.session,"player:chronicles:select",JsonValue::Object{{"houseId",live.house},{"scionId",live.scion},{"mortal",false}});
    check(saved(live.session)["mortalOath"].boolean().value_or(false),"client cannot remove the admitted mortal oath");

    // Arrange an authoritative persisted final-death record, then exercise the
    // ordinary service command after a restart. No client development command.
    Fixture fallen;auto death=saved(fallen.session);
    auto* houses=death.object()->at("chronicle").object()->at("houses").array();
    auto& house=houses->front();
    (*house.object())["crypt"]=house["scions"];
    (*house.object())["scions"]=JsonValue::Array{};
    (*death.object())["lifecycle"]="permadead";(*death.object())["life"]=0;
    (*death.object())["pendingChronicles"]=true;
    ProtocolSession restarted("authority-qa","restarted",123,false);restarted.enable_service();
    if(!restarted.restore_durable(death.stringify()))throw std::runtime_error("death snapshot setup failed");
    send(restarted,"player:chronicles:select",JsonValue::Object{{"houseId",fallen.house},{"scionId",fallen.scion},{"mortal",false}});
    check(!restarted.actor_alive() && text(saved(restarted)["lifecycle"])=="permadead","crypt Scion cannot be resurrected through select");
}
void test_prices_and_bank() {
    for(int forged_price:{0,-100}) {
        Fixture f;const auto coins=count(f.session,"coins"),swords=count(f.session,"bronze-sword");
        menu(f.session,"player:shop:buy",JsonValue::Object{{"id","bronze-sword"},{"price",forged_price}});
        const int added=count(f.session,"bronze-sword")-swords,spent=coins-count(f.session,"coins");
        check((added==0 && spent==0)||(added==1 && spent==15),"forged shop price cannot buy below authoritative price");
    }
    Fixture currency;const auto coins=count(currency.session,"coins");
    menu(currency.session,"player:shop:buy",JsonValue::Object{{"id","coins"},{"price",0}});
    check(count(currency.session,"coins")==coins,"arbitrary shop item cannot mint currency");

    Fixture remote;send(remote.session,"instance:enterSolo");
    const auto before=inventory(remote.session).stringify();
    menu(remote.session,"player:shop:buy",JsonValue::Object{{"id","bronze-sword"},{"price",15}});
    check(inventory(remote.session).stringify()==before,"shop transaction requires town service proximity");

    for(int quantity:{-1,0}) {
        Fixture bank;const auto balance=count(bank.session,"coins");
        auto coin=item(bank.session,"coins");(*coin.object())["qty"]=quantity;
        menu(bank.session,"player:bank:deposit",coin);
        const auto state=saved(bank.session);const auto* rows=state["bankItems"].array();
        bool valid=true;if(rows)for(const auto& row:*rows)valid=valid && row["qty"].number().value_or(0)>0;
        check(count(bank.session,"coins")==balance && valid,"nonpositive bank deposit cannot mint coins or invalid stacks");
    }
}
void test_house_switch() {
    Fixture f;
    // First attempt the ordinary bank action. Once proximity checks are in
    // place, use an authoritative restart fixture to arrange a private bank.
    auto dagger=item(f.session,"bronze-dagger");
    menu(f.session,"player:bank:deposit",dagger);
    // If an implementation now requires explicit NPC access, arrange the same
    // server-owned snapshot as a restart checkpoint, not a client save endpoint.
    auto first=saved(f.session);
    if(!first["bankItems"].array() || first["bankItems"].array()->empty()) {
        auto& progress=first.object()->at("houseProgress").object()->at(f.house);
        (*progress.object())["bank"]=JsonValue::Array{dagger};
        (*first.object())["bankItems"]=JsonValue::Array{dagger};
        auto& pack=first.object()->at("scionLoadouts").object()->at(f.house+":"+f.scion).object()->at("inventory");
        JsonValue::Array retained;for(const auto& row:*pack.array())if(text(row["uuid"])!=text(dagger["uuid"]))retained.push_back(row);
        pack=std::move(retained);
        if(!f.session.restore_durable(first.stringify()))throw std::runtime_error("private bank setup failed");
    }
    const auto second_house=found(f.session,"Authority House B"),second_scion=create(f.session,second_house);
    send(f.session,"chronicles:scion:set-out",JsonValue::Object{{"scionId",second_scion}});
    const auto second=saved(f.session);
    const auto* bank_b=second["houseProgress"][second_house]["bank"].array();
    const auto* bank_a=second["houseProgress"][f.house]["bank"].array();
    check(bank_b && bank_b->empty() && bank_a && bank_a->size()==1,"founding and entering another House preserves private banks");
    check(second["houseProgress"][f.house]["treasury"].number()==std::optional<double>(100),"founding a House checkpoints the previous House treasury");
}
} // namespace

int main() {
    try {test_admission();test_prices_and_bank();test_house_switch();}
    catch(const std::exception& e){std::cerr<<"Fixture failure: "<<e.what()<<'\n';return 2;}
    std::cout<<"service_authority_tests: "<<checks<<" checks, "<<failures<<" failures\n";
    return failures?1:0;
}
