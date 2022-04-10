@echo off
setlocal

set COMPILE_OPTIONS= /FC /W4 /WX /nologo /Zi /std:c++latest /GR- /EHa- /MT /wd4312 /wd4201 /wd4200 /wd4100 /wd4456 /wd4505 /wd4189 /wd4996 /wd4101 /wd4238 /wd4114
set LINK_OPTIONS= /incremental:no
set INCLUDE_PATHS= /I"..\source"
set LIBRARY_PATHS=
set LIBRARIES= User32.lib Gdi32.lib Opengl32.lib Comdlg32.lib Ole32.lib Winmm.lib

set COMPILE_OPTIONS=%COMPILE_OPTIONS% /Od
REM set COMPILE_OPTIONS=%COMPILE_OPTIONS% /O2 /DDO_RELEASE_BUILD

REM rm *.pdb
pushd "build"
cl %COMPILE_OPTIONS% %DEBUG_OPTIONS% %INCLUDE_PATHS% /Fe:Win32AssetProcessor.exe ..\source\win32\win32_asset_processor.cpp /link %LINK_OPTIONS% %LIBRARY_PATHS% 

pushd "..\data"
..\build\Win32AssetProcessor.exe
popd

rc /nologo /fo .\win32_resource.res /I..\data ..\source\win32\win32_resource.rc 

cl %COMPILE_OPTIONS% %DEBUG_OPTIONS% %INCLUDE_PATHS% /Fe:Win32TAGameDebug.exe ..\source\win32\win32_main.cpp /link %LINK_OPTIONS% %LIBRARY_PATHS% %LIBRARIES% .\win32_resource.res

popd
