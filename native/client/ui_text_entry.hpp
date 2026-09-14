#pragma once

namespace skin {
// Name input intentionally retains its existing printable-ASCII, 40-character
// contract. Positions are bytes, which are whole glyphs in that input alphabet.
struct TextEntry {
  std::size_t caret=0,anchor=0;
  int scroll=0;
  void focus(const std::string& value) { caret=anchor=value.size();scroll=0; }
  void clamp(const std::string& value) { caret=std::min(caret,value.size());anchor=std::min(anchor,value.size()); }
  bool erase_selection(std::string& value) {
    clamp(value);if(caret==anchor)return false;
    const auto begin=std::min(caret,anchor),end=std::max(caret,anchor);
    value.erase(begin,end-begin);caret=anchor=begin;return true;
  }
  void character(std::string& value,WPARAM c) {
    clamp(value);
    if(c==8) { if(!erase_selection(value)&&caret) { value.erase(--caret,1);anchor=caret; } }
    else if(c>=32&&c<127) {
      if(caret==anchor&&value.size()>=40)return;
      erase_selection(value);value.insert(value.begin()+caret,static_cast<char>(c));anchor=++caret;
    }
  }
  void key(std::string& value,WPARAM key,bool shift,bool control) {
    clamp(value);
    if(control&&key=='A') { anchor=0;caret=value.size();return; }
    if(key==VK_DELETE) { if(!erase_selection(value)&&caret<value.size())value.erase(caret,1);return; }
    if(key==VK_LEFT) caret=!shift&&caret!=anchor?std::min(caret,anchor):caret?caret-1:0;
    else if(key==VK_RIGHT) caret=!shift&&caret!=anchor?std::max(caret,anchor):std::min(caret+1,value.size());
    else if(key==VK_HOME)caret=0;
    else if(key==VK_END)caret=value.size();
    else return;
    if(!shift)anchor=caret;
  }
};

inline int entry_advance(HDC dc,const std::string& value,std::size_t end) {
  SIZE extent{};text_extent(dc,value.data(),static_cast<int>(end),&extent);return extent.cx;
}
inline void paint_entry(HDC dc,RECT box,const std::string& value,
                        const std::string& placeholder,TextEntry& edit,bool focused) {
  const int saved=SaveDC(dc);SelectObject(dc,font(TextRole::Body));SetBkMode(dc,TRANSPARENT);
  box.left+=12*ui_scale();box.right-=12*ui_scale();
  const int width=std::max(1,int(box.right-box.left));
  TEXTMETRICW metric{};GetTextMetricsW(dc,&metric);
  const int y=box.top+(box.bottom-box.top-metric.tmHeight)/2;
  if(focused)edit.clamp(value);
  if(focused) {
    const int at=entry_advance(dc,value,edit.caret);
    if(at-edit.scroll>width-2)edit.scroll=at-width+2;
    if(at<edit.scroll)edit.scroll=at;
    edit.scroll=std::max(0,edit.scroll);
  }
  const int scroll=focused?edit.scroll:0;
  IntersectClipRect(dc,box.left,box.top,box.right,box.bottom);
  if(focused&&edit.caret!=edit.anchor) {
    RECT selection{box.left+entry_advance(dc,value,std::min(edit.caret,edit.anchor))-scroll,y,
      box.left+entry_advance(dc,value,std::max(edit.caret,edit.anchor))-scroll,y+metric.tmHeight};
    HBRUSH ink=CreateSolidBrush(RGB(62,74,60));FillRect(dc,&selection,ink);DeleteObject(ink);
  }
  SetTextColor(dc,value.empty()?RGB(181,162,119):RGB(207,180,104));
  const auto& shown=value.empty()?placeholder:value;
  text_out(dc,box.left-scroll,y,shown.data(),static_cast<int>(shown.size()));
  if(focused) {
    const int x=box.left+entry_advance(dc,value,edit.caret)-scroll;
    RECT caret{x,y+2*ui_scale(),x+ui_scale(),y+metric.tmHeight-2*ui_scale()};
    HBRUSH ink=CreateSolidBrush(RGB(225,193,116));FillRect(dc,&caret,ink);DeleteObject(ink);
  }
  RestoreDC(dc,saved);
}
inline void click_entry(HDC dc,const std::string& value,TextEntry& edit,
                        const RECT& box,int x,bool shift) {
  const int saved=SaveDC(dc);SelectObject(dc,font_body());
  const int at=x-box.left-12*ui_scale()+edit.scroll;
  std::size_t index=0;int previous=0;
  for(;index<value.size();++index) {
    const int next=entry_advance(dc,value,index+1);
    if(at<(previous+next)/2)break;
    previous=next;
  }
  edit.caret=index;if(!shift)edit.anchor=index;RestoreDC(dc,saved);
}
} // namespace skin
