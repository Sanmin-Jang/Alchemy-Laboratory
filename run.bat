@echo off
setlocal
cd /d "%~dp0"
if exist "build\Release\AlchemistPotionLab.exe" (
    cd build\Release
    start "" AlchemistPotionLab.exe
) else (
    echo [ERROR] Executable not found. Please run build.bat first!
    pause
)
