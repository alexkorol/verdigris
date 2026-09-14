#pragma once

int scenario_typography() {
  ClientState state;scenario_begin(state);
  const auto dir=art_wave_capture_dir();
  skin::ensure_game_fonts();
  scenario_check(skin::game_font_available(),"type: bundled font registered from executable-relative asset");
  HDC dc=CreateCompatibleDC(nullptr);
  BITMAPINFO info{};info.bmiHeader.biSize=sizeof(BITMAPINFOHEADER);
  info.bmiHeader.biWidth=1000;info.bmiHeader.biHeight=-360;
  info.bmiHeader.biPlanes=1;info.bmiHeader.biBitCount=32;
  void* pixels=nullptr;HBITMAP bitmap=CreateDIBSection(dc,&info,DIB_RGB_COLORS,&pixels,nullptr,0);
  const auto old=SelectObject(dc,bitmap);
  RECT all{0,0,1000,360};FillRect(dc,&all,static_cast<HBRUSH>(GetStockObject(BLACK_BRUSH)));
  SetBkMode(dc,TRANSPARENT);SetTextColor(dc,skin::kInk);
  const char* samples[]={"House Verdigris","Prepare for the Tin Road","Your pack is full.",
    "Attack 12   Defense 5   Life 100 / 100","Il1   O0   rn m   \xe2\x80\x98quoted text\xe2\x80\x99   90%",
    "The old road leads through the forest. Take care, traveller."};
  SIZE base{};
  for(int scale:{1,2}) {
    skin::set_ui_scale(scale);SelectObject(dc,skin::font_body());
    wchar_t face[64]{};GetTextFaceW(dc,64,face);
    scenario_check(std::wstring(face)==L"Verdigris Novel","type: selected face is bundled family, never a system substitution");
    const auto font=skin::font_body();
    scenario_check(font==skin::font_body(),"type: role resources reused across paints");
    SIZE extent{};skin::text_extent(dc,samples[3],-1,&extent);
    if(scale==1)base=extent;
    else scenario_check(extent.cx==base.cx*2&&extent.cy==base.cy*2,"type: integer scale doubles actual advances and line metrics");
    for(int i=0;i<6;++i)skin::text_out(dc,8,scale==1?4+i*20:140+i*36,samples[i],-1);
  }
  const auto* bits=static_cast<const std::uint32_t*>(pixels);
  const std::uint32_t rgb=(GetRValue(skin::kInk)<<16)|(GetGValue(skin::kInk)<<8)|GetBValue(skin::kInk);
  bool crisp=true;int ink=0;
  for(int i=0;i<1000*360;++i){const auto c=bits[i]&0xffffff;crisp&=c==0||c==rgb;ink+=c==rgb;}
  scenario_check(crisp&&ink>100,"type: actual GDI glyphs contain only ink and transparent backing, no antialias fringes");
  scenario_check(save_hbitmap_png(state.billboards,bitmap,dir+"/type-specimen.png"),"type: native shared-role specimen saved");
  SelectObject(dc,old);DeleteObject(bitmap);DeleteDC(dc);
  scenario_check(skin::display_text("\xe2\x80\x98quoted\xe2\x80\x99",-1)==L"\u2018quoted\u2019","type: UTF-8 smart quotes retain bundled glyphs");
  scenario_check(skin::display_text("\xf0\x9f\x98\x80",-1)==L"?","type: unsupported supplementary character is one explicit same-family replacement");
  scenario_check(skin::display_text("\xe2\x80\xa6",-1)==L"...","type: ellipsis uses three same-family periods");

  skin::TextEntry edit;std::string value="Willow";edit.focus(value);
  edit.key(value,VK_HOME,false,false);edit.key(value,VK_RIGHT,true,false);edit.character(value,'B');
  scenario_check(value=="Billow"&&edit.caret==1,"type: typing replaces selection at the caret");
  edit.key(value,VK_END,false,false);edit.character(value,8);edit.key(value,VK_LEFT,false,false);edit.key(value,VK_DELETE,false,false);
  scenario_check(value=="Bill","type: backspace and delete operate around moved caret");
  edit.key(value,'A',false,true);edit.character(value,'I');
  scenario_check(value=="I","type: select-all replacement works");
  value=std::string(40,'W');edit.focus(value);edit.character(value,'X');
  scenario_check(value.size()==40,"type: established name length limit retained");

  for(const auto size:{std::pair{960,600},std::pair{1280,800},std::pair{3440,1440}}) {
    const auto suffix=std::to_string(size.first)+".png";
    state.camera.perspective=true;state.lineage_art=true;state.camera.zoom=kCameraDefaultZoom*zoom_height_factor(size.second);
    state.frontend=Frontend::Title;
    scenario_check(reference_present(state,size.first,size.second,dir+"/type-title-"+suffix),"type: production title captured");
    state.frontend=Frontend::Settings;
    scenario_check(reference_present(state,size.first,size.second,dir+"/type-settings-"+suffix),"type: production settings captured");
    state.frontend=Frontend::None;state.screen=Screen::Chronicles;
    state.house_name_input="House of the Long Willow Road";state.scion_name_input=std::string(40,'W');
    state.chronicle_edit="scion-name";state.chronicle_cursor.focus(state.scion_name_input);
    state.chronicle_cursor.anchor=30;
    scenario_check(reference_present(state,size.first,size.second,dir+"/type-entry-"+suffix),"type: production name field selection and scrolling captured (fixture roster)");
    scenario_check(state.chronicle_cursor.caret==40&&state.chronicle_cursor.anchor==30,"type: painting other fields does not move focused caret");
    HDC measure=CreateCompatibleDC(nullptr);SelectObject(measure,skin::font_body());
    for(const auto& hit:state.chronicle_hits)if(hit.action.command=="scion-name") {
      const int at=skin::entry_advance(measure,state.scion_name_input,state.chronicle_cursor.caret)-state.chronicle_cursor.scroll;
      scenario_check(at>=0&&at<=hit.rect.right-hit.rect.left-24*skin::ui_scale(),"type: full long name fits or scrolls with caret inside field");
    }
    DeleteDC(measure);
    state.screen=Screen::Expedition;state.chronicle_edit.clear();
    state.event_log={"Your pack is full.","The old road leads through the forest. Take care, traveller.",
      "Attack 12   Defense 5   Life 100 / 100"};
    scenario_check(reference_present(state,size.first,size.second,dir+"/type-gameplay-log-"+suffix),"type: existing log with multiline fixture and production world labels captured");
    scenario_check(render_list_has(state,render::Op::Hud,"combat-log"),"type: actual existing message log is painted");
  }
  return scenario_failures;
}
