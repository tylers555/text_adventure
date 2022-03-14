@echo off
setlocal

set COMPILE_OPTIONS= /FC /W4 /WX /nologo /Zi /std:c++latest /GR- /Od /EHa- /MT /wd4312 /wd4201 /wd4200 /wd4100 /wd4456 /wd4505 /wd4189 /wd4996 /wd4101 /wd4238
set LINK_OPTIONS= /incremental:no
set INCLUDE_PATHS=
set LIBRARY_PATHS=
set LIBRARIES= User32.lib Gdi32.lib Opengl32.lib Comdlg32.lib Ole32.lib Winmm.lib

pushd "build"

REM rm *.pdb

:ToeTacTic
cl %COMPILE_OPTIONS% %INCLUDE_PATHS% /Fe:Win32TAGame.exe ..\source\os_win32.cpp /link  %LINK_OPTIONS% %LIBRARY_PATHS% %LIBRARIES%
popd
