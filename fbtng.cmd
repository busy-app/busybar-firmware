@echo off

call "%~dp0scripts\toolchain\fbtenv.cmd" env || exit /b

python "%~dp0scripts\toolchain\fbt_ep.py" %*
