// Production grip, occlusion, and cache regression checks. Run test_equipment.ps1.
#ifdef NDEBUG
#error Raster equipment checks require active assertions; compile with /UNDEBUG.
#endif
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include "../../client/raster_equipment.hpp"
#include <cassert>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <set>
#include <vector>

void save(raster_art::detail::Surface& target,const wchar_t* path) {
 GdiFlush();Gdiplus::Bitmap bitmap(target.bitmap,nullptr);
 const CLSID png{0x557cf406,0x1a04,0x11d3,{0x9a,0x73,0x00,0x00,0xf8,0x1e,0xf3,0x2e}};
 assert(bitmap.Save(path,&png,nullptr)==Gdiplus::Ok);
}

void contact_sheet(const std::vector<const raster_equipment::PoseMetadata*>& poses,
                   std::size_t begin, std::size_t end, int scale,
                   const std::filesystem::path& path) {
  int cell_w = 96;
  int cell_h = 112;
  RECT extent{};
  for (std::size_t index = begin; index < end; ++index) {
    for (const auto& weapon : raster_equipment::detail::kWeapons) {
      const auto p = raster_equipment::compute(poses[index]->name, weapon.name,
                                               0, 0, poses[index]->canvas.height);
      assert(p.valid());
      UnionRect(&extent, &extent, &p.actor_bounds);
      UnionRect(&extent, &extent, &p.weapon_bounds);
    }
  }
  cell_w = std::max(cell_w, static_cast<int>(extent.right - extent.left) + 16);
  cell_h = std::max(cell_h, static_cast<int>(extent.bottom - extent.top) + 24);
  const int columns = static_cast<int>(std::size(raster_equipment::detail::kWeapons));
  raster_art::detail::Surface target;
  assert(target.create(cell_w * scale * columns,
                       cell_h * scale * static_cast<int>(end - begin)));
  HBRUSH bg = CreateSolidBrush(RGB(54, 48, 42));
  RECT area{0, 0, target.width, target.height};
  FillRect(target.dc, &area, bg);
  DeleteObject(bg);
  for (std::size_t index = begin; index < end; ++index) {
    const auto& pose = *poses[index];
    for (int col = 0; col < columns; ++col) {
      const auto& weapon = raster_equipment::detail::kWeapons[col];
      const int ox = col * cell_w * scale;
      const int oy = static_cast<int>(index - begin) * cell_h * scale;
      const int center_x = ox + (8 - extent.left) * scale;
      const int feet_y = oy + (8 - extent.top) * scale;
      assert(raster_art::draw_sprite(target.dc, pose.name, center_x, feet_y,
                                     pose.canvas.height * scale));
      assert(raster_equipment::draw_front(target.dc, pose.name, weapon.name,
                                          center_x, feet_y, pose.canvas.height * scale));
      if (scale > 1) {
        std::string label = std::string(pose.name) + " / " + weapon.name;
        SetTextColor(target.dc, RGB(235, 223, 204));
        SetBkMode(target.dc, TRANSPARENT);
        TextOutA(target.dc, ox + 4, oy + cell_h * scale - 18,
                  label.c_str(), static_cast<int>(label.size()));
      }
    }
  }
  save(target, path.c_str());
}

