@echo off
chcp 65001 >nul
title Check repository size before pushing
set "PY=%USERPROFILE%\.cache\codex-runtimes\codex-primary-runtime\dependencies\python\python.exe"
if not exist "%PY%" (
  echo [!] Bundled Python not found. Ask Codex to run the size check instead.
  pause
  exit /b 1
)
set "PYTHONIOENCODING=utf-8"
"%PY%" "%~dp0check_size.py" "%USERPROFILE%\Documents\Codex\CRTC2026"
echo.
pause
