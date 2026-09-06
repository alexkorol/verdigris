import io, sys

PATH = "native/client/main.cpp"
text = open(PATH, encoding="utf-8").read()
lines = text.splitlines(keepends=True)

# locate hunks
hunks = []
i = 0
while i < len(lines):
    if lines[i].startswith("<<<<<<<"):
        start = i
        sep = end = None
        for j in range(i, len(lines)):
            if lines[j].startswith("=======") and sep is None:
                sep = j
            if lines[j].startswith(">>>>>>>"):
                end = j
                break
        hunks.append((start, sep, end))
        i = end + 1
    else:
        i += 1
assert len(hunks) == 46, f"expected 46 hunks, got {len(hunks)}"

def head_text(h):
    s, sep, e = h
    return "".join(lines[s+1:sep])

def theirs_text(h):
    s, sep, e = h
    return "".join(lines[sep+1:e])

R = {}  # hunk number (1-based) -> ("union", sep_text) | ("head",) | ("theirs",) | ("custom", text)

for n in (1, 2, 3, 4, 6, 14, 20, 44, 45, 46):
    R[n] = ("union", "")
R[5] = ("union", "}\n\n")
R[43] = ("union", "}\n")
R[7] = ("head",)
R[22] = ("head",)
R[24] = ("head",)

R[8] = ("custom", '''    case SceneryKind::Gate: {
      verdigris::gpu::Bindings bind{};
      (void)verdigris::gpu::load_bindings(verdigris::gpu::Backend::Software,
                                          verdigris::gpu::kBindingLayoutVersion,
                                          &bind);
      const verdigris::gpu::Light light =
          verdigris::gpu::light_from_tick(static_cast<int>(sway_clock * 12.0));
      const std::uint32_t lit =
          verdigris::gpu::shade_texel_lit(bind, 1, 2, light);
      // Merge: nat-recon bronze-stone GPU material, but the aaa direct-portal
      // hover still whitens the whole gate arch for readability.
      vector_art::road_gate(dc, base.x, base.y, h,
                            portal_hovered ? RGB(255, 255, 255)
                                           : verdigris::art::bronze_stone::gdi(lit),
                            portal_hovered ? RGB(255, 255, 255)
                                           : verdigris::art::bronze_stone::gdi_bronze_rim());
      rl.push_back({render::Op::Hud, static_cast<double>(base.x),
                    static_cast<double>(base.y), 0.0, 0, "material:bronze-stone"});
      rl.push_back({render::Op::Hud, static_cast<double>(base.x),
                    static_cast<double>(base.y), 0.0, 0, "material-light:moving"});
''')

# H9: HEAD verbatim, but edge gets theirs' readability floor (max 0.82).
h9 = head_text(hunks[8]).replace(
    "const COLORREF edge = telegraph_color(visibility, edge_source);",
    "const COLORREF edge =\n      telegraph_color(std::max(0.82, visibility), edge_source);")
R[9] = ("custom", h9)

R[10] = ("custom", '''                  telegraph_color(visibility * 0.82,
                                  volley ? RGB(236, 156, 255) : edge_source), 1);
  }
  // Merge (nat-recon): caption chip under the warning, labeled with the real
  // authoritative action rather than a hardcoded "Sweep".
  {
    const COLORREF caption = vector_art::dc_color(dc, RGB(255, 214, 196));
    RECT chip{base.x - 22, base.y + draw_r + 2, base.x + 22,
              base.y + draw_r + 18};
    HBRUSH chip_bg =
        CreateSolidBrush(vector_art::dc_color(dc, RGB(16, 18, 20)));
    FillRect(dc, &chip, chip_bg);
    DeleteObject(chip_bg);
    SetBkMode(dc, TRANSPARENT);
    SetTextColor(dc, caption);
    TextOutA(dc, base.x - 16, base.y + draw_r + 3, telegraph.action.c_str(),
             static_cast<int>(telegraph.action.size()));
''')

