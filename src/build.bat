@echo off

IF NOT EXIST ..\build mkdir ..\build
pushd ..\build

del *.pdb > NUL 2> NUL
del *.obj > NUL 2> NUL

@echo on
@echo -------------------------------------------------
@echo DEBUG BUILD
@echo -------------------------------------------------
@echo off


cl -O2 /Zi ../src/weight.cpp /I ../src/external/raylib /I ../src/external/raylib/external/glfw/include ../src/external/raylib/*.c -D _WIN32=1 -D PLATFORM_DESKTOP=1 gdi32.lib winmm.lib user32.lib shell32.lib| msvc_color_release.exe


popd