void check_rotated_sampling() {
  // Unique source texels, including transparent and partial-alpha pixels, make
  // interpolation, wrap/clamp, mirror, and angle mistakes observable.
  raster_art::set_asset_root(L".ci-artifacts/raster-equipment/sampling");
  Gdiplus::Bitmap fixture(5, 7, PixelFormat32bppARGB);
  for (int y = 0; y < 7; ++y) for (int x = 0; x < 5; ++x)
    assert(fixture.SetPixel(x, y, Gdiplus::Color(
        x == 0 && y == 0 ? 0 : x == 2 ? 128 : 255,
        static_cast<BYTE>(20 + x * 36), static_cast<BYTE>(10 + y * 28),
        static_cast<BYTE>(30 + x + y))) == Gdiplus::Ok);
  const CLSID png{0x557cf406,0x1a04,0x11d3,{0x9a,0x73,0x00,0x00,0xf8,0x1e,0xf3,0x2e}};
  assert(fixture.Save(L".ci-artifacts/raster-equipment/sampling/rotation.png", &png) == Gdiplus::Ok);
  auto* source = raster_art::detail::asset("rotation");
  assert(source);
  std::uint32_t texels[35]{};
  Gdiplus::BitmapData data{};
  Gdiplus::Rect rect(0, 0, 5, 7);
  assert(source->image->LockBits(&rect, Gdiplus::ImageLockModeRead,
                                 PixelFormat32bppPARGB, &data) == Gdiplus::Ok);
  for (int y = 0; y < 7; ++y)
    std::memcpy(texels + y * 5, static_cast<const BYTE*>(data.Scan0) + y * data.Stride, 20);
  source->image->UnlockBits(&data);
  std::set<std::uint64_t> hashes;
  bool saw_partial = false, saw_transparent = false;
  for (int height : {7, 14, 23}) for (bool flip : {false, true})
    for (int angle : {-65, 15, 35, 90, 130, 270, 360}) {
      const int width = static_cast<int>(std::lround(height * 5.0 / 7.0));
      auto* image = raster_art::detail::scaled(*source, width, height, flip, angle);
      assert(image);
      // This independent oracle evaluates source coordinates in the original
      // canvas; no intermediate image or palette interpolation is permitted.
      const int a = (angle % 360 + 360) % 360;
      double c = std::cos(a * 3.14159265358979323846 / 180.0);
      double s = std::sin(a * 3.14159265358979323846 / 180.0);
      if (a % 90 == 0) { c = std::round(c); s = std::round(s); }
      const double xs[]{0, c * width, -s * height, c * width - s * height};
      const double ys[]{0, s * width, c * height, s * width + c * height};
      const double left = std::floor(*std::min_element(xs, xs + 4));
      const double top = std::floor(*std::min_element(ys, ys + 4));
      const auto* pixels = static_cast<const std::uint32_t*>(image->pixels);
      GdiFlush();
      std::uint64_t hash = 1469598103934665603ull;
      for (int y = 0; y < image->height; ++y) for (int x = 0; x < image->width; ++x) {
        const double px = x + .5 + left, py = y + .5 + top;
        const double u = (c * px + s * py) / width;
        const double v = (-s * px + c * py) / height;
        std::uint32_t expected = 0;
        if (u >= 0 && u < 1 && v >= 0 && v < 1) {
          int sx = static_cast<int>(u * 5);
          if (flip) sx = 4 - sx;
          expected = texels[static_cast<int>(v * 7) * 5 + sx];
        }
        assert(pixels[y * image->width + x] == expected);
        const auto alpha = expected >> 24;
        assert(alpha == 0 || alpha == 128 || alpha == 255);
        saw_partial |= alpha == 128; saw_transparent |= alpha == 0;
        hash = (hash ^ expected) * 1099511628211ull;
      }
      if (height == 14 && !flip) assert(hashes.insert(hash).second);
      const auto count = raster_art::cache_stats().scale_builds;
      assert(image == raster_art::detail::scaled(*source, width, height, flip, angle + 360));
      assert(count == raster_art::cache_stats().scale_builds);
    }
  assert(saw_partial && saw_transparent);
  raster_art::detail::Surface target;
  assert(target.create(64, 64));
  for (int angle : {-65, 0, 15, 35, 90, 130, 180, 270}) for (bool flip : {false, true}) {
    std::memset(target.pixels, 0, target.bytes());
    assert(raster_art::draw_sprite_at_anchor(target.dc, "rotation", 32.25, 31.75,
                                             21, 2.5, 3.5, angle, flip));
    GdiFlush();
    // At 3x the pixel centered nearest the requested hand must still be the
    // uniquely colored grip texel, including its unchanged premultiplied alpha.
    assert(static_cast<const std::uint32_t*>(target.pixels)[31 * 64 + 32] == texels[3 * 5 + 2]);
  }
  // Exercise actual eviction, not just a below-limit cache snapshot.
  for (int angle = 0; angle < 360; ++angle)
    assert(raster_art::detail::scaled(*source, 10, 14, false, angle));
  auto stats = raster_art::cache_stats();
  assert(stats.scaled_bitmaps == 192 && stats.scaled_bytes <= 64u * 1024u * 1024u);
  const double nan = std::numeric_limits<double>::quiet_NaN();
  assert(!raster_art::sprite_transform("rotation", 0, 2.5, 3.5).valid());
  assert(!raster_art::sprite_transform("rotation", 2049, 2.5, 3.5).valid());
  assert(!raster_art::sprite_transform("rotation", 14, nan, 3.5).valid());
  assert(!raster_art::sprite_transform("rotation", 14, -1, 3.5).valid());
  assert(!raster_art::sprite_transform("rotation", 14, 6, 3.5).valid());
  assert(!raster_art::sprite_transform("missing", 14, 2.5, 3.5).valid());
  assert(!raster_art::detail::scaled(*source, 2048, 2048, false, 45));
  const auto t = raster_art::sprite_transform("rotation", 14, 2.5, 3.5, 90);
  assert(t.canvas.width == 14 && t.canvas.height == 10);
  assert(t.anchor_x == 7 && t.anchor_y == 5);
  RECT bounds{};
  assert(!raster_art::anchored_bounds(t, nan, 0, bounds));
  assert(!raster_art::anchored_bounds(t, 1e30, 0, bounds));
  assert(!raster_art::draw_sprite_at_anchor(nullptr, "rotation", 0, 0, 14, 2.5, 3.5, 90));
  assert(!raster_art::draw_sprite_at_anchor(target.dc, "rotation", 0, 0, 14, 2.5, 3.5,
                                           90, false, std::numeric_limits<float>::quiet_NaN()));
  const auto draws = raster_art::cache_stats().draws;
  assert(raster_art::draw_sprite_at_anchor(target.dc, "rotation", 32, 32, 14, 2.5, 3.5,
                                          90, false, 0.0f));
  assert(draws == raster_art::cache_stats().draws);
}

