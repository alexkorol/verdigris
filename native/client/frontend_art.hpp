#pragma once

// Physical artwork and responsive composition; the existing controller owns
// activation, settings persistence, navigation, Continue and House/Scion flow.
void paint_frontend(ClientState& state,HDC dc,const RECT& bounds,render::List& rl) {
  FillRect(dc,&bounds,cached_brush(RGB(7,10,11)));
  const auto& art=state.billboards.menu_gateway;
  const auto& control=state.billboards.menu_control;
  const double fit=bounds.bottom/941.0;
  const int aw=static_cast<int>(1672*fit),ah=static_cast<int>(941*fit);
  const int ox=std::max(0,(int(bounds.right)-aw)/2),oy=(bounds.bottom-ah)/2;
  if(art.ready()) {
    SetStretchBltMode(dc,HALFTONE);
    StretchBlt(dc,ox,oy,aw,ah,art.dc,0,0,art.width,art.height,SRCCOPY);
  }
  skin::set_ui_scale(hud_scale(bounds.bottom));
  const int cx=ox+static_cast<int>(440*fit);
  const int width=std::max(300,static_cast<int>(510*fit));
  const int top=oy+static_cast<int>(300*fit);
  const int stride=std::max(68,static_cast<int>(119*fit));
  const int button_h=static_cast<int>(113*fit);
  const char* heading=state.frontend==Frontend::Title?"VERDIGRIS":
      state.frontend==Frontend::Settings?"Settings":
      state.frontend==Frontend::ConfirmQuit?"Leave Verdigris?":"At rest";
  const int saved=SaveDC(dc);SetBkMode(dc,TRANSPARENT);
  SelectObject(dc,fit<.8?skin::font_heading():skin::font_title());
  SIZE title{};skin::text_extent(dc,heading,static_cast<int>(std::strlen(heading)),&title);
  const int title_y=oy+static_cast<int>(231*fit);
  SetTextColor(dc,RGB(27,13,6));skin::text_out(dc,cx-title.cx/2+2,title_y+2,heading,static_cast<int>(std::strlen(heading)));
  SetTextColor(dc,RGB(239,199,116));skin::text_out(dc,cx-title.cx/2,title_y,heading,static_cast<int>(std::strlen(heading)));
  rl.push_back({render::Op::Hud,double(cx-width/2),double(title_y),0,0,std::string("frontend:")+heading});
  rl.push_back({render::Op::Hud,0,0,0,0,"frontend:illustrated-gateway"});
  const auto rows=frontend_rows(state);
  state.menu_hits.clear();state.menu_selected=std::min(state.menu_selected,rows.size()-1);
  SelectObject(dc,skin::font_heading());
  const double phase=GetTickCount64()/1000.0;
  for(std::size_t i=0;i<rows.size();++i) {
    const int y=top+static_cast<int>(i)*stride;
    RECT button{cx-width/2,y,cx+width/2,y+button_h};
    const bool focus=i==state.menu_selected || PtInRect(&button,state.mouse);
    if(control.ready())skin::relic_control(dc,control.dc,control.width,control.height,button,focus,phase);
    else skin::panel(dc,button,skin::kGold,250);
    std::string label=rows[i];
    SIZE extent{};skin::text_extent(dc,label.c_str(),static_cast<int>(label.size()),&extent);
    const int tx=cx-extent.cx/2,ty=y+(button_h-extent.cy)/2;
    SetTextColor(dc,RGB(19,7,3));skin::text_out(dc,tx+1,ty+2,label.c_str(),static_cast<int>(label.size()));
    SetTextColor(dc,focus?RGB(255,225,134):RGB(212,176,108));
    skin::text_out(dc,tx,ty,label.c_str(),static_cast<int>(label.size()));
    if(state.frontend==Frontend::Settings && (i==1 || i==2)) {
      for(int step: {-1,1}) {
        const int edge=static_cast<int>(width*.12),size=std::max(24,static_cast<int>(button_h*.46));
        const int x=step<0?button.left+edge:button.right-edge-size;
        RECT adjust{x,y+(button_h-size)/2,x+size,y+(button_h+size)/2};
        const char* sign=step<0?"-":"+";SIZE measure{};skin::text_extent(dc,sign,1,&measure);
        skin::text_out(dc,x+(size-measure.cx)/2,adjust.top+(size-measure.cy)/2,sign,1);
        state.menu_hits.push_back({adjust,i,step});
      }
    }
    state.menu_hits.push_back({button,i,0});
    rl.push_back({render::Op::Hud,double(button.left),double(button.top),0,0,"menu:"+rows[i]});
  }
  SelectObject(dc,bounds.right<1000?skin::font(skin::TextRole::CompactValue):skin::font_small());SetTextColor(dc,RGB(161,143,114));
  const std::string version=std::string("Build ")+std::string(VERDIGRIS_BUILD_ID).substr(0,12)+
      (VERDIGRIS_BUILD_DIRTY?" (development)":"");
  const int footer_y=oy+static_cast<int>(873*fit);
  skin::text_out(dc,ox+static_cast<int>(210*fit),footer_y,version.c_str(),static_cast<int>(version.size()));
  if(state.frontend==Frontend::Settings && !state.settings_message.empty()) {
    RECT msg{ox+static_cast<int>(840*fit),oy+static_cast<int>(740*fit),ox+aw-30,oy+ah-30};
    skin::draw_text(dc,state.settings_message.c_str(),static_cast<int>(state.settings_message.size()),&msg,DT_WORDBREAK|DT_NOPREFIX);
  }
  RestoreDC(dc,saved);
}