R[11] = ("custom", '''    const double length = telegraph.reach > 0
                              ? static_cast<double>(telegraph.reach)
                              : (telegraph.action == "sweep"
                                     ? catalog.melee_range
                                     : catalog.thrust_range);
    if (telegraph.shape != "line" || telegraph.action == "sweep" ||
        telegraph.action == "volley")
      draw_sweep_telegraph(dc, state.camera, bounds, telegraph, visibility,
                           telegraph.action == "sweep"
                               ? length
                               : kTileUnits * telegraph.radius_tiles,
                           minimap_side, rl);
    else
      draw_thrust_telegraph(dc, state.camera, bounds, telegraph, visibility,
                            length, minimap_side, rl);
    const auto spec = verdigris::client::actions::spec_from_payload(
        telegraph.action, telegraph.windup_ticks, catalog);
    rl.push_back({render::Op::Hud, 0.0, 0.0, 0.0, 0,
                  verdigris::client::actions::spec_hud(spec)});
''')

R[12] = ("custom", '''          static_cast<int>(kTileUnits * ((fx.critical || fx.finisher) ? 0.44 : 0.34) *
                           base.scale),
          (fx.critical || fx.finisher) ? 16 : 13,
          (fx.critical || fx.finisher) ? 26 : 22);
      HFONT number_font = cached_damage_font(font_h);
''')

R[13] = ("custom", '''int paint_status_chip(const BillboardAssets* assets,
                      const SpriteBitmap* raster_frame, HDC dc, int x, int y,
                      const std::string& text, COLORREF accent,
                      render::List& rl, const std::string& hud_label = {}) {
''')

R[15] = ("custom", '''      {"pane-seat",
       {seat.left, seat.top, seat.right - seat.left, seat.bottom - seat.top}});
  const bool seat_armed = equipped_bonus != 0;
  skin::slot(dc, seat, seat_armed ? skin::kGold : skin::kVerdigris, seat_armed);
''')

R[16] = ("custom", '''  TextOutA(dc, seat.left + 55 * s, seat_top + 4 * s, loadout_value.c_str(),
           static_cast<int>(loadout_value.size()));
  rl.push_back({render::Op::Hud, 0.0, 0.0, 0.0, 0,
                std::string("held-seat:") + equipped_name});

  // Merge: the nat-recon presentation backpack (VG-UI-002 draggable 4-column
  // grid) is the painted layout. The aaa 12x7 server cell index stays
  // authoritative for keyboard navigation (move_inventory_selection) and the
  // per-item detail data below; both op/trace dialects are emitted so both
  // scenario contracts hold.
  reconcile_pack_grid(state);
  const PackGeom pack = make_pack_geom(static_cast<int>(bounds.right),
                                       static_cast<int>(bounds.bottom));
  const int cell_w = pack.cell_w;
  const int cell_h = pack.cell_h;
  const int grid_left = pack.grid_left;
  const int grid_top = pack.grid_top;
  const int grid_gap = pack.gap;
  const int grid_w = kPackColumns * cell_w + (kPackColumns - 1) * grid_gap;
  const int grid_h = kPackRows * cell_h + (kPackRows - 1) * grid_gap;
  state.hud_rect_trace.push_back(
      {"pane-grid", {grid_left, grid_top, grid_w, grid_h}});
  // On the remote path carried ids are uuids; the model's inventory rows
  // carry the stable item id the art catalog is keyed by.
  const auto art_key = [&](std::size_t index) -> std::string {
    if (!state.session) return items[index].id;
    for (const auto& slot_item : state.session->model().inventory)
      if (slot_item.uuid == items[index].id) return slot_item.id;
    return items[index].id;
''')

# H17: theirs verbatim + inventory_hits injection after the cell rect.
h17 = theirs_text(hunks[16]).replace(
    "      RECT cell{cx, cy, cx + cell_w, cy + cell_h};\n",
    "      RECT cell{cx, cy, cx + cell_w, cy + cell_h};\n"
    "      state.inventory_hits.push_back({cell, items[carried_i].id});\n")
assert "inventory_hits.push_back" in h17
R[17] = ("custom", h17)

