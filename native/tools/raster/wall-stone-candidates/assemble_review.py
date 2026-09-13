"""Assemble measured Pixel Respecter planes and review their approved wall geometry."""
from pathlib import Path
from PIL import Image, ImageDraw
import hashlib, json
import numpy as np

HERE = Path(__file__).resolve().parent
ROOT = HERE.parents[3]
RUNTIME = ROOT / "native/client/assets/raster/runtime"
SELECTED = HERE / "selected"
SELECTED.mkdir(exist_ok=True)
sha = lambda p: hashlib.sha256(p.read_bytes()).hexdigest()

cap = Image.open(HERE / "v2-planes/wall_stone_cutaway_cap.png").convert("RGBA")
face = Image.open(HERE / "v2-planes/wall_stone_cutaway_face.png").convert("RGBA")
assert cap.size == (64, 64) and face.size == (64, 32)
wall = Image.new("RGBA", (64, 96))
wall.paste(cap, (0, 0))
wall.paste(face, (0, 64))
wall.save(SELECTED / "wall_stone_cutaway.png")
wall.resize((192, 288), Image.Resampling.NEAREST).save(SELECTED / "native-3x.png")

# A depth-correct 3x3 block: rows advance64, so the next cap covers the prior face.
repeat = Image.new("RGBA", (192, 224))
for y in range(3):
    for x in range(3):
        repeat.alpha_composite(wall, (x * 64, y * 64))
repeat.save(SELECTED / "wall-3x3-1x.png")
repeat.resize((576, 672), Image.Resampling.NEAREST).save(SELECTED / "wall-3x3-3x.png")

earth = Image.open(RUNTIME / "terrain_quiet_earth.png").convert("RGBA")
hero_nw = Image.open(RUNTIME / "hero_nw.png").convert("RGBA")
hero_se = Image.open(RUNTIME / "hero_se.png").convert("RGBA")

def actor_image(im):
    # main.cpp uses full-canvas height =1.75 world tiles, not visible-body fitting.
    return im.resize((round(im.width * 112 / im.height), 112), Image.Resampling.NEAREST)

back, front = actor_image(hero_nw), actor_image(hero_se)
def intersects(a,b):
    return a[0]<b[2] and a[2]>b[0] and a[1]<b[3] and a[3]>b[1]
def scene(cutaway):
    out = Image.new("RGBA", (384,416))
    for y in range(0,416,64):
        for x in range(0,384,64):
            out.alpha_composite(earth,(x,y))
    # Actor is north of the first blocked row, with unchanged ground position.
    bx,by = 192-back.width//2,124-back.height
    out.alpha_composite(back,(bx,by))
    b=back.getchannel("A").getbbox()
    ink=(b[0]+bx,b[1]+by,b[2]+bx,b[3]+by)
    faded=[]
    for y in range(3):
        for x in range(3):
            left,top=96+x*64,96+y*64
            art=wall
            if cutaway and intersects((left,top,left+64,top+96),ink):
                art=wall.copy()
                art.putalpha(80) # Proposed renderer's exact constant opacity.
                faded.append([x,y])
            out.alpha_composite(art,(left,top))
    out.alpha_composite(front,(242-front.width//2,350-front.height))
    return out,faded
opaque,_=scene(False)
faded,overlaps=scene(True)
context=Image.new("RGB",(784,440),(22,22,20))
context.paste(opaque.convert("RGB"),(0,24))
context.paste(faded.convert("RGB"),(400,24))
d=ImageDraw.Draw(context)
d.text((8,7),"Opaque depth order",(238,233,214))
d.text((408,7),"Proposed overlap fade: alpha80",(238,233,214))
context.save(SELECTED / "actor-context-1x.png")
context.resize((1568,880),Image.Resampling.NEAREST).save(SELECTED/"actor-context-2x.png")

v1=Image.open(HERE/"v1/wall_stone_cutaway.png").convert("RGBA")
compare=Image.new("RGB",(160,120),(24,24,22))
compare.paste(v1.convert("RGB"),(8,20));compare.paste(wall.convert("RGB"),(88,20))
d=ImageDraw.Draw(compare);d.text((8,4),"v1",(240,234,214));d.text((88,4),"v2 planes",(240,234,214))
compare.save(SELECTED/"comparison-1x.png")
compare.resize((480,360),Image.Resampling.NEAREST).save(SELECTED/"comparison-3x.png")

a=np.asarray(wall).astype(float)
c=a[:64,:,:3]; f=a[64:,:,:3]
mad=lambda x: round(float(np.abs(x).mean()),3)
measure={
 "canvas":[64,96],"pivot":[32,96],"ground_footprint":[0,32,64,96],
 "cap":[0,0,64,64],"south_face":[0,64,64,96],
 "colors":len(np.unique(a[:,:,:3].reshape(-1,3),axis=0)),
 "alpha_values":np.unique(a[:,:,3]).astype(int).tolist(),
 "cap_edge_mad_left_right":mad(c[:,0]-c[:,-1]),
 "cap_edge_mad_top_bottom":mad(c[0]-c[-1]),
 "cap_ordinary_adjacent_mad_x":mad(c[:,1:]-c[:,:-1]),
 "cap_ordinary_adjacent_mad_y":mad(c[1:]-c[:-1]),
 "face_edge_mad_left_right":mad(f[:,0]-f[:,-1]),
 "face_ordinary_adjacent_mad_x":mad(f[:,1:]-f[:,:-1]),
 "context":{"tile_pixels":64,"tile_world_units":107.25,"hero_full_canvas_height":112,
 "hero_native_source_canvas":[80,96],"hero_height_formula":"1.75*64",
 "wall_ground_origin":[96,128],"north_actor_ground_pivot":[192,124],
 "south_actor_ground_pivot":[242,350],"cutaway_alpha":80,"overlapped_modules":overlaps,
 "scope":"offline geometric approximation of current renderer; not production-game acceptance"},
 "assembly":{"operation":"unaltered cap paste at[0,0], face paste at[0,64]",
 "recipe":"wall-stone-cutaway-v2-planes.json","report":"v2-planes/wall-stone-cutaway-v2-planes.provenance.json",
 "script_sha256":sha(Path(__file__))},
 "output_sha256":sha(SELECTED/"wall_stone_cutaway.png"),
 "reference_hashes":{n:sha(RUNTIME/f"{n}.png") for n in ["terrain_quiet_earth","hero_nw","hero_se"]}}
(SELECTED/"measurements.json").write_text(json.dumps(measure,indent=2)+"\n")
print(json.dumps(measure,indent=2))
