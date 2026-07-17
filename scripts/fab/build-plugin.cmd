@echo off
powershell.exe -NoProfile -NonInteractive -ExecutionPolicy Bypass -File "%~dp0build-plugin.ps1" %*
exit /b %ERRORLEVEL%
