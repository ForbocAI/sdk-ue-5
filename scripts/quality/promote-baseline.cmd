@echo off
powershell.exe -NoProfile -NonInteractive -ExecutionPolicy Bypass -File "%~dp0promote-baseline.ps1" %*
exit /b %ERRORLEVEL%
