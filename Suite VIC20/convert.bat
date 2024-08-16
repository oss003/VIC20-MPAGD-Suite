@echo off
 set bigspr=
 if "%2"=="b" set bigspr=b
 if "%3"=="b" set bigspr=b

 set dither=
 if "%2"=="d" set dither=d
 if "%3"=="d" set dither=d
 echo.

rem Covert ZX snapshot to AGD file
 copy ..\snapshots\%1.sna convert
 if errorlevel 1 goto error
 cd convert

 convert %1 %bigspr% %dither%
 move %1.agd ..\AGDsources
 del %1.sna
 cd ..
 goto end

:error
 echo %1.sna not found .....

:end
