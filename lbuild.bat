@echo off
"c:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Tools\MSVC\14.26.28801\bin\Hostx64\x64\cl.exe" /Fe:lp.exe /Zi /I "C:\Users\user\libraries\SDL2-2.0.12\include" lplacer.c /link  "C:\Users\user\libraries\SDL2-2.0.12\lib\x64\SDL2main.lib" "C:\Users\user\libraries\SDL2-2.0.12\lib\x64\SDL2.lib" "C:\Program Files (x86)\Windows Kits\10\Lib\10.0.18362.0\um\x64\shell32.lib" /SUBSYSTEM:CONSOLE