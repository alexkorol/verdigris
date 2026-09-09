# Reproduce the local reference extraction

Requires Windows, Python 3.9+ and a locally built ANSI x64 CascLib DLL. The
script is independently authored; the parser and extracted files remain in
an external local cache. No game loader is used by this tool.

The parser used for this study is
[CascLib at 2a280f5a231966dc5d1b534978dd9f9f04a374cd](https://github.com/ladislav-zezula/CascLib/tree/2a280f5a231966dc5d1b534978dd9f9f04a374cd)
(MIT). The local build used VS2019 v142. Its only source adjustment was
`#include "afxres.h"` → `#include <windows.h>` in `src/DllMain.rc`, because
the installed toolchain did not provide that MFC resource header. The
extractor does not apply patches or download/build a parser automatically.

Run the following **from the external CascLib source checkout** to build:

```powershell
& 'C:\Program Files (x86)\Microsoft Visual Studio\2019\BuildTools\MSBuild\Current\Bin\MSBuild.exe' CascLib_dll.vcxproj /p:Configuration=Release /p:Platform=x64 /p:PlatformToolset=v142 /verbosity:minimal
```

Run these commands **from the Verdigris repository root** to reproduce the
eleven-file selection. The existing cache/DLL paths below are from this study:

```powershell
$referenceArgs = @(
  '--install', 'Z:\Games\Diablo-2-Resurrected\Diablo II Resurrected Infernal Edition',
  '--dll', 'C:\Users\Alex\Documents\ChatGPT\diablo-reference-cache\CascLib\bin\CascLib_dll\x64\Release\CascLib.dll',
  '--out', 'C:\Users\Alex\Documents\ChatGPT\diablo-reference-cache\reproduced'
)
$referenceSummary = Get-Content docs/rebuild/diablo-reference-study/reference-summary.json -Raw | ConvertFrom-Json
$referencePaths = @($referenceSummary.sources.PSObject.Properties.Value.path)
python docs/rebuild/diablo-reference-study/tools/extract_reference.py @referenceArgs @referencePaths
```

Omitting explicit archive paths selects only skills, monsters, weapons and
inventory tables. `--list` enumerates text-file metadata without extracting
their contents. The tool rejects paths outside its eleven-file allowlist,
outputs inside the installation or a Git checkout, files larger than 2 MB,
short reads and non-UTF-8 data. Inspect `extraction-manifest.json` and compare
SHA256 values against `reference-summary.json`; a changed hash means the
recorded observations need rechecking against the new build.

The UI files use comments, trailing commas, inheritance and symbolic profile
references. Do not run a strict JSON parser on them and assume errors imply
corruption; do not confuse source layout units with measured screen pixels.
