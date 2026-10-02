@echo off
if not exist maze.exe (
    echo maze.exe not found. Building first...
    call build.bat
)
echo Launching Maze Escape 3D...
start "" "%~dp0maze.exe"
