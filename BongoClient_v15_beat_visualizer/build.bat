@echo off
title Building BongoCatClient v15 Beat Visualizer Exe
echo ====================================================
echo   BUILDING BONGOCATCLIENT V15 BEAT VISUALIZER EXE
echo ====================================================
echo.

echo Step 1: Installing/Upgrading python dependencies...
pip install pystray pillow pyserial pycaw comtypes pyinstaller

echo.
echo Step 2: Building Windows Standalone Executable (Single EXE)...
pyinstaller --noconfirm --onefile --windowed --noconsole --name "BongoCatClient_v15" --icon "NONE" --hidden-import comtypes.stream --hidden-import pycaw.pycaw "app.py"

echo.
echo ====================================================
echo   BUILD COMPLETE!
echo   You can find the BongoCatClient_v15.exe in the:
echo   'dist' folder inside BongoClient_v15_beat_visualizer.
echo ====================================================
echo.
pause
