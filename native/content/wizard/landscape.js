/* Continuous terrain realization. The encounter graph is a pacing scaffold,
   never a collection of stamped rooms. Integer arithmetic matches the native port. */
(function(root,factory){
  if(typeof module==='object'&&module.exports)module.exports=factory();
  else root.Landscape=factory();
})(typeof self!=='undefined'?self:this,function(){
  'use strict';
  function realize({seed,recipe,width,height,rooms,edges,floor,space,outdoor}) {
    let state=(seed^0x94d049bb)>>>0;
    const rand=n=>{state=(Math.imul(state,1664525)+1013904223)>>>0;return Math.floor(state/4294967296*n);};
    const clamp=(v,a,b)=>Math.max(a,Math.min(b,v));
    function hash(x,y,salt){let h=(Math.imul(x,374761393)^Math.imul(y,668265263)^seed^salt)>>>0;h=Math.imul(h^(h>>>13),1274126177)>>>0;return (h^(h>>>16))&255;}
    function noise(x,y,p,salt){
      const gx=Math.floor(x/p),gy=Math.floor(y/p),u=x%p,v=y%p;
      const sx=Math.floor(u*u*(3*p-2*u)/(p*p)),sy=Math.floor(v*v*(3*p-2*v)/(p*p));
      const a=hash(gx,gy,salt)*(p-sx)+hash(gx+1,gy,salt)*sx;
      const b=hash(gx,gy+1,salt)*(p-sx)+hash(gx+1,gy+1,salt)*sx;
      return Math.floor((a*(p-sy)+b*sy)/(p*p));
    }
    // Off-grid, separated landmarks. Their influence can merge into a meadow,
    // a broad cave or a coast; no room boundary is retained in collision.
    for(const n of rooms){
      let best=null,bestGap=-1;
      for(let k=0;k<40;k++){
        const x=clamp(21+n.c*18+rand(15)-7,14,width-15),y=clamp(21+n.r*18+rand(15)-7,14,height-15);
        let gap=100000;for(const other of rooms){if(other.id>=n.id)break;gap=Math.min(gap,(x-other.cx)**2+(y-other.cy)**2);}
        if(gap>bestGap){best={x,y};bestGap=gap;}if(gap>=121)break;
      }
      n.cx=best.x;n.cy=best.y;n.rx=5+rand(7);n.ry=4+rand(7);
      n.w=n.rx*2+1;n.h=n.ry*2+1;n.x=n.cx-n.rx;n.y=n.cy-n.ry;
      n.prefab=outdoor?'terrain-landmark':recipe==='sanctuary'?'eroded-court':'eroded-vault';
    }
    const size=width*height,mask=new Uint8Array(size),protectedLand=new Uint8Array(size),trail=new Uint8Array(size);
    const field=new Int32Array(size).fill(-100000);
    const cx=Math.floor(width/2),cy=Math.floor(height/2),rx=Math.floor(width*.47),ry=Math.floor(height*.43);
    for(let y=1;y<height-1;y++)for(let x=1;x<width-1;x++){
      const i=y*width+x;
      if(outdoor){
        const continent=1000-Math.floor((x-cx)**2*1000/(rx*rx))-Math.floor((y-cy)**2*1000/(ry*ry));
        // Low-frequency relief sets the coast. A second band cuts marsh inlets
        // or rocky ridges through it; these are features of the whole map.
        const relief=(noise(x,y,17,71)-128)*5+(noise(x,y,7,163)-128)*2;
        const channel=noise(x,y,11,991);
        const band=recipe==='quarry'?28:recipe==='wildwood'?34:36;
        const cut=recipe==='wildwood'?42:recipe==='quarry'?38:36;
        const incised=Math.max(0,band-Math.abs(channel-128))*cut;
        field[i]=continent+relief-incised;
      }
    }
    function lobe(x0,y0,ax,ay,power){
      for(let y=Math.max(1,y0-ay-3);y<=Math.min(height-2,y0+ay+3);y++)for(let x=Math.max(1,x0-ax-3);x<=Math.min(width-2,x0+ax+3);x++){
        const value=power-Math.floor((x-x0)**2*1000/(ax*ax))-Math.floor((y-y0)**2*1000/(ay*ay));
        const rough=(noise(x,y,6,431)-128)*3+(noise(x,y,3,877)-128);
        field[y*width+x]=Math.max(field[y*width+x],value+rough);
      }
    }
    for(const n of rooms){
      lobe(n.cx,n.cy,n.rx,n.ry,outdoor?650:1000);
      if(!outdoor)lobe(n.cx+rand(9)-4,n.cy+rand(9)-4,3+rand(5),3+rand(5),900);
    }
    for(let i=0;i<size;i++)mask[i]=field[i]>0?1:0;
    function brush(x,y,r,dest){for(let dy=-r;dy<=r;dy++)for(let dx=-r;dx<=r;dx++)if(dx*dx+dy*dy<=r*r){const px=x+dx,py=y+dy;if(px>0&&py>0&&px<width-1&&py<height-1)dest[py*width+px]=1;}}
    for(const e of edges){
      const a=rooms[e.a],b=rooms[e.b],horizontal=Math.abs(b.cx-a.cx)>=Math.abs(b.cy-a.cy);
      const bend=rand(17)-8,mx=Math.floor((a.cx+b.cx)/2)+(horizontal?0:bend),my=Math.floor((a.cy+b.cy)/2)+(horizontal?bend:0);
      const breadth=2+rand(outdoor?2:3);
      for(let k=0;k<=32;k++){
        const t=32-k,x=Math.floor((t*t*a.cx+2*t*k*mx+k*k*b.cx+512)/1024),y=Math.floor((t*t*a.cy+2*t*k*my+k*k*b.cy+512)/1024);
        brush(x,y,breadth,mask);brush(x,y,2,protectedLand);brush(x,y,1,trail);
      }
      // Logical passage bearings remain available for tools, but are not
      // visible prefab doors. Safety anchors sit within the blended clearing.
      const dx=Math.sign(b.c-a.c),dy=Math.sign(b.r-a.r),dir=dy<0?0:dx>0?1:dy>0?2:3;
      a.sockets.push({direction:dir,to:b.id,x:a.cx+dx*2,y:a.cy+dy*2,width:3});
      b.sockets.push({direction:(dir+2)%4,to:a.id,x:b.cx-dx*2,y:b.cy-dy*2,width:3});
    }
    // A few cellular erosion passes blend crossings and soften the silhouette.
    for(let pass=0;pass<3;pass++){
      const next=mask.slice();for(let y=1;y<height-1;y++)for(let x=1;x<width-1;x++){
        let near=0;for(let dy=-1;dy<=1;dy++)for(let dx=-1;dx<=1;dx++)near+=mask[(y+dy)*width+x+dx];
        next[y*width+x]=near>=5?1:0;
      }mask.set(next);
    }
    for(const n of rooms)brush(n.cx,n.cy,4,protectedLand);
    for(let i=0;i<size;i++)if(protectedLand[i])mask[i]=1;
    // Remove detached islands; no long, visible repair tunnels are introduced.
    const reached=new Uint8Array(size),queue=[rooms[0].cy*width+rooms[0].cx];reached[queue[0]]=1;
    for(let k=0;k<queue.length;k++)for(const d of [-width,1,width,-1]){const i=queue[k]+d;if(i>=0&&i<size&&mask[i]&&!reached[i]){reached[i]=1;queue.push(i);}}
    const tiles=new Uint8Array(size).fill(space);
    for(let y=1;y<height-1;y++)for(let x=1;x<width-1;x++){
      const i=y*width+x;if(reached[i]){
        tiles[i]=outdoor&&trail[i]?10:floor;
        // Only an actual wet crossing becomes a bridge, never an entire edge.
        if(recipe==='causeway'&&trail[i]&&field[i]<-250)tiles[i]=11;
      }else if(outdoor&&recipe!=='causeway'&&field[i]>-550)tiles[i]=recipe==='wildwood'?8:9;
    }
    if(!outdoor){for(let y=1;y<height-1;y++)for(let x=1;x<width-1;x++)if(!reached[y*width+x]&&[-width,1,width,-1].some(d=>reached[y*width+x+d]))tiles[y*width+x]=2;}
    return tiles;
  }
  return {realize};
});
