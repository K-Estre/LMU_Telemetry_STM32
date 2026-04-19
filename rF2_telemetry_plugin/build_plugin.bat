@echo off
call "C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\Tools\VsDevCmd.bat" -arch=x64
cd /d C:\Projects\LMU_Telemetry_STM32\rF2_telemetry_plugin
if not exist build mkdir build
cl /nologo /LD /EHsc /std:c++17 /I Include Source\rF2_telemetry_plugin.cpp /link /OUT:build\rF2_telemetry_plugin.dll hid.lib setupapi.lib
