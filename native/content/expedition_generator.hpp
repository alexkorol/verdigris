#pragma once
// Original WIZARD Cartographer 2 grammar. Keep parity with core/expedition.js.
// Pure deterministic content: no assets, platform, window, network or simulation ownership.
#include <algorithm>
#include <array>
#include <cstdint>
#include <string>
#include <vector>

namespace verdigris::cartography {
inline constexpr const char* kVersion = "3.0.0";
struct Point { int x=0,y=0; bool operator==(const Point&) const = default; };
struct Socket { int direction=0,to=0,x=0,y=0,width=3; };
struct Room { int id=0,c=0,r=0,cx=0,cy=0,rx=0,ry=0,variant=0,rotation=0,depth=0,tier=0; std::string role,prefab,landmark; std::vector<Socket> sockets; };
struct Edge { int a=0,b=0; std::string kind; };
struct Spawn { Point position; int room=0,tier=0,count=0; std::string type; };
struct Plan { std::uint32_t seed=0; std::string recipe="wildwood"; int columns=4,rows=3,branches=3,loops=1; };
inline constexpr std::array<Point,4> kDirections{{{0,-1},{1,0},{0,1},{-1,0}}};
inline bool walkable(std::uint8_t t){return t==1||t==3||t==7||t==10||t==11||t==12||t==13||t==14;}
struct Map {
  Plan plan; int width=0,height=0; std::vector<std::uint8_t> tiles;
  std::vector<Room> rooms; std::vector<Edge> edges; std::vector<int> spine;
  Point entrance,exit,boss; std::vector<Spawn> spawns; std::vector<Point> main_path;
  bool can_walk(int x,int y) const {return x>=0&&y>=0&&x<width&&y<height&&walkable(tiles[static_cast<std::size_t>(y)*width+x]);}
};
struct Rng { std::uint32_t state; int next(int n) {state=state*1664525u+1013904223u;return static_cast<int>((static_cast<std::uint64_t>(state)*n)>>32);}};
struct Search {std::vector<int> distance,parent;int count=0;};
inline Search search(const Map& map,Point start){
  Search s;s.distance.assign(map.tiles.size(),-1);s.parent.assign(map.tiles.size(),-1);
  if(!map.can_walk(start.x,start.y))return s;
  const int begin=start.y*map.width+start.x;std::vector<int> queue{begin};s.distance[begin]=0;
  for(std::size_t i=0;i<queue.size();++i){const int a=queue[i];for(const auto d:kDirections){int x=a%map.width+d.x,y=a/map.width+d.y;if(!map.can_walk(x,y))continue;int b=y*map.width+x;if(s.distance[b]<0){s.distance[b]=s.distance[a]+1;s.parent[b]=a;queue.push_back(b);}}}
  s.count=static_cast<int>(queue.size());return s;
}
inline std::vector<Point> path(const Map& map,Point from,Point to){
  if(!map.can_walk(to.x,to.y))return {};const auto s=search(map,from);int i=to.y*map.width+to.x;if(s.distance[i]<0)return {};std::vector<Point> result;
  while(i>=0){result.push_back({i%map.width,i/map.width});i=s.parent[i];}std::reverse(result.begin(),result.end());return result;
}
// Continuous terrain realization, kept in integer parity with landscape.js.
inline void realize_landscape(Map& m,std::uint8_t floor,std::uint8_t space,bool outdoor){
  Rng rng{m.plan.seed^0x94d049bbu};const int width=m.width,height=m.height,size=width*height;
  const auto hash=[&](int x,int y,std::uint32_t salt){std::uint32_t h=static_cast<std::uint32_t>(x)*374761393u^static_cast<std::uint32_t>(y)*668265263u^m.plan.seed^salt;h=(h^(h>>13))*1274126177u;return static_cast<int>((h^(h>>16))&255u);};
  const auto noise=[&](int x,int y,int p,std::uint32_t salt){
    const int gx=x/p,gy=y/p,u=x%p,v=y%p,sx=u*u*(3*p-2*u)/(p*p),sy=v*v*(3*p-2*v)/(p*p);
    const int a=hash(gx,gy,salt)*(p-sx)+hash(gx+1,gy,salt)*sx,b=hash(gx,gy+1,salt)*(p-sx)+hash(gx+1,gy+1,salt)*sx;
    return (a*(p-sy)+b*sy)/(p*p);
  };
  for(auto& n:m.rooms){
    Point best;int best_gap=-1;
    for(int k=0;k<40;++k){
      const int x=std::clamp(21+n.c*18+rng.next(15)-7,14,width-15),y=std::clamp(21+n.r*18+rng.next(15)-7,14,height-15);
      int gap=100000;for(const auto& other:m.rooms){if(other.id>=n.id)break;gap=std::min(gap,(x-other.cx)*(x-other.cx)+(y-other.cy)*(y-other.cy));}
      if(gap>best_gap){best={x,y};best_gap=gap;}if(gap>=121)break;
    }
    n.cx=best.x;n.cy=best.y;n.rx=5+rng.next(7);n.ry=4+rng.next(7);
    n.prefab=outdoor?"terrain-landmark":m.plan.recipe=="sanctuary"?"eroded-court":"eroded-vault";
  }
  std::vector<std::uint8_t> mask(size),protected_land(size),trail(size);
  std::vector<int> field(size,-100000);
  const int cx=width/2,cy=height/2,rx=width*47/100,ry=height*43/100;
  for(int y=1;y<height-1;++y)for(int x=1;x<width-1;++x)if(outdoor){
    const int continent=1000-(x-cx)*(x-cx)*1000/(rx*rx)-(y-cy)*(y-cy)*1000/(ry*ry);
    const int relief=(noise(x,y,17,71)-128)*5+(noise(x,y,7,163)-128)*2,channel=noise(x,y,11,991);
    const int band=m.plan.recipe=="quarry"?28:m.plan.recipe=="wildwood"?34:36,cut=m.plan.recipe=="wildwood"?42:m.plan.recipe=="quarry"?38:36;
    const int incised=std::max(0,band-std::abs(channel-128))*cut;
    field[y*width+x]=continent+relief-incised;
  }
  const auto lobe=[&](int x0,int y0,int ax,int ay,int power){
    for(int y=std::max(1,y0-ay-3);y<=std::min(height-2,y0+ay+3);++y)for(int x=std::max(1,x0-ax-3);x<=std::min(width-2,x0+ax+3);++x){
      const int value=power-(x-x0)*(x-x0)*1000/(ax*ax)-(y-y0)*(y-y0)*1000/(ay*ay);
      const int rough=(noise(x,y,6,431)-128)*3+(noise(x,y,3,877)-128);
      field[y*width+x]=std::max(field[y*width+x],value+rough);
    }
  };
  for(const auto& n:m.rooms){
    lobe(n.cx,n.cy,n.rx,n.ry,outdoor?650:1000);
    if(!outdoor){const int x=n.cx+rng.next(9)-4,y=n.cy+rng.next(9)-4,ax=3+rng.next(5),ay=3+rng.next(5);lobe(x,y,ax,ay,900);}
  }
  for(int i=0;i<size;++i)mask[i]=field[i]>0?1:0;
  const auto brush=[&](int x,int y,int r,std::vector<std::uint8_t>& dest){
    for(int dy=-r;dy<=r;++dy)for(int dx=-r;dx<=r;++dx)if(dx*dx+dy*dy<=r*r){const int px=x+dx,py=y+dy;if(px>0&&py>0&&px<width-1&&py<height-1)dest[py*width+px]=1;}
  };
  for(const auto& e:m.edges){
    auto& a=m.rooms[e.a];auto& b=m.rooms[e.b];const bool horizontal=std::abs(b.cx-a.cx)>=std::abs(b.cy-a.cy);
    const int bend=rng.next(17)-8,mx=(a.cx+b.cx)/2+(horizontal?0:bend),my=(a.cy+b.cy)/2+(horizontal?bend:0),breadth=2+rng.next(outdoor?2:3);
    for(int k=0;k<=32;++k){const int t=32-k,x=(t*t*a.cx+2*t*k*mx+k*k*b.cx+512)/1024,y=(t*t*a.cy+2*t*k*my+k*k*b.cy+512)/1024;brush(x,y,breadth,mask);brush(x,y,2,protected_land);brush(x,y,1,trail);}
    const int dx=(b.c>a.c)-(b.c<a.c),dy=(b.r>a.r)-(b.r<a.r),dir=dy<0?0:dx>0?1:dy>0?2:3;
    a.sockets.push_back({dir,b.id,a.cx+dx*2,a.cy+dy*2,3});b.sockets.push_back({(dir+2)%4,a.id,b.cx-dx*2,b.cy-dy*2,3});
  }
  for(int pass=0;pass<3;++pass){auto next=mask;for(int y=1;y<height-1;++y)for(int x=1;x<width-1;++x){int near=0;for(int dy=-1;dy<=1;++dy)for(int dx=-1;dx<=1;++dx)near+=mask[(y+dy)*width+x+dx];next[y*width+x]=near>=5?1:0;}mask=std::move(next);}
  for(const auto& n:m.rooms)brush(n.cx,n.cy,4,protected_land);
  for(int i=0;i<size;++i)if(protected_land[i])mask[i]=1;
  std::vector<std::uint8_t> reached(size);std::vector<int> queue{m.rooms[0].cy*width+m.rooms[0].cx};reached[queue[0]]=1;
  for(std::size_t k=0;k<queue.size();++k)for(const int d:{-width,1,width,-1}){const int i=queue[k]+d;if(i>=0&&i<size&&mask[i]&&!reached[i]){reached[i]=1;queue.push_back(i);}}
  m.tiles.assign(size,space);
  for(int y=1;y<height-1;++y)for(int x=1;x<width-1;++x){const int i=y*width+x;
    if(reached[i]){m.tiles[i]=outdoor&&trail[i]?10:floor;if(m.plan.recipe=="causeway"&&trail[i]&&field[i]<-250)m.tiles[i]=11;}
    else if(outdoor&&m.plan.recipe!="causeway"&&field[i]>-550)m.tiles[i]=m.plan.recipe=="wildwood"?8:9;
  }
  if(!outdoor)for(int y=1;y<height-1;++y)for(int x=1;x<width-1;++x)if(!reached[y*width+x])for(const int d:{-width,1,width,-1})if(reached[y*width+x+d]){m.tiles[y*width+x]=2;break;}
}
inline Map generate(Plan plan){
  plan.columns=std::clamp(plan.columns,4,10);plan.rows=std::clamp(plan.rows,3,7);plan.branches=std::clamp(plan.branches,0,18);plan.loops=std::clamp(plan.loops,0,8);
  if(plan.recipe!="wildwood"&&plan.recipe!="necropolis"&&plan.recipe!="causeway"&&plan.recipe!="quarry"&&plan.recipe!="sanctuary")plan.recipe="necropolis";
  const bool woods=plan.recipe=="wildwood",wet=plan.recipe=="causeway",hot=plan.recipe=="quarry",star=plan.recipe=="sanctuary";
  const std::uint8_t floor=wet||woods?7:hot?13:1,space=wet?5:hot?6:0;
  const std::string room_type=wet?"island":hot?"cavern":star?"court":"ossuary";
  const std::string landmark=woods?"Forest clearing":wet?"Flood marker":hot?"Kiln circle":star?"Celestial dial":"Ancestor court";
  Rng topology{plan.seed^0xa511e9b3u},variants{plan.seed^0x63d83595u},encounters{plan.seed^0xb5297a4du};
  Map m;m.plan=plan;m.width=plan.columns*18+24;m.height=plan.rows*18+24;m.tiles.assign(static_cast<std::size_t>(m.width)*m.height,space);
  auto find=[&](int c,int r){for(const auto& n:m.rooms)if(n.c==c&&n.r==r)return n.id;return -1;};
  auto node=[&](int c,int r,const std::string& role){const int existing=find(c,r);if(existing>=0)return existing;Room n;n.id=static_cast<int>(m.rooms.size());n.c=c;n.r=r;n.cx=8+c*15;n.cy=8+r*15;n.role=role;n.variant=variants.next(4);n.rotation=variants.next(4);m.rooms.push_back(n);return n.id;};
  auto link=[&](int a,int b,const std::string& kind){for(const auto& e:m.edges)if((e.a==a&&e.b==b)||(e.a==b&&e.b==a))return false;m.edges.push_back({a,b,kind});return true;};
  int row=1+topology.next(plan.rows-2),current=node(0,row,"entry");m.spine.push_back(current);
  for(int col=1;col<plan.columns;++col){
    if(col>1&&col<plan.columns-1&&topology.next(3)==0){const int nr=std::clamp(row+(topology.next(2)?1:-1),0,plan.rows-1);if(nr!=row){const int n=node(col-1,nr,"combat");link(current,n,"main");current=n;row=nr;m.spine.push_back(n);}}
    const int n=node(col,row,col==plan.columns-1?"boss":"combat");link(current,n,"main");current=n;m.spine.push_back(n);
  }
  const int boss_id=current;
  struct Frontier {int from,c,r;};
  for(int k=0;k<plan.branches;++k){std::vector<Frontier> frontier;for(const auto& n:m.rooms){if(n.role=="boss")continue;for(const auto d:kDirections){const int c=n.c+d.x,r=n.r+d.y;if(c>=0&&c<plan.columns&&r>=0&&r<plan.rows&&find(c,r)<0)frontier.push_back({n.id,c,r});}}if(frontier.empty())break;const auto f=frontier[topology.next(static_cast<int>(frontier.size()))];const int n=node(f.c,f.r,"optional");link(f.from,n,"branch");}
  std::vector<std::array<int,2>> candidates;for(const auto& a:m.rooms)for(const auto& b:m.rooms)if(a.id<b.id&&a.role!="boss"&&b.role!="boss"&&std::abs(a.c-b.c)+std::abs(a.r-b.r)==1)candidates.push_back({a.id,b.id});
  int loops=0;while(!candidates.empty()&&loops<plan.loops){const int i=topology.next(static_cast<int>(candidates.size()));const auto pair=candidates[i];candidates.erase(candidates.begin()+i);if(link(pair[0],pair[1],"loop"))++loops;}
  for(auto& n:m.rooms)if(n.role=="optional"){int degree=0;for(const auto& e:m.edges)if(e.a==n.id||e.b==n.id)++degree;if(degree==1)n.role="treasure";}
  realize_landscape(m,floor,space,woods||wet||hot);
  m.entrance={m.rooms[0].cx-3,m.rooms[0].cy};m.boss={m.rooms[boss_id].cx,m.rooms[boss_id].cy};m.exit={m.boss.x+3,m.boss.y};m.main_path=path(m,m.entrance,m.boss);
  const auto distances=search(m,m.entrance);const int max_distance=distances.distance[m.boss.y*m.width+m.boss.x];
  for(auto& n:m.rooms){n.depth=distances.distance[n.cy*m.width+n.cx];n.tier=n.role=="entry"?0:n.role=="boss"?4:std::min(3,1+n.depth*3/std::max(1,max_distance));n.landmark=n.role=="entry"?(woods||wet||hot?"Trailhead":"Processional gate"):n.role=="boss"?(woods||wet||hot?"Guardian clearing":"Guardian sanctuary"):n.role=="treasure"?(woods||wet||hot?"Hidden offering":"Sealed offering"):landmark+" "+std::to_string(n.id+1);if(n.role=="entry")continue;m.spawns.push_back({{n.cx,n.cy},n.id,n.tier,n.role=="boss"?1:3+encounters.next(4),n.role=="boss"?"boss":n.role=="treasure"?"elite":"pack"});}
  return m;
}
inline bool valid(const Map& m){
  const auto s=search(m,m.entrance);int total=0;for(auto t:m.tiles)if(walkable(t))++total;if(s.count!=total||path(m,m.entrance,m.exit).empty())return false;
  std::vector<int> occupied;for(const auto& spawn:m.spawns){const int i=spawn.position.y*m.width+spawn.position.x;if(!m.can_walk(spawn.position.x,spawn.position.y)||s.distance[i]<9||std::find(occupied.begin(),occupied.end(),i)!=occupied.end())return false;occupied.push_back(i);}
  for(const auto& n:m.rooms)for(const auto& socket:n.sockets){if(!m.can_walk(socket.x,socket.y)||socket.to<0||socket.to>=static_cast<int>(m.rooms.size()))return false;bool reciprocal=false;for(const auto& other:m.rooms[socket.to].sockets)if(other.to==n.id&&other.direction==(socket.direction+2)%4)reciprocal=true;if(!reciprocal)return false;}
  return true;
}
} // namespace verdigris::cartography
