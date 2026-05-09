@echo off
setlocal enabledelayedexpansion

echo ==========================================
echo Building BRF Synchronizer (standalone, Qt-free)
echo ==========================================

:: 0. Check for cmake
where cmake >nul 2>nul
if %ERRORLEVEL% NEQ 0 (
    echo [ERROR] cmake not found in PATH.
    echo Please install CMake: https://cmake.org/download/
    pause
    exit /b 1
)

:: 1. Configure
echo.
echo [STEP 1] Configuring with CMake...
cmake -B build_standalone -G "Visual Studio 17 2022" -A x64
if %ERRORLEVEL% NEQ 0 (
    echo [ERROR] CMake configure failed.
    echo.
    echo If Visual Studio 2022 is not installed, try:
    echo   cmake -B build_standalone -G "Visual Studio 16 2019" -A x64
    pause
    exit /b 1
)

:: 2. Build
echo.
echo [STEP 2] Building Release...
cmake --build build_standalone --config Release
if %ERRORLEVEL% NEQ 0 (
    echo [ERROR] Build failed.
    pause
    exit /b 1
)

:: 3. Report
echo.
echo ==========================================
echo [SUCCESS] Build complete!
echo ==========================================
if exist build_standalone\Release\brf_sync.exe (
    echo Tool: %CD%\build_standalone\Release\brf_sync.exe
    for %%F in (build_standalone\Release\brf_sync.exe) do echo Size: %%~zF bytes
    echo.
    echo No Qt DLLs required!
)
echo.
pause
