@echo off
rem Assemble game
 echo.
 echo Assemble: %1

 ca65 -l -o %1.o %1.asm
 if not "%errorlevel%" == "0" goto failed
rem del %1.inc

rem Linking
 echo.
 echo Linking: %1
 ld65 %1.o -C vic20.cfg -o %1.prg
 if not "%errorlevel%" == "0" goto failed
 goto end

:failed
 echo.
 echo Error %errorlevel%
 echo.


:end
copy %1.prg ..\..\CC65\loader.prg
del *.o


