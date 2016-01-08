@echo off

set EMBREE_ROOT="C:\Program Files (x86)\embree2"

if not exist Projects\Windows (
	mkdir Projects\Windows
)
cd Projects\Windows
cmake ..\..\src -G "Visual Studio 12" -DEMBREE_INCLUDE_PATH=%EMBREE_ROOT%\include -DEMBREE_LIBRARY=%EMBREE_ROOT%\lib\embree.lib
pause