@echo off

rem Build asm file
 echo Building: %1
 echo .include "%1.inc" >game.inc

rem Conditional assemble game
 echo Assemble: AGDgame.prg
 ca65 -l game.lst -o %1.o game.asm  
 if not "%errorlevel%" == "0" goto failed

rem Linking
 echo Linking: %1
 ld65  %1.o -C vic20.cfg -o %1.prg 
 if not "%errorlevel%" == "0" goto failed

rem Copying
 echo Copying: %1
 copy %1.prg "..\GTK3VICE-3.8-win64\TAPES" > nul
 echo Finished, created %1.prg
 echo.
 goto end

:failed
echo.
echo Error %errorlevel%
 echo.
 echo Usage: build [agd file] 
 echo.


:end
del *.o


