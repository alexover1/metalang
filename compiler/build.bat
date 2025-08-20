@echo off
if not exist ..\build mkdir ..\build
pushd ..\build
echo * > .gitignore
ctime -begin metalang.ctm
call cl -nologo -Zi -FC ..\compiler\metalang.cpp -Femetalang_msvc_debug.exe
rem call clang -g -fuse-ld=lld ..\metalang.cpp -o metalang_clang_debug.exe
rem call cl -O2 -nologo -Zi -FC ..\metalang.cpp -Femetalang_msvc_release.exe
rem call clang -O3 -g -fuse-ld=lld ..\metalang.cpp -o metalang_clang_release.exe
set LastError=%ERRORLEVEL%
ctime -end metalang.ctm %LastError%
popd