// A new runtime pose must not silently disappear from attachment tests merely
// because nobody added it to the metadata table or to a fixed preview list.
void check_pose_coverage() {
  std::set<std::string> registered;
  for (const auto& pose : raster_equipment::detail::kPoses) {
    assert(registered.insert(pose.name).second);
    assert(pose.canvas.valid());
    assert(pose.hand.x >= pose.fingers.left && pose.hand.x < pose.fingers.right);
    assert(pose.hand.y >= pose.fingers.top && pose.hand.y < pose.fingers.bottom);
    assert(pose.fingers.left >= 0 && pose.fingers.top >= 0);
    assert(pose.fingers.right <= pose.canvas.width &&
           pose.fingers.bottom <= pose.canvas.height);
    if (!raster_art::available(pose.name)) {
      std::fprintf(stderr, "Registered equipment pose has no runtime PNG: %s\n", pose.name);
      std::exit(1);
    }
  }
  for (const auto& entry : std::filesystem::directory_iterator(raster_art::asset_root())) {
    if (!entry.is_regular_file() || entry.path().extension() != ".png") continue;
    const std::string name = entry.path().stem().string();
    if (!name.starts_with("hero_")) continue;
    if (!registered.contains(name)) {
      std::fprintf(stderr, "Runtime hero pose lacks measured equipment metadata: %s\n",
                    name.c_str());
      std::exit(1);
    }
  }
}

void check_strike_socket_pixels() {
  // Independent, visually reviewed V3 hand neighborhoods. A future asset
  // replacement must prompt remeasurement, even if canvas and filenames match.
  // Hash decoded ARGB pixels (transparent RGB normalized by raster_art), not PNG
  // compression bytes. These rectangles deliberately extend beyond replay clips.
  const RECT neighborhoods[]{{43,49,52,58},{46,53,54,60},{56,46,63,53},
                             {65,46,72,54},{51,51,58,59},{41,48,50,57}};
  const std::uint64_t expected[]{0xa4c0346e1872209full,0x335e082e55266c66ull,
      0x1703f63b9551320bull,0xdca18d0b320ced59ull,0x185b0f8b13c2df94ull,
      0xaeec624371162baeull};
  std::ofstream csv(L".ci-artifacts/raster-equipment/strike-sockets-v3.csv");
  assert(csv.good());
  csv << "pose,hand_x,hand_y,fingers_left,fingers_top,fingers_right,fingers_bottom,clockwise_degrees,hand_argb_fnv1a64\n";
  for (int frame = 0; frame < 6; ++frame) {
    const std::string name = "hero_strike" + std::to_string(frame) + "_se";
    const auto* pose = raster_equipment::pose_metadata(name.c_str());
    auto* asset = raster_art::detail::asset(name.c_str());
    assert(pose && asset);
    const RECT& box = neighborhoods[frame];
    std::uint64_t hash = 14695981039346656037ull;
    for (int y = box.top; y < box.bottom; ++y) for (int x = box.left; x < box.right; ++x) {
      Gdiplus::Color pixel;
      assert(asset->image->GetPixel(x,y,&pixel) == Gdiplus::Ok);
      hash = (hash ^ pixel.GetValue()) * 1099511628211ull;
    }
    if (hash != expected[frame]) {
      std::fprintf(stderr,"%s hand pixels changed: remeasure sockets and review before updating V3 fingerprint.\n",name.c_str());
      std::exit(1);
    }
    Gdiplus::Color grip;
    assert(asset->image->GetPixel(static_cast<int>(pose->hand.x),
                                  static_cast<int>(pose->hand.y),&grip) == Gdiplus::Ok);
    assert(grip.GetAlpha() == 255 && grip.GetRed() >= 200 &&
           grip.GetRed() > grip.GetGreen() && grip.GetGreen() > grip.GetBlue());
    csv << name << ',' << pose->hand.x << ',' << pose->hand.y << ',' << pose->fingers.left << ','
        << pose->fingers.top << ',' << pose->fingers.right << ',' << pose->fingers.bottom << ','
        << pose->melee_clockwise_degrees << ',' << std::hex << hash << std::dec << '\n';
  }
  std::puts("PASS: all six V3 hand-region fingerprints and measured palm texels match the reviewed sockets.");
}

