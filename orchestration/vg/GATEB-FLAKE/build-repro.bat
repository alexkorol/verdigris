@echo off
call "C:\Program Files (x86)\Microsoft Visual Studio\2019\BuildTools\VC\Auxiliary\Build\vcvars64.bat" >nul 2>&1
cl /nologo /std:c++20 /EHsc /W4 /I"include" /I"client" /c "tools\gateb_flake_repro.cpp" /Fo"build\gateb_flake_repro.obj" || exit /b 1
cl /nologo "build\gateb_flake_repro.obj" "build\local_session.obj" "build\remote_session.obj" "build\presentation_state.obj" "build\networking.obj" "build\core.obj" "build\seasonal.obj" /Fe"build\gateb_flake_repro.exe" /link ws2_32.lib || exit /b 1
