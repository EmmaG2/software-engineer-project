@echo off
cd /d "%~dp0"
set PATH=C:\Qt\Tools\mingw1310_64\bin;C:\Qt\6.11.1\mingw_64\bin;%PATH%
start "" "%~dp0build\QtDesktopApp.exe"
