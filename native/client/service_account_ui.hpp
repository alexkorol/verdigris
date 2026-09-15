#pragma once
// Admission owns input before any House or Scion is admitted. Secrets never
// enter labels, telemetry, command-line arguments or presentation effects.
bool service_account_open(const ClientState& state) {
  return state.session && state.session->model().service_mode && !state.session->model().authenticated;
}
struct ServiceAccountLayout { RECT panel{}, field{}, enroll{}, login{}, quit{}; };
ServiceAccountLayout service_account_layout(int w,int h) {
  const int s=hud_scale(h),ww=std::min(w-32*s,480*s),hh=std::min(h-32*s,430*s);
  const int x=(w-ww)/2,y=(h-hh)/2;
  return {{x,y,x+ww,y+hh},{x+24*s,y+122*s,x+ww-24*s,y+164*s},
    {x+24*s,y+184*s,x+ww-24*s,y+224*s},{x+24*s,y+236*s,x+ww-24*s,y+276*s},
    {x+24*s,y+288*s,x+ww-24*s,y+328*s}};
}
void service_account_submit(ClientState& state,bool enroll) {
  if(!state.session || state.account_credential.empty())return;
  const auto status=state.session->connection_state();
  if(status!=verdigris::client::ConnectionState::Connected && status!=verdigris::client::ConnectionState::Ready)return;
  verdigris::client::ClientCommand command;command.type=verdigris::client::ClientCommand::Type::Authenticate;
  command.target=state.account_credential;command.value=enroll?1:0;
  state.session->submit(command);
  SecureZeroMemory(command.target.data(),command.target.size());
  SecureZeroMemory(state.account_credential.data(),state.account_credential.size());state.account_credential.clear();
  state.account_cursor.focus(state.account_credential);
}
void service_account_character(ClientState& state,WPARAM c) {
  auto& value=state.account_credential;auto& edit=state.account_cursor;
  edit.clamp(value);
  if(c==8) {if(!edit.erase_selection(value)&&edit.caret){value.erase(--edit.caret,1);edit.anchor=edit.caret;}}
  else if(c>=33&&c<127 && (value.size()<512 || edit.caret!=edit.anchor)) {
    edit.erase_selection(value);value.insert(value.begin()+edit.caret,static_cast<char>(c));edit.anchor=++edit.caret;
  }
}
bool service_account_key(ClientState& state,WPARAM key) {
  if(!service_account_open(state))return false;
  const bool control=(GetKeyState(VK_CONTROL)&0x8000)!=0,shift=(GetKeyState(VK_SHIFT)&0x8000)!=0;
  if(key==VK_RETURN){service_account_submit(state,!shift);return true;}
  if(key==VK_ESCAPE){state.quit_requested=true;return true;}
  if(control && key=='V') {
    if(OpenClipboard(nullptr)) {
      if(auto handle=GetClipboardData(CF_UNICODETEXT))if(const auto* text=static_cast<const wchar_t*>(GlobalLock(handle))) {
        for(std::size_t i=0;i<512 && text[i];++i)service_account_character(state,text[i]);
        GlobalUnlock(handle);
      }
      CloseClipboard();
    }
    return true;
  }
  // Navigation/select-all/deletion only; the shared editor has no clipboard
  // copy operation, so masked credentials cannot be copied accidentally.
  state.account_cursor.key(state.account_credential,key,shift,control);return true;
}
bool service_account_click(ClientState& state,HWND window,POINT point) {
  if(!service_account_open(state))return false;
  state.held_gameplay_attacks.clear();state.primary_down=false;
  RECT rect{};GetClientRect(window,&rect);const auto layout=service_account_layout(rect.right,rect.bottom);
  if(PtInRect(&layout.field,point)) {
    HDC dc=GetDC(window);const std::string masked(state.account_credential.size(),'*');
    skin::click_entry(dc,masked,state.account_cursor,layout.field,point.x,false);ReleaseDC(window,dc);
  }
  if(PtInRect(&layout.enroll,point))service_account_submit(state,true);
  if(PtInRect(&layout.login,point))service_account_submit(state,false);
  if(PtInRect(&layout.quit,point)){state.quit_requested=true;PostQuitMessage(0);}
  return true;
}
void paint_service_account(ClientState& state,HDC dc,const RECT& bounds,render::List& trace) {
  FillRect(dc,&bounds,cached_brush(RGB(7,10,11)));
  const auto& art=state.billboards.menu_gateway;
  if(art.ready()) {
    const double fit=double(bounds.bottom)/art.height;const int ww=int(art.width*fit);
    SetStretchBltMode(dc,HALFTONE);StretchBlt(dc,(bounds.right-ww)/2,0,ww,bounds.bottom,art.dc,0,0,art.width,art.height,SRCCOPY);
  }
  const auto layout=service_account_layout(bounds.right,bounds.bottom);const int s=hud_scale(bounds.bottom);
  skin::panel(dc,layout.panel,skin::kGold,250,7.0f);const auto old=SelectObject(dc,skin::font_heading());
  RECT heading{layout.panel.left+24*s,layout.panel.top+18*s,layout.panel.right-24*s,layout.panel.top+58*s};
  inventory_text(dc,heading,"Verdigris Online",skin::kGold,DT_CENTER|DT_VCENTER|DT_SINGLELINE);
  SelectObject(dc,skin::font_small());
  RECT label{heading.left,heading.bottom+12*s,heading.right,layout.field.top-8*s};
  inventory_text(dc,label,"Enrollment code or account token",skin::kInk,DT_CENTER|DT_VCENTER|DT_SINGLELINE);
  skin::inventory_surface(dc,layout.field,1);
  const std::string masked(state.account_credential.size(),'*');
  skin::paint_entry(dc,layout.field,masked,"Enter or paste credential",state.account_cursor,true);
  const auto status=state.session->connection_state();
  const bool enabled=!state.account_credential.empty() && (status==verdigris::client::ConnectionState::Connected || status==verdigris::client::ConnectionState::Ready);
  inventory_button(dc,layout.enroll,"Enroll with code",PtInRect(&layout.enroll,state.mouse),enabled);
  inventory_button(dc,layout.login,"Sign in with token",PtInRect(&layout.login,state.mouse),enabled);
  inventory_button(dc,layout.quit,"Quit",PtInRect(&layout.quit,state.mouse));
  RECT error{heading.left,layout.quit.bottom+16*s,heading.right,layout.panel.bottom-12*s};
  const auto& account_error=state.session->model().account_error;
  const std::string message=!account_error.empty()?account_error:
      status==verdigris::client::ConnectionState::Connected?"Connected to service":
      status==verdigris::client::ConnectionState::Ready?"Connected to service":
      !state.session->last_error().empty()?state.session->last_error():verdigris::client::connection_state_label(status);
  inventory_text(dc,error,message,skin::kGold,DT_CENTER|DT_WORDBREAK|DT_END_ELLIPSIS);
  SelectObject(dc,old);trace.push_back({render::Op::Hud,0,0,0,0,"service:account-admission"});
}
