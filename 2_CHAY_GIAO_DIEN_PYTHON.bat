@echo off
title NeoDen YY1 SMT Converter Pro (Python GUI)
cd /d "%~dp0"
py app_gui.py
if %ERRORLEVEL% NEQ 0 (
    python app_gui.py
)
pause