void check_directional_socket_pixels() {
  // Reviewed source neighborhoods around the anatomical-left hand. Protect
  // against replacing a same-size motion PNG without remeasuring its grip.
  struct Sample { const char* name; RECT box; std::uint64_t hash; };
  const Sample samples[]{
    {"hero_walk0_sw",{41,63,49,71},0x2dd015b76b412939ull},
    {"hero_walk1_sw",{42,62,51,71},0xdab004ca355358d4ull},
    {"hero_walk2_sw",{43,61,52,70},0x1d05672ba7e4d004ull},
    {"hero_walk3_sw",{47,61,56,69},0x4e817535e70c7183ull},
    {"hero_walk4_sw",{43,62,52,70},0xd0be7f047b8f524full},
    {"hero_walk5_sw",{44,62,52,70},0x36c9bbe39c5622d3ull},
    {"hero_walk6_sw",{40,62,49,71},0xef28e592dad25bc4ull},
    {"hero_walk7_sw",{39,61,48,70},0xc37d16d6e7f676bcull},
    // Only registration changed: sample the same hand pixels at +1y/+5y.
    // The unchanged hashes deliberately prove the hand artwork was preserved.
    {"hero_walk0_nw",{23,58,30,65},0xb0ef7180da7cd6c8ull},
    {"hero_walk1_nw",{21,57,28,65},0xe55b8467c336d88cull},
    {"hero_walk2_nw",{21,60,29,67},0x118dff2839e33297ull},
    {"hero_walk3_nw",{20,58,27,65},0x7ea4e7b82bd5a153ull},
    {"hero_walk4_nw",{22,60,29,67},0x74b5cf5e85ed5334ull},
    {"hero_walk5_nw",{25,60,31,68},0xce973caf2134059cull},
    {"hero_walk6_nw",{22,60,29,67},0x123fa2a2572d53fcull},
    {"hero_walk7_nw",{23,60,29,67},0xfc0845b0fe11a06cull},
    {"hero_walk0_ne",{30,57,36,64},0x1e750cbcc641f0bfull},
    {"hero_walk1_ne",{29,57,36,65},0x31e97d580284060aull},
    {"hero_walk2_ne",{27,56,35,65},0x2fc275d34732f02aull},
    {"hero_walk3_ne",{28,57,36,65},0x6cb6377b2d67a5a4ull},
    {"hero_walk4_ne",{29,58,35,65},0x1d0d3f718573a35aull},
    {"hero_walk5_ne",{28,57,36,65},0xc496d620c633cef8ull},
    {"hero_walk6_ne",{30,57,36,64},0x4474df9febda6c80ull},
    {"hero_walk7_ne",{31,57,37,64},0x2b95729fd809e8a7ull},
    {"hero_strike0_nw",{23,48,30,55},0x86a04c0467ad2866ull},
    {"hero_strike1_nw",{18,43,26,51},0x23b2a7a51a324502ull},
    {"hero_strike2_nw",{11,40,19,48},0x1c4bb2b9737d2b2aull},
    {"hero_strike3_nw",{10,35,18,44},0xe8485c05525de41bull},
    {"hero_strike4_nw",{21,46,29,54},0xa60076b9d07c6ce4ull},
    {"hero_strike5_nw",{23,47,30,56},0xea6098c0f302348eull},
    {"hero_strike0_sw",{53,58,61,66},0xd1f6266fbd0899b4ull},
    {"hero_strike1_sw",{60,45,69,53},0x3ae18b0d76dafef8ull},
    {"hero_strike2_sw",{25,67,35,75},0x16b161cdd6dbb3d7ull},
    {"hero_strike3_sw",{17,67,27,76},0xdb2cf7dab2459227ull},
    {"hero_strike4_sw",{31,58,41,67},0x3d57b906e5927875ull},
    {"hero_strike5_sw",{49,57,58,65},0x8588167ff8c29e6aull},
  };
  std::ofstream csv(L".ci-artifacts/raster-equipment/walk-sockets.csv");
  std::ofstream strikes(L".ci-artifacts/raster-equipment/directional-strike-sockets.csv");
  assert(csv.good() && strikes.good());
  const char* columns="pose,hand_x,hand_y,fingers_left,fingers_top,fingers_right,fingers_bottom,faces_left,behind_actor,clockwise_degrees,hand_argb_fnv1a64\n";
  csv << columns; strikes << columns;
  for (const auto& sample : samples) {
    const auto* pose = raster_equipment::pose_metadata(sample.name);
    auto* asset = raster_art::detail::asset(sample.name);
    assert(pose && asset);
    std::uint64_t hash = 14695981039346656037ull;
    for (int y = sample.box.top; y < sample.box.bottom; ++y)
      for (int x = sample.box.left; x < sample.box.right; ++x) {
        Gdiplus::Color pixel;
        assert(asset->image->GetPixel(x,y,&pixel) == Gdiplus::Ok);
        hash = (hash ^ pixel.GetValue()) * 1099511628211ull;
      }
    if (hash != sample.hash) {
      std::fprintf(stderr,"%s hand pixels changed: remeasure and visually review its equipment grip.\n",sample.name);
      std::exit(1);
    }
    Gdiplus::Color grip;
    assert(asset->image->GetPixel(static_cast<int>(pose->hand.x),
                                  static_cast<int>(pose->hand.y),&grip) == Gdiplus::Ok);
    // NE7's partly hidden hand uses the darker (191,134,86) skin ramp.
    assert(grip.GetAlpha() == 255 && grip.GetRed() >= 180 &&
           grip.GetRed() > grip.GetGreen() && grip.GetGreen() > grip.GetBlue());
    auto& report=std::string_view(pose->name).starts_with("hero_walk") ? csv : strikes;
    report << pose->name << ',' << pose->hand.x << ',' << pose->hand.y << ','
        << pose->fingers.left << ',' << pose->fingers.top << ','
        << pose->fingers.right << ',' << pose->fingers.bottom << ','
        << pose->faces_left << ',' << pose->behind_actor << ',' << pose->melee_clockwise_degrees << ','
        << std::hex << hash << std::dec << '\n';
  }
  std::printf("PASS: %zu reviewed directional hand neighborhoods and palm texels match their measured sockets.\n",std::size(samples));
}

