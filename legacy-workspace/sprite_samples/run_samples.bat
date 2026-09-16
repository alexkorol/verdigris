@echo off
echo Delaford Sprite Sample Generator
echo ==================================
echo.

REM Try Anaconda Python first, then fall back to system python
set PYTHON=python

where conda >nul 2>&1
if %errorlevel%==0 (
    call conda activate base 2>nul
)

cd /d "%~dp0"
%PYTHON% generate_samples.py
echo.
echo Press any key to close...
pause >nul
