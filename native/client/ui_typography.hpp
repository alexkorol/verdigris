#pragma once

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <string>
#include <vector>
#include "ui_font_coverage.hpp"

namespace skin {
// Owner-selected m5x7 at its 32px em: visible 2px steps and 14px capitals.
// Rasterize directly; viewport tiers and titles use deliberate integer sizes.
enum class TextRole { Body, Label, Heading, Compact, Title, CompactValue };
inline constexpr int kMinSmallPx = 10, kMinBodyPx = 12;
inline int& ui_scale_ref() { static int scale=1; return scale; }
inline void set_ui_scale(int scale) { ui_scale_ref()=std::clamp(scale,1,4); }
inline int ui_scale() { return ui_scale_ref(); }
inline const char* owner_type_floor_label() { return "Type floor"; }
inline const char* owner_ink_contrast_label() { return "Ink contrast"; }
inline bool type_floor_strip_covers_hud_fails_review(bool overlap) { return overlap; }

struct FontResource {
  std::vector<char> bytes;
  HANDLE registration=nullptr;
  std::filesystem::path path;
  FontResource() {
    wchar_t exe[32768]{};
    const DWORD length=GetModuleFileNameW(nullptr,exe,32768);
    if(!length || length>=32768) return;
    // Both development and the player package keep native/build/client.exe.
    // Deliberately no cwd or installed-font fallback.
    path=std::filesystem::path(exe).parent_path().parent_path()/
         L"client/assets/fonts/sans/VerdigrisSans.ttf";
    std::ifstream file(path,std::ios::binary);
    if(!file) return;
    bytes.assign(std::istreambuf_iterator<char>(file),{});
    if(bytes.empty()) return;
    DWORD count=0;
    registration=AddFontMemResourceEx(bytes.data(),static_cast<DWORD>(bytes.size()),nullptr,&count);
  }
  ~FontResource() { if(registration) RemoveFontMemResourceEx(registration); }
};
inline FontResource& font_resource() { static FontResource resource; return resource; }
inline void ensure_game_fonts() { (void)font_resource(); }
inline bool game_font_available() { return font_resource().registration!=nullptr; }

inline HFONT font(TextRole role, int scale=0) {
  struct Cache {
    HFONT fonts[3][5]{};
    ~Cache() { for(auto& row:fonts) for(auto f:row) if(f) DeleteObject(f); }
  };
  ensure_game_fonts();
  static Cache cache;
  const int s=scale?std::clamp(scale,1,4):ui_scale();
  // Tiny inventory cells use the same face's native 1px grid for counts.
  const int tier=role==TextRole::CompactValue?0:role==TextRole::Title?2:1;
  const int em=tier==0?16:tier==1?32:64;
  auto& result=cache.fonts[tier][s];
  if(!result && game_font_available())
    result=CreateFontW(-em*s,0,0,0,FW_NORMAL,FALSE,FALSE,FALSE,
        DEFAULT_CHARSET,OUT_TT_ONLY_PRECIS,CLIP_DEFAULT_PRECIS,
        NONANTIALIASED_QUALITY,VARIABLE_PITCH,L"Verdigris Sans");
  return result;
}
inline HFONT font_body() { return font(TextRole::Body); }
inline HFONT font_body_bold() { return font(TextRole::Label); }
inline HFONT font_small() { return font(TextRole::Compact); }
inline HFONT font_title() { return font(TextRole::Title); }
inline HFONT font_heading() { return font(TextRole::Heading); }

inline bool glyph_supported(unsigned int point) {
  return std::binary_search(std::begin(kUiFontCodepoints),std::end(kUiFontCodepoints),point);
}
// Native content is UTF-8. Preserve legacy Windows-1252 byte strings when
// decoding fails. Missing characters visibly use this family's '?' glyph;
// Windows font linking must never silently change a word's typeface.
inline std::wstring display_text(const char* text,int count) {
  if(!text) return {};
  if(count<0) count=static_cast<int>(std::char_traits<char>::length(text));
  UINT page=CP_UTF8;DWORD flags=MB_ERR_INVALID_CHARS;
  int length=MultiByteToWideChar(page,flags,text,count,nullptr,0);
  if(!length && count) { page=1252;flags=0;length=MultiByteToWideChar(page,flags,text,count,nullptr,0); }
  std::wstring decoded(length,L'\0'),result;
  if(length) MultiByteToWideChar(page,flags,text,count,decoded.data(),length);
  result.reserve(decoded.size());
  for(std::size_t i=0;i<decoded.size();++i) {
    const wchar_t c=decoded[i];
    if(c==0x2026) { result+=L"...";continue; }
    if(c>=0xd800 && c<=0xdbff && i+1<decoded.size() && decoded[i+1]>=0xdc00 && decoded[i+1]<=0xdfff) { ++i;result+=L'?';continue; }
    result+=glyph_supported(c)||c==L'\n'||c==L'\r'||c==L'\t'?c:L'?';
  }
  return result;
}
inline BOOL text_out(HDC dc,int x,int y,const char* text,int count) {
  const auto value=display_text(text,count);
  return TextOutW(dc,x,y,value.data(),static_cast<int>(value.size()));
}
inline int draw_text(HDC dc,const char* text,int count,RECT* box,UINT flags) {
  auto value=display_text(text,count);
  // No caller uses DT_MODIFYSTRING; metric and draw conversion are identical.
  return DrawTextW(dc,value.data(),static_cast<int>(value.size()),box,flags);
}
inline BOOL text_extent(HDC dc,const char* text,int count,SIZE* size) {
  const auto value=display_text(text,count);
  return GetTextExtentPoint32W(dc,value.data(),static_cast<int>(value.size()),size);
}
inline int line_height(HDC dc,int leading=3) {
  TEXTMETRICW metric{};GetTextMetricsW(dc,&metric);
  return metric.tmHeight+leading*ui_scale();
}
// DrawText(DT_CALCRECT) can widen its rectangle for a long word. Keep the
// requested width fixed, breaking such words on actual glyph boundaries.
inline std::vector<std::wstring> wrap_text(HDC dc,const std::string& text,int width) {
  const auto value=display_text(text.c_str(),-1);
  std::vector<std::wstring> lines;
  std::size_t start=0;
  while(start<value.size()) {
    std::size_t end=start,last_space=std::wstring::npos;
    while(end<value.size()&&value[end]!=L'\n') {
      SIZE extent{};GetTextExtentPoint32W(dc,value.data()+start,int(end-start+1),&extent);
      if(extent.cx>width&&end>start)break;
      if(value[end]==L' ')last_space=end;
      ++end;
    }
    const bool newline=end<value.size()&&value[end]==L'\n';
    if(!newline&&end<value.size()&&last_space!=std::wstring::npos)end=last_space;
    auto line=value.substr(start,end-start);
    while(!line.empty()&&line.back()==L' ')line.pop_back();
    lines.push_back(std::move(line));start=end;
    if(newline)++start;
    while(start<value.size()&&value[start]==L' ')++start;
  }
  return lines;
}
inline SIZE measure_text(const char* text,TextRole role,int scale) {
  HDC dc=CreateCompatibleDC(nullptr);
  const auto old=SelectObject(dc,font(role,scale));
  SIZE result{};text_extent(dc,text,-1,&result);
  SelectObject(dc,old);DeleteDC(dc);return result;
}
} // namespace skin
