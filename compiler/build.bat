@echo off
if not exist build mkdir build
pushd build
ctime -begin metalang.ctm
call cl -nologo -Zi -FC ..\metalang.cpp -Femetalang_msvc_debug.exe
rem call clang -g -fuse-ld=lld ..\sim86.cpp -o sim86_clang_debug.exe
rem call cl -O2 -nologo -Zi -FC ..\sim86.cpp -Fesim86_msvc_release.exe
rem call clang -O3 -g -fuse-ld=lld ..\sim86.cpp -o sim86_clang_release.exe
set LastError=%ERRORLEVEL%
ctime -end metalang.ctm %LastError%
popd