# H18: theirs verbatim + aaa footprint trace / tablet seal / stack count, and
# hover-selects-item inside the hover hit test.
h18 = theirs_text(hunks[17])
anchor18 = "      SelectObject(dc, cell_font);\n"
inject18 = anchor18 + '''      // Merge (aaa): diptych footprint trace, tablet seal, stack count.
      if (!equipped)
        state.hud_rect_trace.push_back(
            {"pane-item-footprint", {cx, cy, cell_w, cell_h}});
      if (!billboard && items[carried_i].expedition_map) {
        const std::string seal =
            "T" + std::to_string(items[carried_i].map_tier);
        HGDIOBJ seal_font = SelectObject(dc, skin::font_title());
        SIZE seal_extent{};
        GetTextExtentPoint32A(dc, seal.c_str(), static_cast<int>(seal.size()),
                              &seal_extent);
        SetTextColor(dc, skin::kGold);
        TextOutA(dc, (art_cell.left + art_cell.right - seal_extent.cx) / 2,
                 (art_cell.top + art_cell.bottom - seal_extent.cy) / 2,
                 seal.c_str(), static_cast<int>(seal.size()));
        SelectObject(dc, seal_font);
      }
      if (items[carried_i].quantity > 1) {
        const std::string count = std::to_string(items[carried_i].quantity);
        HGDIOBJ count_font = SelectObject(dc, skin::font_small());
        SIZE count_extent{};
        GetTextExtentPoint32A(dc, count.c_str(), static_cast<int>(count.size()),
                              &count_extent);
        SetTextColor(dc, skin::kGold);
        TextOutA(dc, cell.right - count_extent.cx - 3 * s,
                 cell.bottom - count_extent.cy - 2 * s, count.c_str(),
                 static_cast<int>(count.size()));
        SelectObject(dc, count_font);
      }
'''
assert anchor18 in h18
h18 = h18.replace(anchor18, inject18, 1)
h18hover = '''      if (mx >= cx && mx < cx + cell_w && my >= cy && my < cy + cell_h) {
        hover_i = static_cast<int>(carried_i);
        hover_cx = cx;
        hover_cy = cy;
'''
assert h18hover in h18
h18 = h18.replace(h18hover, h18hover +
                  "        state.selected_item = carried_i;  // merge (aaa): hover selects\n", 1)
R[18] = ("custom", h18)

# H19: HEAD detail cards + both progression string decls; chain stays open into common.
h19_head = head_text(hunks[18])
# HEAD side declares progression/progression_detail mid-way; keep as-is and add owner decl.
h19 = h19_head.replace(
    "  std::string progression;\n  std::string progression_detail;\n",
    "  std::string progression;\n  std::string progression_detail;\n"
    "  std::string owner_progression;\n")
assert "owner_progression" in h19
R[19] = ("custom", h19)

R[21] = ("custom", '''  TextOutA(dc, left + 14 * s, progression_y, progression.c_str(),
           static_cast<int>(progression.size()));
  if (!progression_detail.empty()) {
    rl.push_back(
        {render::Op::PaneStat, 0.0, 0.0, 0.0, 0, progression_detail});
    SIZE extent{};
    GetTextExtentPoint32A(dc, progression_detail.c_str(),
                          static_cast<int>(progression_detail.size()), &extent);
    const int detail_y = bottom - 66 * s;
    state.hud_rect_trace.push_back(
        {"pane-progression-detail",
         {left + 14 * s, detail_y, extent.cx, extent.cy}});
    TextOutA(dc, left + 14 * s, detail_y, progression_detail.c_str(),
             static_cast<int>(progression_detail.size()));
  }
  TextOutA(dc, left + 14 * s, bottom - 74 * s, owner_progression.c_str(),
           static_cast<int>(owner_progression.size()));
  const std::string controls =
      selected && selected->expedition_map
          ? state.world.has_extraction
                ? "Arrows select | Tablet sealed until Crossroads | I close"
                : "Arrows select | V rechart 50g | Enter break | I close"
          : "Arrows select | Enter equip | U unequip | I close";
''')

