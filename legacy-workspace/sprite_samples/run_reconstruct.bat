@echo off
echo Delaford Sprite Reconstruction (pixel-perfecter)
echo ================================================
cd /d "%~dp0"
where conda >nul 2>&1
if %errorlevel%==0 call conda activate base 2>nul
echo.
python reconstruct_samples.py %*
echo.
echo Press any key to close...
pause >nul
