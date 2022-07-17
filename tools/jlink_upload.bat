@echo off
rem: Note %~dp0 get path of this batch file
rem: Need to change drive if My Documents is on a drive other than C:
set driverLetter=%~dp0
set driverLetter=%driverLetter:~0,2%
%driverLetter%
cd %~dp0

rem: the two line below are needed to fix path issues with incorrect slashes before the bin file name
set tmpBinFilePath=%1
set tmpBinFilePath=%tmpBinFilePath:/=\%

rem: create commander script file with the tmp bin that the Arduino IDE creates

@echo h > %tmpbinfilepath%.jlink
@echo loadbin %tmpbinfilepath% , 0x0 >> %tmpbinfilepath%.jlink
@echo r >> %tmpbinfilepath%.jlink
@echo q >> %tmpbinfilepath%.jlink

JLink.exe -device %2 -if SWD -speed auto -autoconnect 1 -NoGui 1 -CommanderScript %tmpBinFilePath%.jlink
del %tmpbinfilepath%.jlink