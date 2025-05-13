@echo off
cls
date /t
time /t

echo Flashing Bootloader and Application image to STM

set /A DEC_SN=23110000+%1%
setlocal EnableDelayedExpansion
call :ConvertDecToHex %DEC_SN% HEX_SN

@echo off

set PATH=%PATH%;"C:\Program Files (x86)\STMicroelectronics\STM32 ST-LINK Utility\ST-LINK Utility"
start /b /wait ST-LINK_CLI.exe -c SWD -ME
start /b /wait ST-LINK_CLI.exe -c SWD -P StepperMeas-BTL.bin 0x8000000 -V "after_programming"
start /b /wait ST-LINK_CLI.exe -c SWD -P StepperMeas-APP-20231107-1.bin 0x8001000 -V "after_programming"
start /b /wait ST-LINK_CLI.exe -c SWD -w32 0x08000200 %HEX_SN% -HardRst

Exit /B

: A function to convert Decimal to Hexadecimal
:: you need to pass the Decimal as first parameter
:: and return it in the second
:: This function needs setlocal EnableDelayedExpansion to be set at the start if this batch file
:: Refer to https://gist.github.com/ijprest/1207832
:ConvertDecToHex
set LOOKUP=0123456789abcdef
set HEXSTR=
set PREFIX=

if "%1"=="" echo 0&goto :EOF
set /A A=%1 || exit /b 1
if !A! LSS 0 set /A A=0xfffffff + !A! + 1 & set PREFIX=f
:loop
set /A B=!A! %% 16 & set /A A=!A! / 16
set HEXSTR=!LOOKUP:~%B%,1!%HEXSTR%
if %A% GTR 0 Goto :loop
set "%2=%PREFIX%%HEXSTR%"
Goto:eof
:: End of :ConvertDecToHex function