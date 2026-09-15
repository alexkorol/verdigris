#pragma once
// Admission owns input before any House or Scion is admitted. Secrets never
// enter labels, telemetry, command-line arguments or presentation effects.
bool service_account_open(const ClientState& state) {
  return state.session && state.session->model().service_mode && !state.session->model().authenticated &&
      state.frontend!=Frontend::Settings && state.frontend!=Frontend::ConfirmQuit;
}
void prepare_service_account(ClientState& state) {
  if(service_account_open(state)&&(state.frontend==Frontend::None||state.frontend==Frontend::Pause)) {
    state.account_entry_open=false;
    SecureZeroMemory(state.account_credential.data(),state.account_credential.size());state.account_credential.clear();
    state.account_cursor.focus(state.account_credential);open_frontend(state,Frontend::Title);
  }
}
struct ServiceAccountLayout { RECT panel{}, field{}, enroll{}, login{}, quit{}; };
ServiceAccountLayout service_account_layout(int w,int h) {
  const auto g=gateway_layout(w,h);const auto outer=g.row(0);const int inset=int(g.width*.12);
  RECT field{outer.left+inset,outer.top+int(g.button_h*.2),outer.right-inset,outer.bottom-int(g.button_h*.2)};
  return {outer,field,g.row(1),g.row(2),g.row(3)};
}
void service_account_back(ClientState& state) {
  SecureZeroMemory(state.account_credential.data(),state.account_credential.size());state.account_credential.clear();
  state.account_cursor.focus(state.account_credential);state.account_entry_open=false;
  open_frontend(state,Frontend::Title);
}
void service_account_submit(ClientState& state,bool enroll) {
  if(!state.account_entry_open || !state.session || state.account_credential.empty())return;
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
  if(!state.account_entry_open)return;
  auto& value=state.account_credential;auto& edit=state.account_cursor;
  edit.clamp(value);
  if(c==8) {if(!edit.erase_selection(value)&&edit.caret){value.erase(--edit.caret,1);edit.anchor=edit.caret;}}
  else if(c>=33&&c<127 && (value.size()<512 || edit.caret!=edit.anchor)) {
    edit.erase_selection(value);value.insert(value.begin()+edit.caret,static_cast<char>(c));edit.anchor=++edit.caret;
  }
}
bool service_account_key(ClientState& state,WPARAM key) {
  if(!service_account_open(state))return false;
  prepare_service_account(state);
  if(!state.account_entry_open)return handle_frontend_key(state,key);
  const bool control=(GetKeyState(VK_CONTROL)&0x8000)!=0,shift=(GetKeyState(VK_SHIFT)&0x8000)!=0;
  if(key==VK_RETURN){service_account_submit(state,!shift);return true;}
  if(key==VK_ESCAPE){service_account_back(state);return true;}
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
  prepare_service_account(state);
  state.held_gameplay_attacks.clear();state.primary_down=false;
  if(!state.account_entry_open) {
    for(const auto& hit:state.menu_hits)if(PtInRect(&hit.rect,point)){state.menu_selected=hit.index;activate_frontend_row(state,hit.direction);break;}
    return true;
  }
  RECT rect{};GetClientRect(window,&rect);const auto layout=service_account_layout(rect.right,rect.bottom);
  if(PtInRect(&layout.field,point)) {
    HDC dc=GetDC(window);const std::string masked(state.account_credential.size(),'*');
    skin::click_entry(dc,masked,state.account_cursor,layout.field,point.x,false);ReleaseDC(window,dc);
  }
  if(PtInRect(&layout.enroll,point))service_account_submit(state,true);
  if(PtInRect(&layout.login,point))service_account_submit(state,false);
  if(PtInRect(&layout.quit,point))service_account_back(state);
  return true;
}
void paint_service_account(ClientState& state,HDC dc,const RECT& bounds,render::List& trace) {
  prepare_service_account(state);
  if(!state.account_entry_open){paint_frontend(state,dc,bounds,trace);return;}
  const auto g=gateway_layout(bounds.right,bounds.bottom);paint_gateway_backdrop(state,dc,bounds,g);
  skin::set_ui_scale(hud_scale(bounds.bottom));
  const auto layout=service_account_layout(bounds.right,bounds.bottom);const int saved=SaveDC(dc);SetBkMode(dc,TRANSPARENT);
  SelectObject(dc,g.fit<.8?skin::font_heading():skin::font_title());
  RECT heading{g.cx-g.width/2,g.oy+int(231*g.fit),g.cx+g.width/2,g.top};
  inventory_text(dc,heading,"Enter online",RGB(239,199,116),DT_CENTER|DT_SINGLELINE);
  paint_gateway_control(state,dc,layout.panel,"",true);
  const std::string masked(state.account_credential.size(),'*');
  skin::paint_entry(dc,layout.field,masked,"Code or token",state.account_cursor,true);
  const auto status=state.session->connection_state();
  const bool enabled=!state.account_credential.empty() && (status==verdigris::client::ConnectionState::Connected || status==verdigris::client::ConnectionState::Ready);
  paint_gateway_control(state,dc,layout.enroll,"Enroll",PtInRect(&layout.enroll,state.mouse),enabled);
  paint_gateway_control(state,dc,layout.login,"Sign in",PtInRect(&layout.login,state.mouse),enabled);
  paint_gateway_control(state,dc,layout.quit,"Back",PtInRect(&layout.quit,state.mouse));
  SelectObject(dc,skin::font_small());
  RECT error{heading.left,layout.quit.bottom+int(12*g.fit),heading.right,g.oy+int(868*g.fit)};
  const auto& account_error=state.session->model().account_error;
  const std::string message=!account_error.empty()?account_error:
      status==verdigris::client::ConnectionState::Connected?"Connected to service":
      status==verdigris::client::ConnectionState::Ready?"Connected to service":
      !state.session->last_error().empty()?state.session->last_error():verdigris::client::connection_state_label(status);
  inventory_text(dc,error,message,skin::kGold,DT_CENTER|DT_WORDBREAK|DT_END_ELLIPSIS);
  RestoreDC(dc,saved);trace.push_back({render::Op::Hud,0,0,0,0,"service:account-admission"});
  trace.push_back({render::Op::Hud,0,0,0,0,"frontend:illustrated-gateway"});
}
