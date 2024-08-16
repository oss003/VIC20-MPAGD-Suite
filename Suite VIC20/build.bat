@echo off

rem Copy AGD file to AGD dir
 cls
 echo ==========================================
 echo MPAGD VIC20 port
 echo ==========================================
 echo.
 echo Copy %1.agd to compiler directory ...
 copy AGDsources\%1.agd agd >nul
 if errorlevel 1 goto error
 cd AGD

rem Compile AGD file
 echo Compiling %1.agd
 AGD %1 
 copy %1.inc ..\cc65\ >nul
 copy game.cfg ..\cc65\ >nul
 del %1.*
 del game.cfg

rem Assemble file
 cd ..\cc65
 call make %1 %2 %3 %4 %5 %6 %7 %8 %9
rem del %1.*

rem Start emulator
 echo Starting Vice with %1.prg
 cd ..\GTK3VICE-3.8-win64\bin
 xvic -silent -memory all -ntsc ..\TAPES\%1.prg
 echo Quiting Vice
 cd ..\..
 goto end

:error
 echo %1.agd not found .....

:end
