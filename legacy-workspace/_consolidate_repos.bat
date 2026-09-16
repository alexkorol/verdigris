@echo off
echo ============================================
echo Consolidating Delaford repos
echo Keeper: alexkorol/delaford_game (gets all work from both)
echo ============================================
cd /d "Z:\Code\Games\delaford\delaford_game"

echo.
echo [1/4] Removing stray temp files left in .git...
del /q ".unlink_test" 2>nul
del /q ".git\objects\maintenance.lock" 2>nul
del /s /q ".git\objects\tmp_obj_*" 2>nul

echo [2/4] Pointing origin at delaford_game (the repo being kept)...
git remote set-url origin https://github.com/alexkorol/delaford_game.git
git fetch origin

echo [3/4] Merging delaford_game/master (socket auth fix + HUD work) into local master...
git -c core.autocrlf=false merge -X theirs --no-edit -m "Merge delaford_game/master: socket auth security fix (PR #60) and gameplay controls/HUD work (PR #61)" origin/master

echo [4/4] Pushing merged master to delaford_game...
git push origin master

echo.
echo ============================================
echo Done. delaford_game master now contains ALL work from both repos.
echo.
echo Remaining manual steps on github.com:
echo   1. Delete https://github.com/alexkorol/delaford (Settings - Danger Zone)
echo   2. Optional: rename delaford_game to "delaford" (stars and old links are kept)
echo.
echo Recommended afterwards: npm run test:unit
echo ============================================
pause
