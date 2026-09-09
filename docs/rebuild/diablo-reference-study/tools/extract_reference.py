"""Read a deliberately small reference set from a local CASC installation.
No game executables are loaded. No external keys, online storage, or bypass flags.
"""
import ctypes as c
import argparse, hashlib, json, pathlib, sys

parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument('--install', type=pathlib.Path, required=True)
parser.add_argument('--dll', type=pathlib.Path, required=True, help='Trusted, locally built CascLib ANSI x64 DLL')
parser.add_argument('--out', type=pathlib.Path, required=True, help='Reference cache outside installation and game repository')
parser.add_argument('--list', action='store_true', help='Write text path metadata only')
parser.add_argument('paths', nargs='*', help='Selected allowlisted CASC paths; defaults to four gameplay tables')
args = parser.parse_args()
BASE = args.out.resolve()
INSTALL = args.install.resolve(strict=True)
DLL = args.dll.resolve(strict=True)
if BASE.is_relative_to(INSTALL) or INSTALL.is_relative_to(BASE):
    parser.error('--out must be separate from the installation')
if any((parent / '.git').exists() for parent in (BASE, *BASE.parents)):
    parser.error('--out must be outside a Git checkout; raw reference files stay local')
tables = [f'data:data\\global\\excel\\{name}.txt' for name in ['skills','monstats','weapons','inventory']]
layouts = [f'data:data\\global\\ui\\layouts\\{name}.json' for name in ['hudpanelhd','globaldatahd','hudmonsterhealthhd','playerinventoryexpansionlayouthd','tooltipspanelhd','_profilehd','playerinventoryoriginallayouthd']]
names = args.paths or tables
if any(name not in tables + layouts for name in names):
    parser.error('Requested path is outside the explicit 11-file reference allowlist')
BASE.mkdir(parents=True, exist_ok=True)
lib = c.WinDLL(str(DLL), use_last_error=True)
ptr = c.c_void_p
u32 = c.c_uint32
def api(name, types, result=c.c_bool):
    fn = getattr(lib,name); fn.argtypes=types; fn.restype=result
    return fn
open_storage=api('CascOpenStorage',[c.c_char_p,u32,c.POINTER(ptr)])
open_file=api('CascOpenFile',[ptr,c.c_char_p,u32,u32,c.POINTER(ptr)])
read_file=api('CascReadFile',[ptr,ptr,u32,c.POINTER(u32)])
file_size=api('CascGetFileSize64',[ptr,c.POINTER(c.c_uint64)])
close_file=api('CascCloseFile',[ptr])
close_storage=api('CascCloseStorage',[ptr])
storage=ptr()
if not open_storage(str(INSTALL).encode('mbcs'),2,c.byref(storage)):
    raise OSError(c.get_last_error(),'CascOpenStorage failed')
if args.list:
    class Find(c.Structure):
        _fields_=[('name',c.c_char*260),('ckey',c.c_byte*16),('ekey',c.c_byte*16),('tag',c.c_uint64),('size',c.c_uint64),('plain',c.c_char_p),('id',u32),('locale',u32),('content',u32),('spans',u32),('available',u32),('type',c.c_int)]
    first=api('CascFindFirstFile',[ptr,c.c_char_p,c.POINTER(Find),c.c_char_p],ptr)
    next_file=api('CascFindNextFile',[ptr,c.POINTER(Find)])
    close_find=api('CascFindClose',[ptr])
    found=Find(); handle=first(storage,b'*',c.byref(found),None); results=[]
    if handle and handle != c.c_void_p(-1).value:
        while True:
            name=found.name.decode(errors='replace')
            if name.endswith(('.txt','.json')): results.append({'path':name,'bytes':found.size})
            if not next_file(handle,c.byref(found)): break
        close_find(handle)
    close_storage(storage)
    (BASE/'text-file-index.json').write_text(json.dumps(results,indent=2))
    print(json.dumps(results[:30],indent=2)); print('Total text paths:',len(results)); sys.exit()
manifest=[]
try:
    for name in names:
        file=ptr()
        if not open_file(storage,name.encode(),2,0,c.byref(file)):
            manifest.append({'path':name,'error':c.get_last_error()}); continue
        try:
            size=c.c_uint64()
            if not file_size(file,c.byref(size)): raise OSError(c.get_last_error())
            if size.value > 2_000_000: raise ValueError('Reference file exceeds 2 MB cap')
            buf=c.create_string_buffer(size.value); count=u32()
            if not read_file(file,buf,size.value,c.byref(count)): raise OSError(c.get_last_error())
            data=buf.raw[:count.value]
            if count.value != size.value: raise ValueError('Short read')
            data.decode('utf-8-sig') # Only plain text accepted.
            target=BASE/'extracted'/pathlib.PurePosixPath(name.replace('data:','',1).replace('\\','/'))
            if not target.resolve().is_relative_to((BASE/'extracted').resolve()): raise ValueError('Unsafe path')
            target.parent.mkdir(parents=True,exist_ok=True); target.write_bytes(data)
            manifest.append({'path':name,'bytes':len(data),'sha256':hashlib.sha256(data).hexdigest(),'output':str(target)})
        except Exception as error: manifest.append({'path':name,'error':str(error)})
        finally: close_file(file)
finally: close_storage(storage)
print(json.dumps(manifest,indent=2))
(BASE/'extraction-manifest.json').write_text(json.dumps(manifest,indent=2))
sys.exit(1 if any('error' in item for item in manifest) else 0)
