#pragma once

#include <array>
#include <charconv>
#include <string>
#include <algorithm>
#include <cstdlib>
#include <string_view>

namespace verdigris::inventory_extensions {
// WIZARD rpg_inventory AUX_WINDOWS/PACK_DEFS and geometric_skilltree
// GATE_UNLOCKS/SUBTREES. Display names follow the owner's native UI request.
struct Definition {
  std::string_view id;
  std::string_view name;
  std::string_view unlock;
  std::string_view node;
  std::string_view seat;
  std::string_view pack;
};
inline constexpr std::array<Definition,6> definitions{{
  {"warhorn","Warhorn","war_call_slot","-10,10","warhorn",""},
  {"quick_rig","Quiver / Quick Rig","quick_rig_slot","0,-10","quick_rig",""},
  {"attendant","Attendant","attendant_focus_slot","10,0","attendant",""},
  {"trophy_rack","Trophy Rack","spoils_pack","-10,0","","spoils"},
  {"reagents_pouch","Reagents Pouch","preparations_pack","10,-10","","preparations"},
  {"relics_altar","Relics Altar","reliquary_pack","0,10","","reliquary"}
}};
// The existing native tree uses free root + one point per node; conduit
// choices remain separately charged. Coordinates match WIZARD's rim-10 lattice.
inline bool node_position(std::string_view id,int& q,int& r) {
  const auto comma=id.find(',');
  if(comma==std::string_view::npos || comma==0 || comma+1==id.size())return false;
  const auto a=std::from_chars(id.data(),id.data()+comma,q);
  const auto b=std::from_chars(id.data()+comma+1,id.data()+id.size(),r);
  return a.ec==std::errc{} && b.ec==std::errc{} && a.ptr==id.data()+comma && b.ptr==id.data()+id.size() &&
      q>=-10 && q<=10 && r>=-10 && r<=10 && std::abs(q+r)<=10 &&
      id==std::to_string(q)+","+std::to_string(r);
}
inline bool adjacent(std::string_view a,std::string_view b) {
  int aq=0,ar=0,bq=0,br=0;
  return node_position(a,aq,ar) && node_position(b,bq,br) &&
      (std::max)({std::abs(aq-bq),std::abs(ar-br),std::abs(aq+ar-bq-br)})==1;
}
inline constexpr const Definition* for_pack(std::string_view pack) {
  for(const auto& def:definitions)if(!def.pack.empty() && def.pack==pack)return &def;
  return nullptr;
}
inline constexpr const Definition* for_seat(std::string_view seat) {
  for(const auto& def:definitions)if(!def.seat.empty() && def.seat==seat)return &def;
  return nullptr;
}
inline constexpr int columns(std::string_view pack) {return pack=="main"?12:for_pack(pack)?4:0;}
inline constexpr int rows(std::string_view pack) {return pack=="main"?7:for_pack(pack)?4:0;}
} // namespace verdigris::inventory_extensions
