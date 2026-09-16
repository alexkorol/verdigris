from pathlib import Path
import json
root=Path('native/client/assets/first-slice/monsters-v2');m=json.loads((root/'transfer-manifest-v3.json').read_text());refs=json.loads((root/'reference-manifest.json').read_text())['clips']
for dr in ('front','right','back','left'):
 for action,row in [('attack',0),('hit',128)]:
  key=f'pack-wolf-{action}-{dr}-fromcombat'
  m['clips'][key]={**refs[f'pack-wolf-{action}-{dr}'],'layout':{'columns':4,'slot':[96,128],'inset':[0,16],'scale':4,'row_offset':row}}
(root/'transfer-manifest-v3.json').write_text(json.dumps(m,indent=2))
p=Path('native/tools/first_slice_monsters_transfer.py');s=p.read_text();start=s.index('def guides():');end=s.index('def recover(');s=s[:start]+s[end:]
s=s.replace('transfer-manifest.json','transfer-manifest-v3.json')
s=s.replace("scale=3 if w==128 else 4; slot_h=160 if w==128 else 128; inset=32 if w==128 else 16", "layout=m['layout']; scale=layout['scale']; columns=layout['columns'];slot_w,slot_h=layout['slot'];inset_x,inset=layout['inset'];row_offset=layout.get('row_offset',0)")
s=s.replace('(i%4)*w;y=round(-oy/scale)+(i//4)*slot_h','(i%columns)*slot_w;y=round(-oy/scale)+(i//columns)*slot_h+row_offset')
s=s.replace('x+w,y+slot_h','x+slot_w,y+slot_h')
s=s.replace('(-dx,inset-dy,w-dx,inset+h-dy)','(inset_x-dx,inset-dy,inset_x+w-dx,inset+h-dy)')
s=s.replace("p.add_argument('--guides',action='store_true');",'')
s=s.replace('    if a.guides:guides()\n    else:recover(a.raw,a.clip)','    recover(a.raw,a.clip)')
Path('native/tools/first_slice_monsters_transfer_v3.py').write_text(s)
