@echo off
title EquipTrack Server
echo ============================================
echo   EquipTrack - Equipment Management System
echo ============================================
echo.

REM Check if g++ is installed
where g++ >nul 2>nul
if %errorlevel% neq 0 (
    echo ERROR: g++ not found!
    echo Please install MinGW and add it to your PATH.
    echo Download from: https://www.mingw-w64.org/
    pause
    exit /b 1
)

echo [1/3] Compiling C++ backend...
cd backend
g++ -std=c++17 main.cpp equipment.cpp -lws2_32 -o server.exe
if %errorlevel% neq 0 (
    echo.
    echo ERROR: Compilation failed!
    pause
    exit /b 1
)
echo     Compilation successful.
cd ..

echo.
echo [2/3] Starting EquipTrack server...
echo.
echo ============================================
echo   Server is running!
echo   Open your browser to: http://localhost:8080
echo   Press Ctrl+C to stop the server.
echo ============================================
echo.

REM Start the server
backend\server.exe

pause
