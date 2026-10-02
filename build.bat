@echo off
echo =======================================
echo Building Maze Escape 3D...
echo =======================================
g++ -O2 -Iinclude game.cpp -o maze.exe -lopengl32 -lglu32 -lgdi32 -lwinmm
if %ERRORLEVEL% EQU 0 (
    echo [SUCCESS] maze.exe compiled successfully!
) else (
    echo [ERROR] Compilation failed with error code %ERRORLEVEL%.
)
