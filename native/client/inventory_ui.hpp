// Included by the native painter after the inventory command/layout helpers.
// WIZARD composition; authority continues to own capacity, seats and item data.
static constexpr const char* kEquipmentNames[] = {
    "Head", "Neck", "Main hand", "Body", "Cloak", "Off hand", "Hands",
    "Waist", "Feet", "First ring", "Second ring", "Warhorn", "Quick rig", "Attendant"};

RECT gear_close_rect(int w, int h) {
  const auto p = gear_pane_rect(w,h); const int s = hud_scale(h);
  return {p.x+p.w-38*s,p.y+12*s,p.x+p.w-12*s,p.y+36*s};
}
RECT gear_action_rect(int w, int h, int index) {
  const auto p = gear_pane_rect(w,h); const int s = hud_scale(h);
  const int mid = p.x+p.w/2;
  return index == 0 ? RECT{p.x+18*s,p.y+p.h-40*s,mid-4*s,p.y+p.h-14*s}
                    : RECT{mid+4*s,p.y+p.h-40*s,p.x+p.w-18*s,p.y+p.h-14*s};
}
void inventory_text(HDC dc, RECT r, const std::string& text, COLORREF ink,
                    UINT flags = DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS) {
  SetBkMode(dc, TRANSPARENT); SetTextColor(dc,ink);
  DrawTextA(dc,text.c_str(),-1,&r,flags);
}
void inventory_button(HDC dc, RECT r, const std::string& text, bool hover) {
  skin::inventory_surface(dc,r,hover ? 1 : 0);
  inventory_text(dc,r,text,skin::kInk,DT_CENTER|DT_VCENTER|DT_SINGLELINE);
}
void empty_equipment_outline(HDC dc,RECT r,std::size_t seat) {
  // Restrained outlines follow WIZARD's empty-slot icon vocabulary.
  Gdiplus::Graphics g(dc);g.SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);
  const float size=float(std::min(r.right-r.left,r.bottom-r.top))*0.45f;
  g.TranslateTransform(float(r.left+r.right)/2-size/2,float(r.top+r.bottom)/2-size/2);
  g.ScaleTransform(size/24,size/24);
  Gdiplus::Pen pen(Gdiplus::Color(255,68,62,49),1.0f);
  auto line=[&](float a,float b,float c,float d){g.DrawLine(&pen,a,b,c,d);};
  if(seat==9 || seat==10)g.DrawEllipse(&pen,5,6,14,14);
  else if(seat==0){g.DrawArc(&pen,3,3,18,20,180,180);line(3,13,3,20);line(3,20,8,20);line(16,20,21,20);line(21,20,21,13);}
  else if(seat==2){line(9,19,18,3);line(18,3,19,8);line(19,8,11,20);line(7,17,13,21);line(8,20,6,23);}
  else if(seat==5){line(4,4,20,4);line(20,4,20,14);line(20,14,12,22);line(12,22,4,14);line(4,14,4,4);}
  else if(seat==1){g.DrawArc(&pen,3,1,18,16,0,180);line(12,17,9,21);line(9,21,15,21);line(15,21,12,17);}
  else if(seat==3 || seat==4){line(9,3,4,6);line(4,6,1,12);line(1,12,6,14);line(6,14,6,22);line(6,22,18,22);line(18,22,18,14);line(18,14,23,12);line(23,12,20,6);line(20,6,15,3);g.DrawArc(&pen,9,0,6,7,0,180);}
  else if(seat==7){g.DrawRectangle(&pen,2,9,20,7);g.DrawRectangle(&pen,9,7,6,11);}
  else if(seat==8){line(7,3,16,3);line(16,3,16,15);line(16,15,22,18);line(22,18,22,22);line(22,22,6,22);line(6,22,7,3);}
  else if(seat==6){line(6,18,6,9);line(6,9,9,9);line(9,9,9,3);line(9,3,18,3);line(18,3,18,18);g.DrawArc(&pen,6,12,12,10,0,180);}
  else if(seat==11){g.DrawArc(&pen,2,2,18,18,20,180);line(3,13,2,20);line(2,20,8,20);line(8,20,8,16);}
  else if(seat==12){g.DrawRectangle(&pen,4,5,16,16);line(4,11,20,11);}
  else {g.DrawEllipse(&pen,9,2,6,6);line(12,8,5,21);line(5,21,19,21);line(19,21,12,8);}
}
std::string inventory_art_key(const ClientState& state, const WorldCarriedItem& item) {
  if (!state.session) return item.id;
  const auto& model = state.session->model();
  auto key = [](const auto& row) { return row.art_key.empty() ? row.id : row.art_key; };
  for (const auto& row : model.inventory) if (row.uuid == item.id) return key(row);
  for (const auto& worn : model.worn) if (worn.item.uuid == item.id) return key(worn.item);
  return item.id;
}
void activate_inventory_item(ClientState& state) {
  if (state.world.carried.empty()) return;
  const auto& item = state.world.carried[std::min(state.selected_item,state.world.carried.size()-1)];
  if (item.equipped && state.session) {
    verdigris::client::ClientCommand command;
    command.type=verdigris::client::ClientCommand::Type::Unequip;
    command.target=item.equip_seat;
    state.session->submit(command);
  } else submit_equip(state,item.id,item.equip_seat);
}
bool compatible_equipment(const WorldCarriedItem& item, int seat) {
  if (seat < 0 || seat >= static_cast<int>(std::size(kDollSeats)) || item.equipped) return false;
  const std::string target=kDollSeats[seat];
  return item.equip_seat == target ||
      ((item.equip_seat=="ring" || item.equip_seat=="ring2") && (target=="ring" || target=="ring2"));
}
void paint_gear_overlay(ClientState& state,HDC dc,const RECT& bounds,render::List& rl) {
  if (!state.gear_overlay) return;
  const int w=bounds.right,h=bounds.bottom,s=hud_scale(h);
  const auto pane=gear_pane_rect(w,h); const auto pack=make_pack_geom(w,h);
  const RECT panel{pane.x,pane.y,pane.x+pane.w,pane.y+pane.h};
  const POINT pointer{state.mouse.x,state.mouse.y};
  const int saved=SaveDC(dc);
  skin::inventory_surface(dc,panel);
  dress_owned_pane(state.billboards,dc,panel);
  state.hud_rect_trace.push_back({"pane-frame",pane});
  auto font=SelectObject(dc,skin::font_small());
  RECT title{pane.x+18*s,pane.y+12*s,pane.x+pane.w-48*s,pane.y+36*s};
  inventory_text(dc,title,"Equipment",skin::kGold);
  const auto close=gear_close_rect(w,h);
  inventory_button(dc,close,"x",PtInRect(&close,pointer));
  reconcile_pack_grid(state);
  const auto& items=state.world.carried;
  int hover=-1; int hover_seat=-1; RECT anchor{};
  auto draw_object=[&](const WorldCarriedItem& item,RECT r) {
    InflateRect(&r,-3*s,-3*s);
    auto key=inventory_art_key(state,item);
    if(key=="hide-girdle")key="girdle_hide";
    if(key=="bronze-gloves")key="grips_bronzescale";
    if(key=="bronze-med-helm")key="crest_bronze";
    bool art=false;
    if(key=="gold-ring" || key=="ring" || key=="coins") {
      Gdiplus::Graphics g(dc);
      const int d=std::max(6L,std::min(r.right-r.left,r.bottom-r.top)-8*s);
      const int cx=(r.left+r.right)/2,cy=(r.top+r.bottom)/2;
      Gdiplus::Pen gold(Gdiplus::Color(255,194,155,65),3.0f*s);
      g.SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);
      g.DrawEllipse(&gold,cx-d/2,cy-d/2,d,d);
      if(key=="coins") { Gdiplus::SolidBrush b(Gdiplus::Color(255,132,101,43));g.FillEllipse(&b,cx-d/2+2,cy-d/2+2,d-4,d-4); }
      art=true;
    } else if(state.billboards.item_art.contains(key)) art=draw_item_art(state.billboards,dc,key,r);
    else if(key.find('_')==std::string::npos && item.equip_seat=="right_hand") art=draw_item_art(state.billboards,dc,key,r);
    if (!art) {
      // An unknown owned item remains named honestly. Never substitute a sword.
      const int clip=SaveDC(dc);IntersectClipRect(dc,r.left,r.top,r.right,r.bottom);
      inventory_text(dc,r,item.name,skin::kInkDim,DT_CENTER|DT_WORDBREAK|DT_END_ELLIPSIS);
      RestoreDC(dc,clip);
    }
    return art;
  };
  for (std::size_t i=0;i<std::size(kDollSeats);++i) {
    const RECT r=pack.seats[i]; const WorldCarriedItem* item=nullptr;
    for (std::size_t j=0;j<items.size();++j)
      if (items[j].equipped && (items[j].equip_seat==kDollSeats[i] ||
          (items[j].equip_seat.empty() && i==2))) { item=&items[j]; break; }
    const bool over=PtInRect(&r,pointer);
    int focus=over ? 1 : 0;
    if (over && state.pack_drag_live) {
      const auto j=carried_index_for_pack_id(state,state.pack_drag_id);
      focus=j<items.size() && compatible_equipment(items[j],static_cast<int>(i)) ? 1 : -1;
    }
    skin::inventory_surface(dc,r,focus);
    if (item) {
      draw_object(*item,r);
      if (over || (state.gear_keyboard_focus && static_cast<std::size_t>(item-items.data())==state.selected_item)) {
        hover=static_cast<int>(item-items.data());anchor=r;
      }
    } else {
      empty_equipment_outline(dc,r,i);
    }
    if (over) hover_seat=static_cast<int>(i);
    state.hud_rect_trace.push_back({"pane-doll-slot",{r.left,r.top,r.right-r.left,r.bottom-r.top}});
    rl.push_back({render::Op::Hud,double(r.left),double(r.top),0,item ? 1 : 0,
        std::string("paperdoll-slot:")+kDollSeats[i]+(item ? ":filled" : ":empty")});
    if (i==2) {
      const std::string name=item ? item->name : "(empty)";
      rl.push_back({render::Op::PaneWeapon,0,0,0,0,name});
      rl.push_back({render::Op::Hud,0,0,0,0,"held-seat:"+name});
      state.hud_rect_trace.push_back({"pane-seat",{r.left,r.top,r.right-r.left,r.bottom-r.top}});
    }
  }
  const int cells=static_cast<int>(std::count_if(state.pack_grid.occupancy.begin(),state.pack_grid.occupancy.end(),[](auto id){return id!=0;}));
  RECT bag_title{pack.grid_left,pack.grid_top-22*s,pack.grid_left+12*pack.cell_w,pack.grid_top-3*s};
  inventory_text(dc,bag_title,"Backpack",skin::kInkDim);
  inventory_text(dc,bag_title,std::to_string(cells)+" / 84",skin::kInkDim,DT_RIGHT|DT_VCENTER|DT_SINGLELINE);
  for (int y=0;y<kPackRows;++y) for (int x=0;x<kPackColumns;++x) {
    RECT r{pack.grid_left+x*pack.cell_w,pack.grid_top+y*pack.cell_h,
        pack.grid_left+(x+1)*pack.cell_w,pack.grid_top+(y+1)*pack.cell_h};
    skin::inventory_surface(dc,r);
    state.hud_rect_trace.push_back({"pane-backpack-cell",{r.left,r.top,pack.cell_w,pack.cell_h}});
  }
  for (std::uint8_t i=0;i<state.pack_grid.count;++i) {
    const auto& placed=state.pack_grid.items[i];
    const auto j=carried_index_for_pack_id(state,placed.id); if(j>=items.size())continue;
    RECT r{pack.grid_left+placed.x*pack.cell_w,pack.grid_top+placed.y*pack.cell_h,
      pack.grid_left+(placed.x+placed.width)*pack.cell_w,pack.grid_top+(placed.y+placed.height)*pack.cell_h};
    const bool over=PtInRect(&r,pointer);
    const bool selected=state.gear_keyboard_focus && j==state.selected_item;
    skin::inventory_surface(dc,r,over||selected ? 1 : 0);
    const bool art=draw_object(items[j],r);
    if (items[j].quantity>1) inventory_text(dc,r,std::to_string(items[j].quantity),skin::kInk,DT_RIGHT|DT_BOTTOM|DT_SINGLELINE);
    if(over || selected) { hover=static_cast<int>(j);anchor=r; }
    state.hud_rect_trace.push_back({"pane-cell",{r.left,r.top,r.right-r.left,r.bottom-r.top}});
    rl.push_back({render::Op::PaneItem,double(r.left),double(r.top),0,items[j].attack_bonus,items[j].name});
    rl.push_back({render::Op::Hud,double(placed.x),double(placed.y),0,art?1:0,art?"pack-glyph:billboard":"pack-glyph:unmapped"});
    rl.push_back({render::Op::Hud,double(placed.x),double(placed.y),0,int(placed.id),"pack:"+std::to_string(placed.x)+","+std::to_string(placed.y)});
  }
  if(state.pack_drag_live) {
    int x=-1,y=-1;pack_hit_cell(pack,pointer.x,pointer.y,x,y);
    state.pack_preview_x=x-state.pack_grab_x;state.pack_preview_y=y-state.pack_grab_y;
    state.pack_preview_ok=pack_can_land(state.pack_grid,state.pack_drag_id,state.pack_preview_x,state.pack_preview_y);
    const auto j=carried_index_for_pack_id(state,state.pack_drag_id);
    if(j<items.size()) {
      RECT ghost{pointer.x-state.pack_grab_x*pack.cell_w,pointer.y-state.pack_grab_y*pack.cell_h,0,0};
      ghost.right=ghost.left+items[j].width*pack.cell_w;ghost.bottom=ghost.top+items[j].height*pack.cell_h;
      const bool valid=hover_seat>=0 ? compatible_equipment(items[j],hover_seat) : state.pack_preview_ok;
      OffsetRect(&ghost,std::clamp(int(ghost.left),0,std::max(0,w-int(ghost.right-ghost.left)))-ghost.left,
          std::clamp(int(ghost.top),0,std::max(0,h-int(ghost.bottom-ghost.top)))-ghost.top);
      skin::inventory_surface(dc,ghost,valid?1:-1);draw_object(items[j],ghost);
      RECT feedback{ghost.left,std::max(0L,ghost.top-22*s),std::min(LONG(w),ghost.left+160*s),std::max(0L,ghost.top-22*s)+20*s};
      skin::inventory_surface(dc,feedback,valid?1:-1);
      inventory_text(dc,feedback,valid?"Fits":"Cannot place here",skin::kInk);
      rl.push_back({render::Op::Hud,double(x),double(y),0,valid?1:0,valid?"pack-preview:ok":"pack-preview:reject"});
    }
  }
  const auto action=gear_action_rect(w,h,0),character=gear_action_rect(w,h,1);
  const auto pick=items.empty()?0:std::min(state.selected_item,items.size()-1);
  inventory_button(dc,action,state.equip_view.pending?"Equipping...":items.empty()?"Select an item":items[pick].equipped?"Unequip":"Equip",PtInRect(&action,pointer));
  inventory_button(dc,character,"Character",PtInRect(&character,pointer));
  if(state.equip_view.pending) {
    rl.push_back({render::Op::Hud,0,0,0,0,"compare:pending"});
  }
  // Stats are authoritative, compact, and available in the deliberate sheet.
  const auto& p=state.world.player;
  rl.push_back({render::Op::PaneStat,0,0,0,0,"ATK "+std::to_string(p.attack+p.gear_attack)+" DEF "+std::to_string(p.defense)+" LVL "+std::to_string(p.level)});
  rl.push_back({render::Op::Hud,0,0,0,p.defense,"gear:stats-def"});
  rl.push_back({render::Op::Hud,0,0,0,p.level,"gear:stats-lvl"});
  if(!state.pack_last_drop.empty())rl.push_back({render::Op::Hud,0,0,0,0,"pack-drop:"+state.pack_last_drop});
  if(hover>=0 && !state.pack_drag_live) {
    const auto& item=items[hover];
    std::vector<std::string> facts;
    if(item.attack_bonus)facts.push_back("Attack rating +"+std::to_string(item.attack_bonus));
    facts.push_back(item.equipped?"Equipped":"Carried");
    if(item.two_handed)facts.push_back("Requires both hands");
    paint_compare_plate(state,dc,anchor.left,anchor.top,bounds,pane,item.name,skin::kInk,facts,rl);
    rl.push_back({render::Op::Hud,0,0,0,0,"pack-name:full"});
    rl.push_back({render::Op::Hud,0,0,0,0,item.equipped?"compare:equipped":"compare:candidate"});
  } else if(hover_seat>=0 && !state.pack_drag_live) {
    const auto r=pack.seats[hover_seat];
    paint_compare_plate(state,dc,r.left,r.top,bounds,pane,kEquipmentNames[hover_seat],skin::kInk,{},rl);
  }
  SelectObject(dc,font);RestoreDC(dc,saved);
}
