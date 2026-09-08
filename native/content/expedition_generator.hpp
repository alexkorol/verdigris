#pragma once
// Original WIZARD Cartographer 2 grammar. Keep parity with core/expedition.js.
// Pure deterministic content: no assets, platform, window, network or simulation ownership.
#include <algorithm>
#include <array>
#include <cstdint>
#include <string>
#include <vector>

namespace verdigris::cartography {
inline constexpr const char* kVersion = "2.0.0";
struct Point { int x=0,y=0; bool operator==(const Point&) const = default; };
struct Socket { int direction=0,to=0,x=0,y=0,width=3; };
struct Room { int id=0,c=0,r=0,cx=0,cy=0,variant=0,rotation=0,depth=0,tier=0; std::string role,prefab,landmark; std::vector<Socket> sockets; };
struct Edge { int a=0,b=0; std::string kind; };
struct Spawn { Point position; int room=0,tier=0,count=0; std::string type; };
struct Plan { std::uint32_t seed=0; std::string recipe="necropolis"; int columns=6,rows=4,branches=5,loops=2; };
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
inline Map generate(Plan plan){
  plan.columns=std::clamp(plan.columns,4,10);plan.rows=std::clamp(plan.rows,3,7);plan.branches=std::clamp(plan.branches,0,18);plan.loops=std::clamp(plan.loops,0,8);
  if(plan.recipe!="necropolis"&&plan.recipe!="causeway"&&plan.recipe!="quarry"&&plan.recipe!="sanctuary")plan.recipe="necropolis";
  const bool wet=plan.recipe=="causeway",hot=plan.recipe=="quarry",star=plan.recipe=="sanctuary";
  const std::uint8_t floor=wet?7:hot?13:1,space=wet?5:hot?6:0;
  const std::string room_type=wet?"island":hot?"cavern":star?"court":"ossuary";
  const std::string landmark=wet?"Flood marker":hot?"Kiln circle":star?"Celestial dial":"Ancestor court";
  Rng topology{plan.seed^0xa511e9b3u},variants{plan.seed^0x63d83595u},encounters{plan.seed^0xb5297a4du};
  Map m;m.plan=plan;m.width=plan.columns*15+2;m.height=plan.rows*15+2;m.tiles.assign(static_cast<std::size_t>(m.width)*m.height,space);
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
  auto paint=[&](int x,int y,std::uint8_t t){if(x>0&&y>0&&x<m.width-1&&y<m.height-1)m.tiles[static_cast<std::size_t>(y)*m.width+x]=t;};
  for(auto& n:m.rooms){n.prefab=n.role=="boss"?"guardian-court":n.role=="entry"?"processional-gate":room_type+"-"+std::to_string(n.variant);
    for(int dy=-5;dy<=5;++dy)for(int dx=-5;dx<=5;++dx){const bool cut=(hot||wet||n.variant==1)&&std::abs(dx)+std::abs(dy)>8;if(!cut)paint(n.cx+dx,n.cy+dy,floor);}
    if(n.role!="boss"&&!wet&&!hot&&n.variant%2==0)for(const int dx:{-3,3})for(const int dy:{-3,3})paint(n.cx+dx,n.cy+dy,2);
  }
  for(const auto& e:m.edges){auto& a=m.rooms[e.a];auto& b=m.rooms[e.b];const int dx=(b.cx>a.cx)-(b.cx<a.cx),dy=(b.cy>a.cy)-(b.cy<a.cy);int dir=0;for(int i=0;i<4;++i)if(kDirections[i]==Point{dx,dy})dir=i;
    for(int step=0;step<=15;++step)for(int q=-1;q<=1;++q)paint(a.cx+dx*step+(dy?q:0),a.cy+dy*step+(dx?q:0),wet||hot?11:floor);
    a.sockets.push_back({dir,b.id,a.cx+dx*5,a.cy+dy*5,3});b.sockets.push_back({(dir+2)%4,a.id,b.cx-dx*5,b.cy-dy*5,3});
  }
  if(!wet&&!star){std::vector<int> floors;for(int i=0;i<static_cast<int>(m.tiles.size());++i)if(walkable(m.tiles[i]))floors.push_back(i);for(int i:floors)for(auto d:kDirections){int x=i%m.width+d.x,y=i/m.width+d.y;if(x>0&&y>0&&x<m.width-1&&y<m.height-1&&m.tiles[static_cast<std::size_t>(y)*m.width+x]==space)paint(x,y,2);}}
  m.entrance={m.rooms[0].cx-3,m.rooms[0].cy};m.boss={m.rooms[boss_id].cx,m.rooms[boss_id].cy};m.exit={m.boss.x+3,m.boss.y};m.main_path=path(m,m.entrance,m.boss);
  const auto distances=search(m,m.entrance);const int max_distance=distances.distance[m.boss.y*m.width+m.boss.x];
  for(auto& n:m.rooms){n.depth=distances.distance[n.cy*m.width+n.cx];n.tier=n.role=="entry"?0:n.role=="boss"?4:std::min(3,1+n.depth*3/std::max(1,max_distance));n.landmark=n.role=="entry"?"Processional gate":n.role=="boss"?"Guardian sanctuary":n.role=="treasure"?"Sealed offering":landmark+" "+std::to_string(n.id+1);if(n.role=="entry")continue;m.spawns.push_back({{n.cx,n.cy},n.id,n.tier,n.role=="boss"?1:3+encounters.next(4),n.role=="boss"?"boss":n.role=="treasure"?"elite":"pack"});}
  return m;
}
inline bool valid(const Map& m){
  const auto s=search(m,m.entrance);int total=0;for(auto t:m.tiles)if(walkable(t))++total;if(s.count!=total||path(m,m.entrance,m.exit).empty())return false;
  std::vector<int> occupied;for(const auto& spawn:m.spawns){const int i=spawn.position.y*m.width+spawn.position.x;if(!m.can_walk(spawn.position.x,spawn.position.y)||s.distance[i]<9||std::find(occupied.begin(),occupied.end(),i)!=occupied.end())return false;occupied.push_back(i);}
  for(const auto& n:m.rooms)for(const auto& socket:n.sockets){if(!m.can_walk(socket.x,socket.y)||socket.to<0||socket.to>=static_cast<int>(m.rooms.size()))return false;bool reciprocal=false;for(const auto& other:m.rooms[socket.to].sockets)if(other.to==n.id&&other.direction==(socket.direction+2)%4)reciprocal=true;if(!reciprocal)return false;}
  return true;
}
} // namespace verdigris::cartography