void write_contact_sheets() {
  std::map<std::string, std::vector<const raster_equipment::PoseMetadata*>> groups;
  for (const auto& pose : raster_equipment::detail::kPoses) {
    const std::string_view name(pose.name);
    std::string group = "idle";
    if (name.starts_with("hero_walk"))
      group = "walk-" + std::string(name.substr(name.find_last_of('_') + 1));
    else if (name.starts_with("hero_strike"))
      group = "strike-" + std::string(name.substr(name.find_last_of('_') + 1));
    else if (name.starts_with("hero_attack"))
      group = "attack";
    groups[group].push_back(&pose);
  }
  const std::filesystem::path directory = L".ci-artifacts/raster-equipment";
  std::ofstream index(directory / "equipment-coverage.txt");
  assert(index.good());
  for (auto& [group, poses] : groups) {
    std::sort(poses.begin(), poses.end(), [](const auto* a, const auto* b) {
      return std::string_view(a->name) < std::string_view(b->name);
    });
    for (std::size_t begin = 0; begin < poses.size(); begin += 4) {
      const std::size_t end = std::min(begin + 4, poses.size());
      const std::string prefix = group + "-" + std::to_string(begin / 4) + "-composites-";
      contact_sheet(poses, begin, end, 1, directory / (prefix + "native.png"));
      contact_sheet(poses, begin, end, 3, directory / (prefix + "3x.png"));
      index << prefix << "{native,3x}.png\n";
      for (std::size_t row = begin; row < end; ++row)
        index << "  " << poses[row]->name
              << (poses[row]->transitional ? " (transitional)" : "") << '\n';
    }
  }
}

void write_walk_animation() {
  const std::filesystem::path directory = L".ci-artifacts/raster-equipment";
  for (const char* facing : {"sw", "nw", "ne"}) {
    std::vector<const raster_equipment::PoseMetadata*> poses;
    for (int frame = 0; frame < 8; ++frame) {
      const std::string name = "hero_walk" + std::to_string(frame) + "_" + facing;
      if (const auto* pose = raster_equipment::pose_metadata(name.c_str())) poses.push_back(pose);
    }
    if (poses.empty()) continue;
    assert(poses.size() == 8);
    // A separate single-frame contact_sheet would derive per-frame extents.
    // Keep one fixed logical 128x120 cell and pivot per weapon for playback.
    for (int frame = 0; frame < 8; ++frame) for (int scale : {1,3}) {
      raster_art::detail::Surface target;
      assert(target.create(128*5*scale,120*scale));
      HBRUSH bg = CreateSolidBrush(RGB(54,48,42));
      RECT area{0,0,target.width,target.height}; FillRect(target.dc,&area,bg); DeleteObject(bg);
      int col = 0;
      for (const auto& weapon : raster_equipment::detail::kWeapons) {
        const int center=(col*128+64)*scale, feet=104*scale;
        const auto p=raster_equipment::compute(poses[frame]->name,weapon.name,center,feet,96*scale);
        assert(p.valid());
        assert(p.weapon_bounds.left >= col*128*scale && p.weapon_bounds.right <= (col+1)*128*scale);
        assert(p.weapon_bounds.top >= 0 && p.weapon_bounds.bottom <= 120*scale);
        assert(raster_art::draw_sprite(target.dc,poses[frame]->name,center,feet,96*scale));
        assert(raster_equipment::draw_front(target.dc,p));
        ++col;
      }
      const std::string filename=std::string("walk-")+facing+"-action-frame"+std::to_string(frame)+"-"+std::to_string(scale)+"x.png";
      save(target,(directory/filename).c_str());
    }
    std::ofstream html(directory/(std::string("walk-")+facing+"-action-review.html"));
    assert(html.good());
    html << R"HTML(<!doctype html><meta charset="utf-8"><title>Walk equipment review</title>
<style>body{background:#26221e;color:#eadfcc;font:16px system-ui}img{display:block;image-rendering:pixelated}button,input{margin:8px}</style>
<p>Fixed pivot, actual production draw. Axe, sword, staff, bow, club. 100ms per pose is review timing only.</p>
<button id="play">Pause</button><label>Frame <input id="phase" type="range" min="0" max="7" value="0"></label><span id="label"></span>
<img id="native" width="640" height="120"><img id="large" width="1920" height="360">
<script>const facing=')HTML" << facing << R"HTML(';let cursor=0,playing=true,last=0;
function show(i){native.src=`walk-${facing}-action-frame${i}-1x.png`;large.src=`walk-${facing}-action-frame${i}-3x.png`;phase.value=i;label.textContent=`${facing} F${i}`;}
play.onclick=()=>{playing=!playing;play.textContent=playing?'Pause':'Play';last=0;};
phase.oninput=()=>{playing=false;play.textContent='Play';cursor=+phase.value;show(cursor);};
function tick(now){if(playing&&now-last>100){cursor=(cursor+1)%8;show(cursor);last=now;}requestAnimationFrame(tick);}show(0);requestAnimationFrame(tick);</script>)HTML";
  }
}