# H23: theirs wizard-orb wrappers + HEAD framekit orb chrome.
R[23] = ("custom", theirs_text(hunks[22]) + head_text(hunks[22]))

# H25: HEAD minimap op/label + theirs zoom/opacity ops (adapted names) +
# HEAD overlay block + theirs paint_route_card function.
h25_head = head_text(hunks[24])
h25_theirs = theirs_text(hunks[24])
# theirs starts with the "panel" op + zoom/opacity ops; we take HEAD's first
# op (richer label) and theirs' zoom/opacity ops with HEAD-side variable names.
zoom_ops = '''  rl.push_back({render::Op::Hud, static_cast<double>(panel.left),
                static_cast<double>(panel.top),
                minimap_zoom_factor(state.minimap_zoom_step),
                state.minimap_zoom_step,
                "minimap-zoom:" + std::to_string(state.minimap_zoom_step)});
  rl.push_back({render::Op::Hud, static_cast<double>(panel.left),
                static_cast<double>(panel.top),
                static_cast<double>(
                    minimap_body_alpha(state.minimap_opacity_step)),
                state.minimap_opacity_step,
                "map-opacity:" +
                    std::to_string(static_cast<int>(
                        minimap_body_alpha(state.minimap_opacity_step)))});
'''
route_card_fn = h25_theirs[h25_theirs.index("void paint_route_card"):]
R[25] = ("custom", h25_head + zoom_ops + "}\n\n" + route_card_fn)

R[26] = ("custom", '''  keep_out(minimap_rect(width, height, minimap_side));
  // Route card occupies the left column under the minimap. When the gear
  // pane is open at 960, that column is the wrap ladder for controls; hide
  // the card instead of colliding chips into the map.
  if (!gear_open) keep_out(route_card_rect(height));
''')

R[27] = ("custom", '''  const HudRect geometry = character_pane_rect(
      static_cast<int>(bounds.right), static_cast<int>(bounds.bottom));
  const int left = geometry.x;
  const int top = geometry.y;
  const int pane_w = geometry.w;
  const int bottom = geometry.y + geometry.h;
  const int row_h = 26 * s;  // merge (nat-recon): stat rows advance by row_h
  RECT pane{left, top, left + pane_w, bottom};
  if (!draw_framekit_nine(state.billboards, dc, state.billboards.fk_panel, pane))
    skin::panel(dc, pane, skin::kVerdigris, 245, 8.0f);
  dress_owned_pane(state.billboards, dc, pane);
  state.hud_rect_trace.push_back({"character-pane-frame", geometry});
''')

# H28: theirs portrait block (closes its own if), then HEAD doll seats (the
# common "}" closes HEAD's for loop).
R[28] = ("custom", theirs_text(hunks[27]) + "  }\n" + head_text(hunks[27]))

# H29: HEAD stats card (complete its footer TextOut), then theirs stat rows
# (weapon fixed to main_hand), whose final TextOut is completed by common tail.
h29_head = head_text(hunks[28])
h29_theirs = theirs_text(hunks[28])
h29_theirs = h29_theirs.replace('{"Weapon", weapon}',
                                '{"Weapon", main_hand ? main_hand->name : std::string("(unarmed)")}')
assert 'weapon}' not in h29_theirs
R[29] = ("custom", h29_head +
         "           static_cast<int>(strlen(footer)));\n" + h29_theirs)

R[30] = ("custom", '''                          scenery, rl,
                          world.theme == "town" || world.theme == "tin" ||
                              world.route_id.find(":1:") != std::string::npos,
                          state.breathe_phase * 2.0 * kPi, portal_hovered);
        if (portal_hovered)
          rl.push_back(
              {render::Op::Hud, 0, 0, 0, 1, "portal-hover:" + hovered->id});
''')

R[31] = ("custom", '''  if (!(state.gear_overlay && state.character_pane))
    paint_minimap(state, dc, bounds, rl);
  paint_route_card(state, dc, bounds, rl);
  paint_vital_orbs(state.billboards, player, world.tick,
                   state.screen_pulse_ticks, dc, bounds, rl,
                   &state.hud_rect_trace);
''')

