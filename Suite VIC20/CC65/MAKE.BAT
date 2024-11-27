@echo off
rem Set flags

 set if=0
 if "%2"=="i" set if=1
 if "%3"=="i" set if=1
 if "%4"=="i" set if=1
 if "%5"=="i" set if=1
 if "%6"=="i" set if=1

 set af=0
 if "%2"=="a" set af=1
 if "%3"=="a" set af=1
 if "%4"=="a" set af=1
 if "%5"=="a" set af=1
 if "%6"=="a" set af=1

 set hf=0
 if "%2"=="h" set hf=1
 if "%3"=="h" set hf=1
 if "%4"=="h" set hf=1
 if "%5"=="h" set hf=1
 if "%6"=="h" set hf=1

 set gf=0
 if "%2"=="g" set gf=1
 if "%3"=="g" set gf=1
 if "%4"=="g" set gf=1
 if "%5"=="g" set gf=1
 if "%6"=="g" set gf=1

 set xf=0
 if "%2"=="x" set xf=1
 if "%3"=="x" set xf=1
 if "%4"=="x" set xf=1
 if "%5"=="x" set xf=1
 if "%6"=="x" set xf=1

rem Build asm file
 echo.
 echo Build %1.inc
 echo .include "%1.inc" >game.inc

rem Conditional assemble game
 echo.
 echo Assemble: %1

 ca65 -l -o %1.o game.asm -D modelflag=%2 -D iflag=%if% -D aflag=%af% -D hflag=%hf% -D gflag=%gf% -D xflag=%xf%
 if not "%errorlevel%" == "0" goto failed
 del %1.inc

rem Linking
 echo.
 echo Linking: %1
 ld65 %1.o -C vic20.cfg -o %1.bin
 if not "%errorlevel%" == "0" goto failed
 echo Finished, created %1.bin
 goto end

:failed
echo.
echo Error %errorlevel%
 echo.
 echo Usage: build [agd file] m p s
 echo.
 echo Conditional assembly:
 echo a = Adventuremode
 echo b = Big Sprites (16x24)
 echo g = Graphic colour mode
 echo h = Hidden sprite mode
 echo i = Invertmode
 echo.


:end
del *.o


