@echo off
echo Vendoring pixel-perfecter into the repo...
set SRC=Z:\Code\Python\pixel-perfecter\pixel_perfecter
set DST=Z:\Code\Games\delaford\delaford-sprite-overhaul\vendor\pixel_perfecter
robocopy "%SRC%" "%DST%" /E /XD __pycache__ .git .pytest_cache /XF *.pyc .env
echo.
echo Done (exit code %errorlevel%). Files copied to:
echo %DST%
echo.
pause
