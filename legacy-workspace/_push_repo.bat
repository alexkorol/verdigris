@echo off
echo Pushing delaford-sprite-overhaul to GitHub...
echo ============================================
cd /d "Z:\Code\Games\delaford\delaford-sprite-overhaul"

git init
git add -A
git -c core.autocrlf=false commit -m "Initial commit: Delaford sprite overhaul pipeline, vendored pixel-perfecter, source spritesheets, samples, and Codex handoff docs"
git branch -M main
git remote remove origin 2>nul
git remote add origin https://github.com/alexkorol/delaford-sprite-overhaul.git
git push -u origin main

echo.
echo ============================================
echo Done. If you see "Writing objects... done" and a branch set up above, the push succeeded.
echo If it asks you to sign in to GitHub, complete that and it will finish.
pause
