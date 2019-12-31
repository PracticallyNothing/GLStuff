@echo off

pushd C:\Users\User\GLStuff\

cd C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\VC\Auxiliary\Build

call vcvars64.bat

popd

mkdir build

cd build

cl.exe /I "..\glad_Core-33\include" /I "..\SDL2\include" /I "..\freetype-windows-binaries\include" ..\*.c ..\glad_Core-33\src\glad.c opengl32.lib ..\SDL2\lib\x64\SDL2main.lib ..\SDL2\lib\x64\SDL2.lib ..\freetype-windows-binaries\win64\freetype.lib /Zi /link /out:GLSpiral.exe /SUBSYSTEM:WINDOWS

pause
