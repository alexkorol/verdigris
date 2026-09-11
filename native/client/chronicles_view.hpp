#pragma once

// Included beside the existing Chronicle commands: clickable presentation only.
void paint_lineage_door(ClientState& state,HDC dc,const RECT& bounds,render::List& trace) {
  const auto& model=state.session?state.session->model():verdigris::client::ClientModel{};
  const int s=hud_scale(bounds.bottom);
  skin::set_ui_scale(s);
  const int saved=SaveDC(dc);SetBkMode(dc,TRANSPARENT);
  HBRUSH back=CreateSolidBrush(RGB(18,17,15));FillRect(dc,&bounds,back);DeleteObject(back);
  const int width=std::min(int(bounds.right)-32*s,1100*s),left=(bounds.right-width)/2;
  const int top=std::max(60*s,(int(bounds.bottom)-680*s)/2);
  const int bottom=std::min(int(bounds.bottom)-24*s,top+650*s);
  RECT page{left,top,left+width,bottom};skin::hud_panel(dc,page,255,20*s);
  auto text=[&](const std::string& value,RECT box,COLORREF color,HFONT font,UINT flags=DT_LEFT|DT_WORDBREAK) {
    SelectObject(dc,font);SetTextColor(dc,color);
    DrawTextA(dc,value.c_str(),int(value.size()),&box,flags|DT_NOPREFIX);
  };
  auto tag=[&](const std::string& label,const RECT& box) {
    trace.push_back({render::Op::Chronicles,double(box.left),double(box.top),0,box.right-box.left,label});
  };
  text("VERDIGRIS",{left+30*s,top+22*s,left+width-30*s,top+65*s},skin::kInk,skin::font_title());
  tag("title",page);
  std::string account=!state.session||state.session->connection_state()==verdigris::client::ConnectionState::Disconnected
      ?"The chronicles lie closed - connection lost.":!model.chronicle.present?"Opening the chronicles...":
      "Account of "+(model.chronicle.account_name.empty()?std::string("the guest"):model.chronicle.account_name);
  text(account,{left+32*s,top+67*s,left+width-32*s,top+94*s},skin::kInkDim,skin::font_small());
  tag("account",page);
  state.chronicle_hits.clear();state.chronicles_menu=chronicle_actions(state);
  const int split=left+width*44/100;
  const int content_top=top+105*s,footer=bottom-90*s;
  auto button=[&](const ChronicleAction& action,const std::string& label,RECT box,bool active=false) {
    const bool hover=PtInRect(&box,state.mouse)!=FALSE;
    skin::hud_panel(dc,box,active||hover?255:235,active?14*s:10*s);
    RECT caption{box.left+15*s,box.top+9*s,box.right-15*s,box.bottom-8*s};
    text(label,caption,active||hover?skin::kGold:skin::kInk,skin::font_body(),DT_CENTER|DT_VCENTER|DT_SINGLELINE|DT_END_ELLIPSIS);
    state.chronicle_hits.push_back({box,action});
    tag("action:"+action.command+(action.arg.empty()?"":":"+action.arg),box);
  };
  text("A new Scion",{left+32*s,content_top,split-20*s,content_top+32*s},skin::kGold,skin::font_heading());
  const int gap=12*s,card_width=(split-left-64*s-gap)/2;
  const int card_top=content_top+40*s,card_bottom=std::max(card_top+110*s,footer-52*s);
  for(int i=0;i<2;++i) {
    const std::string appearance=i?"female":"male";
    RECT card{left+32*s+i*(card_width+gap),card_top,left+32*s+i*(card_width+gap)+card_width,card_bottom};
    const bool active=state.selected_appearance==appearance;
    skin::hud_panel(dc,card,active?255:230,active?14*s:10*s);
    const int draw_height=std::min(330*s,int(card.bottom-card.top-40*s)*192/135);
    // Runtime registration leaves32px below the shared foot pivot.
    const int feet=card.bottom-36*s+draw_height*32/192;
    raster_art::draw_sprite(dc,("hero_"+appearance+"_s").c_str(),(card.left+card.right)/2,feet,draw_height);
    text(i?"Female":"Male",{card.left+8*s,card.bottom-34*s,card.right-8*s,card.bottom-8*s},
         active?skin::kGold:skin::kInk,skin::font_heading(),DT_CENTER|DT_SINGLELINE);
    state.chronicle_hits.push_back({card,{"","appearance",appearance,""}});
    tag("appearance:"+appearance+(active?":selected":""),card);
  }
  text("Appearance changes the character's look.\nAbilities and starting equipment are the same.",
       {left+32*s,card_bottom+8*s,split-20*s,footer},skin::kInkDim,skin::font_small());
  const bool has_house=model.chronicle.present&&!model.chronicle.houses.empty();
  if(has_house) button({"C","create-scion","",""},"Create "+std::string(state.selected_appearance=="female"?"female":"male")+" Scion",
      {left+32*s,footer+8*s,split-20*s,footer+53*s});
  else text("Found a House to begin your lineage.",{left+32*s,footer+8*s,split-20*s,footer+58*s},skin::kInkDim,skin::font_body());

  const int roster_left=split+16*s,roster_right=left+width-32*s;
  int y=content_top;
  for(const auto& house:model.chronicle.houses) {
    RECT line{roster_left,y,roster_right,y+32*s};
    text(painted_house_name(house.name),line,skin::kGold,skin::font_heading());tag("house "+house.name,line);
    y+=36*s;break;
  }
  if(!has_house) {
    RECT line{roster_left,y,roster_right,y+70*s};
    text("Your House carries its history across generations. Found it, then choose your first Scion.",line,skin::kInk,skin::font_body());tag("prompt",line);
    button({"F","found-house","",""},"Found your House",{roster_left,y+85*s,roster_right,y+135*s});
  } else {
    std::vector<ChronicleAction> admissions;
    for(const auto& action:state.chronicles_menu)
      if(action.command=="set-out"||action.command=="select-scion") admissions.push_back(action);
    const int capacity=std::max(1,(footer-y-82*s)/(66*s));
    const int pages=std::max(1,(int(admissions.size())+capacity-1)/capacity);
    state.chronicle_page=std::clamp(state.chronicle_page,0,pages-1);
    for(int i=state.chronicle_page*capacity;i<std::min(int(admissions.size()),(state.chronicle_page+1)*capacity);++i) {
      const auto& action=admissions[i];
      button(action,action.label,{roster_left,y,roster_right,y+54*s});
      tag("scion "+action.arg,{roster_left,y,roster_right,y+54*s});y+=66*s;
    }
    if(admissions.empty()) {text("No living Scion. Create one to set out.",{roster_left,y,roster_right,y+48*s},skin::kInkDim,skin::font_body());y+=54*s;}
    if(pages>1) {
      const int half=(roster_right-roster_left-8*s)/2;
      button({"","roster-prev","",""},"Previous",{roster_left,y,roster_left+half,y+35*s});
      button({"","roster-next","",""},"Next",{roster_left+half+8*s,y,roster_right,y+35*s});y+=40*s;
    }
    for(const auto& house:model.chronicle.houses) for(const auto& entry:house.crypt) {
      if(y+22*s>footer-16*s) break;
      RECT row{roster_left,y,roster_right,y+22*s};
      text("In the crypt: "+entry.name+" - heirloom "+entry.relic_status,row,skin::kInkDim,skin::font_small(),DT_LEFT|DT_SINGLELINE|DT_END_ELLIPSIS);
      tag("crypt "+entry.id,row);y+=24*s;
    }
    button({"M","oath-toggle","",""},state.chronicles_oath?"Mortal oath: armed":"Mortal oath: not taken",
        {roster_left,footer+8*s,roster_right,footer+53*s},state.chronicles_oath);
  }
  tag(state.chronicles_oath?"oath:on":"oath:off",page);
  if(!model.chronicle.fallen.name.empty()) {
    const RECT epitaph{roster_left,bottom-32*s,roster_right,bottom-9*s};
    text("Remember "+model.chronicle.fallen.name,epitaph,skin::kInkDim,skin::font_small());
    tag("fallen:"+model.chronicle.fallen.scion_id,epitaph);
  }
  if(state.relic_toast_ticks>0&&!state.relic_toast.empty()) {
    text(state.relic_toast,{left+32*s,bottom-31*s,left+width-32*s,bottom-8*s},skin::kGold,skin::font_small());tag("relic-toast",page);
  }
  RestoreDC(dc,saved);
}