void write_directional_strike_animation() {
  const std::filesystem::path directory=L".ci-artifacts/raster-equipment";
  const char* weapons[]{"weapon_axe","weapon_sword","weapon_club"};
  for (const char* facing : {"nw","sw","ne"}) {
    const std::string first=std::string("hero_strike0_")+facing;
    if (!raster_equipment::pose_metadata(first.c_str())) continue;
    const std::string prefix=std::string("strike-")+facing+"-action";
    for (int frame=0;frame<7;++frame) {
      const std::string pose=frame==6?std::string("hero_")+facing:
          "hero_strike"+std::to_string(frame)+"_"+facing;
      raster_art::detail::Surface target;
      assert(target.create(224*3*3,176*3));
      HBRUSH bg=CreateSolidBrush(RGB(54,48,42));
      RECT area{0,0,target.width,target.height};FillRect(target.dc,&area,bg);DeleteObject(bg);
      for (int col=0;col<3;++col) {
        const int center=(col*224+112)*3,feet=144*3;
        const auto p=raster_equipment::compute(pose.c_str(),weapons[col],center,feet,288);
        assert(p.valid());
        assert(p.weapon_bounds.left>=col*672 && p.weapon_bounds.right<=(col+1)*672);
        assert(p.weapon_bounds.top>=0 && p.weapon_bounds.bottom<=528);
        assert(raster_art::draw_sprite(target.dc,pose.c_str(),center,feet,288));
        assert(raster_equipment::draw_front(target.dc,p));
        const std::string label=std::string(weapons[col])+" / "+pose+" / "+std::to_string(p.clockwise_degrees)+" deg";
        SetTextColor(target.dc,RGB(235,223,204));SetBkMode(target.dc,TRANSPARENT);
        TextOutA(target.dc,col*672+12,490,label.c_str(),static_cast<int>(label.size()));
      }
      save(target,(directory/(prefix+"-frame"+std::to_string(frame)+"-3x.png")).c_str());
    }
    std::ofstream html(directory/(prefix+"-review.html"));assert(html.good());
    html << R"HTML(<!doctype html><meta charset="utf-8"><title>Directional strike equipment review</title>
<style>body{background:#26221e;color:#eadfcc;font:16px system-ui}img{display:block;image-rendering:pixelated}button,input{margin:8px}</style>
<p>Actual 288px actor height. Axe, sword, club; fixed pivot includes complete weapon arc and idle transition.
90ms per strike pose is review timing, not simulation contact timing.</p>
<button id="play">Pause</button><label>Pose <input id="phase" type="range" min="0" max="6" value="6"></label><span id="label"></span>
<img id="frame" width="2016" height="528"><script>const prefix=')HTML" << prefix << R"HTML(';
const order=[6,0,1,2,3,4,5,6];let cursor=0,playing=true,last=0;
function show(i){frame.src=`${prefix}-frame${i}-3x.png`;phase.value=i;label.textContent=i===6?'idle':`strike ${i}`;}
play.onclick=()=>{playing=!playing;play.textContent=playing?'Pause':'Play';last=0;};
phase.oninput=()=>{playing=false;play.textContent='Play';show(+phase.value);};
function tick(now){if(playing&&now-last>(order[cursor]===6?360:90)){cursor=(cursor+1)%order.length;show(order[cursor]);last=now;}requestAnimationFrame(tick);}show(6);requestAnimationFrame(tick);</script>)HTML";
  }
}

