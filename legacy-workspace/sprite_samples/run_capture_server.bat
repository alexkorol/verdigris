@echo off
echo Delaford Sprite Capture Server
echo ==============================
cd /d "%~dp0"
where conda >nul 2>&1
if %errorlevel%==0 call conda activate base 2>nul
echo.
echo Leave THIS window open while you capture images from ChatGPT.
echo Close it (Ctrl+C or just close the window) when you're done.
echo.
python receive_images.py
pause
