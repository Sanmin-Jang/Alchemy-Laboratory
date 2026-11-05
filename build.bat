@echo off
setlocal
cd /d "%~dp0"
echo =================================================================
echo Building Alchemist's Potion Lab - OpenGL 3.3 Core Profile
echo =================================================================

cmake -S . -B build
if %ERRORLEVEL% NEQ 0 (
    echo [ERROR] CMake configuration failed.
    if "%1"=="" pause
    exit /b %ERRORLEVEL%
)

cmake --build build --config Release
if %ERRORLEVEL% NEQ 0 (
    echo [ERROR] Build compilation failed.
    if "%1"=="" pause
    exit /b %ERRORLEVEL%
)

echo.
echo =================================================================
echo Build Succeeded!
echo Executable: build\Release\AlchemistPotionLab.exe
echo Run using:  run.bat
echo =================================================================
if "%1"=="" pause
