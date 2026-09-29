@echo off
rem ============================================================
rem  build.cmd - сборка игры "Питон" (C++17) в Windows
rem  Использует g++ (MinGW-w64), либо cl.exe (Visual Studio)
rem  Результат: python_game.exe в папке проекта
rem ============================================================
setlocal
cd /d "%~dp0"

where g++ >nul 2>nul
if %errorlevel%==0 goto :build_gcc

where cl >nul 2>nul
if %errorlevel%==0 goto :build_msvc

echo [ERROR] C++ compiler not found.
echo Install one of:
echo   1. MinGW-w64 g++ ^(MSYS2: pacman -S mingw-w64-x86_64-gcc^) and add bin\ to PATH;
echo   2. Visual Studio workload "Desktop development with C++"
echo      ^(run this file from "x64 Native Tools Command Prompt"^).
pause
exit /b 1

:build_gcc
echo [INFO] Found g++ ^(MinGW^). Building...
if not exist build mkdir build
g++ -std=c++17 -Wall -Wextra -O2 -Isrc src\console_io.cpp src\database.cpp src\game_field.cpp src\python_snake.cpp src\main.cpp -o python_game.exe
if errorlevel 1 (
    echo [ERROR] Compilation failed.
    pause
    exit /b 1
)
goto :success

:build_msvc
echo [INFO] Found cl.exe ^(Visual Studio^). Building...
if not exist build mkdir build
cl /nologo /std:c++17 /EHsc /W4 /O2 /Isrc /Fobuild\ src\console_io.cpp src\database.cpp src\game_field.cpp src\python_snake.cpp src\main.cpp /link /OUT:python_game.exe
if errorlevel 1 (
    echo [ERROR] Compilation failed.
    pause
    exit /b 1
)
del /q *.obj 2>nul
goto :success

:success
echo [OK] Build finished: python_game.exe
echo Use run.cmd to start the game.
pause
endlocal
