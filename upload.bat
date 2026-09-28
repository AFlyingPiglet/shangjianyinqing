@echo off
chcp 65001 >nul
title Upload CRTC2026 docs
powershell -NoProfile -ExecutionPolicy Bypass -File "%~dp0upload_to_git.ps1"
echo.
echo (If you see any red error above, take a screenshot for Codex.)
pause
