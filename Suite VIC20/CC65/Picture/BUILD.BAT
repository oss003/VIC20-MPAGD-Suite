@echo off
echo Assembling %1
ca65 -l -o %1.o %1.asm 
if not "%errorlevel%" == "0" goto failed
echo Linking %1
ld65 %1.o -C atom.cfg -o %1
if not "%errorlevel%" == "0" goto failed

goto end

:failed
echo.
echo Error %errorlevel%
echo.

:end

