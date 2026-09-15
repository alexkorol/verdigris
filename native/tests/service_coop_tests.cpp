#include "remote_session.hpp"
#include "verdigris/service_store.hpp"
#include <chrono>
#include <filesystem>
#include <iostream>
#include <thread>
#include <stdexcept>
namespace verdigris::networking {
struct WebSocketServerTestAccess {
  static std::vector<std::weak_ptr<ProtocolSession>> sessions(WebSocketServer& server) {
    std::lock_guard lock(server.authority_mutex_);
    std::vector<std::weak_ptr<ProtocolSession>> result;
    for(const auto& entry:server.sessions_)result.push_back(entry.second);
    return result;
  }
};
}
using namespace verdigris;
using namespace verdigris::client;
using namespace verdigris::networking;
static int checks=0;
static void check(bool ok,const char* label){if(!ok)throw std::runtime_error(label);++checks;std::cout<<"PASS "<<label<<std::endl;}
static auto now_ms(){return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();}
static std::vector<RemoteProtocolSession*> parked;
static void pump(RemoteProtocolSession& a,RemoteProtocolSession& b,int ms=250){for(int i=0;i<ms;i+=10){a.poll();b.poll();for(auto* peer:parked)if(peer!=&a&&peer!=&b)peer->poll();std::this_thread::sleep_for(std::chrono::milliseconds(10));}}
template<class F>static void until(RemoteProtocolSession& a,RemoteProtocolSession& b,F condition,const char* label){for(int i=0;i<800&&!condition();++i)pump(a,b,10);check(condition(),label);}
static void party(RemoteProtocolSession& s,const char* verb,std::string ref={},int value=0){ClientCommand c;c.type=ClientCommand::Type::PartyAction;c.target=verb;c.extra=std::move(ref);c.value=value;s.submit(c);}
static void admit(RemoteProtocolSession& a,RemoteProtocolSession& b,RemoteProtocolSession& s,const std::string& code,const char* name){
  ClientCommand auth;auth.type=ClientCommand::Type::Authenticate;auth.target=code;auth.value=1;s.submit(auth);
  until(a,b,[&]{return s.model().authenticated && s.model().chronicles_pending;},"authenticated account reaches native Chronicles");
  s.submit(ClientCommand::found_house(std::string(name)+" House"));
  until(a,b,[&]{return !s.model().chronicle.houses.empty();},"owned House created");
  s.submit(ClientCommand::create_scion(name));
  until(a,b,[&]{return !s.model().chronicle.houses.front().scions.empty();},"owned Scion created");
  s.submit(ClientCommand::set_out(s.model().chronicle.houses.front().scions.front().id));
  until(a,b,[&]{return s.connection_state()==ConnectionState::Ready && !s.model().chronicles_pending;},"owned living Scion admitted");
}
int main(){
  const auto root=std::filesystem::temp_directory_path()/("verdigris-coop-"+std::to_string(now_ms()));
  try{
    auto store=std::make_shared<service::Store>(root.string());std::string error;
    check(store->open(&error),"isolated transactional store opens");
    auto code_a=store->issue_enrollment(now_ms(),600000,&error),code_b=store->issue_enrollment(now_ms(),600000,&error);
    WebSocketServer server(7291,{},store);check(server.start(&error),"independent service listener starts");
    auto a=RemoteProtocolSession::online("ws://127.0.0.1:7291/game"),b=RemoteProtocolSession::online("ws://localhost:7291/game");
    check(a->start(&error)&&b->start(&error),"native async clients start");
    until(*a,*b,[&]{return a->connection_state()==ConnectionState::Connected && b->connection_state()==ConnectionState::Connected;},"maintained hostname transports connect");
    admit(*a,*b,*a,code_a,"Ash");admit(*a,*b,*b,code_b,"Bee");
    check(a->model().player.uuid!=b->model().player.uuid,"distinct runtime actor identities");
    check(a->model().chronicle.houses.front().id!=b->model().chronicle.houses.front().id,"distinct owned Houses");
    until(*a,*b,[&]{return a->model().peers.size()==1&&b->model().peers.size()==1;},"both clients see shared hub peers");
    const auto x=b->model().player.x,y=b->model().player.y;
    for(int i=0;i<6;++i){a->submit(ClientCommand::move(1,0));pump(*a,*b,50);}a->submit(ClientCommand::move(0,0));pump(*a,*b,250);
    check(b->model().player.x==x&&b->model().player.y==y,"A movement does not move B");
    check(a->model().player.x>x,"A moves under instance clock");
    party(*a,"party:create");until(*a,*b,[&]{return !a->model().party.id.empty();},"native party create acknowledged");
    party(*b,"party:invite:accept",a->model().party.id);pump(*a,*b);
    check(b->model().party.id.empty()&&!b->model().party.error.empty(),"uninvited acceptance rejected");
    party(*a,"party:invite",b->model().player.uuid);until(*a,*b,[&]{return !b->model().party.invite_id.empty();},"intended peer receives invitation");
    party(*b,"party:invite:accept",b->model().party.invite_id);
    until(*a,*b,[&]{return a->model().party.members.size()==2 && b->model().party.members.size()==2;},"stored invitation admits two members");
    party(*b,"party:startInstance");pump(*a,*b);check(b->model().party.state=="lobby","nonleader start rejected");
    party(*a,"party:startInstance");pump(*a,*b);check(a->model().party.state=="lobby","unready start rejected");
    party(*a,"party:ready",{},1);party(*b,"party:ready",{},1);pump(*a,*b);
    party(*a,"party:startInstance");
    until(*a,*b,[&]{return a->model().scene.type=="instance" && b->model().scene.type=="instance";},"native clients enter expedition together");
    check(a->model().player.scene_id==b->model().player.scene_id,"both clients occupy same instance generation");
    until(*a,*b,[&]{return a->model().peers.size()==1&&b->model().peers.size()==1&&!a->model().monsters.empty();},"shared actors and encounter delivered without dev state");
    const auto scene=b->model().player.scene_id;
    a->send_raw("dev:give",JsonValue::Object{{"itemId","coins"},{"qty",99999}});pump(*a,*b);
    check(a->model().last_message.find("unavailable")!=std::string::npos,"service denies development mutation");
    auto code_c=store->issue_enrollment(now_ms(),600000,&error),code_d=store->issue_enrollment(now_ms(),600000,&error);
    auto c=RemoteProtocolSession::online("ws://127.0.0.1:7291/game"),d=RemoteProtocolSession::online("ws://127.0.0.1:7291/game");
    parked={a.get(),b.get()};check(c->start(&error)&&d->start(&error),"second pair of native transports starts");
    until(*c,*d,[&]{return c->connection_state()==ConnectionState::Connected&&d->connection_state()==ConnectionState::Connected;},"second pair connects independently");
    admit(*c,*d,*c,code_c,"Cedar");admit(*c,*d,*d,code_d,"Dune");
    until(*c,*d,[&]{return c->model().peers.size()==1&&d->model().peers.size()==1;},"hub excludes first party expedition actors");
    party(*c,"party:create");until(*c,*d,[&]{return !c->model().party.id.empty();},"second party creates independently");
    party(*c,"party:invite",d->model().player.uuid);until(*c,*d,[&]{return !d->model().party.invite_id.empty();},"second invitation is privately delivered");
    party(*d,"party:invite:accept",d->model().party.invite_id);until(*c,*d,[&]{return c->model().party.members.size()==2;},"second party admits own recipient");
    party(*c,"party:ready",{},1);party(*d,"party:ready",{},1);pump(*c,*d);party(*c,"party:startInstance");
    until(*c,*d,[&]{return c->model().scene.type=="instance"&&d->model().scene.type=="instance";},"two parties run concurrently");
    check(c->model().scene.id!=a->model().scene.id,"concurrent parties have distinct instance generations");
    until(*c,*d,[&]{return c->model().peers.size()==1&&c->model().peers.front().uuid==d->model().player.uuid&&a->model().peers.size()==1&&a->model().peers.front().uuid==b->model().player.uuid;},"peer rosters isolate both concurrent encounters");
    const auto cx=c->model().player.x,cy=c->model().player.y;
    a->submit(ClientCommand::move(1,0));pump(*c,*d,300);a->submit(ClientCommand::move(0,0));
    check(c->model().player.x==cx&&c->model().player.y==cy,"first instance input leaves second party actor unchanged");
    const auto session_lifetimes=WebSocketServerTestAccess::sessions(server);
    check(session_lifetimes.size()==4,"lifetime check observes all four admitted sessions");
    c->shutdown();d->shutdown();parked.clear();
    a->shutdown();pump(*a,*b,300);
    check(server.healthy() && b->connection_state()==ConnectionState::Ready && b->model().player.scene_id==scene,"client exit preserves service and ally world");
    b->shutdown();server.stop();store.reset();
    for(const auto& session:session_lifetimes)check(session.expired(),"shutdown releases session and closed connection callback ownership");
    std::cout<<"native service co-op tests PASS: "<<checks<<" checks; storage "<<root.string()<<'\n';return 0;
  }catch(const std::exception& e){std::cerr<<"native service co-op FAIL: "<<e.what()<<" ("<<checks<<" checks)\n";return 1;}
}
