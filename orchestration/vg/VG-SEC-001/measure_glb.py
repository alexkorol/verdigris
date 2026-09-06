import struct, sys

p = 'native/client/assets/wizard/splash/world/celestial_world_runtime_tapered.glb'
d = open(p, 'rb').read()
magic, ver, total = struct.unpack('<III', d[:12])
jlen, jtype = struct.unpack('<II', d[12:20])
print('total', total, 'json_chunk', jlen, 'type', hex(jtype))
js = d[20:20 + jlen].decode('utf-8', errors='replace')
depth = 0
mx = 0
instr = False
esc = False
for ch in js:
    if instr:
        if esc:
            esc = False
        elif ch == '\\':
            esc = True
        elif ch == '"':
            instr = False
        continue
    if ch == '"':
        instr = True
    elif ch in '{[':
        depth += 1
        mx = max(mx, depth)
    elif ch in '}]':
        depth -= 1
print('glb json max depth:', mx)
