// Character information is a deliberate companion to equipment management.
RECT character_close_rect(int w,int h) {
  const auto p=character_pane_rect(w,h,0);const int s=hud_scale(h);
  return {p.x+p.w-38*s,p.y+12*s,p.x+p.w-12*s,p.y+36*s};
}
RECT character_detail_rect(int w,int h) {
  const auto p=character_pane_rect(w,h,0);const int s=hud_scale(h);
  return {p.x+18*s,p.y+p.h-38*s,p.x+p.w-18*s,p.y+p.h-12*s};
}
void paint_character_pane(ClientState& state,HDC dc,const RECT& bounds,render::List& rl) {
  if(!state.character_pane)return;
  const int s=hud_scale(bounds.bottom);const auto p=character_pane_rect(bounds.right,bounds.bottom,0);
  RECT r{p.x,p.y,p.x+p.w,p.y+p.h};skin::inventory_surface(dc,r);dress_owned_pane(state.billboards,dc,r);
  state.hud_rect_trace.push_back({"character-pane-frame",p});
  const int saved=SaveDC(dc);IntersectClipRect(dc,r.left,r.top,r.right,r.bottom);
  SelectObject(dc,skin::font_body());
  RECT title{p.x+18*s,p.y+12*s,p.x+p.w-44*s,p.y+36*s};
  inventory_text(dc,title,state.world.scion_name.empty()?"Character":state.world.scion_name,skin::kGold);
  const auto close=character_close_rect(bounds.right,bounds.bottom);
  inventory_button(dc,close,"x",false);
  const auto& a=state.world.player;
  std::vector<std::pair<std::string,std::string>> rows={
    {"Level",std::to_string(a.level)},
    {"Life",std::to_string(a.life)+" / "+std::to_string(a.life_max)},
    {"Resource",std::to_string(a.resource)+" / "+std::to_string(a.resource_max)},
    {"Attack",std::to_string(a.attack+a.gear_attack)},
    {"Defense",std::to_string(a.defense)}};
  if(state.session) {
    const auto& m=state.session->model();
    rows.push_back({"Strength",std::to_string(m.attr_strength)});
    rows.push_back({"Dexterity",std::to_string(m.attr_dexterity)});
    rows.push_back({"Intelligence",std::to_string(m.attr_intelligence)});
    if(m.progression.present) rows.push_back({"Skill points",std::to_string(m.progression.unspent_points)});
  }
  if(state.stat_atk_expanded) {
    rows.push_back({"Base attack",std::to_string(a.attack)});
    rows.push_back({"Equipment",std::to_string(a.gear_attack)});
  }
  const int available=p.h-94*s;
  const int row_h=std::min(28*s,available/static_cast<int>(rows.size()));
  int y=p.y+44*s;
  for(const auto& row:rows) {
    RECT label{p.x+18*s,y,p.x+p.w/2,y+row_h};
    RECT value{p.x+p.w/2,y,p.x+p.w-18*s,y+row_h};
    inventory_text(dc,label,row.first,skin::kInkDim);
    inventory_text(dc,value,row.second,skin::kInk,DT_RIGHT|DT_VCENTER|DT_SINGLELINE|DT_END_ELLIPSIS);
    rl.push_back({render::Op::Hud,double(label.left),double(y),0,0,"char:"+row.first+":"+row.second});
    y+=row_h;
  }
  inventory_button(dc,character_detail_rect(bounds.right,bounds.bottom),state.stat_atk_expanded?"Hide attack details":"Attack details",false);
  rl.push_back({render::Op::Hud,0,0,0,0,"character-pane"});
  rl.push_back({render::Op::Hud,0,0,0,a.attack+a.gear_attack,std::string("char:atk-expanded:")+(state.stat_atk_expanded?"1":"0")});
  RestoreDC(dc,saved);
}
