@echo off
:: call "c:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvars64.bat" 
"c:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Tools\MSVC\14.26.28801\bin\Hostx64\x64\cl.exe"  /Zi /I "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Tools\MSVC\14.26.28801\include" /I "C:\Users\user\libraries\glfw-3.3.2.bin.WIN64\glfw-3.3.2.bin.WIN64\include" /I "C:\Users\user\libraries\glew-2.1.0-win32\glew-2.1.0\include" main.c cb_lib/cb_string.c boombox.c renderer.c game.c ui.c /link "C:\Users\user\libraries\glfw-3.3.2.bin.WIN64\glfw-3.3.2.bin.WIN64\lib-vc2019\glfw3dll.lib" "C:\Users\user\libraries\glew-2.1.0-win32\glew-2.1.0\lib\Release\x64\glew32s.lib" "C:\Program Files (x86)\Windows Kits\10\Lib\10.0.18362.0\um\x64\OpenGL32.Lib"