void write_strike_animation() {
  // Fixed 160x144 logical cell/pivot includes the entire weapon arc. PNGs are
  // rendered at the game's 3x actor scale, not enlarged from 1x previews.
  const char* weapons[]{"weapon_axe", "weapon_sword", "weapon_club"};
  const std::filesystem::path directory = L".ci-artifacts/raster-equipment";
  for (int frame = 0; frame < 7; ++frame) {
    const std::string pose = frame == 6 ? "hero_se" : "hero_strike" + std::to_string(frame) + "_se";
    raster_art::detail::Surface target;
    assert(target.create(160 * 3 * 3, 144 * 3));
    HBRUSH bg = CreateSolidBrush(RGB(54, 48, 42));
    RECT area{0, 0, target.width, target.height};
    FillRect(target.dc, &area, bg); DeleteObject(bg);
    for (int col = 0; col < 3; ++col) {
      const int center = col * 480 + 192, feet = 336;
      auto p = raster_equipment::compute(pose.c_str(), weapons[col], center, feet, 288);
      assert(p.valid());
      assert(p.actor_bounds.left >= col * 480 && p.actor_bounds.right <= (col + 1) * 480);
      assert(p.weapon_bounds.left >= col * 480 && p.weapon_bounds.right <= (col + 1) * 480);
      assert(p.weapon_bounds.top >= 0 && p.weapon_bounds.bottom <= 432);
      assert(raster_art::draw_sprite(target.dc, pose.c_str(), center, feet, 288));
      assert(raster_equipment::draw_front(target.dc, p));
      SetTextColor(target.dc, RGB(235, 223, 204)); SetBkMode(target.dc, TRANSPARENT);
      const std::string label = std::string(weapons[col]) + " / " + pose + " / " +
                                std::to_string(p.clockwise_degrees) + " deg";
      TextOutA(target.dc, col * 480 + 12, 390, label.c_str(), static_cast<int>(label.size()));
    }
    save(target, (directory / ("strike-action-frame" + std::to_string(frame) + "-3x.png")).c_str());
  }
  std::ofstream html(directory / "strike-action-review.html");
  assert(html.good());
  html << R"HTML(<!doctype html><meta charset="utf-8"><title>SE strike equipment review</title>
<style>body{background:#26221e;color:#eadfcc;font:16px system-ui;margin:16px}img{display:block;image-rendering:pixelated}button,input{margin:8px}</style>
<p>Actual 288px actor height / axe, sword, club. Six authored equipment orientations; idle entry and exit included.
Preview timing is 90ms per strike pose, not a simulation contact test.</p>
<button id="play">Pause</button><label>Pose <input id="phase" type="range" min="0" max="6" value="6"></label><span id="label"></span>
<img id="frame" width="1440" height="432" alt="Three held-weapon composites at gameplay size">
<script>
const pictures=Array.from({length:7},(_,i)=>{const im=new Image();im.src=`strike-action-frame${i}-3x.png`;return im;});
const order=[6,0,1,2,3,4,5,6]; let cursor=0,playing=true,last=0;
function show(i){frame.src=pictures[i].src;phase.value=i;label.textContent=i===6?'idle':`strike ${i}`;}
play.onclick=()=>{playing=!playing;play.textContent=playing?'Pause':'Play';last=0;};
phase.oninput=()=>{playing=false;play.textContent='Play';show(+phase.value);};
function tick(now){if(playing&&now-last>(order[cursor]===6?360:90)){cursor=(cursor+1)%order.length;show(order[cursor]);last=now;}requestAnimationFrame(tick);}
show(6);requestAnimationFrame(tick);
</script>)HTML";
}

