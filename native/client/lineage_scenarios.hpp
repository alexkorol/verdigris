#pragma once

int scenario_lineage_art() {
  ClientState state;
  scenario_begin(state);
  state.lineage_art=true;
  const std::string dir=art_wave_capture_dir();
  if(dir.empty()) {scenario_check(false,"lineage: contained evidence folder");return scenario_failures;}
  const RasterDirectionalClip headings[]={{"n",0,0,-1},{"ne",0,1,-1},{"e",0,1,0},{"se",0,1,1},
      {"s",0,0,1},{"sw",0,-1,1},{"w",0,-1,0},{"nw",0,-1,-1}};
  raster_art::detail::Surface sheet;
  scenario_check(sheet.create(1280,1152),"lineage: review surface allocated");
  if(!sheet.dc) return scenario_failures;
  for(const char* sex:{"male","female"}) {
    std::fill_n(static_cast<std::uint32_t*>(sheet.pixels),1280*1152,0xff292725u);
    const std::string family=std::string("hero_")+sex;
    state.world.player.appearance=sex;
    scenario_check(std::string(player_raster_family(state))==family,"lineage: saved appearance selects its own family");
    for(int d=0;d<8;++d) {
      const auto& h=headings[d];
      scenario_check(std::string(actor_raster_direction(family.c_str(),h.dx,h.dy))==h.direction,
          "lineage: all eight headings resolve independently");
      for(int pose=0;pose<6;++pose) {
        const double attack=pose>=3?(pose-3+.1)/3.0:-1;
        const double moving=pose==1||pose==2?1:0,walk=pose==2?.75:.25;
        const std::string expected=family+(pose>=3?"_strike"+std::to_string(pose-3):
            pose>0?"_walk"+std::to_string(pose-1):"")+"_"+h.direction;
        const auto size=raster_art::dimensions(expected.c_str());
        scenario_check(size.width==160&&size.height==192,("lineage: common canvas "+expected).c_str());
        scenario_check(raster_art::content_dimensions(expected.c_str()).valid(),("lineage: visible body "+expected).c_str());
        std::string resolved;
        const ScreenPoint at{d*160+80,(pose+1)*192,1};
        const bool drawn=draw_raster_actor(sheet.dc,family.c_str(),at,192,h.dx,h.dy,attack,moving,walk,&resolved);
        scenario_check(drawn&&resolved==expected,("lineage: exact authored pose without idle fallback "+expected).c_str());
        scenario_check(fable_world::actor_pose(family.c_str(),h.dx,h.dy,attack,moving,walk)==expected,
            "lineage: hardware and legacy pose selection agree");
        const auto* attachment=raster_equipment::pose_metadata(expected.c_str());
        scenario_check(attachment&&attachment->canvas.width==160&&attachment->canvas.height==192,
            ("lineage: equipment follows measured pose "+expected).c_str());
        const auto equipment=raster_equipment::compute(expected.c_str(),"weapon_sword",at.x,at.y,192);
        scenario_check(equipment.valid()&&raster_equipment::draw_front(sheet.dc,equipment),
            ("lineage: real sword attachment paints "+expected).c_str());
      }
    }
    scenario_check(save_hbitmap_png(state.billboards,sheet.bitmap,dir+"/lineage-"+sex+"-all.png"),
        "lineage: production raster path captured all48 poses");
  }
  scenario_check(std::string(actor_raster_direction("hero_male",1,.1))=="e"&&
      std::string(actor_raster_direction("hero_female",-.1,-1))=="n","lineage: cardinal sectors tolerate small aim offsets");
  state.camera.perspective=true;scenario_follow_camera(state);
  for(const char* sex:{"male","female"}) {
    // Change the local authority's appearance through the supported constructor;
    // normal paint_scene refreshes its WorldView from that authority.
    state.simulation=std::make_unique<verdigris::Simulation>(7,"Art review",sex);
    state.simulation->dispatch(verdigris::Command::enter("route:tin:1:0"));
    sync_world(state);generate_scenery(state);scenario_follow_camera(state);
    scenario_check(reference_present(state,1366,768,dir+"/lineage-"+sex+"-world.png"),"lineage: production hardware scene captured");
    bool correct=false;
    for(const auto& op:state.render_list) if(op.op==render::Op::Player&&op.label.rfind(std::string("hero_")+sex+"_",0)==0)correct=true;
    scenario_check(correct&&state.camera.perspective,"lineage: hardware scene paints selected appearance");
  }
  return scenario_failures;
}
