@echo off
echo Committing + pushing handoff doc updates...
cd /d "Z:\Code\Games\delaford\delaford-sprite-overhaul"
git add -A
git -c core.autocrlf=false commit -m "Refine handoff: creative direction (mods/PoE1/D2), polished-but-primitive bar, lumpy-conversion as top priority, autonomous /go workflow, one-sprite-at-a-time scope"
git push
echo.
echo === Done. Look for "Writing objects... done" above. ===
pause
