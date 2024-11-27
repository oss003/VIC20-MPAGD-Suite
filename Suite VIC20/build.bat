@echo off

rem -------------------------------------------------------
rem Check parameters
rem -------------------------------------------------------

 set model=0
 if "%2"=="pal" set model=1
 if "%3"=="pal" set model=1
 if "%4"=="pal" set model=1
 if "%5"=="pal" set model=1
 if "%6"=="pal" set model=1

 if "%2"=="ntsc" set model=0
 if "%3"=="ntsc" set model=0
 if "%4"=="ntsc" set model=0
 if "%5"=="ntsc" set model=0
 if "%6"=="ntsc" set model=0

 set turbo=0
 if "%2"=="turbo" set turbo=1
 if "%3"=="turbo" set turbo=1
 if "%4"=="turbo" set turbo=1
 if "%5"=="turbo" set turbo=1
 if "%6"=="turbo" set turbo=1

rem -------------------------------------------------------
rem Compile AGD file
rem -------------------------------------------------------

 copy scripts\%1.agd agd
 if errorlevel 1 goto copyerror
 cd AGD
 agd %1 %2 %3 %4 %5 %6
 if errorlevel 1 goto agderror
 copy %1.inc ..\cc65\ >nul
 if errorlevel 1 goto copyerror
 copy game.cfg ..\cc65\ >nul
 if errorlevel 1 goto copyerror
 del %1.*
 del game.cfg

rem -------------------------------------------------------
rem Assemble file
rem -------------------------------------------------------

 cd ..\cc65
 call make %1 %model% %2 %3 %4 %5 %6 %7 %8 %9
 copy %1.bin "..\GTK3VICE-3.8-win64\TAPES\%1.prg"
 if errorlevel 1 goto copyerror
 del %1.bin

rem -------------------------------------------------------
rem Create discimage 
rem -------------------------------------------------------

 echo.
 echo Creating diskimage %1.d64
 cd ..\GTK3VICE-3.8-win64\bin
 c1541 -format "diskimage,id" d64 %1.d64 

rem -------------------------------------------------------
rem Add TURBODISK to image if turbo parameter given
rem -------------------------------------------------------

 if %turbo%==1 (
   c1541 -attach %1.d64 -write ..\..\CC65\loader.prg %1.prg 
   c1541 -attach %1.d64 -write ..\TAPES\%1.prg agdgame.prg
   c1541 -attach %1.d64 -write ..\..\CC65\turbodisk turbodisk 
 ) else (
   c1541 -attach %1.d64 -write ..\TAPES\%1.prg %1.prg
 )

rem -------------------------------------------------------
rem Add music player to image
rem -------------------------------------------------------

 if exist ..\..\MUSIC\%1.mus (
  c1541 -attach %1.d64 -write ..\..\MUSIC\%1.mus player1 
 ) else (
  c1541 -attach %1.d64 -write ..\..\MUSIC\player1 player1 
 )

rem -------------------------------------------------------
rem Add pictures to image
rem -------------------------------------------------------

 for %%f in (..\..\pictures\%1*.*) do c1541 -attach %1.d64 -write %%f %%~nf

rem -------------------------------------------------------
rem Start emulator
rem -------------------------------------------------------

 move %1.d64 ..\discs >nul
 echo Starting Vice with %1.prg

 if %model%==0 (
   xvic -silent -memory all -ntsc "..\discs\%1.d64:%1.prg"
 ) else (
   xvic -silent -memory all -pal "..\discs\%1.d64:%1.prg"
 )
 cd ..\..

 goto end

rem -------------------------------------------------------
rem Error handling
rem -------------------------------------------------------

:copyerror
 echo *** COPY ERROR ***, %1.agd not found .....
 goto end

:agderror
 del %1.*
 del game.cfg
 cd ..
 echo.
 echo *** AGD COMPILER ERROR ***, errors found in %1.AGD
 goto end

:end