int main() {
 check_rotated_sampling();
 raster_art::set_asset_root(L"native/client/assets/raster/runtime");
 using namespace raster_equipment;
 check_pose_coverage();
 check_strike_socket_pixels();
 check_directional_socket_pixels();
 assert(!compute(nullptr,"weapon_axe",0,0,288).valid());
 assert(!compute("unknown_pose","weapon_axe",0,0,288).valid());
 assert(!compute("hero_se","unknown",0,0,288).valid());
 assert(!compute("hero_se","weapon_axe",0,0,0).valid());
 assert(!compute("hero_se","weapon_axe",std::numeric_limits<int>::max(),0,288).valid());
 assert(!compute("hero_se","weapon_axe",0,std::numeric_limits<int>::min(),288).valid());
 auto axe=compute("hero_se","weapon_axe",120,288,288);
 assert(axe.valid()&&axe.flip&&axe.weapon_height==192);
 assert(axe.weapon_bounds.left==123&&axe.weapon_bounds.top==27);
 assert(axe.weapon_bounds.right==219&&axe.weapon_bounds.bottom==219);
 for(const auto& pose:detail::kPoses) for(const auto& weapon:detail::kWeapons)
  for(int height:{96,173,288}) {
   auto p=compute(pose.name,weapon.name,200,300,height);assert(p.valid());
   const double grip_x=p.weapon_bounds.left+p.weapon_transform.anchor_x;
   const double grip_y=p.weapon_bounds.top+p.weapon_transform.anchor_y;
   assert(std::abs(grip_x-p.hand_screen.x)<=0.500001);
   assert(std::abs(grip_y-p.hand_screen.y)<=0.500001);
   if (!weapon.authored_melee || !std::string_view(pose.name).starts_with("hero_strike"))
     assert(p.clockwise_degrees==0);
  }

 int target_width=360,target_height=360;
 for(const auto& pose:detail::kPoses) {
  target_width=std::max(target_width,pose.canvas.width*3+32);
  target_height=std::max(target_height,pose.canvas.height*3+32);
 }
 raster_art::detail::Surface target;assert(target.create(target_width,target_height));
 const auto check_occlusion=[&](const char* pose_name,const char* weapon_name) {
  const auto* pose=pose_metadata(pose_name);assert(pose);
  const int center_x=16+pose->canvas.width*3/2;
  const int feet_y=16+pose->canvas.height*3;
  auto plan=compute(pose_name,weapon_name,center_x,feet_y,pose->canvas.height*3);assert(plan.valid());
  std::memset(target.pixels,80,target.bytes());
  assert(raster_art::draw_sprite(target.dc,pose_name,center_x,feet_y,pose->canvas.height*3));GdiFlush();
  std::vector<std::uint32_t> before(static_cast<std::uint32_t*>(target.pixels),static_cast<std::uint32_t*>(target.pixels)+target_width*target_height);
  assert(draw_front(target.dc,plan));GdiFlush();
  auto* source=raster_art::detail::asset(pose_name);
  const RECT box=plan.behind_actor?RECT{0,0,pose->canvas.width,pose->canvas.height}:pose->fingers;
  for(int y=box.top;y<box.bottom;++y) for(int x=box.left;x<box.right;++x) {
   Gdiplus::Color color;assert(source->image->GetPixel(x,y,&color)==Gdiplus::Ok);
   assert(color.GetAlpha()==0||color.GetAlpha()==255);
   if(color.GetAlpha()!=255) continue;
   for(int sy=0;sy<3;++sy)for(int sx=0;sx<3;++sx) {
    const int tx=16+x*3+sx,ty=16+y*3+sy,index=ty*target_width+tx;
    assert((static_cast<std::uint32_t*>(target.pixels)[index]&0xffffff)==(before[index]&0xffffff));
   }
  }
 };
 for(const auto& pose:detail::kPoses)for(const auto& weapon:detail::kWeapons)
   check_occlusion(pose.name,weapon.name);
 // Zero degrees preserves the previous full-canvas placement and exact pixels.
 raster_art::detail::Surface ordinary;assert(ordinary.create(target_width,target_height));
 for(const auto& pose:detail::kPoses)for(const auto& weapon:detail::kWeapons) {
   const auto p=compute(pose.name,weapon.name,160,320,173);
   if(p.clockwise_degrees!=0)continue;
   std::memset(ordinary.pixels,0,ordinary.bytes());std::memset(target.pixels,0,target.bytes());
   assert(raster_art::draw_sprite(ordinary.dc,weapon.name,p.weapon_center_x,p.weapon_feet_y,p.weapon_height,p.flip));
   assert(raster_art::draw_sprite_at_anchor(target.dc,weapon.name,p.hand_screen.x,p.hand_screen.y,p.weapon_height,weapon.grip.x,weapon.grip.y,0,p.flip));
   GdiFlush();assert(std::memcmp(ordinary.pixels,target.pixels,target.bytes())==0);
 }
 // Warm and repeatedly replay every actual strike/weapon variant at 3x.
 std::vector<Placement> strikes;
 for(const auto& pose:detail::kPoses)if(std::string_view(pose.name).starts_with("hero_strike"))
   for(const auto& weapon:detail::kWeapons) {
     strikes.push_back(compute(pose.name,weapon.name,160,320,288));
     assert(draw_front(target.dc,strikes.back()));
   }
 const auto cached=raster_art::cache_stats();
 const DWORD gdi_before=GetGuiResources(GetCurrentProcess(),GR_GDIOBJECTS);
 RECT prior_clip{0,0,301,302};IntersectClipRect(target.dc,0,0,301,302);
 for(int i=0;i<100;++i) {
   assert(draw_front(target.dc,axe));
   for(const auto& p:strikes)assert(draw_front(target.dc,p));
 }
 RECT after_clip{};GetClipBox(target.dc,&after_clip);assert(EqualRect(&prior_clip,&after_clip));
 const DWORD gdi_after=GetGuiResources(GetCurrentProcess(),GR_GDIOBJECTS);
 auto repeated=raster_art::cache_stats();
 assert(cached.image_loads==repeated.image_loads&&cached.scale_builds==repeated.scale_builds);
 assert(gdi_after==gdi_before);
 assert(repeated.scaled_bitmaps<=192&&repeated.scaled_bytes<=64u*1024u*1024u);
 std::printf("Stable repeat: %zu strike placements x 100 plus idle; image-load delta %llu, bitmap-build delta %llu, GDI handles %lu -> %lu; cache %zu bitmaps / %zu bytes.\n",
     strikes.size(), static_cast<unsigned long long>(repeated.image_loads-cached.image_loads),
     static_cast<unsigned long long>(repeated.scale_builds-cached.scale_builds),
     static_cast<unsigned long>(gdi_before),static_cast<unsigned long>(gdi_after),
     repeated.scaled_bitmaps,repeated.scaled_bytes);

 write_contact_sheets();
 write_walk_animation();
 write_directional_strike_animation();
 write_strike_animation();
 std::printf("PASS: %zu poses x %zu weapons at three scales; grip error <=0.5 px, exact actor occlusion, unchanged zero-angle pixels, direct PARGB nearest samples, normalized angle identity, 360-angle bounded eviction, stable full-strike repeat cache/GDI handles and invalid-parameter rejection.\n",std::size(detail::kPoses),std::size(detail::kWeapons));
}
