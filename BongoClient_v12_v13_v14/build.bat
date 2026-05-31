@echo off
echo Installing requirements...
pip install pynput pyserial pystray pillow pyinstaller

echo Building Windows Executable...
pyinstaller --noconfirm --onedir --windowed --noconsole --name "BongoCatClient" --icon "NONE"  "app.py"

echo Build complete! You can find the executable in the 'dist/BongoCatClient' folder.
pause
