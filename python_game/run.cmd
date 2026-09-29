@echo off
rem ============================================================
rem  run.cmd - start of the game "Python" (Snake) in Windows
rem  If python_game.exe is missing, builds it first via build.cmd
rem ============================================================
setlocal
chcp 65001 >nul
cd /d "%~dp0"

if not exist python_game.exe (
    echo [INFO] python_game.exe not found - building...
    call build.cmd
    if not exist python_game.exe (
        echo [ERROR] Build failed. Run build.cmd manually.
        pause
        exit /b 1
    )
)

echo [INFO] Starting the game...
python_game.exe
if errorlevel 1 echo [ERROR] Game exited with code %errorlevel%.
pause
endlocal
