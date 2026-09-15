#pragma once
// Native party controls consume authoritative state and submit typed intents.
struct PartyHit { RECT rect{}; std::string label, verb, extra; int value=0; bool enabled=true; };
struct PartyLayout { RECT toggle{}, panel{}; std::vector<PartyHit> hits; };
bool party_visible(const ClientState& state) {
  return state.screen!=Screen::Chronicles && state.frontend==Frontend::None &&
      !state.gear_overlay && !state.character_pane && !state.tree_pane && !trade_pane_open(state) &&
      (is_remote(state) || !state.world.peers.empty() || !state.world.party.id.empty());
}
PartyLayout party_layout(const ClientState& state,int w,int h) {
  PartyLayout out;const int s=hud_scale(h);
  const auto menu=player_menu_rect(w,h,2);
  out.toggle={menu.left,menu.bottom+5*s,menu.right,menu.bottom+33*s};
  const int ww=std::min(w-24*s,340*s),x=w-ww-16*s,y=out.toggle.bottom+8*s;
  out.panel={x,y,x+ww,std::min(h-104*s,y+410*s)};
  if(!state.party_open)return out;
  const auto& p=state.world.party;
  const int row=30*s, pad=12*s;
  int yy=y+42*s;
  const auto add=[&](std::string label,std::string verb,std::string extra="",int value=0,bool enabled=true) {
    out.hits.push_back({{x+pad,yy,x+ww-pad,yy+row},std::move(label),std::move(verb),std::move(extra),value,enabled});yy+=row+5*s;
  };
  if(!p.invite_id.empty()) { add("Accept invitation","party:invite:accept",p.invite_id);add("Decline invitation","party:invite:decline",p.invite_id); }
  else if(p.id.empty()) add("Create party","party:create");
  else {
    bool ready=false;for(const auto& m:p.members)if(m.uuid==state.world.player.id)ready=m.ready;
    if(p.state!="instance" && p.state!="in-instance" && p.state!="inInstance") {
      add(ready?"Unready":"Ready","party:ready","",ready?0:1);
      if(p.leader_id==state.world.player.id) {
        const bool all= !p.members.empty() && std::all_of(p.members.begin(),p.members.end(),[](const auto& m){return m.ready;});
        add("Start expedition","party:startInstance","",0,all);
      }
    } else add("Return to town","party:returnToTown");
    add("Leave party","party:leave");
  }
  // One paged roster, bounded by available height. Members and available peers
  // have readable labels; invite targets are server actor UUIDs, never typed names.
  std::vector<PartyHit> roster;
  for(const auto& m:p.members) {
    std::string label=m.name.empty()?"Scion":m.name;
    if(m.uuid==p.leader_id)label+=" (leader)";
    label+=m.ready?" - ready":" - not ready";
    roster.push_back({{},label,"","",0,false});
  }
  if(p.invite_id.empty()) for(const auto& peer:state.world.peers) {
    if(std::any_of(p.members.begin(),p.members.end(),[&](const auto& m){return m.uuid==peer.id;}))continue;
    const bool can_invite=!p.id.empty() && p.leader_id==state.world.player.id && p.state!="instance" && p.state!="in-instance" && p.state!="inInstance";
    roster.push_back({{},(can_invite?"Invite ":"")+(peer.name.empty()?std::string("Scion"):peer.name),
        can_invite?"party:invite":"",peer.id,0,can_invite});
  }
  const int count=std::max(1,int((out.panel.bottom-yy-120*s)/(row+5*s)));
  const int pages=std::max(1,int((roster.size()+count-1)/count));
  const int page=std::clamp(state.party_page,0,pages-1);
  for(int i=page*count;i<std::min(int(roster.size()),(page+1)*count);++i)
    add(roster[i].label,roster[i].verb,roster[i].extra,0,roster[i].enabled);
  if(pages>1)add("More players ("+std::to_string(page+1)+"/"+std::to_string(pages)+")","page","",(page+1)%pages);
  return out;
}
bool handle_party_click(ClientState& state,HWND window,POINT point) {
  if(!party_visible(state))return false;
  RECT client{};GetClientRect(window,&client);const auto layout=party_layout(state,client.right,client.bottom);
  if(!PtInRect(&layout.toggle,point) && !state.party_open)return false;
  // Modal ownership includes empty panel space and clicks outside it. Opening
  // or operating the party must never enqueue a world attack behind the panel.
  state.held_gameplay_attacks.erase(VK_LBUTTON);state.primary_down=false;
  if(PtInRect(&layout.toggle,point)){state.party_open=!state.party_open;return true;}
  if(!PtInRect(&layout.panel,point)){state.party_open=false;return true;}
  for(const auto& hit:layout.hits)if(PtInRect(&hit.rect,point) && hit.enabled) {
    if(hit.verb=="page"){state.party_page=hit.value;break;}
    if(!hit.verb.empty() && state.session) {
      verdigris::client::ClientCommand command;command.type=verdigris::client::ClientCommand::Type::PartyAction;
      command.target=hit.verb;command.extra=hit.extra;command.value=hit.value;state.session->submit(command);
    }
    break;
  }
  return true;
}
void paint_party_ui(ClientState& state,HDC dc,const RECT& bounds,render::List& trace) {
  if(!party_visible(state))return;
  const auto layout=party_layout(state,bounds.right,bounds.bottom);const int s=hud_scale(bounds.bottom);
  const auto font=SelectObject(dc,skin::font_small());
  const auto& p=state.world.party;
  inventory_button(dc,layout.toggle,!p.invite_id.empty()?"Invitation":state.party_open?"Close party":"Party",PtInRect(&layout.toggle,state.mouse));
  if(state.party_open) {
    skin::panel(dc,layout.panel,skin::kGold,245,7.0f);
    RECT title{layout.panel.left+12*s,layout.panel.top+8*s,layout.panel.right-12*s,layout.panel.top+36*s};
    inventory_text(dc,title,!p.invite_id.empty()?"Party invitation":p.id.empty()?"Players nearby":"Your party",skin::kGold);
    for(const auto& hit:layout.hits) {
      if(hit.verb.empty())inventory_text(dc,hit.rect,hit.label,skin::kInk);
      else inventory_button(dc,hit.rect,hit.label,PtInRect(&hit.rect,state.mouse),hit.enabled);
      trace.push_back({render::Op::Hud,double(hit.rect.left),double(hit.rect.top),0,0,"party:"+hit.label});
    }
    if(!p.error.empty()) {
      RECT error{layout.panel.left+12*s,layout.panel.bottom-102*s,layout.panel.right-12*s,layout.panel.bottom-10*s};
      inventory_text(dc,error,p.error,skin::kGold,DT_LEFT|DT_WORDBREAK|DT_END_ELLIPSIS);
    }
  }
  SelectObject(dc,font);
}
