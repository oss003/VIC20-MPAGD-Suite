@echo off

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
 call make %1 %model%
rem del %1.*

rem Start emulator
 echo Starting Vice with %1.prg
 cd ..\GTK3VICE-3.8-win64\bin

rem Check if PAL or NTSC machine
 if "%model%" == "1" goto pal
   xvic -silent -memory all -ntsc ..\TAPES\%1.prg
   goto cont
:pal
   xvic -silent -memory all -pal ..\TAPES\%1.prg

:cont
 echo Quiting Vice
 cd ..\..
 goto end

:error
 echo %1.agd not found .....

:end