R[32] = ("custom", '''        "WASD move | mouse aim | LMB attack | RMB/Space dash | Q E R skills | "
        "X take | Z names | I gear | J journal | T hail | TAB map | click portals";
    const std::string& art_text = state.billboards.framekit_status;
    const bool plates_ready =
        state.billboards.player.ready() && state.billboards.raider.ready() &&
        state.billboards.boss.ready();
    const bool show_art_chip = state.debug_overlay || !plates_ready;
    const bool show_mute_chip =
        state.audio_sink && state.audio_sink->muted();
    const char* mute_text = "audio muted";
    (void)show_art_chip;
''')

R[33] = ("custom", '''    paint_status_chip(&state.billboards, &state.billboards.fk_banner, dc,
                      objective_at.x, objective_at.y, objective_owner, accent,
                      rl, objective);
''')

R[34] = ("custom", '''    const bool plates = state.billboards.fk_panel_ornate.ready() &&
                        state.billboards.fk_orb_life.ready() &&
                        state.billboards.fk_orb_resource.ready();
    const COLORREF art_accent =
        plates ? RGB(120, 214, 168) : RGB(239, 190, 78);
    paint_status_chip(&state.billboards, &state.billboards.fk_button, dc,
                      art_at.x, art_at.y, art_text, art_accent, rl);
    state.hud_rect_trace.push_back(
        {"art", {art_at.x, art_at.y, art_size.w, art_size.h}});
    if (show_mute_chip) {
      const int mute_y = art_at.y + std::max(art_size.h, 20) + 4;
      paint_status_chip(&state.billboards, nullptr, dc, art_at.x, mute_y,
                        mute_text, RGB(238, 226, 197), rl);
      rl.push_back({render::Op::Hud, static_cast<double>(art_at.x),
                    static_cast<double>(mute_y), 0.0, 1, "audio:muted"});
      state.hud_rect_trace.push_back(
          {"audio-muted", {art_at.x, mute_y, 132, 24}});
      paint_audio_mixer_hud(state, dc, art_at.x, mute_y + 28, rl);
    } else if (state.audio_sink) {
      paint_audio_mixer_hud(state, dc, art_at.x, art_at.y, rl);
    }
    if (state.link_lost) {
      const int lost_y = art_at.y + std::max(art_size.h, 20) + 32;
      paint_status_chip(&state.billboards, nullptr, dc, art_at.x, lost_y,
                        "extract uncommitted", RGB(255, 80, 70), rl);
    }
''')

# H35: theirs poll_pad + HEAD front-door condition.
R[35] = ("custom", theirs_text(hunks[34]) + head_text(hunks[34]))

# H36: HEAD minimap-close block, theirs text_entry block, HEAD quest_journal open.
R[36] = ("custom", '''  if (state.minimap_mode == MinimapMode::Overlay) {
    state.minimap_mode = MinimapMode::Corner;
    show_hint(state, "Tactical chart closed");
    return;
  }
  if (state.text_entry) {
    state.text_entry = false;
    return;
  }
  if (state.quest_journal) {
    state.quest_journal = false;
''')

R[37] = ("custom", '''      apply_bound_key_down(*state, wparam);
      if (wparam == 'N' && state->session && state->debug_overlay)
''')

R[38] = ("custom", '''      if (wparam == 'C') {
        if (state->text_entry || trade_pane_open(*state)) break;
        state->minimap_mode = MinimapMode::Corner;
        state->quest_journal = false;
        state->character_pane = !state->character_pane;
      }
      if (wparam == 'B') {
        if (state->text_entry || trade_pane_open(*state) || !state->character_pane)
          break;
        state->stat_atk_expanded = !state->stat_atk_expanded;
        show_hint(*state, state->stat_atk_expanded ? "ATK sources open"
                                                   : "ATK sources closed");
      }
      if (wparam == 'M' && !(GetKeyState(VK_SHIFT) & 0x8000) &&
          state->audio_sink) {
''')

