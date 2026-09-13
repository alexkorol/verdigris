// Current owner contract replaces checks demanding a permanent developer HUD.
int scenario_native_ui_layout(bool with_inventory) {
  ClientState state;scenario_begin(state);state.camera.perspective=true;state.lineage_art=true;
  state.gear_overlay=with_inventory;state.debug_overlay=false;state.hint_ticks=0;
  const auto dir=art_wave_capture_dir();
  for(const auto size:{std::pair{960,600},std::pair{1280,800},std::pair{1366,768},std::pair{3440,1440}}) {
    for(bool sheet:{false,true}) {
      state.character_pane=with_inventory&&sheet;
      state.camera.zoom=kCameraDefaultZoom*zoom_height_factor(size.second);
      scenario_check(reference_present(state,size.first,size.second,dir+"/ui-"+
          (with_inventory?"inventory-":"play-")+std::to_string(size.first)+(sheet?"-sheet":"")+".png"),
          "native-ui: production perspective frame captured");
      auto rect=[&](const char* key)->const HudRect* {
        for(const auto& entry:state.hud_rect_trace)if(entry.first==key)return &entry.second;return nullptr;
      };
      for(const char* label:{"controls","controls-second","identity","audio-mixer","art"})
        scenario_check(rect(label)==nullptr,"native-ui: ordinary play excludes permanent diagnostic/instruction blocks");
      const auto* gear=rect("pane-frame");const auto* character=rect("character-pane-frame");
      if(with_inventory) {
        scenario_check(gear!=nullptr,"native-ui: actual gear frame exists");
        const auto g=make_pack_geom(size.first,size.second);
        scenario_check(g.cell_w==g.cell_h && g.cell_w>=20,"native-ui: all 84 cells are square and usable");
        scenario_check(g.grid_top>g.seats[8].bottom,"native-ui: backpack follows equipment");
        scenario_check(g.seat.bottom-g.seat.top>2*(g.seats[9].bottom-g.seats[9].top),
            "native-ui: weapon region dominates a ring region");
        int cells=0;
        for(const auto& entry:state.hud_rect_trace) {
          if(entry.first=="pane-backpack-cell")++cells;
          if(entry.first=="pane-doll-slot" || entry.first=="pane-backpack-cell") {
            const auto& r=entry.second;
            scenario_check(gear && r.x>=gear->x && r.y>=gear->y && r.x+r.w<=gear->x+gear->w && r.y+r.h<=gear->y+gear->h,
                "native-ui: shared paint/hit/drop rectangles stay inside the owner frame");
          }
        }
        scenario_check(cells==84,"native-ui: authoritative capacity unchanged");
        for(std::size_t i=0;i<g.seats.size();++i)for(std::size_t j=i+1;j<g.seats.size();++j) {
          RECT overlap{};scenario_check(!IntersectRect(&overlap,&g.seats[i],&g.seats[j]),"native-ui: equipment regions do not overlap");
        }
        if(sheet)scenario_check(character && gear && !hud_rects_overlap(*gear,*character),"native-ui: character and inventory fit together");
        for(const char* label:{"orb-life","orb-resource","quickbar-strip","xp-meter"}) {
          const auto* r=rect(label);
          if(r && gear)scenario_check(!hud_rects_overlap(*r,*gear),"native-ui: equipment clears combat controls");
        }
      }
    }
  }
  return scenario_failures;
}

int scenario_authoritative_stat_details() {
  ClientState state;scenario_begin(state);state.camera.perspective=true;state.character_pane=true;
  state.sheet_passive_atk=999;state.sheet_cond_atk=999;state.sheet_cond_active=true;
  scenario_present(state);
  const auto expect=std::to_string(state.world.player.attack+state.world.player.gear_attack);
  scenario_check(render_list_has(state,render::Op::Hud,"char:Attack:"+expect),"stat-details: client fixture guesses cannot change authoritative rating");
  scenario_check(!render_list_has(state,render::Op::Hud,"char:Cond:"),"stat-details: no unimplemented conditional row");
  scenario_check(!render_list_has(state,render::Op::Hud,"char:Base attack:"),"stat-details: calculations are collapsed by default");
  state.stat_atk_expanded=true;scenario_present(state);
  scenario_check(render_list_has(state,render::Op::Hud,"char:Base attack:"+std::to_string(state.world.player.attack)),"stat-details: expanded base matches authority");
  scenario_check(render_list_has(state,render::Op::Hud,"char:Equipment:"+std::to_string(state.world.player.gear_attack)),"stat-details: expanded equipment matches authority");
  return scenario_failures;
}
