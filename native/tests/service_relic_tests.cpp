#include "verdigris/networking.hpp"
#include <iostream>
#include <stdexcept>
using namespace verdigris;using namespace verdigris::networking;
int main(){int checks=0;auto check=[&](bool ok,const char* label){++checks;if(!ok)throw std::runtime_error(label);};
try {
 ServiceRelicLedger ledger;auto item=create_game_item("bronze-sword",CreateItemOptions{});check(bool(item),"fixture item");
 ledger.queue(*item,"account-a","house-a","scion-a","Alder");ledger.queue(*item,"account-a","house-a","scion-a","Alder");
 check(ledger.records().size()==1,"death replay has one owner");check(!ledger.release("account-b","instance-b"),"other account does not take source queue");
 auto release=ledger.release("account-a","instance-a");check(bool(release)&&release->item.uuid==item->uuid,"release preserves exact identity");
 check(!ledger.release("account-a","instance-a"),"released relic cannot release twice");
 ServiceRelicLedger restarted;check(restarted.restore(ledger.serialize()),"journal restores");
 check(bool(restarted.release("account-a","instance-new")),"retired instance requeues unclaimed relic");
 check(restarted.claim(item->uuid),"pickup claims released identity");check(!restarted.claim(item->uuid),"pickup replay fails");
 restarted.retire_instances({});check(!restarted.release("account-a","again"),"claimed record cannot requeue");
 restarted.queue(*item,"account-a","house-a","scion-a","Alder");check(!restarted.release("account-a","again"),"replayed death cannot reissue claimed record");
 ServiceRelicLedger cold;check(cold.restore(restarted.serialize())&&!cold.release("account-a","cold"),"restart preserves claim");
 // The same physical item can circulate again after its recipient Scion's
 // later authoritative death. A replay of any older death must stay inert.
 cold.queue(*item,"account-a","house-a","scion-b","Briar");
 check(cold.records().size()==1&&cold.records().at(item->uuid).state=="queued"&&cold.records().at(item->uuid).scion=="scion-b",
       "new Scion death requeues the exact previously claimed physical item");
 cold.queue(*item,"account-a","house-a","scion-a","Alder");
 cold.queue(*item,"account-a","house-a","scion-b","Briar");
 check(cold.records().size()==1&&cold.records().at(item->uuid).scion=="scion-b","old and repeated new death do not overwrite current provenance");
 auto second=cold.release("account-a","instance-second-death");
 check(bool(second)&&second->item.uuid==item->uuid&&second->scion=="scion-b","second death release retains exact item UUID and new source");
 check(!cold.release("account-a","instance-second-death")&&cold.claim(item->uuid),"second death releases and claims exactly once");
 cold.queue(*item,"account-a","house-a","scion-a","Alder");
 cold.queue(*item,"account-a","house-a","scion-b","Briar");
 check(!cold.release("account-a","old-replay"),"any earlier death remains inert after second claim");
 ServiceRelicLedger second_restart;check(second_restart.restore(cold.serialize()),"multi-death provenance survives restart");
 second_restart.queue(*item,"account-a","house-a","scion-a","Alder");
 second_restart.queue(*item,"account-a","house-a","scion-b","Briar");
 check(!second_restart.release("account-a","restart-replay"),"restart cannot replay either prior Scion death");
 second_restart.queue(*item,"account-a","house-a","scion-c","Cedar");
 auto third=second_restart.release("account-a","instance-third-death");
 check(bool(third)&&third->item.uuid==item->uuid&&third->scion=="scion-c","third distinct Scion death can circulate that same physical item");
 for(const auto* invalid:{"+1",".5","01","0x10","1.","1e","NaN","Infinity","1e400","{\"x\":1,\"x\":2}"}){JsonValue value;check(!parse_json(invalid,value),"strict JSON rejects invalid number or duplicate");}
 JsonValue value;check(parse_json("-1.25e+2",value)&&value.number()==std::optional<double>(-125),"valid JSON number");
 std::cout<<"service_relic_tests: "<<checks<<" checks PASS\n";return 0;
}catch(const std::exception& e){std::cerr<<"service_relic_tests FAIL: "<<e.what()<<" after "<<checks<<"\n";return 1;}}