R[39] = ("custom", '''      if (wparam == 'P') {
        if (state->text_entry || trade_pane_open(*state)) break;
        state->minimap_mode = MinimapMode::Corner;
        state->quest_journal = false;
''')

# H40: theirs note_input + HEAD creation/title/chronicles chain into common trade branch.
R[40] = ("custom", '''      if (state) {
        verdigris::client::input::note_input(state->input_latency);
        if (state->startup_creation) {
          const POINT point{GET_X_LPARAM(lparam),GET_Y_LPARAM(lparam)};
          if (PtInRect(&state->creation_back,point)) activate_creation_control(*state,3);
          else if (PtInRect(&state->creation_confirm,point)) activate_creation_control(*state,2);
          else if (state->creation_wait==ClientState::CreationWait::None && PtInRect(&state->creation_input,point)) state->creation_focus=0;
          else if (PtInRect(&state->creation_oath,point)) { state->creation_focus=1; activate_creation_control(*state,1); }
        } else if (state->title_open) {
          const POINT point{GET_X_LPARAM(lparam),GET_Y_LPARAM(lparam)};
          bool menu=false;
          for (size_t i=0;i<state->title_action_hits.size();++i)
            if (PtInRect(&state->title_action_hits[i],point)) {
              activate_title_action(*state,static_cast<int>(i)); menu=true; break;
            }
          if (!menu) { state->title_orbit.previous=point; state->title_orbit.dragging=true; SetCapture(window); }
        } else if (state->screen == Screen::Chronicles) {
          activate_chronicle_at(*state, GET_X_LPARAM(lparam),
                                GET_Y_LPARAM(lparam));
        } else if (trade_pane_open(*state)) {
''')

R[41] = ("custom", '''          const int mx = GET_X_LPARAM(lparam);
          const int my = GET_Y_LPARAM(lparam);
          if (state->rechart_tablet_hit_valid &&
              mx >= state->rechart_tablet_hit.left &&
              mx < state->rechart_tablet_hit.right &&
              my >= state->rechart_tablet_hit.top &&
              my < state->rechart_tablet_hit.bottom) {
            rechart_selected_tablet(*state);
          } else {
            RECT client{};
            GetClientRect(window, &client);
            const PackGeom pack =
                make_pack_geom(static_cast<int>(client.right),
                               static_cast<int>(client.bottom));
            reconcile_pack_grid(*state);
            if (pack_hit_seat(pack, mx, my)) {
              equip_selected(*state);
            } else {
              int gx = -1;
              int gy = -1;
              if (pack_hit_cell(pack, mx, my, gx, gy))
                pack_begin_drag(*state, gx, gy);
              else
                activate_inventory_at(*state, mx, my);
            }
          }
''')

R[42] = ("custom", '''      if (state && state->title_orbit.dragging) { state->title_orbit.dragging=false; ReleaseCapture(); }
      if (state) release_held_gameplay_attack(*state);
      if (state && state->gear_overlay && state->pack_drag_live) {
        RECT client{};
        GetClientRect(window, &client);
        const PackGeom pack = make_pack_geom(static_cast<int>(client.right),
                                             static_cast<int>(client.bottom));
        const int mx = GET_X_LPARAM(lparam);
        const int my = GET_Y_LPARAM(lparam);
        int gx = -1;
        int gy = -1;
        if (pack_hit_cell(pack, mx, my, gx, gy)) {
          state->pack_preview_x = gx;
          state->pack_preview_y = gy;
          state->pack_preview_ok = pack_can_land(
              state->pack_grid, state->pack_drag_id, gx, gy);
        } else {
          state->pack_preview_ok = false;
        }
        pack_commit_drop(*state, pack_hit_seat(pack, mx, my));
      }
      break;
    case WM_CAPTURECHANGED:
    case WM_KILLFOCUS:
      if (state) { state->title_orbit.dragging=false; state->w=state->a=state->s=state->d=false; }
      break;
    case WM_RBUTTONDOWN:
      if (state) {
        verdigris::client::input::note_input(state->input_latency);
        if (state->title_open || state->startup_creation) break;
        RECT click_bounds;
        GetClientRect(window, &click_bounds);
        if (!pointer_ui_blocks_world(*state, click_bounds,
                                     GET_X_LPARAM(lparam), GET_Y_LPARAM(lparam)))
          dispatch_dash(*state);
      }
      break;
    case WM_SIZE:
      if (state) {
        state->creation_input=state->creation_oath=state->creation_confirm=state->creation_back=RECT{};
        state->title_action_hits.clear();
        state->inventory_hits.clear();
        state->rechart_tablet_hit_valid = false;
''')

# apply resolutions (from last hunk to first to keep indices valid)
out = lines[:]
for n in range(46, 0, -1):
    s, sep, e = hunks[n-1]
    mode = R[n][0]
    if mode == "union":
        repl = "".join(out[s+1:sep]) + R[n][1] + "".join(out[sep+1:e])
    elif mode == "head":
        repl = "".join(out[s+1:sep])
    elif mode == "theirs":
        repl = "".join(out[sep+1:e])
    else:
        repl = R[n][1]
    out[s:e+1] = [repl]

text2 = "".join(out)

# E1: delete the clean-merged aaa 12x7 BackpackDraw footprint loop (superseded
# by the pack-grid painter; its semantics are re-emitted from that loop).
e1 = '''  std::vector<BackpackDraw> backpack_draws;
  backpack_draws.reserve(items.size());
  for (std::size_t i = 0; i < items.size(); ++i) {
    if (items[i].equipped) continue;
    int slot = items[i].inventory_slot;
    if (slot < 0 || slot >= kGridColumns * kGridRows)
      slot = static_cast<int>(i % (kGridColumns * kGridRows));
    const int col = slot % kGridColumns;
    const int row = slot / kGridColumns;
    const int span_w = std::clamp(items[i].width, 1, kGridColumns - col);
    const int span_h = std::clamp(items[i].height, 1, kGridRows - row);
    RECT footprint{
        grid_left + col * (cell_w + grid_gap),
        grid_top + row * (cell_h + grid_gap),
        grid_left + col * (cell_w + grid_gap) + span_w * cell_w +
            (span_w - 1) * grid_gap,
        grid_top + row * (cell_h + grid_gap) + span_h * cell_h +
            (span_h - 1) * grid_gap};
    backpack_draws.push_back({i, footprint});
    state.inventory_hits.push_back({footprint, items[i].id});
    if (state.mouse.x >= footprint.left && state.mouse.x < footprint.right &&
        state.mouse.y >= footprint.top && state.mouse.y < footprint.bottom)
      state.selected_item = i;
  }
'''
assert e1 in text2, "E1 anchor not found"
text2 = text2.replace(e1, "", 1)

# E2: tooltip tail gains theirs' shape/contrast ops.
e2anchor = '''  rl.push_back({render::Op::Hud, static_cast<double>(box_x),
                static_cast<double>(box_y), 0.0, 0, "tooltip:" + title});
}

// A thin bottom-edge strip'''
e2new = '''  rl.push_back({render::Op::Hud, static_cast<double>(box_x),
                static_cast<double>(box_y), 0.0, 0, "tooltip:" + title});
  // Merge (nat-recon VG-UI-007): tooltip certification ops.
  rl.push_back({render::Op::Hud, static_cast<double>(box_x),
                static_cast<double>(box_y), 0.0, 0, "tooltip-shape:foe"});
  if (skin::contrast_ratio(skin::kInk, skin::kPanelMid) >= 4.5)
    rl.push_back({render::Op::Hud, static_cast<double>(box_w),
                  static_cast<double>(box_y), 0.0, 0, "tooltip-contrast:ok"});
}

// A thin bottom-edge strip'''
assert e2anchor in text2, "E2 anchor not found"
text2 = text2.replace(e2anchor, e2new, 1)

open(PATH, "w", encoding="utf-8", newline="").write(text2)
print("resolutions applied")
